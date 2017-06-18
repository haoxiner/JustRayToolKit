#include "ResourceManager.h"
#include "TinyObjLoader.h"
#include "MathUtil.h"
#include "Vertex.h"
#include <vector>
#include <iostream>
#include <memory>

namespace JustRay
{
std::string ResourceManager::ReadFileToString(const std::string& fileName)
{
    std::ifstream fileInputStream(fileName);
    if (!fileInputStream) {
        return "";
    }
    std::istreambuf_iterator<char> fileBegin(fileInputStream), fileEnd;
    return std::string(fileBegin, fileEnd);
}
}