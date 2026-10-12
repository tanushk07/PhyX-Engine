#include "Chapters.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

// ---- small helpers to build scenes ---------------------------------------------------------

// BodyDesc: { Position, Mass, Restitution, Shape, Friction, RollingResistance, isDynamic }
static void Floor(PhysicsEngine& e, float friction = 0.5f, float restitution = 1.0f)
{
    e.CreateBody({ Vec3{0, 0, 0}, 0.f, restitution, PlaneShape{Vec3{0, 1, 0}, 0.f}, friction, 0.f, false });
}

static void Walls(PhysicsEngine& e, float half)   // invisible walls that keep things on screen
{
    e.CreateBody({ Vec3{-half, 0, 0}, 0.f, 0.5f, PlaneShape{Vec3{ 1, 0, 0}, -half}, 0.5f, 0.f, false });
    e.CreateBody({ Vec3{ half, 0, 0}, 0.f, 0.5f, PlaneShape{Vec3{-1, 0, 0}, -half}, 0.5f, 0.f, false });
    e.CreateBody({ Vec3{0, 0, -half}, 0.f, 0.5f, PlaneShape{Vec3{0, 0,  1}, -half}, 0.5f, 0.f, false });
    e.CreateBody({ Vec3{0, 0,  half}, 0.f, 0.5f, PlaneShape{Vec3{0, 0, -1}, -half}, 0.5f, 0.f, false });
}

static RigidBody& Ball(PhysicsEngine& e, Vec3 p, float radius, float restitution, float friction, float rolling, float mass = 1.f)
{
    return e.CreateBody({ p, mass, restitution, SphereShape{radius}, friction, rolling, true });
}

static RigidBody& Box(PhysicsEngine& e, Vec3 p, Vec3 half, Quat q = Quat(), float mass = 1.f)
{
    RigidBody& b = e.CreateBody({ p, mass, 0.2f, OBBShape{half}, 0.6f, 0.f, true });
    b.Orientation = q;
    b.Recalculate();
    return b;
}

static void Push(RigidBody& b, Vec3 velocity, Vec3 angularMomentum = Vec3{})
{
    b.LinearMomentum  = velocity * b.Mass;
    b.AngularMomentum = angularMomentum;
    b.Recalculate();
}

static void Pyramid(PhysicsEngine& e, Vec3 base, int rows)
{
    for (int row = 0; row < rows; ++row)
        for (int i = 0; i < rows - row; ++i)
            Box(e, base + Vec3{(i - (rows - row - 1) * 0.5f) * 1.05f, 0.5f + row * 1.0f, 0}, Vec3{0.5f, 0.5f, 0.5f});
}

static std::string Format(const char* fmt, double a = 0, double b = 0, double c = 0, double d = 0)
{
    char buffer[160];
    std::snprintf(buffer, sizeof(buffer), fmt, a, b, c, d);
    return buffer;
}

// ---- 1. Bouncing -----------------------------------------------------------------------------

static RigidBody* g_bounce[3];
static float      g_bounceApex[3], g_bouncePrevVy[3];
static int        g_bounceCount[3];
static const float BounceE[3] = { 0.2f, 0.5f, 0.9f };
static const float BounceDrop = 4.0f;

static void BuildBounce(PhysicsEngine& e)
{
    Floor(e, 0.3f, 1.0f);                              // restitution is min(ball, floor) = the ball's e
    for (int i = 0; i < 3; ++i)
    {
        g_bounce[i] = &Ball(e, Vec3{-3.f + 3.f * i, BounceDrop + 0.5f, 0}, 0.5f, BounceE[i], 0.3f, 0.f);
        g_bounceApex[i] = 0.f; g_bouncePrevVy[i] = 0.f; g_bounceCount[i] = 0;
    }
}

static void ReadBounce(const PhysicsEngine&, std::vector<std::string>& out)
{
    for (int i = 0; i < 3; ++i)
    {
        const RigidBody& b = *g_bounce[i];
        if (g_bouncePrevVy[i] < 0.f && b.Velocity.y > 0.f) ++g_bounceCount[i];     // it just bounced
        if (g_bounceCount[i] == 1) g_bounceApex[i] = std::max(g_bounceApex[i], b.Position.y - 0.5f);
        g_bouncePrevVy[i] = b.Velocity.y;
        const float theory = BounceE[i] * BounceE[i] * BounceDrop;
        out.push_back(Format("e = %.1f:  first bounce rose %.2f m   (theory e^2 x 4 m = %.2f m)", BounceE[i], g_bounceApex[i], theory));
    }
}

