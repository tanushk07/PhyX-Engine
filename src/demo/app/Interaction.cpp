#include "Interaction.h"
#include "RaylibConvert.h"
#include <cmath>
#include <variant>

RigidBody* PickBody(const PhysicsEngine& engine, Ray ray, Vec3& hitPoint, float& distance)
{
    RigidBody* best = nullptr;
    distance = 1e30f;
    for (const auto& entry : engine.GetEntries())
    {
        RigidBody* b = entry.Body.get();
        if (!b->IsDynamic()) continue;
        RayCollision hit{};
        std::visit([&](const auto& s)
        {
            using S = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<S, SphereShape>)
                hit = GetRayCollisionSphere(ray, ToRaylib(b->Position), s.radius);
            else if constexpr (std::is_same_v<S, BoxShape> || std::is_same_v<S, OBBShape>)
            {
                // turn the ray into the box's own space, where the box is an axis-aligned box
                const Quat inv = b->Orientation.conjugate();
                const Ray local{ ToRaylib(inv.rotate(ToVec3(ray.position) - b->Position)),
                                 ToRaylib(inv.rotate(ToVec3(ray.direction))) };
                hit = GetRayCollisionBox(local, BoundingBox{ ToRaylib(-s.HalfExtents), ToRaylib(s.HalfExtents) });
                if (hit.hit)
                    hit.point = ToRaylib(b->Position + b->Orientation.rotate(ToVec3(hit.point)));
            }
        }, entry.shape);

        if (hit.hit && hit.distance < distance)
        {
            distance = hit.distance;
            hitPoint = ToVec3(hit.point);
            best = b;
        }
    }
    return best;
}

Vec3 AlongRay(Ray ray, float distance)
{
    return ToVec3(ray.position) + ToVec3(ray.direction) * distance;
}

void StartGrab(Grab& grab, const PhysicsEngine& engine, Ray ray)
{
    Vec3 hit; float distance;
    if (RigidBody* body = PickBody(engine, ray, hit, distance))
    {
        grab.body        = body;
        grab.localAnchor = body->Orientation.conjugate().rotate(hit - body->Position);
        grab.distance    = distance;
        body->WakeUp();
    }
}

Vec3 GrabAnchor(const Grab& grab)
{
    return grab.body->Position + grab.body->Orientation.rotate(grab.localAnchor);
}

// Stiff enough to lift the body, damped so it doesn't wobble.
void ApplyGrabForce(const Grab& grab, Vec3 target)
{
    RigidBody& b = *grab.body;
    const Vec3 anchor = GrabAnchor(grab);
    const Vec3 anchorVelocity = b.Velocity + b.AngularVelocity.cross(anchor - b.Position);
    const float stiffness = 150.f * b.Mass;
    const float damping   = 2.f * std::sqrt(stiffness * b.Mass) * 0.7f;
    b.AddForceAtPoint((target - anchor) * stiffness - anchorVelocity * damping, anchor);
}

void ThrowBall(PhysicsEngine& engine, Ray ray)
{
    RigidBody& ball = engine.CreateBody({ AlongRay(ray, 1.5f), 1.f, 0.3f, SphereShape{0.4f}, 0.5f, 0.02f, true });
    ball.LinearMomentum = ToVec3(ray.direction) * 18.f * ball.Mass;   // 18 m/s along the view
    ball.Recalculate();
}

void DropBox(PhysicsEngine& engine, Ray ray)
{
    if (ray.direction.y >= -0.01f) return;                 // pointing at the sky: nowhere to drop
    const float t = -ray.position.y / ray.direction.y;     // where the ray meets the floor (y = 0)
    const Vec3 spot = AlongRay(ray, t);
    RigidBody& box = engine.CreateBody({ spot + Vec3{0, 4, 0}, 1.f, 0.2f, OBBShape{Vec3{0.5f, 0.5f, 0.5f}}, 0.6f, 0.f, true });
    box.Orientation = Quat::fromAxisAngle(0.5f, Vec3{0.3f, 1, 0.2f});
    box.Recalculate();
}
