#pragma once
#include <memory>
#include <vector>
#include "Vector.h"
#include "core/RigidBody.h"
#include "BoundingSphere.h"
#include "AABB.h"
#include "PlaneCollider.h"
#include "IntersectionData.h"
#include "Shape.h"
#include "BodyDesc.h"
#include "Contact.h"
template <typename> inline constexpr bool always_false = false;
using Collider = std::variant<BoundingSphere, AABB, PlaneCollider>;

struct BodyEntry
{
    std::unique_ptr<RigidBody> Body;   
    Shape                      shape;  
};

class PhysicsEngine
{
private:
    std::vector<Contact> ContactPoints ;
    Vec3 gravity{0.f, -9.8f, 0};
    static Vec3 GetInertia(const Shape &shape, float mass);
    std::vector<BodyEntry> Entries;
public:
    void step(float dt);

    RigidBody& CreateBody(const BodyDesc& bodyDesc);
    std::vector<Contact> GetContactPoints(){return ContactPoints;}
    void IntegrateForces(float dt);
    void DetectCollisions();
    Collider MakeCollider(const RigidBody& body, const Shape& shape) const;
    void ResolveCollisions() const;
    const std::vector<BodyEntry>& GetEntries() const { return Entries; }
    
    template <typename A, typename B>
    static void Collide(const A& a, const B& b, IntersectionData& data)
    {
        if constexpr (requires { a.Intersect(b, data); })
            a.Intersect(b, data);
        else if constexpr (requires { b.Intersect(a, data); }) {
            b.Intersect(a, data);
            data.IntersectionNormal = -data.IntersectionNormal;
        } else static_assert(always_false<A>, "No Intersect() for this collider pair");
    }
};

inline Vec3 PhysicsEngine::GetInertia(const Shape &shape, float mass) 
{
    return std::visit([mass](const auto& actualshape)
    {
        return actualshape.CalculateInertia(mass);
    },shape);
}