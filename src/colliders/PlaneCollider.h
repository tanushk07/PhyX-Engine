#pragma once
#include "Vector.h"
#include "BoundingSphere.h"

class AABB;

class PlaneCollider
{
    Vec3 Normal;
    float Offset;

public:
    PlaneCollider(Vec3 Normal, float Offset) : Normal(Normal.normalize()), Offset(Offset) {}
    void IntersectWithBS(BoundingSphere other, struct IntersectionData& Data);
    void IntersectWithAABB(const AABB& other, IntersectionData& Data);

    inline const Vec3 GetPlaneNormal() const { return Normal; }
    inline const float GetPlaneOffset() const { return Offset; }
};