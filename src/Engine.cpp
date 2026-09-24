#include "Engine.h"
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
        bodyDesc.DynamicFrictionCoeff,
        bodyDesc.StaticFrictionCoeff,
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
    ResolveCollisions();
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
void PhysicsEngine::ResolveCollisions() const
{
    if (Entries.empty() || ContactPoints.empty()) return;
    
    for (auto &c : ContactPoints)
    {
        if (c.Data.IntersectionDepth <= 0.f || c.Data.IntersectionNormal.lengthsqr() < 1e-12f) continue;
        auto [bodya, bodyb, data] = c;
        
        if (bodya->IsStatic() && bodyb->IsStatic()) continue;
        if (bodya->IsDynamic() && bodyb->IsDynamic()) continue;
        auto Dynamic_body = bodya->IsDynamic() == true? bodya : (bodyb->IsDynamic()==true? bodyb:nullptr);
        Vec3 normal = Dynamic_body == bodya? -c.Data.IntersectionNormal :  c.Data.IntersectionNormal;
        Dynamic_body->Position += normal* c.Data.IntersectionDepth;
        
        if (Dynamic_body->Velocity.dot(normal) <= 0)
        {
            Vec3 NormalVelocity = normal*(Dynamic_body->Velocity.dot(normal));
            Vec3 TangentialVelocity = Dynamic_body->Velocity - NormalVelocity;
            Vec3 DampedTangentialVelocity = TangentialVelocity*(1-Dynamic_body->StaticFrictionCoeff);
            Vec3 Reflected_NormalVel = -NormalVelocity * Dynamic_body->Restitution;
            Dynamic_body->LinearMomentum = (DampedTangentialVelocity+Reflected_NormalVel)*Dynamic_body->Mass;
            Dynamic_body->Recalculate();
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
        else
            return PlaneCollider{ s.Normal, s.offset };
    }, shape);
}
