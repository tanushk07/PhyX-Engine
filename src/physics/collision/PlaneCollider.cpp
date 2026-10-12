#include "PlaneCollider.h"

#include <algorithm>

#include "AABB.h"
#include <cmath>

void PlaneCollider::Intersect(BoundingSphere other, IntersectionData& Data) const
{
    float centerDist = (Normal.dot(other.getPosition()) - Offset);
    float sign = centerDist<0.0f ? -1.0f : 1.0f;
    float surfaceDist = fabs(centerDist) - other.getRadius();

    if (surfaceDist < 0) {
        Data.hasCollided = true;
        float IntersectionDepth = fabs(surfaceDist);
        Data.IntersectionNormal = Normal*sign;
        Vec3 IntersectionPoint = other.getPosition() - Data.IntersectionNormal * (other.getRadius() - IntersectionDepth * 0.5f);
        
        Data.AddPoint(IntersectionPoint, IntersectionDepth);
    }
    else {
        Data.hasCollided = false;
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
    float IntersectionDepth = penetration;
    Data.IntersectionNormal = n;
    Vec3 IntersectionPoint  = deepest + n * (penetration * 0.5f);
    
    Data.AddPoint(IntersectionPoint, IntersectionDepth);
}
void PlaneCollider::Intersect(const PlaneCollider&, IntersectionData& Data) const
{
    Data.hasCollided = false;    
}


void PlaneCollider::Intersect(const OBB& other, IntersectionData& Data) const
{
    const Vec3  center     = other.GetPosition();
    const float r          = other.ProjectedRadius(Normal);           // how far the box reaches along the normal
    const float signedDist = Normal.dot(center) - Offset;
    if (fabs(signedDist) - r >= 0.0f) { Data.hasCollided = false; return; }   // even the deepest corner doesn't reach

    const float side = (signedDist < 0.0f) ? -1.0f : 1.0f;
    const Vec3  n    = Normal * side;                                  // plane -> box

    ContactPointData below[8];
    int found = 0;
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sy = -1; sy <= 1; sy += 2)
            for (int sz = -1; sz <= 1; sz += 2)
            {
                const Vec3 corner = center + other.Axis(0) * (sx * other.HalfExtent(0))
                                           + other.Axis(1) * (sy * other.HalfExtent(1))
                                           + other.Axis(2) * (sz * other.HalfExtent(2));
                const float d = (Normal.dot(corner) - Offset) * side;         // + on the box's side, - through the plane
                if (d < 0.0f)
                    below[found++] = { corner + n * (-d * 0.5f), -d };
            }

    std::sort(below, below + found, [](const ContactPointData& a, const ContactPointData& b)
              { return a.IntersectionDepth > b.IntersectionDepth; });  // deepest first; AddPoint stops at 4

    Data.hasCollided        = found > 0;
    Data.IntersectionNormal = n;
    for (int i = 0; i < found; ++i)
        Data.AddPoint(below[i].IntersectionPoint, below[i].IntersectionDepth);
}