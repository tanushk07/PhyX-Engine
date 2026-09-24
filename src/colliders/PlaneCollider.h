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
    void Intersect(BoundingSphere other, struct IntersectionData& Data) const;
    void Intersect(const AABB& other, IntersectionData& Data) const;
    void Intersect(const PlaneCollider&, IntersectionData& Data) const;

    inline Vec3 GetPlaneNormal() const { return Normal; }
    inline float GetPlaneOffset() const { return Offset; }
};