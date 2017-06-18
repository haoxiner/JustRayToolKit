#include "Cubemap.h"
#include "Json.h"
#include "ResourceManager.h"
#include <FreeImage.h>
#include <fstream>
#include <iostream>

namespace JustRay
{
const std::string Cubemap::FACE_NAME[6] = { "PX", "NX", "PY", "NY", "PZ", "NZ" };
const std::string Cubemap::DIRECTION_NAME[6] = { "right", "left", "front", "back", "up", "down" };

Cubemap::Cubemap(const std::string& directory, const std::string& fileID, bool enableHDR)
{
    //auto json = ResourceManager::ReadFileToString(directory + "/" + fileID + ".json");
    Json::JsonObject config;
    Json::Parser parser(&config, ResourceManager::ReadFileToString(directory + "/" + fileID + ".json"));
    int configMaxMipLevel = 0;
    bool loadMipmapFromFile = false;
    if (!parser.HasError()) {
        std::cerr << "Load Config" << std::endl;
        configMaxMipLevel = config.GetValue("max_mip_level").AsInt();
        loadMipmapFromFile = config.GetValue("load_mipmap_from_file").AsBool();
    }
    maxMipLevel_ = configMaxMipLevel;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID_);

    for (int face = 0; face < 6; face++) {
        std::string fileName = directory + "/" + fileID + "_" + FACE_NAME[face] + ".exr";
        auto bitmap = FreeImage_Load(FIF_EXR, fileName.c_str());
        auto bpp = FreeImage_GetBPP(bitmap);
        auto width = FreeImage_GetWidth(bitmap);
        auto height = FreeImage_GetHeight(bitmap);
        if (face == 0) {
            width_ = width;
            height_ = height;
            std::cerr << width << "," << height << ", " << bpp << std::endl;
            std::cerr << "MAX_MIP_LEVEL: " << maxMipLevel_ << std::endl;
            glTextureStorage2D(textureID_, maxMipLevel_ + 1, enableHDR ? GL_RGB16F : GL_RGB8, width, height);
        }
        FreeImage_FlipHorizontal(bitmap);
        FreeImage_FlipVertical(bitmap);
        auto bits = FreeImage_GetBits(bitmap);
        glTextureSubImage3D(textureID_, 0, 0, 0, face, width, height, 1, GL_RGB, GL_FLOAT, bits);
        FreeImage_Unload(bitmap);
    }
    if (loadMipmapFromFile) {
        for (int face = 0; face < 6; face++) {
            for (int level = 1; level <= maxMipLevel_; level++) {
                std::string fileName = directory + "/" + fileID + "_" + FACE_NAME[face] + "_" + std::to_string(level) + ".exr";
                auto bitmap = FreeImage_Load(FIF_EXR, fileName.c_str());
                auto bpp = FreeImage_GetBPP(bitmap);
                auto width = FreeImage_GetWidth(bitmap);
                auto height = FreeImage_GetHeight(bitmap);
                FreeImage_FlipHorizontal(bitmap);
                FreeImage_FlipVertical(bitmap);
                auto bits = FreeImage_GetBits(bitmap);
                glTextureSubImage3D(textureID_, level, 0, 0, face, width, width, 1, GL_RGB, GL_FLOAT, bits);
                FreeImage_Unload(bitmap);
            }
        }
    } else {
        if (maxMipLevel_ > 0) {
            glGenerateTextureMipmap(textureID_);
            std::cerr << "GENERATE" << std::endl;
        }
    }
    std::cerr << "===" << maxMipLevel_ << std::endl;
    glCreateSamplers(1, &samplerID_);
    glSamplerParameteri(samplerID_, GL_TEXTURE_CUBE_MAP_SEAMLESS, 1);
    glSamplerParameteri(samplerID_, GL_TEXTURE_MIN_FILTER, maxMipLevel_ > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glSamplerParameteri(samplerID_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerID_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(samplerID_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(samplerID_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(samplerID_, GL_TEXTURE_MAX_LOD, maxMipLevel_);
    glSamplerParameteri(samplerID_, GL_TEXTURE_MIN_LOD, 0);
}

Cubemap::~Cubemap()
{
    glDeleteSamplers(1, &samplerID_);
    glDeleteTextures(1, &textureID_);
}

void Cubemap::BindTextureUint(int unit) const
{
    glBindTextureUnit(unit, textureID_);
    glBindSampler(unit, samplerID_);
}
}