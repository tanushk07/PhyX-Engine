#pragma once
#include <vector>
#include "Vector.h"
#include "core/RigidBody.h"
#include "BoundingSphere.h"
#include "AABB.h"
#include "PlaneCollider.h"
#include "IntersectionData.h"

class PhysicsEngine
{
private:
    std::vector<RigidBody*> RigidObjects;
    Vec3 gravity{0, -9.81f, 0};

public:
    void step(float dt);
    void addRigidBody(RigidBody* rb);

    void ResolvePlaneCollision(RigidBody* rb, BoundingSphere& sphere, PlaneCollider& plane);

    bool TestSphereVsSphere(BoundingSphere s1, BoundingSphere s2, IntersectionData& data);
    bool TestAABBvsAABB(AABB a1, AABB a2, IntersectionData& data);
    bool TestSphereVsAABB(BoundingSphere sphere, AABB aabb, IntersectionData& data);
    bool TestPlaneVsSphere(PlaneCollider plane, BoundingSphere sphere, IntersectionData& data);
    bool TestPlaneVsAABB(PlaneCollider plane, AABB aabb, IntersectionData& data);
};