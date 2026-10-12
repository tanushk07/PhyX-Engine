#include "SceneDrawing.h"
#include "Interaction.h"
#include <algorithm>
#include <cmath>
#include <variant>

// ---- bodies ----------------------------------------------------------------------------------

// A calm palette; each body gets the next colour. A held body is bright yellow; a sleeping one
// is half greyed here and then washed out towards white by the shader (SleepWashOut).
static Color BodyColor(int index, bool awake, bool grabbed)
{
    static const Color Palette[] = {
        { 235, 137,  52, 255 },   // orange
        {  64, 156, 222, 255 },   // blue
        {  86, 196, 120, 255 },   // green
        { 226,  88,  88, 255 },   // red
        { 162, 110, 214, 255 },   // purple
        {  52, 190, 180, 255 },   // teal
    };
    if (grabbed) return Color{ 255, 214, 64, 255 };
    const Color c = Palette[index % (sizeof(Palette) / sizeof(Palette[0]))];
    if (awake) return c;
    const Color grey{ 200, 203, 210, 255 };
    return Color{ (unsigned char)((c.r + grey.r) / 2), (unsigned char)((c.g + grey.g) / 2),
                  (unsigned char)((c.b + grey.b) / 2), 255 };
}

// How far a box reaches down from its centre: the same sum as OBB::ProjectedRadius, along world up.
static float DownwardReach(const Quat& orientation, Vec3 halfExtents)
{
    const Vec3 up{0, 1, 0};
    return halfExtents.x * std::fabs(orientation.rotate(Vec3{1, 0, 0}).dot(up))
         + halfExtents.y * std::fabs(orientation.rotate(Vec3{0, 1, 0}).dot(up))
         + halfExtents.z * std::fabs(orientation.rotate(Vec3{0, 0, 1}).dot(up));
}

constexpr float SleepWashOut = 0.6f;    // how far a sleeping body fades towards white

// Lit look: textured, shaded shapes with a soft shadow underneath.
static void DrawBodyLit(Visualizer& vis, const BodyEntry& entry, int index, bool grabbed)
{
    const RigidBody& body = *entry.Body;
    const Color color = BodyColor(index, body.IsAwake(), grabbed);
    const float washOut = body.IsAwake() || grabbed ? 0.0f : SleepWashOut;

    std::visit([&](const auto& s)
    {
        using S = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<S, SphereShape>)
        {
            vis.DrawShadow(body.Position, s.radius, body.Position.y - s.radius);
            vis.DrawSphereLit(body.Position, s.radius, body.Orientation, color, washOut);
        }
        else if constexpr (std::is_same_v<S, BoxShape> || std::is_same_v<S, OBBShape>)
        {
            const float reach = DownwardReach(body.Orientation, s.HalfExtents);
            vis.DrawShadow(body.Position, std::max(s.HalfExtents.x, s.HalfExtents.z) * 1.3f, body.Position.y - reach);
            vis.DrawBoxLit(body.Position, s.HalfExtents, body.Orientation, color, washOut);
        }
        else if constexpr (std::is_same_v<S, PlaneShape>)
        {
            if (s.Normal.y > 0.99f) vis.DrawFloor(300.0f);       // walls stay invisible
        }
        else
            static_assert(always_false<S>, "DrawBodyLit: unhandled shape");
    }, entry.shape);
}

// Wireframe look: see-through shapes, green boxes, purple balls, a pale tint of the same when asleep.
static void DrawBodyWire(Visualizer& vis, const BodyEntry& entry, bool grabbed)
{
    const RigidBody& body = *entry.Body;
    const Vec3 p = body.Position;

    std::visit([&](const auto& s)
    {
        using S = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<S, SphereShape>)
            vis.DrawSphereDebug(p, s.radius, grabbed ? ORANGE : (body.IsAwake() ? PURPLE : ColorBrightness(PURPLE, 0.45f)));
        else if constexpr (std::is_same_v<S, BoxShape> || std::is_same_v<S, OBBShape>)
            vis.DrawBoxDebug(p, s.HalfExtents, body.Orientation, grabbed ? ORANGE : (body.IsAwake() ? LIME : ColorBrightness(LIME, 0.45f)));
        else if constexpr (std::is_same_v<S, PlaneShape>)
        {
            if (s.Normal.y > 0.99f) vis.DrawPlaneDebug(p, s.Normal, 40.0f, Fade(DARKGRAY, 0.25f));   // walls stay invisible
        }
        else
            static_assert(always_false<S>, "DrawBodyWire: unhandled shape");
    }, entry.shape);
}

