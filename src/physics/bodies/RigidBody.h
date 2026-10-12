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
    bool isDynamic = false;
    
    // --DERIVED
    Vec3 Velocity = Vec3(0.f,0.f,0.f);
    Vec3 AngularVelocity = Vec3(0.f,0.f,0.f);
    //Vec3 Acceleration;

    // -- CONSTANTS
    float Mass;
    float Restitution;
    float FrictionCoeff;
    float RollingResistance = 0.f;    
    Vec3 InertiaBody = Vec3(0.f,0.f,0.f);
    RigidBody(Vec3 position,
              float mass,
              float restitution,
              Vec3 inertiaBody,
              float FrictionCoeff,
              float RollingResistance,
              bool isDynamic)
    :   Position(position),
        isDynamic(isDynamic),
        Mass(mass),
        Restitution(restitution),
        FrictionCoeff(FrictionCoeff),
        RollingResistance(RollingResistance),
        InertiaBody(inertiaBody),
        inverseMass(isDynamic? (mass > 0.f ? 1.f / mass : 0.f):0.f),
        InvInertiaBody(isDynamic ? SafeReciprocal(inertiaBody):Vec3{0,0,0})
    {}
    
    void AddForce(Vec3 force);
    void IntegrateVelocity(float dt);
    void IntegratePosition(float dt);
    void Update(float dt);
    void Recalculate();
    void AddForceAtPoint(Vec3 force, Vec3 point);
    bool IsStatic() const {return isDynamic==false;}
    bool IsDynamic() const {return isDynamic==true;}
    float InverseMass() const { return Awake ? inverseMass : 0.f; }

    Vec3 InvInertiaWorld(const Vec3& v) const          
    {
        if (!Awake) return Vec3(0.f,0.f,0.f);
        const Vec3 b = Orientation.conjugate().rotate(v);                     
        return Orientation.rotate(Vec3(b.x * InvInertiaBody.x,                
                                       b.y * InvInertiaBody.y,
                                       b.z * InvInertiaBody.z));             
    }
    
    Vec3 GetInverseInertiaBody() const {return InvInertiaBody;}
    bool IsAwake() const { return Awake; }
    void WakeUp() { if (!Awake) { Awake = true; SleepTime = 0.f; } }
    void Sleep();
    bool CanMove() const { return isDynamic && Awake; }
    bool MovedLastStep() const { return CanMove() && SleepTime == 0.f; }
    float SleepTime = 0.f;
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
    bool Awake = true;
    void ClearAccumulators();
};