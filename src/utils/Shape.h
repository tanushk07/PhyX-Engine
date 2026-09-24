#pragma once
#include "Vector.h"
#include "Inertia.h"
#include <variant>

struct SphereShape
{
    float radius;
    Vec3 CalculateInertia(float mass) const
    {
        return Inertia::Sphere(radius, mass);
    }
};
struct BoxShape
{
    Vec3 HalfExtents;
    Vec3 CalculateInertia(float mass) const
    {
        return Inertia::Box(HalfExtents * 2.0f, mass);
    }
};
struct PlaneShape
{
    Vec3 Normal;
    float offset;
    Vec3 CalculateInertia(float mass) const
    {
        return Vec3{0, 0, 0}; // planes are static
    }
};

using Shape = std::variant<SphereShape, BoxShape, PlaneShape>;