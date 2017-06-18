// #version 450 core
// layout(local_size_x = 8, local_size_y = 8) in;
#define M_PI 3.14159265358979323846
#define M_INV_PI (1.0/M_PI)
#define FLOAT_EPSILON 1e-7

// arguments for prefilter
layout (binding = 0, std140) uniform InputBuffer
{
	vec4 inputArg0;
	vec4 inputArg1;
};

const uint INPUT_ENVMAP_MAX_MIPLEVEL = uint(inputArg0[0]);
const vec2 OUPUT_SIZE = vec2(inputArg0[1]);
const uint NUM_OF_SAMPLES = uint(inputArg0[2]);

// input
layout (binding = 0) uniform samplerCube inputEnvmap;
// output
layout (rgba32f, binding = 0) uniform image2D outputImage;

// clamp to [0.0, 1.0]
float Saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}
vec2 Saturate(vec2 value)
{
	return vec2(Saturate(value.x), Saturate(value.y));
}

// tell if a==b
bool FloatEqual(float a, float b)
{
	return abs(a - b) > FLOAT_EPSILON;
}

// local reference frame
struct LocalFrame
{
	vec3 tangentX;
	vec3 tangentY;
};

// note: we treat N as Z axis, {tX, tY, N} are orthographic
LocalFrame CreateLocalFrame(vec3 N)
{
	LocalFrame frame;
	vec3 upVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	frame.tangentX = normalize(cross(upVector, N));
	frame.tangentY = cross(N, frame.tangentX);
	return frame;
}

// reverse bits of a float
// use bitfieldReverse in OpenGL 4.0+
float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Hammersley sequence
vec2 Hammersley(uint i, uint N)
{
    float ri = float(bitfieldReverse(i)) * 2.3283064365386963e-10;
    return vec2(float(i)/float(N), ri);
}

// wrapper of random samples
vec2 GetSample(uint i, uint total)
{
	return Hammersley(i, total);
}

// ======================== no need to support panorama ======================================== //
vec4 SamplePanorama(sampler2D panorama, vec3 direction)
{
	vec2 uv = vec2(M_PI + atan(direction.x, -direction.z), acos(-direction.y)) / vec2(2.0 * M_PI, M_PI);
	return texture(panorama, uv);
}

vec4 SamplePanorama(sampler2D panorama, vec3 direction, float mipmapLevel)
{
	vec2 uv = vec2(M_PI + atan(direction.x, -direction.z), acos(-direction.y)) / vec2(2.0 * M_PI, M_PI);
	return textureLod(panorama, uv, mipmapLevel);
}
// ======================== no need to support panorama ======================================== //
vec4 SampleCubemap(samplerCube cubemap, vec3 direction, float lod)
{
	vec4 color = textureLod(cubemap, direction, lod);
	return color;
}

// schlick approximation of the fresnel equation
// f90 = 1.0 all the time in our Shiny Engine
vec3 F_Schlick(in vec3 f0, in vec3 f90, in float u)
{
	return f0 + (f90 - f0) * pow(1.f - u, 5.f);
}

// height correlated version of smith GGX V
// used in analytic lighting
float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	// Original formulation of G_SmithGGX Correlated
	// lambda_v = (-1 + sqrt(alphaG2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;
	// lambda_l = (-1 + sqrt(alphaG2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l);
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0f * NdotL * NdotV);

	// This is the optimize version
	float alphaG2 = alphaG * alphaG;
	// Caution: the "NdotL *" and "NdotV *" are explicitely inversed , this is not a mistake.
	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);

	return 0.5 / (Lambda_GGXV + Lambda_GGXL);
}

// height correlated version of smith GGX G
float G_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	float alphaG2 = alphaG * alphaG;
	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
	return 2.0 * NdotL * NdotV / (Lambda_GGXV + Lambda_GGXL);
}

// GGX NDF without being divided by PI
float D_GGX(float NdotH, float m)
{
	// Divide by PI is apply later
	float m2 = m * m;
	float f = (NdotH * m2 - NdotH) * NdotH + 1;
	return m2 / (f * f);
}

// D/PI
float D_GGX_Divide_Pi(float NdotH, float alphaG)
{
    return D_GGX(NdotH, alphaG) / M_PI;
}

// diffuse brdf from disney
float Fr_DisneyDiffuse(
	float NdotV,
	float NdotL,
	float LdotH,
	float linearRoughness)
{
	float energyBias = mix(0.0, 0.5, linearRoughness);
	float energyFactor = mix(1.0, 1.0 / 1.51, linearRoughness);
	float fd90 = energyBias + 2.0 * LdotH*LdotH * linearRoughness;
	vec3 f0 = vec3(1.0f, 1.0f, 1.0f);
	float lightScatter = F_Schlick(f0, vec3(fd90), NdotL).r;
	float viewScatter = F_Schlick(f0, vec3(fd90), NdotV).r;
	return lightScatter * viewScatter * energyFactor;
}

