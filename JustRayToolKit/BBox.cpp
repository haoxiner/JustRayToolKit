#include "BBox.h"
#include <algorithm>
void JustRay::BBox::Union(const Float3& rhs)
{
    for (int i = 0; i < 3; i++) {
        maxPoint[i] = std::max(maxPoint[i], rhs[i]);
        minPoint[i] = std::min(minPoint[i], rhs[i]);
    }
}
