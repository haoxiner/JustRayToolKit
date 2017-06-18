#pragma once
#include "Platform.h"
#include <string>
namespace JustRay
{
class Cubemap
{
public:
    Cubemap(const std::string& directory, const std::string& fileID, bool enableHDR = true);
    ~Cubemap();
    void BindTextureUint(int unit) const;
    int GetMaxMipLevel() const { return maxMipLevel_; }
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    static const std::string FACE_NAME[6];
    static const std::string DIRECTION_NAME[6];
private:
    GLuint textureID_ = 0;
    GLuint samplerID_ = 0;
    int width_ = 0;
    int height_ = 0;
    int maxMipLevel_ = 0;
};
}