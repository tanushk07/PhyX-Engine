#include "RigidBody.h"

void RigidBody::AddForce(Vec3 force)
{
    // Acceleration += force / Mass;
    forceAcc+=force;
}

void RigidBody::Update(float dt)
{
    
    Momentum += forceAcc * dt;
    Recalculate();
    Position += Velocity * dt;
    forceAcc = Vec3(0.f,0.f,0.f);
}

void RigidBody::Recalculate()
{
    inverseMass = 1/Mass;
    Velocity = Momentum*inverseMass;
    //Acceleration = Vec3(0, 0, 0);
}