// importance sample cosine direction
void ImportanceSampleCosDir(
	in vec2 u,
	in vec3 N,
	out vec3 L,
	out float NdotL,
	out float pdf)
{
	// Local referencial
	LocalFrame referenceFrame = CreateLocalFrame(N);
	float u1 = u.x;
	float u2 = u.y;
	float r = sqrt(u1);
	float phi = u2 * M_PI * 2;
	L = vec3(r*cos(phi), r*sin(phi), sqrt(max(0.0f,1.0f-u1)));
	L = normalize(referenceFrame.tangentX * L.y + referenceFrame.tangentY * L.x + N * L.z);
	NdotL = dot(L,N);
	pdf = NdotL * M_INV_PI;
}

// return micro-surface normal of GGX
vec3 ImportanceSampleGGX(vec2 Xi, float alphaG, vec3 N, LocalFrame referential)
{
	// float a = Roughness * Roughness;
	float a = alphaG;
	float Phi = 2 * M_PI * Xi.x;
	float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );
	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;
	// Tangent to world space
	return referential.tangentX * H.x + referential.tangentY * H.y + N * H.z;
}

// calculate micro-surface reflection L,H according to V,N
void ImportanceSampleGGXDir(in vec2 Xi, in vec3 V, in vec3 N, in float alphaG, out vec3 H, out vec3 L)
{
	LocalFrame referential = CreateLocalFrame(N);
	H = ImportanceSampleGGX(Xi, alphaG, N, referential);
	L = 2 * dot( V, H ) * H - V;
}

// G: bidirectional shadowing masking
void ImportanceSampleGGX_G(
	in vec2 u, in vec3 V, in vec3 N,
	in LocalFrame referential, in float alphaG,
	out float NdotH, out float LdotH, out vec3 L, out float G)
{
	vec3 H = ImportanceSampleGGX(u, alphaG, N, referential);
	L = 2 * dot( V, H ) * H - V;
	NdotH = Saturate(dot(N, H));
	LdotH = Saturate(dot(L, H));
	float NdotL = Saturate(dot(N, L));
	float NdotV = Saturate(dot(N, V)); 
	G = G_SmithGGXCorrelated(NdotL, NdotV, alphaG);
}

// DFG: the brdf part of the split sum
vec4 IntegrateDFGOnly(
	in vec3 V,
	in vec3 N,
	in float roughness,
	in float alphaG)
{
	float NdotV = Saturate(dot(N, V));
	vec4 acc = vec4(0.0);
	float accWeight = 0.0;
	// Compute pre-integration
	LocalFrame referential = CreateLocalFrame(N);
	for (uint i=0; i<NUM_OF_SAMPLES; ++i)
	{
		vec2 u = GetSample(i, NUM_OF_SAMPLES);
		vec3 L = vec3(0);
		float NdotH = 0;
		float LdotH = 0;
		float G = 0;
		// See [Karis13] for implementation
		ImportanceSampleGGX_G(u, V, N, referential, alphaG, NdotH , LdotH , L, G);
		// specular GGX DFG preIntegration
		float NdotL = Saturate(dot(N, L));
		if (NdotL > 0 && G > 0.0)
		{
			float GVis = G * LdotH / (NdotH * NdotV);
			float Fc = pow(1-LdotH , 5.f);
			acc.x += (1-Fc) * GVis;
			acc.y += Fc*GVis;
		}
		// diffuse Disney preIntegration
		u = fract(u + 0.5);
		float pdf;
		// The pdf is not use because it cancel with other terms
		// (The 1/PI from diffuse BRDF and the NdotL from Lambert ’s law).
		ImportanceSampleCosDir(u, N, L, NdotL , pdf);
		if (NdotL >0)
		{
			float LdotH = Saturate(dot(L, normalize(V + L)));
			float NdotV = Saturate(dot(N, V));
			acc.z += Fr_DisneyDiffuse(NdotV , NdotL , LdotH , roughness);
		}
		accWeight += 1.0;
	}
	return acc * (1.0f / accWeight);
}

// cosine sample diffuse lighting
vec4 IntegrateDiffuseCube(in vec3 N)
{
	vec3 accBrdf = vec3(0);
	for (uint i=0; i<NUM_OF_SAMPLES; ++i)
	{
		vec2 eta = GetSample(i, NUM_OF_SAMPLES);
		vec3 L;
		float NdotL;
		float pdf;
		// see reference code in appendix
		ImportanceSampleCosDir(eta, N, L, NdotL , pdf);
		if (NdotL >0)
			accBrdf += SampleCubemap(inputEnvmap, L, 0.0).rgb;//IBLCube.Sample(incomingLightSampler , L).rgb;
	}
	return vec4(accBrdf * (1.0 / float(NUM_OF_SAMPLES)), 1.0);
}

