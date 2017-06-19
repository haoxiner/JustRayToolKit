#pragma once
#include "Cubemap.h"
#include "MathUtil.h"
#include "ShaderProgram.h"
#include "Half\half.h"
#include "Platform.h"
#include <string>
#include <vector>
namespace JustRay
{
class PreIntegrator
{
public:
    void IntegrateIBLDFG(const std::string& outputID, const std::string& outputDirectory);
    void IntegrateIBLDiffuseAndSpecular(const std::string& inputDirectory,
                                        const std::string& inputID,
                                        const std::string& outputDirectory,
                                        const std::string& outputID);
private:
    void Output(short w, short h, short channel, short maxMipLevel, const std::string& directory, const std::string& fileID);
private:
    void Save(GLuint textureID, const std::string& fileName, int width, int height, bool invertHorizontal, bool invertVertical);
    struct ArgumentsBlock
    {
        Float4 inputArg0;
        Float4 inputArg1;
    };
    std::vector<half> output_;
};
}