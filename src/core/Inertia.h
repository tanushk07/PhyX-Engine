#pragma once
#include "Vector.h"

namespace Inertia
{
    inline Vec3 Sphere(float radius, float mass)
    {
        float i = 0.4f*mass*radius*radius;
        return Vec3(i,i,i);
    }
    inline Vec3 Box(Vec3 fullSize,float mass)
    {
        float k = mass/12.f;
        float a = fullSize.x, b = fullSize.y, c = fullSize.z;
        return Vec3(k*(b*b + c*c), k*(a*a + c*c), k*(a*a + b*b));
    }
}