void DrawBodies(Visualizer& vis, const PhysicsEngine& engine, const Grab& grab)
{
    const auto& entries = engine.GetEntries();
    for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    {
        const bool grabbed = entries[i].Body.get() == grab.body;
        if (vis.IsLitLook()) DrawBodyLit(vis, entries[i], i, grabbed);
        else                 DrawBodyWire(vis, entries[i], grabbed);
    }
}

// ---- overlays --------------------------------------------------------------------------------

constexpr float MetresPerNewton = 0.05f;    // force arrows: 1 N = 5 cm
constexpr float MaxArrow        = 3.0f;     // very large forces are capped at 3 m

// C, F, P: drawn at every contact point.
static void DrawContactOverlays(Visualizer& vis, const PhysicsEngine& engine, const DemoState& state, float dt)
{
    for (const Contact& c : engine.GetContactPoints())
    {
        // Draw the push on the body being held up: the one that can move, or the higher one if both can.
        // The solver pushes B along +normal and A along -normal.
        const bool onB = c.BodyB->IsDynamic() && (!c.BodyA->IsDynamic() || c.BodyB->Position.y > c.BodyA->Position.y);
        const RigidBody& body = onB ? *c.BodyB : *c.BodyA;
        const Vec3 pushDir = onB ? c.Data.IntersectionNormal : -c.Data.IntersectionNormal;
        const float sign = onB ? -1.f : 1.f;    // friction total is stored as the push on A

        for (int i = 0; i < c.Data.Count; ++i)
        {
            const ContactPointData& p = c.Data.Points[i];
            if (state.showContacts)
            {
                vis.DrawPoint(p.IntersectionPoint, 0.07f, RED);
                vis.DrawArrow(p.IntersectionPoint, p.IntersectionPoint + pushDir * 0.6f, ORANGE);
            }
            if (state.showForces)
            {
                // impulse over one step = force (newtons)
                const float normalForce = std::min(p.NormalImpulseTotal / dt * MetresPerNewton, MaxArrow);
                vis.DrawArrow(p.IntersectionPoint, p.IntersectionPoint + pushDir * normalForce, GOLD);

                const Vec3 friction = p.FrictionImpulseTotal * (sign / dt * MetresPerNewton);
                const float frictionLength = std::min(friction.length(), MaxArrow);
                if (frictionLength > 0.01f)
                    vis.DrawArrow(p.IntersectionPoint, p.IntersectionPoint + friction.normalize() * frictionLength, SKYBLUE);
            }
            if (state.showPointVelocity)
            {
                // the velocity of the touching point itself: v + w x r
                const Vec3 r = p.IntersectionPoint - body.Position;
                const Vec3 vP = body.Velocity + body.AngularVelocity.cross(r);
                vis.DrawArrow(p.IntersectionPoint, p.IntersectionPoint + vP * 0.3f, GREEN);
            }
        }
    }
}

// V, X: drawn at every moving body's centre.
static void DrawBodyOverlays(Visualizer& vis, const PhysicsEngine& engine, const DemoState& state)
{
    for (const auto& entry : engine.GetEntries())
    {
        const RigidBody& b = *entry.Body;
        if (!b.IsDynamic()) continue;
        if (state.showVelocities)
        {
            vis.DrawArrow(b.Position, b.Position + b.Velocity * 0.3f, BLUE);              // 1 m/s = 30 cm
            vis.DrawArrow(b.Position, b.Position + b.AngularVelocity * 0.3f, MAGENTA);    // along the spin axis
        }
        if (state.showAxes)
        {
            vis.DrawBodyAxes(b.Position, b.Orientation, 1.5f);
            vis.DrawPoint(b.Position, 0.08f, BLACK);                                       // centre of mass
        }
    }
}

void DrawOverlays(Visualizer& vis, const PhysicsEngine& engine, const DemoState& state, float dt)
{
    DrawContactOverlays(vis, engine, state, dt);
    DrawBodyOverlays(vis, engine, state);
}

void DrawGrab(Visualizer& vis, const Grab& grab, Vec3 target)
{
    if (!grab.body) return;
    vis.DrawLine(GrabAnchor(grab), target, ORANGE);
    vis.DrawPoint(target, 0.08f, ORANGE);
}
