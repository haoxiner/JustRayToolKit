#include "ModelGroup.h"
#include "TinyObjLoader.h"
#include "MathUtil.h"
#include "BBox.h"
#include "glm/gtx/wrap.hpp"
#include <iostream>
#include <fstream>
namespace JustRay
{
bool ModelGroup::LoadFromObj(const std::string& filepath)
{
    vertices.clear();
    indices_.clear();
    meshes_.clear();

    tinyobj::attrib_t attrib;
    std::string err;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str());
    if (!ret) {
        std::cerr << "ERROR When Reading OBJ: " << err << std::endl;
        return false;
    }
    int numOfVertices = attrib.vertices.size() / 3;
    std::cerr << "obj vertices: " << numOfVertices << std::endl;
    std::ofstream log("../../Resources/log.txt");
    // combine equal vertices
    std::map<Vertex, int> vertexMap;
    std::vector<Float2> texCoords;
    for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shapes[shapeIdx].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[shapeIdx].mesh.num_face_vertices[f];
            if (fv != 3) {
                std::cerr << "Only support triangle mesh. " << std::endl;
                continue;
            }
            for (size_t v = 0; v < fv; v++) {
                auto idx = shapes[shapeIdx].mesh.indices[indexOffset + v];
                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];
                float nx = attrib.normals[3 * idx.normal_index + 0];
                float ny = attrib.normals[3 * idx.normal_index + 1];
                float nz = attrib.normals[3 * idx.normal_index + 2];
                float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                float ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                log << tx << ", " << ty << ":             ";
                //while (tx > 1.0f) {
                //    tx -= 1.0f;
                //}
                //while (tx < 0.0f) {
                //    tx += 1.0f;
                //}
                //while (ty > 1.0f) {
                //    ty -= 1.0f;
                //}
                //while (ty < 0.0f) {
                //    ty += 1.0f;
                //}
                //auto vt = glm::repeat(glm::vec2(tx, ty));
                //tx = vt.x;
                //ty = vt.y;
                //log << tx << ", " << ty << std::endl;
                Vertex vertex = {
                    { vx,vy,vz },
                    PackFloat3ToInt2_10_10_10({ nx,ny,nz }),
                    PackFloat3ToInt2_10_10_10({ 0,0,0 }),
                    PackFloat3ToInt2_10_10_10({ 0,0,0 }),
                    half(tx), half(ty)
                };
                bbox_.Union(vertex.position);
                auto vMapIter = vertexMap.find(vertex);
                if (vMapIter == vertexMap.end()) {
                    int index = static_cast<int>(vertices.size());
                    indices_.emplace_back(index);
                    vertexMap[vertex] = index;
                    vertices.emplace_back(vertex);
                    texCoords.emplace_back(tx, ty);
                } else {
                    indices_.emplace_back(vMapIter->second);
                }
            }
            indexOffset += fv;
        }
    }
    std::vector<Float3> tangents(vertices.size(), Float3(0,0,0));
    std::vector<Float3> bitangents(vertices.size(), Float3(0,0,0));
    for (int i = 0; i < indices_.size(); i += 3) {
        auto idx0 = indices_[i + 0];
        auto idx1 = indices_[i + 1];
        auto idx2 = indices_[i + 2];
        auto& v0 = vertices[idx0];
        auto& v1 = vertices[idx1];
        auto& v2 = vertices[idx2];
        auto& p0 = v0.position;
        auto& p1 = v1.position;
        auto& p2 = v2.position;
        auto& uv0 = texCoords[idx0];
        auto& uv1 = texCoords[idx1];
        auto& uv2 = texCoords[idx2];
        auto deltaPos1 = p1 - p0;
        auto deltaPos2 = p2 - p0;
        auto deltaUV1 = uv1 - uv0;
        auto deltaUV2 = uv2 - uv0;
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
        auto bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
        tangents[idx0] += tangent;
        tangents[idx1] += tangent;
        tangents[idx2] += tangent;
        bitangents[idx0] += bitangent;
        bitangents[idx1] += bitangent;
        bitangents[idx2] += bitangent;
    }
    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].tangent = PackFloat3ToInt2_10_10_10(Normalize(tangents[i]));
    }
    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].bitangent = PackFloat3ToInt2_10_10_10(Normalize(bitangents[i]));
    }
    return true;
}
std::tuple<int, int> ModelGroup::WriteToFile(const std::string& filepath)
{
    std::ofstream output(filepath, std::ios::binary);
    int sizeOfVertices = sizeof(Vertex) * vertices.size();
    output.write(reinterpret_cast<char*>(vertices.data()), sizeOfVertices);
    int sizeOfIndices = sizeof(unsigned int) * indices_.size();
    output.write(reinterpret_cast<char*>(indices_.data()), sizeOfIndices);
    std::cerr << indices_[0] << "," << indices_[1];
    return std::make_tuple(0, sizeOfVertices);
}
}