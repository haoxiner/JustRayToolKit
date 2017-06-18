#pragma once
#include "MathUtil.h"
#include <cfloat>
namespace JustRay
{
struct BBox
{
    BBox(): minPoint(std::numeric_limits<float>::max()), maxPoint(std::numeric_limits<float>::min()){}
    void Union(const Float3& rhs);
    Float3 minPoint;
    Float3 maxPoint;
};
}