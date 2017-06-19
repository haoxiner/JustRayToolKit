#pragma once
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtx\transform.hpp"
#include "glm\gtc\constants.hpp"
#include <algorithm>

namespace JustRay
{
/*
* PI
*/
constexpr float PI = glm::pi<float>();
constexpr float TWO_PI = 2.0f * PI;
constexpr float INV_PI = 1.0f / PI;
constexpr float INV_TWO_PI = 1.0f / TWO_PI;

/*
    Wrap an angle to [-pi, pi]
    Angle must be close to [-pi, pi], otherwise the performance will be low
*/
inline float WrapAngle(float angle)
{
    //    const float invTwoPi = 1.0f / twoPi;
    //    return angle - twoPi * std::floorf(angle * invTwoPi);
    while (angle > PI) {
        angle -= TWO_PI;
    }
    while (angle < -PI) {
        angle += TWO_PI;
    }
    return angle;
}

/*
    degree to radians
*/
inline constexpr float DegreesToRadians(float degrees)
{
    return glm::radians(degrees);
}

using Float2 = glm::vec2;
using Float3 = glm::vec3;
using Float4 = glm::vec4;
using Int2 = glm::ivec2;
using Int3 = glm::ivec3;
using Int4 = glm::ivec4;
using Quaternion = Float4;
using Matrix4x4 = glm::mat4;

/*
    normalize
*/
inline Quaternion Normalize(const Quaternion& quaternion)
{
    return Quaternion(glm::normalize(quaternion));
}
inline Float3 Normalize(const Float3& v)
{
    return glm::normalize(v);
}
/*
clamp
*/
inline float Clamp(float value, float low, float high)
{
    return std::fmaxf(std::fminf(value, high), low);
}
/*
Saturate
*/
inline float Saturate(float value)
{
    return Clamp(value, 0.0f, 1.0f);
}
/*
    Make ViewToProjection Transform
*/
inline Matrix4x4 MakePerspectiveProjectionMatrix(float verticalFov, float aspect, float zNear, float zFar)
{
    return glm::perspective(verticalFov, aspect, zNear, zFar);
}

/*
    Quaternion To Matrix4x4
*/
inline Matrix4x4 QuaternionToMatrix(const Quaternion& quaternion)
{
    float xy = quaternion.x * quaternion.y;
    float yz = quaternion.y * quaternion.z;
    float zx = quaternion.z * quaternion.x;
    float x2 = quaternion.x * quaternion.x;
    float y2 = quaternion.y * quaternion.y;
    float z2 = quaternion.z * quaternion.z;
    float xw = quaternion.x * quaternion.w;
    float yw = quaternion.y * quaternion.w;
    float zw = quaternion.z * quaternion.w;
    return Matrix4x4(1.0 - 2.0*(y2 + z2), 2.0*(xy - zw), 2.0*(zx + yw), 0.0,
                     2.0*(xy + zw), 1.0 - 2.0*(x2 + z2), 2.0*(yz - xw), 0.0,
                     2.0*(zx - yw), 2.0*(yz + xw), 1.0 - 2.0*(x2 + y2), 0.0,
                     0.0, 0.0, 0.0, 1.0);
}

/*
    Translate By XYZ
*/
inline Matrix4x4 MakeTranslationMatrix(float x, float y, float z)
{
    return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     x, y, z, 1.0f);
}
/*
    Translate By Vector3f
*/
inline Matrix4x4 MakeTranslationMatrix(const Float3& vector3f)
{
    return MakeTranslationMatrix(vector3f.x, vector3f.y, vector3f.z);
}

/*
    Scale By XYZ
*/
inline Matrix4x4 MakeScaleMatrix(float x, float y, float z)
{
    return Matrix4x4(x, 0.0f, 0.0f, 0.0f,
                     0.0f, y, 0.0f, 0.0f,
                     0.0f, 0.0f, z, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f);
}
/*
Scale By Vector3f
*/
inline Matrix4x4 MakeScaleMatrix(const Float3& vector3f)
{
    return MakeScaleMatrix(vector3f.x, vector3f.y, vector3f.z);
}