// ---- 2. Contact point velocity ---------------------------------------------------------------

static void BuildSpin(PhysicsEngine& e)
{
    Floor(e, 0.5f, 1.0f);
    RigidBody& box = Box(e, Vec3{-2, 1.6f, 0}, Vec3{0.5f, 0.5f, 0.5f}, Quat::fromAxisAngle(0.3f, Vec3{0, 0, 1}));
    box.Restitution = 0.5f;
    Push(box, Vec3{2, 0, 0}, Vec3{0, 0, -1.5f});      // moving right, spinning fast
}

// ---- 3. Friction makes rolling ---------------------------------------------------------------

static RigidBody* g_roller = nullptr;

static void BuildRolling(PhysicsEngine& e)
{
    // heavy (5 kg) so the force arrows are long enough to see: friction = 0.2 x 49 N = 9.8 N, ~50 cm
    Floor(e, 0.2f, 1.0f);
    g_roller = &Ball(e, Vec3{-5, 1, 0}, 1.f, 0.f, 0.2f, 0.f, 5.f);
    Push(*g_roller, Vec3{4, 0, 0});                   // sliding, no spin
}

static void ReadRolling(const PhysicsEngine&, std::vector<std::string>& out)
{
    const RigidBody& b = *g_roller;
    const float speed = Vec3(b.Velocity.x, 0, b.Velocity.z).length();
    const float spinSpeed = b.AngularVelocity.length() * 1.f;    // spin x radius
    out.push_back(Format("speed %.3f m/s     spin x radius %.3f m/s", speed, spinSpeed));
    out.push_back(std::fabs(speed - spinSpeed) < 0.01f ? "ROLLING: the contact point no longer slides, so friction stops"
                                                       : "SLIDING: friction slows the ball and spins it up");
    out.push_back(Format("theory: rolling at 5/7 x 4 = %.3f m/s", 4.0 * 5.0 / 7.0));
}

// ---- 4. Rolling resistance -------------------------------------------------------------------

static RigidBody* g_freeBall = nullptr;
static RigidBody* g_resistBall = nullptr;

static void BuildResistance(PhysicsEngine& e)
{
    Floor(e, 0.3f, 1.0f);
    g_freeBall   = &Ball(e, Vec3{-10, 0.8f, -2}, 0.8f, 0.f, 0.3f, 0.00f);
    g_resistBall = &Ball(e, Vec3{-10, 0.8f,  2}, 0.8f, 0.f, 0.3f, 0.05f);
    Push(*g_freeBall,   Vec3{3, 0, 0});
    Push(*g_resistBall, Vec3{3, 0, 0});
}

static void ReadResistance(const PhysicsEngine&, std::vector<std::string>& out)
{
    out.push_back(Format("far ball  (no rolling resistance):   %.2f m/s", Vec3(g_freeBall->Velocity.x, 0, g_freeBall->Velocity.z).length()));
    out.push_back(Format("near ball (rolling resistance 0.05): %.2f m/s", Vec3(g_resistBall->Velocity.x, 0, g_resistBall->Velocity.z).length()));
}

// ---- 5. Contact manifolds --------------------------------------------------------------------

static void BuildManifold(PhysicsEngine& e)
{
    Floor(e, 0.5f, 1.0f);
    Box(e, Vec3{0, 1.8f, 0}, Vec3{1.0f, 0.5f, 0.75f}, Quat::fromAxisAngle(0.6f, Vec3{1, 0, 0.5f}));
}

static void ReadManifold(const PhysicsEngine& engine, std::vector<std::string>& out)
{
    int points = 0;
    for (const Contact& c : engine.GetContactPoints()) points = std::max(points, c.Data.Count);
    out.push_back(Format("contact points between box and floor: %.0f", points));
}

// ---- 6. Stacking -----------------------------------------------------------------------------

static void BuildStack(PhysicsEngine& e)
{
    Floor(e, 0.6f, 1.0f);
    for (int i = 0; i < 10; ++i)
        Box(e, Vec3{-3.f + 0.02f * (i % 3), 0.5f + i * 1.0f, 0}, Vec3{0.5f, 0.5f, 0.5f}, Quat::fromAxisAngle(0.05f * i, Vec3{0, 1, 0}));
    Pyramid(e, Vec3{3, 0, 0}, 4);
}

// ---- 7. Sleeping -----------------------------------------------------------------------------

