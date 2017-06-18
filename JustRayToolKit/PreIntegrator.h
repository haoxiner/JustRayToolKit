#pragma once
#include "Cubemap.h"
#include "MathUtil.h"
#include "ShaderProgram.h"
#include "Platform.h"
#include <string>
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
    void Save(GLuint textureID, const std::string& fileName, int width, int height, bool invertHorizontal, bool invertVertical);
    struct ArgumentsBlock
    {
        Float4 inputArg0;
        Float4 inputArg1;
    };
};
}