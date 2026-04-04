#include "BoundingSphere.h"
#include "AABB.h"
#include "PlaneCollider.h"
#include "IntersectionData.h"
#include "Visualizer.h"
#include "core/RigidBody.h"
#include "Engine.h"
#include <cmath>

int main()
{
    Visualizer visualizer(1920, 1080, "phyX");

    PhysicsEngine engine;
    float bounceTimer = 0.0f;

    // Rigidbody sphere (gravity + bounce demo) — center of the scene
    BoundingSphere physicsSphere{Vec3{0, 8, 0}, 0.6f};
    RigidBody physicsBody{physicsSphere.getPosition(), 2.0f, 0.82f};
    engine.addRigidBody(&physicsBody);

    // Ground plane at y = 0
    PlaneCollider ground{Vec3{0, 1, 0}, 0};

    // Sphere vs Sphere pair — left side
    Vec3 sphereA{-6, 1, 0};
    Vec3 sphereB{-4, 1, 0};

    // AABB vs AABB pair — right side
    Vec3 boxAMin{4, 0, -1};
    Vec3 boxAMax{6, 2, 1};
    Vec3 boxBMin{5, 0, -1};
    Vec3 boxBMax{7, 2, 1};

    while (!visualizer.ShouldClose()) {
        visualizer.UpdateCamera();

        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;

        // --- Physics step ---
        engine.step(dt);
        physicsSphere = BoundingSphere{physicsBody.Position, physicsSphere.getRadius()};

        float prevVelY = physicsBody.Velocity.y;
        engine.ResolvePlaneCollision(&physicsBody, physicsSphere, ground);
        if (prevVelY < 0 && physicsBody.Velocity.y > 0)
            bounceTimer = 3.0f;

        physicsSphere = BoundingSphere{physicsBody.Position, physicsSphere.getRadius()};

        // Animate sphere pair (oscillate toward each other)
        float t = (float)GetTime();
        sphereB.x = -4.0f + 1.8f * sinf(t * 1.5f);

        IntersectionData sphereResult;
        engine.TestSphereVsSphere(
            BoundingSphere{sphereA, 0.8f},
            BoundingSphere{sphereB, 0.8f},
            sphereResult
        );

        // Animate AABB pair (slide toward each other)
        float boxShift = 2.5f * sinf(t * 1.2f);
        boxBMin = Vec3{5.0f + boxShift, 0, -1};
        boxBMax = Vec3{7.0f + boxShift, 2, 1};

        IntersectionData aabbResult;
        engine.TestAABBvsAABB(
            AABB{boxAMin, boxAMax},
            AABB{boxBMin, boxBMax},
            aabbResult
        );

        // --- Rendering ---
        visualizer.BeginRender();

        // Physics sphere
        visualizer.DrawSphereDebug(
            physicsSphere.getPosition(), physicsSphere.getRadius(), PURPLE);

        // Sphere vs Sphere
        Color colA = sphereResult.hasCollided ? RED : GREEN;
        Color colB = sphereResult.hasCollided ? RED : BLUE;
        visualizer.DrawSphereDebug(sphereA, 0.8f, colA);
        visualizer.DrawSphereDebug(sphereB, 0.8f, colB);

        // AABB vs AABB
        Color boxColA = aabbResult.hasCollided ? ORANGE : LIME;
        Color boxColB = aabbResult.hasCollided ? ORANGE : PINK;
        visualizer.DrawAABBDebug(boxAMin, boxAMax, boxColA);
        visualizer.DrawAABBDebug(boxBMin, boxBMax, boxColB);

        // Ground
        visualizer.DrawPlaneDebug(Vec3{0, 0, 0}, Vec3{0, 1, 0}, 80.0f, DARKGRAY);

        visualizer.EndRender();

        // --- HUD ---
        visualizer.DrawFPS(10, 10);

        if (sphereResult.hasCollided)
            visualizer.DrawText("SPHERE vs SPHERE", 10, 40, 20, MAROON);

        if (aabbResult.hasCollided)
            visualizer.DrawText("AABB vs AABB", 10, 65, 20, ORANGE);

        if (bounceTimer > 0.0f) {
            visualizer.DrawText("RIGIDBODY BOUNCE", 10, 90, 20, PURPLE);
            bounceTimer -= dt;
        }
    }

    return 0;
}
