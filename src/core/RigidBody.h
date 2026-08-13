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
    float FrictionCoeff;
    Vec3 Momentum;
    RigidBody(Vec3 posi,float mass, Vec3 Momentum, float Restitution, float FrictionCoeff) : Position(posi),Velocity({0,0,0}),Acceleration({0,0,0}),Mass(mass),Restitution(Restitution),FrictionCoeff(FrictionCoeff),Momentum(Momentum) {}

       
    void AddForce(Vec3 force);
    void Update(float dt);
    void Recalculate();
private:
    Vec3 forceAcc = Vec3(0.f,0.f,0.f);
    float inverseMass;
};