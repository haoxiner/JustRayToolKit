#include "ModelGroup.h"
#include "Display.h"
#include "PreIntegrator.h"
#include "FreeImage.h"
#include <iostream>
#include <fstream>
using namespace std;

void ConvertObj()
{
    JustRay::ModelGroup modelGroup;
    bool ret = modelGroup.LoadFromObj("F:/haoxin/convert/cube.obj");
    if (ret) {
        auto result = modelGroup.WriteToFile("F:/haoxin/convert", "cube");

        std::cerr << "NUM OF Vertices: " << modelGroup.GetNumOfVertices() << std::endl;
        std::cerr << "NUM OF INDICES: " << modelGroup.GetNumOfIndices() << std::endl;
        std::cerr << "NUM OF MESHES: " << modelGroup.GetNumOfMeshes() << std::endl;
        std::cerr << std::get<0>(result) << ", " << std::get<1>(result) << std::endl;
    }
}
void PreIntegrate()
{
    constexpr int xResolution = 800;
    constexpr int yResolution = 600;
    JustRay::Display display;
    if (!display.Startup(xResolution, yResolution, false)) {
        return;
    }

    JustRay::PreIntegrator preIntegrator;
    //preIntegrator.IntegrateIBLDFG("dfg", "../../Resources/output");
    preIntegrator.IntegrateIBLDiffuseAndSpecular("../../Resources/Environment/glacier/src", "glacier", "../../Resources/Environment/output/glacier", "glacier");
}
float MaterialGamma(float v, bool apply)
{
    //std::cerr << v << ", ";
    if (apply) {
        return std::powf(v, 1.0 / 2.2);
    }
    return v;
}
void GenMaterial(const std::string& inputDirectory, const std::string& outputDirectory, bool gamma = false)
{
    
    {
        std::vector<unsigned char> output;
        std::string basecolorFilePath = inputDirectory + "/basecolor.exr";
        auto basecolorBitmap = FreeImage_Load(FIF_EXR, basecolorFilePath.c_str());
        auto basecolorBPP = FreeImage_GetBPP(basecolorBitmap);
        std::cerr << "basecolor bpp: " << basecolorBPP << std::endl;
        auto basecolorWidth = FreeImage_GetWidth(basecolorBitmap);
        auto basecolorHeight = FreeImage_GetHeight(basecolorBitmap);
        auto basecolorBits = FreeImage_GetBits(basecolorBitmap);

        std::string metallicFilePath = inputDirectory + "/metallic.exr";
        auto metallicBitmap = FreeImage_Load(FIF_EXR, metallicFilePath.c_str());
        auto metallicBPP = FreeImage_GetBPP(metallicBitmap);
        auto metallicBits = FreeImage_GetBits(metallicBitmap);
        std::cerr << "metallic bpp: " << metallicBPP << std::endl;

        float* basecolorFloats = reinterpret_cast<float*>(basecolorBits);
        float* metallicFloats = reinterpret_cast<float*>(metallicBits);
        for (int i = 0; i < basecolorWidth * basecolorHeight; i++) {
            output.emplace_back(JustRay::MapToUnsignedByte(basecolorFloats[i * (basecolorBPP / 32)]));
            output.emplace_back(JustRay::MapToUnsignedByte(basecolorFloats[i * (basecolorBPP / 32) + 1]));
            output.emplace_back(JustRay::MapToUnsignedByte(basecolorFloats[i * (basecolorBPP / 32) + 2]));
            output.emplace_back(JustRay::MapToUnsignedByte(MaterialGamma(metallicFloats[i * (metallicBPP / 32)], gamma)));
        }
        FreeImage_Unload(basecolorBitmap);
        FreeImage_Unload(metallicBitmap);
        std::ofstream file(outputDirectory + "/basecolor.rgba8", std::ios::binary);
        unsigned short w = basecolorWidth;
        unsigned short h = basecolorHeight;
        unsigned short numOfChannel = 4;
        unsigned short maxMipLevel = 0;
        file.write(reinterpret_cast<char*>(&w), sizeof(w));
        file.write(reinterpret_cast<char*>(&h), sizeof(h));
        file.write(reinterpret_cast<char*>(&numOfChannel), sizeof(numOfChannel));
        file.write(reinterpret_cast<char*>(&maxMipLevel), sizeof(maxMipLevel));
        file.write(reinterpret_cast<char*>(output.data()), output.size());
        file.close();
    }
    {
        std::vector<unsigned char> output;
        std::string roughnessFilePath = inputDirectory + "/roughness.exr";
        auto roughnessBitmap = FreeImage_Load(FIF_EXR, roughnessFilePath.c_str());
        auto roughnessBPP = FreeImage_GetBPP(roughnessBitmap);
        std::cerr << "Roughness bpp: " << roughnessBPP << std::endl;
        auto roughnessWidth = FreeImage_GetWidth(roughnessBitmap);
        auto roughnessHeight = FreeImage_GetHeight(roughnessBitmap);
        auto roughnessBits = FreeImage_GetBits(roughnessBitmap);

        std::string normalFilePath = inputDirectory + "/normal.exr";
        auto normalBitmap = FreeImage_Load(FIF_EXR, normalFilePath.c_str());
        auto normalBPP = FreeImage_GetBPP(normalBitmap);
        auto normalBits = FreeImage_GetBits(normalBitmap);
        std::cerr << "Normal bpp: " << normalBPP << std::endl;

        float* roughnessFloats = reinterpret_cast<float*>(roughnessBits);
        float* normalFloats = reinterpret_cast<float*>(normalBits);

        
        for (int i = 0; i < roughnessWidth * roughnessHeight; i++) {
            output.emplace_back(JustRay::MapToUnsignedByte(MaterialGamma(normalFloats[i * (normalBPP/32)], gamma)));
            output.emplace_back(JustRay::MapToUnsignedByte(MaterialGamma(normalFloats[i * (normalBPP/32) + 1], gamma)));
            output.emplace_back(JustRay::MapToUnsignedByte(MaterialGamma(normalFloats[i * (normalBPP/32) + 2], gamma)));
            output.emplace_back(JustRay::MapToUnsignedByte(MaterialGamma(roughnessFloats[i * (roughnessBPP / 32)], gamma)));
        }
        FreeImage_Unload(roughnessBitmap);
        FreeImage_Unload(normalBitmap);
        std::ofstream file(outputDirectory + "/roughness.rgba8", std::ios::binary);
        unsigned short w = roughnessWidth;
        unsigned short h = roughnessHeight;
        unsigned short numOfChannel = 4;
        unsigned short maxMipLevel = 0;
        file.write(reinterpret_cast<char*>(&w), sizeof(w));
        file.write(reinterpret_cast<char*>(&h), sizeof(h));
        file.write(reinterpret_cast<char*>(&numOfChannel), sizeof(numOfChannel));
        file.write(reinterpret_cast<char*>(&maxMipLevel), sizeof(maxMipLevel));
        file.write(reinterpret_cast<char*>(output.data()), output.size());
        file.close();
    }

}
int main()
{
    //GenMaterial("../../Resources/output/material/car_paint", "../../Resources/output/material/car_paint");
    //GenMaterial("../../Resources/output/material/brushed_metal", "../../Resources/output/material/brushed_metal");
    GenMaterial("F:/haoxin/material/silver", "F:/haoxin/material/silver", false);
    //PreIntegrate();
    //ConvertObj();
    return 0;
}