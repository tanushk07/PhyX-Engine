#pragma once
#include "IntersectionData.h"
#include "RigidBody.h"

struct Contact
{
    RigidBody* BodyA;
    RigidBody* BodyB;
    IntersectionData Data;
};
