#include "RigidBody.h"

void RigidBody::AddForce(Vec3 force)
{
    Acceleration += force / Mass;
}

void RigidBody::Update(float dt)
{
    Velocity += Acceleration * dt;
    Position += Velocity * dt;
    Acceleration = Vec3(0, 0, 0);
}