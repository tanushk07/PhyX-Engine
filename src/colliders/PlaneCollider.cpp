#include "PlaneCollider.h"
#include "AABB.h"
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

void PlaneCollider::IntersectWithAABB(const AABB& other, IntersectionData& Data)
{
    Vec3 center = (other.getMinExtend() + other.getMaxExtend()) * 0.5f;
    Vec3 halfExtent = (other.getMaxExtend() - other.getMinExtend()) * 0.5f;

    float projectedRadius = fabs(Normal.x) * halfExtent.x
                          + fabs(Normal.y) * halfExtent.y
                          + fabs(Normal.z) * halfExtent.z;

    float centerDist = fabs(Normal.dot(center) - Offset);
    float surfaceDist = centerDist - projectedRadius;

    if (surfaceDist < 0) {
        Data.hasCollided = true;
        Data.IntersectionRadius = surfaceDist;
    }
    else {
        Data.hasCollided = false;
        Data.IntersectionRadius = 0;
    }
}