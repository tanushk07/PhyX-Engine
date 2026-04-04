#include "PlaneCollider.h"
#include <cmath>

void PlaneCollider::IntersectWithBS(BoundingSphere other, IntersectionData& Data)
{
    float centerDist = fabs(Normal.dot(other.getPosition()) - Offset);
    float surfaceDist = centerDist - other.getRadius();

    if (surfaceDist < 0) {
        Data.hasCollided = true;
        Data.IntersectionRadius = surfaceDist;
    }
    else {
        Data.hasCollided = false;
        Data.IntersectionRadius = 0;
    }
}