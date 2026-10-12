#include "Visualizer.h"
#include "Engine.h"
#include "DemoState.h"
#include "Controls.h"
#include "Interaction.h"
#include "SceneDrawing.h"
#include "Hud.h"
#include "Theme.h"
#include <algorithm>

constexpr float FixedDt = 1.0f / 120.0f;

// Runs as many fixed steps as the real (or slowed-down) time that passed calls for.
static void StepPhysics(PhysicsEngine& engine, DemoState& state, const Grab& grab, Vec3 grabTarget, float& accumulator)
{
    if (!state.paused)
        accumulator += GetFrameTime() * (state.slowMotion ? 0.2f : 1.0f);
    accumulator = std::min(accumulator, 0.25f);   // catch-up limit, not one step

    int steps = 0;
    while (accumulator >= FixedDt) { ++steps; accumulator -= FixedDt; }
    steps += state.stepsQueued;
    state.stepsQueued = 0;
    for (int i = 0; i < steps; ++i)
    {
        if (grab.body) ApplyGrabForce(grab, grabTarget);   // forces are cleared every step
        engine.step(FixedDt);
    }
}

static void DrawFrame(Visualizer& vis, const Theme& theme, const PhysicsEngine& engine, const DemoState& state, const Grab& grab, Vec3 grabTarget)
{
    vis.BeginRender();
    DrawBodies(vis, engine, grab);

    vis.BeginOverlay();
    DrawOverlays(vis, engine, state, FixedDt);
    DrawGrab(vis, grab, grabTarget);
    vis.EndOverlay();
    vis.EndScene();

    DrawHud(theme, engine, state);
    vis.EndFrame();
}

int main()
{
    Visualizer visualizer(1920, 1080, "phyX");
    Theme theme;                                  // after the window: it loads fonts
    PhysicsEngine engine;
    DemoState state;
    Grab grab;
    StartChapter(0, state, engine, visualizer, grab);

    float accumulator = 0.0f;
    while (!visualizer.ShouldClose())
    {
        visualizer.UpdateCamera();
        HandleInput(state, engine, visualizer, grab);
        const Vec3 grabTarget = AlongRay(visualizer.MouseRay(), grab.distance);

        StepPhysics(engine, state, grab, grabTarget, accumulator);
        DrawFrame(visualizer, theme, engine, state, grab, grabTarget);
    }

    return 0;
}
