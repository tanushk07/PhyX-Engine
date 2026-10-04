#pragma once
#include "Vector.h"
#include "Shape.h"

struct BodyDesc
{
    Vec3 Position;
    float Mass;
    float Restitution;
    Shape Shape;
    float FrictionCoeff;
    float RollingResistance;
    bool isDynamic;
};
