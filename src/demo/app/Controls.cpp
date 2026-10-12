#include "Controls.h"
#include "Chapters.h"
#include "Interaction.h"

void StartChapter(int index, DemoState& state, PhysicsEngine& engine, Visualizer& vis, Grab& grab)
{
    const auto& chapters = GetChapters();
    state.chapter = (index + static_cast<int>(chapters.size())) % static_cast<int>(chapters.size());
    const Chapter& ch = chapters[state.chapter];

    engine = PhysicsEngine{};
    ch.Build(engine);
    grab = Grab{};

    state.paused            = false;
    state.showContacts      = ch.Contacts;
    state.showForces        = ch.Forces;
    state.showVelocities    = ch.Velocities;
    state.showPointVelocity = ch.PointVelocities;
    state.showAxes          = ch.Axes;
    state.slowMotion        = ch.SlowMotion;
    vis.SetCamera(ch.CameraPosition, ch.CameraTarget);
}

// [<-] [->] [1]-[9] [R]
static void HandleChapterKeys(DemoState& state, PhysicsEngine& engine, Visualizer& vis, Grab& grab)
{
    const int count = static_cast<int>(GetChapters().size());
    if (IsKeyPressed(KEY_RIGHT)) StartChapter(state.chapter + 1, state, engine, vis, grab);
    if (IsKeyPressed(KEY_LEFT))  StartChapter(state.chapter - 1, state, engine, vis, grab);
    for (int i = 0; i < count && i < 9; ++i)
        if (IsKeyPressed(KEY_ONE + i)) StartChapter(i, state, engine, vis, grab);
    if (IsKeyPressed(KEY_R)) StartChapter(state.chapter, state, engine, vis, grab);
}

// [N] while paused: a tap is one step; held down, it keeps stepping, like a key repeating in a text box.
static void HandleStepKey(DemoState& state)
{
    constexpr float RepeatDelay = 0.3f;              // hold this long before it starts repeating
    constexpr float RepeatRate  = 30.0f;             // then this many steps per second (1/4 of real time)
    static float held = 0.0f;                        // how long N has been down

    if (!state.paused || !IsKeyDown(KEY_N)) { held = 0.0f; return; }
    if (IsKeyPressed(KEY_N)) { ++state.stepsQueued; return; }

    // count the repeats whose time came between last frame and this one
    const auto repeats = [&](float t) { return t < RepeatDelay ? 0 : static_cast<int>((t - RepeatDelay) * RepeatRate) + 1; };
    const float before = held;
    held += GetFrameTime();
    state.stepsQueued += repeats(held) - repeats(before);
}

// Keys that only flip a switch.
static void HandleToggleKeys(DemoState& state, Visualizer& vis)
{
    if (IsKeyPressed(KEY_SPACE)) state.paused = !state.paused;
    if (IsKeyPressed(KEY_T)) state.slowMotion = !state.slowMotion;
    if (IsKeyPressed(KEY_C)) state.showContacts = !state.showContacts;
    if (IsKeyPressed(KEY_F)) state.showForces = !state.showForces;
    if (IsKeyPressed(KEY_V)) state.showVelocities = !state.showVelocities;
    if (IsKeyPressed(KEY_P)) state.showPointVelocity = !state.showPointVelocity;
    if (IsKeyPressed(KEY_X)) state.showAxes = !state.showAxes;
    if (IsKeyPressed(KEY_H)) state.showHelp = !state.showHelp;
    if (IsKeyPressed(KEY_G)) vis.SetLitLook(!vis.IsLitLook());
}

// Things you do in the world with the mouse ray: throw, drop, grab.
static void HandleWorldKeys(PhysicsEngine& engine, const Visualizer& vis, Grab& grab)
{
    const Ray ray = vis.MouseRay();
    if (IsKeyPressed(KEY_B)) ThrowBall(engine, ray);
    if (IsKeyPressed(KEY_K)) DropBox(engine, ray);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))  StartGrab(grab, engine, ray);
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) grab = Grab{};
}

void HandleInput(DemoState& state, PhysicsEngine& engine, Visualizer& vis, Grab& grab)
{
    HandleChapterKeys(state, engine, vis, grab);
    HandleToggleKeys(state, vis);
    HandleStepKey(state);
    HandleWorldKeys(engine, vis, grab);
}
