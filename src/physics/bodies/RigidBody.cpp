#include "RigidBody.h"

#include <iostream>
#include <ostream>


void RigidBody::AddForce(Vec3 force)
{
    WakeUp();
    forceAcc+=force;
}

void RigidBody::IntegrateVelocity(float dt)
{
    LinearMomentum += forceAcc * dt;
    AngularMomentum += torqueAcc * dt;
    Recalculate();
    ClearAccumulators();
}

void RigidBody::IntegratePosition(float dt)
{
    Position += Velocity * dt;
    Orientation = (Orientation + Quat(0,AngularVelocity)*Orientation*0.5f*dt).normalize();
}

void RigidBody::AddForceAtPoint(Vec3 force, Vec3 point)
{
    WakeUp();
    torqueAcc+= (point-Position).cross(force) ;
    forceAcc+=force;
}

void RigidBody::Sleep()
{
    Awake = false;
    SleepTime = 0.f;
    LinearMomentum = Vec3(0.f,0.f,0.f);
    AngularMomentum = Vec3(0.f,0.f,0.f);
    Recalculate();
    ClearAccumulators();
}

void RigidBody::Update(float dt)
{
    IntegrateVelocity(dt);
    IntegratePosition(dt);
}

void RigidBody::Recalculate()
{
    Velocity = LinearMomentum*inverseMass;
    
    Vec3 Lbody = Orientation.conjugate().rotate(AngularMomentum);
    
    Vec3 wbody(Lbody.x*InvInertiaBody.x,
        Lbody.y*InvInertiaBody.y,
        Lbody.z*InvInertiaBody.z);
    
    AngularVelocity = Orientation.rotate(wbody);
}

void RigidBody::ClearAccumulators()
{
    forceAcc = Vec3(0.f,0.f,0.f);
    torqueAcc = Vec3(0.f,0.f,0.f);
}
