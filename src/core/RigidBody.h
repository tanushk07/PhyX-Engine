#pragma once
#include "Quaternion.h"
#include "Vector.h"
#include "Inertia.h"

class RigidBody 
{
public:
    // --STATE
    Vec3 Position = Vec3(0.f,0.f,0.f);
    Vec3 AngularMomentum = Vec3(0.f,0.f,0.f);
    Vec3 LinearMomentum = Vec3(0.f,0.f,0.f);
    Quat Orientation;
    
    // --DERIVED
    Vec3 Velocity = Vec3(0.f,0.f,0.f);
    Vec3 AngularVelocity = Vec3(0.f,0.f,0.f);
    //Vec3 Acceleration;

    // -- CONSTANTS
    float Mass;
    float Restitution;
    float FrictionCoeff;
    Vec3 InertiaBody = Vec3(0.f,0.f,0.f);
        
    RigidBody(Vec3 position,
              float mass,
              float restitution,
              Vec3 linearMomentum,
              Vec3 angularMomentum,
              Vec3 inertiaBody,
              float frictionCoeff)
        // Members are initialised in the order they are DECLARED above, not the
        // order written here. This list is kept in declaration order so the two
        // never disagree (that mismatch is what -Wreorder warns about).
        : Position(position),
          AngularMomentum(angularMomentum),
          LinearMomentum(linearMomentum),
          Mass(mass),
          Restitution(restitution),
          FrictionCoeff(frictionCoeff),
          InertiaBody(inertiaBody),
          inverseMass(mass > 0.f ? 1.f/mass : 0.f),
          InvInertiaBody(SafeReciprocal(inertiaBody))
    {}
    
    void AddForce(Vec3 force);
    void Update(float dt);
    void Recalculate();
    void AddForceAtPoint(Vec3 force, Vec3 point);
private:
    // --ACCUMULATORS
    Vec3 forceAcc = Vec3(0.f,0.f,0.f);
    Vec3 torqueAcc = Vec3(0.f,0.f,0.f);
    
    // --INVERSES
    float inverseMass;
    Vec3 InvInertiaBody;
    
    // --HELPER FUNCTIONS
    static Vec3 SafeReciprocal(Vec3 v)
    {
        return Vec3(v.x > 0.f ? 1.f/v.x : 0.f,
                    v.y > 0.f ? 1.f/v.y : 0.f,
                    v.z > 0.f ? 1.f/v.z : 0.f);
    }
};