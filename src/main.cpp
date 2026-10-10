#include "Visualizer.h"
#include "Engine.h"
#include <algorithm>
#include <variant>

static void BuildScene(PhysicsEngine& engine)
{
    // BodyDesc: { Position, Mass, Restitution, Shape, DynamicFriction, StaticFriction, RollingResistance, isDynamic }
    engine.CreateBody({ Vec3{-3.f, 1.f, 0.f}, 1.f, 0.5f, SphereShape{1.0f},             0.2f, .02f,true }); // RollingResistance should be much under Radius*StaticFriction
    engine.CreateBody({ Vec3{ 0.f, 10.f, 0.f}, 1.f, 0.5f, SphereShape{2.f},            0.3f, .03f, true });
    engine.CreateBody({ Vec3{ 6.f, 10.f, 0.f}, 1000.f, 0.5f, SphereShape{5.f},             0.3f, .015f, true });
    engine.CreateBody({ Vec3{ 8.f, 2.f, 0.f}, 1.f, 0.5f, OBBShape{Vec3{1.f, 1.f, 1.f}},  0.3f, 0.f, true});
    engine.CreateBody({ Vec3{ 8.f, 4.f, 0.f}, 1.f, 0.5f, OBBShape{Vec3{1.f, 1.f, 1.f}},  0.3f,0.f, true});
    engine.CreateBody({Vec3{0,0,0},0.f,1,PlaneShape{Vec3{0.f,1.f,0.f},0.f},  1.f, 0.f, false}); // position should be equal to normal*offset passed in the collider
    engine.CreateBody({Vec3{-20,0,0},0.f,1,PlaneShape{Vec3{1.f,0.f,0.f},-20.f},  1.f, 0.f, false});
    engine.CreateBody({Vec3{20,0,0},0.f,1,PlaneShape{Vec3{-1.f,0.f,0.f},-20.f}, 1.f, 0.f, false});
    engine.CreateBody({Vec3{0,20,0},0.f,1,PlaneShape{Vec3{0.f,-1.f,0.f},-20.f},  1.f, 0.f,false});
    engine.CreateBody({Vec3{0,0,-20},0.f,1,PlaneShape{Vec3{0.f,0.f,1.f},-20.f}, 1.f, 0.f,false});
    engine.CreateBody({Vec3{0,0,20},0.f,1,PlaneShape{Vec3{0.f,0.f,-1.f},-20.f},  1.f, 0.f,false});
}

static void DrawBody(Visualizer& vis, const BodyEntry& entry)
{
    const RigidBody& body = *entry.Body;
    const Vec3 p = body.Position;

    std::visit([&](const auto& s)
    {
        using S = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<S, SphereShape>)
            vis.DrawSphereDebug(p, s.radius, body.IsAwake() ? PURPLE : GRAY);
        else if constexpr (std::is_same_v<S, BoxShape> || std::is_same_v<S, OBBShape>)
            vis.DrawBoxDebug(p, s.HalfExtents, body.Orientation, body.IsAwake() ? LIME : GRAY);
        else if constexpr (std::is_same_v<S, PlaneShape>)
            vis.DrawPlaneDebug(p, s.Normal, 40.0f, Fade(DARKGRAY, 0.25f));
        else
            static_assert(always_false<S>, "DrawBody: unhandled shape");
    }, entry.shape);

    if (body.IsDynamic())
        vis.DrawBodyAxes(p, body.Orientation, 10.5f);
}
int main()
{
    Visualizer visualizer(1920, 1080, "phyX");
    PhysicsEngine engine;
    BuildScene(engine);

    constexpr float FixedDt = 1.0f / 120.0f;
    float accumulator = 0.0f;

    while (!visualizer.ShouldClose())
    {
        visualizer.UpdateCamera();

        accumulator += GetFrameTime();
        accumulator = std::min(accumulator, 0.25f);   // catch-up limit, not one step
        
        while (accumulator >= FixedDt)
        {
            engine.step(FixedDt);
            accumulator -= FixedDt;
        }

        visualizer.BeginRender();
        
        for (const auto& entry : engine.GetEntries())
            DrawBody(visualizer, entry);
        visualizer.EndRender();
    }

    return 0;
}