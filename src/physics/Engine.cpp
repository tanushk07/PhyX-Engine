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
/*
void PhysicsEngine::step(float dt)
{
    IntegrateVelocities(dt);
    DetectCollisions();
    ResolveCollisions(dt);
    IntegratePositions(dt);
}*/
void PhysicsEngine::step(float dt)
{
    IntegrateVelocities(dt);
    DetectCollisions();
    WakeTouchedBodies();
    ResolveCollisions(dt);
    IntegratePositions(dt);
    UpdateSleep(dt);
}

void PhysicsEngine::IntegrateVelocities(float dt)
{
    for (auto &[d,s] : Entries)
    {
        if (!d->CanMove()) continue;
        d->AddForce(gravity * d->Mass);
        d->IntegrateVelocity(dt);
    }
}

void PhysicsEngine::IntegratePositions(float dt)
{
    for (auto &[d,s] : Entries)
    {
        if (!d->CanMove()) continue;
        d->IntegratePosition(dt);
    }
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
    
    std::vector<Contact> previous;
    previous.swap(ContactPoints);
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
            CarryOverImpulses(previous, ContactPoints.back());
        }
    }
}

void PhysicsEngine::CarryOverImpulses(const std::vector<Contact>& previous, Contact& contact)
{
    constexpr float MatchDistance = 0.05f;
    constexpr float MinNormalDot  = 0.95f;

    for (const Contact& old : previous)
    {
        if (old.BodyA != contact.BodyA || old.BodyB != contact.BodyB) continue;
        if (old.Data.IntersectionNormal.dot(contact.Data.IntersectionNormal) < MinNormalDot) return;

        const Vec3 n = contact.Data.IntersectionNormal;
        contact.Data.RollingTotal = old.Data.RollingTotal;
        for (int i = 0; i < contact.Data.Count; ++i)
        {
            ContactPointData& point = contact.Data.Points[i];
            float bestDistSq = MatchDistance * MatchDistance;
            for (int k = 0; k < old.Data.Count; ++k)
            {
                const ContactPointData& oldPoint = old.Data.Points[k];
                const float distSq = (oldPoint.IntersectionPoint - point.IntersectionPoint).lengthsqr();
                if (distSq >= bestDistSq) continue;
                bestDistSq = distSq;
                point.NormalImpulseTotal   = oldPoint.NormalImpulseTotal;
                point.FrictionImpulseTotal = oldPoint.FrictionImpulseTotal - n * oldPoint.FrictionImpulseTotal.dot(n);
            }
        }
        return;
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
        if (A->CanMove()) { A->LinearMomentum += J; A->AngularMomentum += rA.cross(J); A->Recalculate(); }
        if (B->CanMove()) { B->LinearMomentum -= J; B->AngularMomentum -= rB.cross(J); B->Recalculate(); }
    };
    auto ApplyAngular = [](RigidBody* A, RigidBody* B, const Vec3& J)
    {
        if (A->CanMove()) { A->AngularMomentum += J; A->Recalculate(); }
        if (B->CanMove()) { B->AngularMomentum -= J; B->Recalculate(); }
    };
        
    for (auto &c : ContactPoints)
    {
        RigidBody* bodya = c.BodyA;
        RigidBody* bodyb = c.BodyB;
        if (!bodya->CanMove() && !bodyb->CanMove()) continue;
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
    
    //to be understood
    for (auto &c : ContactPoints)
    {
        RigidBody* bodya = c.BodyA;
        RigidBody* bodyb = c.BodyB;
        if (!bodya->CanMove() && !bodyb->CanMove()) continue;
        if (c.Data.IntersectionNormal.lengthsqr() < 1e-12f) continue;
        const Vec3 n = -c.Data.IntersectionNormal;
        for (int i = 0; i < c.Data.Count; ++i)
        {
            const auto &contactPoint = c.Data.Points[i];
            const auto r_A = contactPoint.IntersectionPoint - bodya->Position;
            const auto r_B = contactPoint.IntersectionPoint - bodyb->Position;
            Apply(bodya, bodyb, r_A, r_B, n * contactPoint.NormalImpulseTotal + contactPoint.FrictionImpulseTotal);
        }
        ApplyAngular(bodya, bodyb, c.Data.RollingTotal);
    }
    
    
    for (int pass = 0; pass < Iterations; ++pass)
    {
        for (auto &c : ContactPoints)
        {
            RigidBody* bodya = c.BodyA;
            RigidBody* bodyb = c.BodyB;
            if (!bodya->CanMove() && !bodyb->CanMove()) continue;
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
                const float OldTotal = contactPoint.NormalImpulseTotal;
                contactPoint.NormalImpulseTotal = std::max(0.f, OldTotal + (contactPoint.TargetVn - vn) / k);
                const float j        = contactPoint.NormalImpulseTotal - OldTotal;
                Apply(bodya, bodyb, r_A, r_B, n * j);
                
                // B2. Friction
                const auto Relative_Velocity = RelVel(bodya, bodyb, r_A, r_B);
                const auto TangentVelocity   = Relative_Velocity - n * Relative_Velocity.dot(n);
                if (TangentVelocity.lengthsqr() < 1e-8f) continue;
                const auto  t  = TangentVelocity.normalize();
                const float kt = K(bodya, bodyb, r_A, r_B, t);
                const float jt = -Relative_Velocity.dot(t) / kt;
                
                const float MaxFrictionImpulse = contactPoint.NormalImpulseTotal * StaticFriction;
                Vec3 NewFrictionImpulse = contactPoint.FrictionImpulseTotal + t * jt;
                if (NewFrictionImpulse.length() > MaxFrictionImpulse) NewFrictionImpulse = NewFrictionImpulse.normalize() * MaxFrictionImpulse;
                Apply(bodya, bodyb, r_A, r_B, NewFrictionImpulse - contactPoint.FrictionImpulseTotal);
                contactPoint.FrictionImpulseTotal = NewFrictionImpulse;
            }
            
            // B3. Rolling resistance
            const float RollingResistance = std::max(bodya->RollingResistance, bodyb->RollingResistance);
            if (RollingResistance <= 0.f) continue;
            
            float Pressed = 0.f;
            for (int i = 0; i < c.Data.Count; ++i) Pressed += c.Data.Points[i].NormalImpulseTotal;
            
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
            ApplyAngular(bodya, bodyb, jr);
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


void PhysicsEngine::WakeTouchedBodies() const
{
    for (const Contact& c : ContactPoints)
    {
        if (c.BodyA->IsDynamic() && !c.BodyA->IsAwake() && c.BodyB->MovedLastStep()) c.BodyA->WakeUp();
        if (c.BodyB->IsDynamic() && !c.BodyB->IsAwake() && c.BodyA->MovedLastStep()) c.BodyB->WakeUp();
    }
}

void PhysicsEngine::UpdateSleep(float dt)
{
    if (!SleepingEnabled) return;

    constexpr float LinearSleepTolerance  = 0.01f;     // m/s
    constexpr float AngularSleepTolerance = 0.0349f;   // rad/s (2 degrees per second)
    constexpr float TimeToSleep           = 0.5f;      // s

    for (auto &[b,s] : Entries)
    {
        if (!b->CanMove()) continue;
        const bool still = b->Velocity.lengthsqr()        < LinearSleepTolerance  * LinearSleepTolerance
                        && b->AngularVelocity.lengthsqr() < AngularSleepTolerance * AngularSleepTolerance;
        b->SleepTime = still ? b->SleepTime + dt : 0.f;
        if (b->SleepTime >= TimeToSleep) b->Sleep();
    }
}