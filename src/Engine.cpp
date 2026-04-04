#include "Engine.h"

void PhysicsEngine::addRigidBody(RigidBody* rb)
{
    if (rb != nullptr) {
        RigidObjects.push_back(rb);
    }
}

void PhysicsEngine::step(float dt)
{
    for (auto d : RigidObjects)
    {
        Vec3 gravityForce{
            gravity.x * d->Mass,
            gravity.y * d->Mass,
            gravity.z * d->Mass
        };

        d->AddForce(gravityForce);
        d->Update(dt);
    }
}

void PhysicsEngine::ResolvePlaneCollision(RigidBody* rb, BoundingSphere& sphere, PlaneCollider& plane)
{
    IntersectionData data;
    plane.IntersectWithBS(sphere, data);

    if (data.hasCollided && rb->Velocity.y < 0)
    {
        rb->Position.y -= data.IntersectionRadius;
        rb->Velocity.y = -rb->Velocity.y * rb->Restitution;
    }
}

bool PhysicsEngine::TestSphereVsSphere(BoundingSphere s1, BoundingSphere s2, IntersectionData& data)
{
    s1.intersection(s2, data);
    return data.hasCollided;
}

bool PhysicsEngine::TestAABBvsAABB(AABB a1, AABB a2, IntersectionData& data)
{
    a1.Intersect(a2, data);
    return data.hasCollided;
}

bool PhysicsEngine::TestSphereVsAABB(BoundingSphere sphere, AABB aabb, IntersectionData& data)
{
    sphere.IntersectWithAABB(aabb, data);
    return data.hasCollided;
}

bool PhysicsEngine::TestPlaneVsSphere(PlaneCollider plane, BoundingSphere sphere, IntersectionData& data)
{
    plane.IntersectWithBS(sphere, data);
    return data.hasCollided;
}

bool PhysicsEngine::TestPlaneVsAABB(PlaneCollider plane, AABB aabb, IntersectionData& data)
{
    plane.IntersectWithAABB(aabb, data);
    return data.hasCollided;
}