#include "PreIntegrator.h"
#include "ResourceManager.h"
#include "Cubemap.h"
#include <FreeImage.h>
#include <iostream>

namespace JustRay
{
void PreIntegrator::IntegrateIBLDFG(const std::string& fileID, const std::string& outputPath)
{
    const int outputWidth = 128;
    const int samplesPerPixel = 1024;
    // pass output image texture
    GLuint outputTextureID;
    glCreateTextures(GL_TEXTURE_2D, 1, &outputTextureID);
    glTextureStorage2D(outputTextureID, 1, GL_RGBA32F, outputWidth, outputWidth);
    glBindImageTexture(0, outputTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // pass arguments
    GLuint argsBufferID;
    glCreateBuffers(1, &argsBufferID);
    ArgumentsBlock args = {
        Float4(0, outputWidth, samplesPerPixel, 0),
        Float4(0, 0, 0, 0)
    };
    glNamedBufferStorage(argsBufferID, sizeof(ArgumentsBlock), &args, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, argsBufferID);
    // pass output image texture
    glBindTextureUnit(0, outputTextureID);
    // dispatch computation works
    const int local_size_x = 32;
    const int local_size_y = local_size_x;
    const int local_size_z = 1;

    auto shaderSource_ = ResourceManager::ReadFileToString("./Shader/PreIntegrator.cs.glsl");
    ShaderProgram computeShaderProgram;
    computeShaderProgram.Startup(shaderSource_, local_size_x, local_size_y, local_size_z, { "INTEGRATE_DFG" });
    computeShaderProgram.Use();

    glDispatchCompute(outputWidth / local_size_x, outputWidth / local_size_y, local_size_z);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    computeShaderProgram.Shutdown();
    glDeleteBuffers(1, &argsBufferID);

    // write to file
    Save(outputTextureID, outputPath + "/" + fileID + ".exr", outputWidth, outputWidth, false, false);
    glDeleteTextures(1, &outputTextureID);
}

void PreIntegrator::IntegrateIBLDiffuseAndSpecular(const std::string& inputDirectory,
                                                          const std::string& inputID,
                                                          const std::string& outputDirectory,
                                                          const std::string& outputID)
{
    const int diffuseOutputWidth = 64;
    const int samplesPerPixel = 1024 * 4;

    const std::string FACE_NAME[] = { "PX", "NX", "PY", "NY", "PZ", "NZ" };
    // arguments buffer
    GLuint argsBufferID;
    glCreateBuffers(1, &argsBufferID);
    glNamedBufferStorage(argsBufferID, sizeof(ArgumentsBlock), nullptr, GL_MAP_WRITE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, argsBufferID);

    Cubemap cubemap(inputDirectory, inputID);
    cubemap.BindTextureUint(0);
    auto srcMaxLevel = cubemap.GetMaxMipLevel();
    auto shaderSource_ = ResourceManager::ReadFileToString("./Shader/PreIntegrator.cs.glsl");
    ShaderProgram computeShaderProgram;
    const int local_size_x = 32;
    const int local_size_y = local_size_x;
    const int local_size_z = 1;
    computeShaderProgram.Startup(shaderSource_, local_size_x, local_size_y, local_size_z, { "INTEGRATE_DIFFUSE" });
    for (int face = 0; face < 6; face++) {
        auto inputBuffer = reinterpret_cast<ArgumentsBlock*>(glMapNamedBuffer(argsBufferID, GL_WRITE_ONLY));
        inputBuffer->inputArg0 = { cubemap.GetWidth(), diffuseOutputWidth, samplesPerPixel, 0 };
        inputBuffer->inputArg1.x = cubemap.GetMaxMipLevel();
        inputBuffer->inputArg1.y = 0;
        inputBuffer->inputArg1.z = 0;
        inputBuffer->inputArg1.w = face;
        glUnmapNamedBuffer(argsBufferID);
        GLuint outputTextureID;
        glCreateTextures(GL_TEXTURE_2D, 1, &outputTextureID);
        glBindImageTexture(0, outputTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glTextureStorage2D(outputTextureID, 1, GL_RGBA32F, diffuseOutputWidth, diffuseOutputWidth);
        // dispatch computation works
        computeShaderProgram.Use();
        glDispatchCompute(diffuseOutputWidth / local_size_x, diffuseOutputWidth / local_size_y, local_size_z);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        Save(outputTextureID, outputDirectory + "/" + outputID + "_diffuse_" + FACE_NAME[face] + ".exr", diffuseOutputWidth, diffuseOutputWidth, true, true);
        glDeleteTextures(1, &outputTextureID);
    }
    computeShaderProgram.Shutdown();

    const int outputMaxMipLevel = 6;
    int specularOutputWidth = 512;
    for (int level = 0; level <= outputMaxMipLevel; level++) {
        int local_size_x = std::min(specularOutputWidth, 32);
        int local_size_y = local_size_x;
        int local_size_z = 1;
        ShaderProgram computeSpecular;
        computeSpecular.Startup(shaderSource_, local_size_x, local_size_y, local_size_z, { "INTEGRATE_SPECULAR" });
        for (int face = 0; face < 6; face++) {
            auto inputBuffer = reinterpret_cast<ArgumentsBlock*>(glMapNamedBuffer(argsBufferID, GL_WRITE_ONLY));
            inputBuffer->inputArg0 = { cubemap.GetWidth(), specularOutputWidth, samplesPerPixel, 0 };
            inputBuffer->inputArg1.x = cubemap.GetMaxMipLevel();
            inputBuffer->inputArg1.y = level;
            inputBuffer->inputArg1.z = outputMaxMipLevel;
            inputBuffer->inputArg1.w = face;
            glUnmapNamedBuffer(argsBufferID);
            GLuint outputTextureID;
            glCreateTextures(GL_TEXTURE_2D, 1, &outputTextureID);
            glBindImageTexture(0, outputTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glTextureStorage2D(outputTextureID, 1, GL_RGBA32F, specularOutputWidth, specularOutputWidth);
            computeSpecular.Use();
            glDispatchCompute(specularOutputWidth / local_size_x, specularOutputWidth / local_size_y, local_size_z);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            Save(outputTextureID, outputDirectory + "/" + outputID + "_specular_" + FACE_NAME[face] + (level > 0 ? ("_" + std::to_string(level)) : "") + ".exr", specularOutputWidth, specularOutputWidth, true, true);
            glDeleteTextures(1, &outputTextureID);
        }
        computeSpecular.Shutdown();
        std::cerr << level << ": " << specularOutputWidth << std::endl;
        specularOutputWidth /= 2;
    }
    glDeleteBuffers(1, &argsBufferID);
}

void PreIntegrator::Save(GLuint textureID, const std::string& fileName, int width, int height, bool invertHorizontal, bool invertVertical)
{
    auto texSize = sizeof(float) * 3 * width * height;
    auto bitmap = FreeImage_AllocateT(FIT_RGBF, width, height);
    auto pixels = FreeImage_GetBits(bitmap);
    glGetTextureImage(textureID, 0, GL_RGB, GL_FLOAT, texSize, pixels);
    if (invertHorizontal) {
        FreeImage_FlipHorizontal(bitmap);
    }
    if (invertVertical) {
        FreeImage_FlipVertical(bitmap);
    }
    FreeImage_Save(FIF_EXR, bitmap, fileName.c_str(), EXR_DEFAULT);
    FreeImage_Unload(bitmap);
}
}