// calculate specular sum part
vec3 IntegrateCubeLDOnly(
	in vec3 V,
	in vec3 N,
	in float roughness)
{
	vec3 accBrdf = vec3(0);
	float accBrdfWeight = 0;
	float alphaG = roughness * roughness;
	for (uint i=0; i<NUM_OF_SAMPLES; ++i)
	{
		vec2 eta = GetSample(i, NUM_OF_SAMPLES);
		vec3 L;
		vec3 H;
		ImportanceSampleGGXDir(eta, V, N, alphaG , H, L);
		float NdotL = dot(N,L);
		if (NdotL >0)
		{
			// Use pre-filtered importance sampling (i.e use lower mipmap
			// level for fetching sample with low probability in order
			// to reduce the variance).
			// (Reference: GPU Gem3)
			//
			// Since we pre-integrate the result for normal direction ,
			// N == V and then NdotH == LdotH. This is why the BRDF pdf
			// can be simplifed from:
			// pdf = D_GGX_Divide_Pi(NdotH , roughness)*NdotH/(4*LdotH);
			// to
			// pdf = D_GGX_Divide_Pi(NdotH , roughness) / 4;
			//
			// The mipmap level is clamped to something lower than 8x8
			// in order to avoid cubemap filtering issues
			//
			// - OmegaS: Solid angle associated to a sample
			// - OmegaP: Solid angle associated to a pixel of the cubemap
			float mipLevel = 0;
			float NdotH = Saturate(dot(N, H));
			float LdotH = Saturate(dot(L, H));
			float pdf = D_GGX_Divide_Pi(NdotH , alphaG) * NdotH/(4*LdotH);
			if (pdf > 0.0) {
				float omegaS = 1.0 / (float(NUM_OF_SAMPLES) * pdf);
				float omegaP = 4.0 * M_PI / (6.0 * float(gl_NumWorkGroups.x * gl_WorkGroupSize.x * gl_NumWorkGroups.y * gl_WorkGroupSize.y));
				float maxMipLevel = float(INPUT_ENVMAP_MAX_MIPLEVEL);
				mipLevel = clamp(0.5 * log2(omegaS/omegaP), 0, maxMipLevel);
			}
			vec4 Li = SampleCubemap(inputEnvmap, L, mipLevel);

			accBrdf += Li.rgb * NdotL;
			accBrdfWeight += NdotL;
		}
	}
	if (accBrdfWeight <= 0.0) {
		return vec3(0.0);
	}
	return accBrdf * (1.0f / accBrdfWeight);
}

// calculates a normal vector from the specified texture coordinates and cube face.
vec3 CubeFaceTexCoordToDirection(vec2 uv, uint face)
{
    vec3 n = vec3(0.0); // This is a normal vector.
    vec3 t = vec3(0.0); // This is a tangent vector.
    if (face == 0)
    {
        n = vec3(1, 0, 0);
        t = vec3(0, 1, 0);
    }
    else if (face == 1)
    {
        n = vec3(-1, 0, 0);
        t = vec3(0, 1, 0);
    }
    else if (face == 2)
    {
        n = vec3(0, -1, 0);
        t = vec3(0, 0, -1);
    }
    else if (face == 3)
    {
        n = vec3(0, 1, 0);
        t = vec3(0, 0, 1);
    }
    else if (face == 4)
    {
        n = vec3(0, 0, -1);
        t = vec3(0, 1, 0);
    }
    else
    {
        n = vec3(0, 0, 1);
        t = vec3(0, 1, 0);
    }
    // Calculate a binormal vector.
    vec3 b = cross(n, t);
    // Convert the texture coordinates from [0, 1] to [-1, 1] range.
    uv = uv * 2 - vec2(1);
    // Calculate a new normal vector for this pixel (current texture coordinates).
    n = n + t * uv.y + b * uv.x;
    n.y = -n.y;
    n.z = -n.z;
    return normalize(n);
}

void main()
{
	vec4 outputColor = vec4(0.0);

	// bias 0.5, both for dfg(90 degree not include) and cubemap(avoid seams)
	vec2 outputTexCoord = Saturate((vec2(gl_GlobalInvocationID.xy) + vec2(0.5)) / (OUPUT_SIZE));
	vec3 direction = CubeFaceTexCoordToDirection(outputTexCoord, uint(inputArg1.w));
	
	#ifdef INTEGRATE_DFG
	float NdotV = outputTexCoord.x;
	float alphaG = outputTexCoord.y;
	float roughness = sqrt(alphaG);
	vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0, NdotV);
	outputColor = IntegrateDFGOnly(vec3(0.0, 0.0, 1.0), V, roughness, alphaG);
	
	#elif defined (INTEGRATE_SPECULAR)
	// we call linearRoughness as roughness
	// level/maxLevel = sqrt(roughness)
	// roughness = pow(level/maxlevel, 2.0)
	// GGX alpha = roughness * roughness = pow(level/maxlevel, 4.0)
	float sqrtRoughness = inputArg1.y / inputArg1.z;
	float roughness = sqrtRoughness * sqrtRoughness;
	outputColor.xyz = IntegrateCubeLDOnly(direction, direction, roughness);
	
	#elif defined (INTEGRATE_DIFFUSE)
	vec3 n = CubeFaceTexCoordToDirection(outputTexCoord, uint(inputArg1.w));
	outputColor = IntegrateDiffuseCube(n);
	#endif
	
	imageStore(outputImage, ivec2(gl_GlobalInvocationID.xy), outputColor);
}