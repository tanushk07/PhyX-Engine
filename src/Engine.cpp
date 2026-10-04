#include "Engine.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>

RigidBody& PhysicsEngine::CreateBody(const BodyDesc& bodyDesc)
{
    Vec3 InertiaVector= GetInertia(bodyDesc.Shape, bodyDesc.Mass);
    auto Body = std::make_unique<RigidBody>(
        bodyDesc.Position,
        bodyDesc.Mass,
        bodyDesc.Restitution,
        InertiaVector,
        bodyDesc.FrictionCoeff,
        bodyDesc.RollingResistance,
        bodyDesc.isDynamic
    );
    RigidBody& handle = *Body;
    Entries.push_back({std::move(Body), bodyDesc.Shape});
    return handle;
}

void PhysicsEngine::step(float dt)
{
    IntegrateForces(dt);
    DetectCollisions();
    ResolveCollisions(dt);
}

void PhysicsEngine::IntegrateForces(float dt)
{
    if (Entries.empty()) return;
    
    for (auto &[d,s] : Entries)
    {
        if (!d->isDynamic) continue;
        Vec3 gravityForce{
            gravity.x * d->Mass,
            gravity.y * d->Mass,
            gravity.z * d->Mass
        };

        d->AddForce(gravityForce);
        d->Update(dt);
    }
}

void PhysicsEngine::DetectCollisions()
{
    if (Entries.empty()) return;
    
    ContactPoints.clear();
    std::vector<Collider> colliders;                 
    colliders.reserve(Entries.size());
    for (auto &[body,shape] : Entries)
    {
        const auto collider = MakeCollider(*body,shape);
        colliders.push_back(collider);
    }
    
    for (int i = 0; i < static_cast<int>(Entries.size()); i++)
    {
        for (int j = i+1; j < static_cast<int>(Entries.size()); j++)
        {
            auto &[bodyA,shapeA] = Entries[j];
            auto &[bodyB,shapeB] = Entries[i];
            IntersectionData Data;
            std::visit([ &Data](auto& a, auto& b)
            {
                Collide(a,b,Data);
            },colliders[j],colliders[i]);
            
            if (!Data.hasCollided) continue;
            ContactPoints.push_back({bodyA.get(),bodyB.get(),Data});
        }
    }
}

