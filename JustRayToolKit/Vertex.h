#pragma once
#include "MathUtil.h"
#include <memory>
namespace JustRay
{
struct Vertex
{
    Float3 position;
    Int_2_10_10_10 normal;
    Int_2_10_10_10 tangent;
    Int_2_10_10_10 bitangent;
    unsigned short tx;
    unsigned short ty;
    bool operator<(const Vertex& rhs) const
    {
        return std::memcmp((void*)this, (void*)&rhs, sizeof(Vertex)) < 0;
    };
};
}