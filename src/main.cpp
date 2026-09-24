#include "Visualizer.h"
#include "Engine.h"
#include <algorithm>
#include <variant>

static void BuildScene(PhysicsEngine& engine)
{
    // BodyDesc: { Position, Mass, Restitution, Shape, DynamicFriction, StaticFriction, isDynamic }
    engine.CreateBody({ Vec3{-3.f, 1.f, 0.f}, 1.f, 0.5f, SphereShape{1.0f},            0.3f, 0.3f, false });
    engine.CreateBody({ Vec3{ 0.f, 10.f, 0.f}, 1.f, 0.5f, SphereShape{0.6f},            0.3f, 0.3f, true });
    engine.CreateBody({ Vec3{ 3.f, 1.f, 0.f}, 1.f, 0.5f, BoxShape{Vec3{1.f, 1.f, 1.f}}, 0.3f, 0.3f, true});
    engine.CreateBody({Vec3{0,0,0},0,1,PlaneShape{Vec3{0.f,1.f,0.f},0.f}, 0.3f, 0.3f, false}); // position should be equal to normal*offset passed in the collider
    engine.CreateBody({Vec3{-20,0,0},0,1,PlaneShape{Vec3{1.f,0.f,0.f},-20.f}, 0.3f, 0.3f, false});
    engine.CreateBody({Vec3{20,0,0},0,1,PlaneShape{Vec3{-1.f,0.f,0.f},-20.f}, 0.3f, 0.3f, false});
    engine.CreateBody({Vec3{0,20,0},0,1,PlaneShape{Vec3{0.f,-1.f,0.f},-20.f}, 0.3f, 0.3f, false});
    engine.CreateBody({Vec3{0,0,-20},0,1,PlaneShape{Vec3{0.f,0.f,1.f},-20.f}, 0.3f, 0.3f, false});
    engine.CreateBody({Vec3{0,0,20},0,1,PlaneShape{Vec3{0.f,0.f,-1.f},-20.f}, 0.3f, 0.3f, false});
}

static void DrawBody(Visualizer& vis, const BodyEntry& entry)
{
    const Vec3 p = entry.Body->Position;

    std::visit([&](const auto& s)
    {
        using S = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<S, SphereShape>)
            vis.DrawSphereDebug(p, s.radius, PURPLE);
        else if constexpr (std::is_same_v<S, BoxShape>)
            vis.DrawAABBDebug(p - s.HalfExtents, p + s.HalfExtents, LIME);
        else
            vis.DrawPlaneDebug(p, s.Normal, 800.0f, BLACK);
    }, entry.shape);
}

int main()
{
    Visualizer visualizer(1280, 720, "phyX");

    PhysicsEngine engine;
    BuildScene(engine);

    const float FixedDt = 1.0f / 120.0f;
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