void PhysicsEngine::ResolveCollisions(float dt)
{
    if (Entries.empty() || ContactPoints.empty()) return;
    
    constexpr int   Iterations       = 10;
    constexpr float RestThreshold    = 1.0f;    // m/s: slower impacts don't bounce
    constexpr float Beta             = 0.2f;    // remove 20% of the overlap per step
    constexpr float Slop             = 0.01f;   // allow 1 cm of overlap, so resting contacts 
    
    auto K = [](RigidBody* A, RigidBody* B, const Vec3& rA, const Vec3& rB, const Vec3& d)
    {
        const Vec3 a = rA.cross(d), b = rB.cross(d);
        return A->InverseMass() + B->InverseMass() + a.dot(A->InvInertiaWorld(a)) + b.dot(B->InvInertiaWorld(b));
    };
    auto RelVel = [](RigidBody* A, RigidBody* B, const Vec3& rA, const Vec3& rB)
    {
        return (A->Velocity + A->AngularVelocity.cross(rA)) - (B->Velocity + B->AngularVelocity.cross(rB));
    };
    auto Apply = [](RigidBody* A, RigidBody* B, const Vec3& rA, const Vec3& rB, const Vec3& J)
    {
        A->LinearMomentum  += J;            B->LinearMomentum  -= J;
        A->AngularMomentum += rA.cross(J);  B->AngularMomentum -= rB.cross(J);
        A->Recalculate();                   B->Recalculate();
    };
        
    for (auto &c : ContactPoints)
    {
        RigidBody* bodya = c.BodyA;
        RigidBody* bodyb = c.BodyB;
        if (bodya->IsStatic() && bodyb->IsStatic()) continue;
        if (c.Data.IntersectionNormal.lengthsqr() < 1e-12f) continue;
        Vec3 n = -c.Data.IntersectionNormal; //n goes from b to a
        const float Restitution = std::min(bodya->Restitution, bodyb->Restitution);
        for (int i = 0; i < c.Data.Count; ++i)
        {
            auto &contactPoint = c.Data.Points[i];
            const auto r_A = contactPoint.IntersectionPoint - bodya->Position;
            const auto r_B = contactPoint.IntersectionPoint - bodyb->Position;
            
            const auto vn = RelVel(bodya,bodyb,r_A,r_B).dot(n);
            const float Bias   = Beta / dt * std::max(0.f, contactPoint.IntersectionDepth - Slop);
            const float Bounce = vn < -RestThreshold ? -Restitution * vn : 0.f;
            contactPoint.TargetVn = std::max(Bounce, Bias);
        }
    }
    
    for (int pass = 0; pass < Iterations; ++pass)
    {
        for (auto &c : ContactPoints)
        {
            RigidBody* bodya = c.BodyA;
            RigidBody* bodyb = c.BodyB;
            if (bodya->IsStatic() && bodyb->IsStatic()) continue;
            if (c.Data.IntersectionNormal.lengthsqr() < 1e-12f) continue;
            
            const Vec3  n              = -c.Data.IntersectionNormal;
            const float StaticFriction = std::sqrt(bodya->FrictionCoeff * bodyb->FrictionCoeff);
            
            for (int i = 0; i < c.Data.Count; ++i)
            {
                auto &contactPoint = c.Data.Points[i];
                if (contactPoint.IntersectionDepth <= 0.f) continue;
                
                const auto r_A = contactPoint.IntersectionPoint - bodya->Position;
                const auto r_B = contactPoint.IntersectionPoint - bodyb->Position;
                
                // B1. Normal
                const float vn       = RelVel(bodya, bodyb, r_A, r_B).dot(n);
                const float k        = K(bodya, bodyb, r_A, r_B, n);
                const float OldTotal = contactPoint.NormalTotal;
                contactPoint.NormalTotal = std::max(0.f, OldTotal + (contactPoint.TargetVn - vn) / k);
                const float j        = contactPoint.NormalTotal - OldTotal;
                Apply(bodya, bodyb, r_A, r_B, n * j);
                
                // B2. Friction
                const auto Relative_Velocity = RelVel(bodya, bodyb, r_A, r_B);
                const auto TangentVelocity   = Relative_Velocity - n * Relative_Velocity.dot(n);
                if (TangentVelocity.lengthsqr() < 1e-8f) continue;
                const auto  t  = TangentVelocity.normalize();
                const float kt = K(bodya, bodyb, r_A, r_B, t);
                const float jt = -Relative_Velocity.dot(t) / kt;
                
                const float MaxFriction = contactPoint.NormalTotal * StaticFriction;
                Vec3 NewFriction = contactPoint.FrictionTotal + t * jt;
                if (NewFriction.length() > MaxFriction) NewFriction = NewFriction.normalize() * MaxFriction;
                Apply(bodya, bodyb, r_A, r_B, NewFriction - contactPoint.FrictionTotal);
                contactPoint.FrictionTotal = NewFriction;
            }
            
            // B3. Rolling resistance
            const float RollingResistance = std::max(bodya->RollingResistance, bodyb->RollingResistance);
            if (RollingResistance <= 0.f) continue;
            
            float Pressed = 0.f;
            for (int i = 0; i < c.Data.Count; ++i) Pressed += c.Data.Points[i].NormalTotal;
            
            const Vec3  wRel = bodya->AngularVelocity - bodyb->AngularVelocity;
            const float wLen = wRel.length();
            if (wLen < 1e-6f) continue;
            const Vec3  axis = wRel / wLen;
            const float kr   = axis.dot(bodya->InvInertiaWorld(axis)) + axis.dot(bodyb->InvInertiaWorld(axis));
            if (kr <= 0.f) continue;
            
            const float MaxRolling = RollingResistance * Pressed;
            Vec3 NewRolling = c.Data.RollingTotal - axis * (wLen / kr);
            if (NewRolling.length() > MaxRolling) NewRolling = NewRolling.normalize() * MaxRolling;
            const Vec3 jr = NewRolling - c.Data.RollingTotal;
            bodya->AngularMomentum += jr;
            bodyb->AngularMomentum -= jr;
            bodya->Recalculate();
            bodyb->Recalculate();
            c.Data.RollingTotal = NewRolling;
        }        
    }    
}


Collider PhysicsEngine::MakeCollider(const RigidBody& body, const Shape& shape) const
{
    return std::visit([&body](const auto& s) -> Collider
    {
        using S = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<S, SphereShape>)
            return BoundingSphere{ body.Position, s.radius };
        else if constexpr (std::is_same_v<S, BoxShape>)
            return AABB{ body.Position - s.HalfExtents, body.Position + s.HalfExtents };
        else if constexpr (std::is_same_v<S, OBBShape>)
            return OBB{ body.Position, body.Orientation, s.HalfExtents};
        else
            return PlaneCollider{ s.Normal, s.offset };
    }, shape);
}
