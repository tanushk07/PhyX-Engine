#include "PlaneCollider.h"
#include "AABB.h"
#include <cmath>

void PlaneCollider::Intersect(BoundingSphere other, IntersectionData& Data) const
{
    float centerDist = (Normal.dot(other.getPosition()) - Offset);
    float sign = centerDist<0.0f ? -1.0f : 1.0f;
    float surfaceDist = fabs(centerDist) - other.getRadius();

    if (surfaceDist < 0) {
        Data.hasCollided = true;
        Data.IntersectionDepth = fabs(surfaceDist);
        Data.IntersectionNormal = Normal*sign;
        Data.IntersectionPoint = other.getPosition() - Data.IntersectionNormal * (other.getRadius() - Data.IntersectionDepth * 0.5f);
    }
    else {
        Data.hasCollided = false;
        Data.IntersectionDepth = 0;
    }
}

void PlaneCollider::Intersect(const AABB& other, IntersectionData& Data) const
{
    Vec3 center     = (other.getMinExtend() + other.getMaxExtend()) * 0.5f;
    Vec3 halfExtent = (other.getMaxExtend() - other.getMinExtend()) * 0.5f;

    float projectedRadius = fabs(Normal.x) * halfExtent.x
                          + fabs(Normal.y) * halfExtent.y
                          + fabs(Normal.z) * halfExtent.z;

    float signedDist  = Normal.dot(center) - Offset;
    float surfaceDist = fabs(signedDist) - projectedRadius;

    if (surfaceDist >= 0.0f) {
        Data.hasCollided = false;
        Data.IntersectionDepth = 0.0f;
        return;
    }

    float side = (signedDist < 0.0f) ? -1.0f : 1.0f;
    Vec3  n    = Normal * side;

    Vec3 deepest(
        center.x - copysignf(halfExtent.x, n.x),
        center.y - copysignf(halfExtent.y, n.y),
        center.z - copysignf(halfExtent.z, n.z)
    );

    float penetration = -surfaceDist;

    Data.hasCollided        = true;
    Data.IntersectionDepth = penetration;
    Data.IntersectionNormal = n;
    Data.IntersectionPoint  = deepest + n * (penetration * 0.5f);
}
void PlaneCollider::Intersect(const PlaneCollider&, IntersectionData& Data) const
{
    Data.hasCollided = false;    
}