#pragma once
#include <fstream>
#include <string>
#include <vector>
namespace JustRay
{
class ResourceManager
{
public:
    static std::string ReadFileToString(const std::string& path);
};
}