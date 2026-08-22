#include "RigidBody.h"


void RigidBody::AddForce(Vec3 force)
{
    // Acceleration += force / Mass;
    forceAcc+=force;
}

void RigidBody::AddForceAtPoint(Vec3 force, Vec3 point)
{
    torqueAcc+= (point-Position).cross(force) ;
    forceAcc+=force;
}


void RigidBody::Update(float dt)
{
    
    LinearMomentum += forceAcc * dt;
    AngularMomentum += torqueAcc * dt;
    Recalculate();
    Position += Velocity * dt;
    Orientation = (Orientation + Quat(0,AngularVelocity)*Orientation*0.5f*dt).normalize();
    forceAcc = Vec3(0.f,0.f,0.f);
    torqueAcc = Vec3(0.f,0.f,0.f);
}

void RigidBody::Recalculate()
{
    Velocity = LinearMomentum*inverseMass;

    Vec3 Lbody = Orientation.conjugate().rotate(AngularMomentum);
    
    Vec3 wbody(Lbody.x*InvInertiaBody.x,
        Lbody.y*InvInertiaBody.y,
        Lbody.z*InvInertiaBody.z);
    
    AngularVelocity = Orientation.rotate(wbody);
    //Acceleration = Vec3(0, 0, 0);
}