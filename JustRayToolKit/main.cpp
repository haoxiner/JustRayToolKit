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
    bool ret = modelGroup.LoadFromObj("../../output/mitsuba.obj");
    if (ret) {
        auto result = modelGroup.WriteToFile("../../output/mistuba.mg");

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
    preIntegrator.IntegrateIBLDFG("dfg", "../../Resources/output");
    preIntegrator.Output("../../Resources/output/pkg", "dfg");
    preIntegrator.IntegrateIBLDiffuseAndSpecular("../../Resources/Environment/uffizi/src", "uffizi", "../../Resources/output", "uffizi");
    preIntegrator.Output("../../Resources/output/pkg", "uffizi.ibl");
}
void GenMaterial()
{
    
    {
        std::vector<unsigned char> output;
        std::string baseColorFilePath = "../../Resources/output/basecolor.exr";
        auto bitmap = FreeImage_Load(FIF_EXR, baseColorFilePath.c_str());
        auto bpp = FreeImage_GetBPP(bitmap);
        std::cerr << bpp << std::endl;
        auto width = FreeImage_GetWidth(bitmap);
        auto height = FreeImage_GetHeight(bitmap);
        auto bits = FreeImage_GetBits(bitmap);
        float* floatPixels = reinterpret_cast<float*>(bits);
        for (int i = 0; i < width * height * 4; i++) {
            output.emplace_back(JustRay::MapToUnsignedByte(floatPixels[i]));
        }
        FreeImage_Unload(bitmap);
        std::ofstream file("../../Resources/output/basecolorMetallic.raw", std::ios::binary);
        unsigned short w = width;
        unsigned short h = height;
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
        std::string roughnessFilePath = "../../Resources/output/roughness.exr";
        auto bitmap = FreeImage_Load(FIF_EXR, roughnessFilePath.c_str());
        auto bpp = FreeImage_GetBPP(bitmap);
        std::cerr << bpp << std::endl;
        auto width = FreeImage_GetWidth(bitmap);
        auto height = FreeImage_GetHeight(bitmap);
        auto bits = FreeImage_GetBits(bitmap);
        float* floatPixels = reinterpret_cast<float*>(bits);
        for (int i = 0; i < width * height * 1; i++) {
            output.emplace_back(JustRay::MapToUnsignedByte(0));
            output.emplace_back(JustRay::MapToUnsignedByte(0));
            output.emplace_back(JustRay::MapToUnsignedByte(1));
            output.emplace_back(JustRay::MapToUnsignedByte(floatPixels[i]));
        }
        FreeImage_Unload(bitmap);
        std::ofstream file("../../Resources/output/normalRoughness.raw", std::ios::binary);
        unsigned short w = width;
        unsigned short h = height;
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
    GenMaterial();
    PreIntegrate();
    return 0;
}