/*
euler to rotation matrix
*/
inline Matrix4x4 EulerToRotationMatrix(float yaw, float pitch, float roll)
{
    //float x = DegreesToRadians(pitch);
    //float y = DegreesToRadians(roll);
    //float z = DegreesToRadians(yaw);

    //float sz = std::sinf(z);
    //float cz = std::cosf(z);
    //float sy = std::sinf(y);
    //float cy = std::cosf(y);
    //float sx = std::sinf(x);
    //float cx = std::cosf(x);

    //return Matrix4x4(
    //    cz * cy, sz*cy, -sy, 0,
    //    cz*sy*sx - sz*cy, sz*sy*sx + cz*cx, cy*sx, 0,
    //    cz*sy*cx + sz*sx, sz*sy*cx - cz*sx, cy*cx, 0,
    //    0, 0, 0, 1
    //);
    auto viewMatrix = glm::rotate(DegreesToRadians(pitch), glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, DegreesToRadians(roll), glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, DegreesToRadians(yaw), glm::vec3(0, 0, 1));
    return viewMatrix;
}
inline Float3 Cross(const Float3& from, const Float3& to)
{
    return glm::cross(from, to);
}
inline float Dot(const Float3& v1, const Float3& v2)
{
    return glm::dot(v1, v2);
}
inline Matrix4x4 MakeRotationMatrix(const Float3& axis, float angle)
{
    return glm::rotate(angle, axis);
}

/*
map float of [-1.0, 1.0] to short
*/
inline short MapToShort(float value)
{
    value = std::min(1.0f, std::max(-1.0f, value));
    constexpr float mapToPositive = 32767.0f;
    constexpr float mapToNegative = -32768.0f;
    return static_cast<short>((value >= 0 ? mapToPositive * value + 0.5f : -(mapToNegative * value + 0.5f)));
}

/*
map float of [0.0, 1.0] to unsigned short
*/
inline unsigned short MapToUnsignedShort(float value)
{
    value = std::min(1.0f, std::max(0.0f, value));
    constexpr float mapToPositive = 65535.0f;
    return static_cast<short>(mapToPositive * value + 0.5f);
}

/*
map float of [0.0, 1.0] to unsigned char
*/
inline unsigned char MapToUnsignedByte(float value)
{
    value = std::min(1.0f, std::max(0.0f, value));
    constexpr float mapToPositive = 255.0f;
    return static_cast<short>(mapToPositive * value + 0.5f);
}

/*
map float of [-1.0, 1.0] to 10 bit int
*/
inline int MapTo10BitInt(float value)
{
    value = std::min(1.0f, std::max(-1.0f, value));
    constexpr float mapToPositive = 511.0f;
    constexpr float mapToNegative = -512.0f;
    return static_cast<int>((value >= 0 ? mapToPositive * value + 0.5f : -(mapToNegative * value + 0.5f)));
}

/*
map float of [0.0, 1.0] to 10 bit unsigned int
*/
inline unsigned int MapTo10BitUInt(float value)
{
    value = std::min(1.0f, std::max(0.0f, value));
    constexpr float mapToPositive = 1023.0f;
    return static_cast<unsigned int>(mapToPositive * value + 0.5f);
}
struct Int_2_10_10_10
{
    int x : 10;
    int y : 10;
    int z : 10;
    int a : 2;
};
struct UInt_2_10_10_10
{
    unsigned int x : 10;
    unsigned int y : 10;
    unsigned int z : 10;
    unsigned int a : 2;
};
//Pack float3 to 32 bit unsigned integer
inline UInt_2_10_10_10 PackFloat3ToUInt2_10_10_10(const Float3& float3)
{
    return UInt_2_10_10_10{ MapTo10BitUInt(float3.x), MapTo10BitUInt(float3.y), MapTo10BitUInt(float3.z), 0 };
}
//Pack float3 to 32 bit integer
inline Int_2_10_10_10 PackFloat3ToInt2_10_10_10(const Float3& float3)
{
    return Int_2_10_10_10{ MapTo10BitInt(float3.x), MapTo10BitInt(float3.y), MapTo10BitInt(float3.z), 0 };
}
}