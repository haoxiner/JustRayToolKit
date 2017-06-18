#pragma once
#include "Vertex.h"
#include "BBox.h"
#include <vector>
#include <tuple>
#include <string>
namespace JustRay
{
class ModelGroup
{
public:
    bool LoadFromObj(const std::string& filepath);
    std::tuple<int, int> WriteToFile(const std::string& filepath);
    int GetNumOfIndices() { return indices_.size(); }
    int GetNumOfMeshes() { return meshes_.size(); }
    int GetNumOfVertices() { return vertices.size(); }
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices_;
    std::vector<std::tuple<int, int, int>> meshes_;
    BBox bbox_;
};
}