static void BuildSleeping(PhysicsEngine& e)
{
    Floor(e, 0.6f, 1.0f);
    Pyramid(e, Vec3{0, 0, 0}, 5);
}

// ---- 8. Free play ----------------------------------------------------------------------------

static void BuildFreePlay(PhysicsEngine& e)
{
    Floor(e, 0.5f, 1.0f);
    Walls(e, 15.f);
    Pyramid(e, Vec3{-4, 0, -3}, 3);
    for (int i = 0; i < 4; ++i)
        Box(e, Vec3{3.f + 1.2f * i, 0.5f, 2}, Vec3{0.5f, 0.5f, 0.5f}, Quat::fromAxisAngle(0.4f * i, Vec3{0, 1, 0}));
    Box(e, Vec3{6, 1.5f, -3}, Vec3{0.4f, 1.5f, 0.4f});                         // a tall post to knock over
    Ball(e, Vec3{-1, 0.6f, 4}, 0.6f, 0.4f, 0.4f, 0.02f);
    Ball(e, Vec3{ 1, 1.0f, 4}, 1.0f, 0.4f, 0.4f, 0.03f, 3.f);
}

// ---- the list --------------------------------------------------------------------------------

const std::vector<Chapter>& GetChapters()
{
    static const std::vector<Chapter> chapters = {
        { "Bouncing: restitution",
          { "Three balls dropped from 4 m with restitution e = 0.2, 0.5 and 0.9.",
            "Newton's law of restitution: speed after a bounce = e x speed before,",
            "so the first bounce reaches e^2 of the drop height." },
          BuildBounce, false, false, true, false, false, false,
          Vec3{0, 4, 14}, Vec3{0, 2, 0}, ReadBounce },

        { "The contact point moves differently from the centre",
          { "A spinning box lands. The solver needs the velocity of the touching point P,",
            "not of the centre:  v_P = v + w x r   (green arrows at the contacts).",
            "Slow motion is on. Pause with Space, then tap or hold N to step through the impact." },
          BuildSpin, true, false, true, true, true, true,
          Vec3{0, 3, 9}, Vec3{0, 1, 0}, nullptr },

        { "Friction turns sliding into rolling",
          { "Starts sliding at 4 m/s with no spin, so its bottom point slides too (green).",
            "Friction (light blue) opposes that slide: it slows the ball and spins it up.",
            "At speed = spin x radius the bottom point stops, and so does friction." },
          BuildRolling, true, true, true, true, false, true,
          Vec3{-1, 3, 9}, Vec3{-1.5f, 0.8f, 0}, ReadRolling },     // close up: it slides from x = -5 to about -3

        { "Rolling resistance",
          { "Both balls start at 3 m/s. A rolling ball's contact point doesn't slide,",
            "so friction can't stop it: the far ball rolls on forever.",
            "The near ball also has rolling resistance, which slows its spin until it stops." },
          BuildResistance, false, false, true, false, false, false,
          Vec3{0, 6, 18}, Vec3{0, 0, 0}, ReadResistance },

        { "Contact manifolds",
          { "A tilted box lands on a corner (1 point), tips onto an edge (2), then a face (4).",
            "One normal for the pair, its own depth for each point.",
            "Gold arrows: the support force at each point. Slow motion is on." },
          BuildManifold, true, true, false, false, false, true,
          Vec3{3, 4, 9}, Vec3{0, 0.5f, 0}, ReadManifold },

        { "Stacking: sequential impulses + warm starting",
          { "Every step the solver passes over all contacts 10 times, starting from",
            "last step's answer (warm starting), so tall stacks stand still.",
            "Gold arrows: each contact carries the weight of everything above it." },
          BuildStack, false, true, false, false, false, false,
          Vec3{0, 6, 20}, Vec3{0, 4, 0}, nullptr },

        { "Sleeping",
          { "A body that stays still for 0.5 s falls asleep (pale) and stops being simulated.",
            "Press B to throw a ball: what it hits wakes up, settles, and sleeps again.",
            "Watch 'awake' in the counters at the top right." },
          BuildSleeping, false, false, false, false, false, false,
          Vec3{0, 5, 16}, Vec3{0, 2, 0}, nullptr },

        { "Free play",
          { "Left mouse on a body: grab it and swing it around. Let go to throw it.",
            "B: throw a ball from the camera    K: drop a box where you point",
            "Every overlay key works here too." },
          BuildFreePlay, false, false, false, false, false, false,
          Vec3{0, 9, 22}, Vec3{0, 1, 0}, nullptr },
    };
    return chapters;
}