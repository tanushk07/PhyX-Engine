#pragma once
#include "AABB.h"
#include "BoundingSphere.h"
#include "IntersectionData.h"
#include "Quaternion.h"
#include "Vector.h"

class OBB
{
    Vec3 Position, HalfExtents;
    Quat Orientation;
    Vec3 Axes[3];
public:
    OBB(Vec3 Position, Quat Orientation, Vec3 HalfExtents) : Position(Position),HalfExtents(HalfExtents), Orientation(Orientation), Axes{
        Orientation.rotate({1,0,0}),
        Orientation.rotate({0,1,0}),
        Orientation.rotate({0,0,1}),
    }  {}
    Vec3 GetPosition() const { return Position; }
    Vec3 GetHalfExtent() const { return HalfExtents; }
    Quat GetOrientation() const { return Orientation; }
    const Vec3& Axis(int i) const { return Axes[i]; }
    float HalfExtent(int i) const { return i == 0 ? HalfExtents.x : (i == 1 ? HalfExtents.y : HalfExtents.z); }
    
    
    float ProjectedRadius(const Vec3& n) const
    {
        return HalfExtents.x * fabsf(n.dot(Axes[0]))
             + HalfExtents.y * fabsf(n.dot(Axes[1]))
             + HalfExtents.z * fabsf(n.dot(Axes[2]));
    }

    // Points rotate AND move. Directions only rotate.
    Vec3 PointToWorld(const Vec3& p) const { return Position + Orientation.rotate(p); }
    Vec3 PointToLocal(const Vec3& p) const { return Orientation.conjugate().rotate(p - Position); }
    Vec3 DirToWorld  (const Vec3& d) const { return Orientation.rotate(d); }
    Vec3 DirToLocal  (const Vec3& d) const { return Orientation.conjugate().rotate(d); }
    
    void Intersect(const OBB& other, IntersectionData &Data) const;
    void Intersect(const BoundingSphere& other, IntersectionData &Data) const;
    void Intersect(const AABB& other, IntersectionData &Data) const;
};
