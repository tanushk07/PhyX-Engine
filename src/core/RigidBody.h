#pragma once
#include "Vector.h"

class RigidBody 
{
public:
    Vec3 Position;
    Vec3 Velocity;
    Vec3 Acceleration;
    float Mass;
    float Restitution;
    RigidBody(Vec3 posi,float mass, float Restitution) : Position(posi),Velocity({0,0,0}),Acceleration({0,0,0}),Mass(mass),Restitution(Restitution){}

    void AddForce(Vec3 force);
    void Update(float dt);
};