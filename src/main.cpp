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

    // Ground plane at y = 0 (the ONE plane for everything)
    PlaneCollider ground{Vec3{0, 1, 0}, 0};

    // Rigidbody sphere (gravity + bounce) — far left
    BoundingSphere physicsSphere{Vec3{-20, 8, 0}, 0.6f};
    RigidBody physicsBody{physicsSphere.getPosition(), 2.0f, 0.82f,
                          Vec3{0, 0, 0}, Vec3{0, 0, 0},
                          Inertia::Sphere(0.6f, 2.0f), 0.3f};
    engine.addRigidBody(&physicsBody);

    // --- Test 1: Sphere vs Sphere (x = -10) ---
    Vec3 sphereA{-12, 1.5f, 0};
    Vec3 sphereB{-8, 1.5f, 0};

    // --- Test 2: AABB vs AABB (x = 0) ---
    Vec3 boxAMin{-2, 0.5f, -1};
    Vec3 boxAMax{0, 2.5f, 1};
    Vec3 boxBMin{2, 0.5f, -1};
    Vec3 boxBMax{4, 2.5f, 1};

    // --- Test 3: Sphere vs AABB (x = 10) ---
    Vec3 sphereC{8, 1.5f, 0};
    Vec3 boxCMin{11, 0.5f, -1};
    Vec3 boxCMax{13, 2.5f, 1};

    // --- Test 4: Plane vs Sphere (x = 20) — sphere bobs vertically into ground ---

    // --- Test 5: Plane vs AABB (x = 30) — AABB bobs vertically into ground ---

    while (!visualizer.ShouldClose()) {
        visualizer.UpdateCamera();

        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;
        float t = (float)GetTime();

        // --- Physics step (rigidbody bounce) ---
        engine.step(dt);
        physicsSphere = BoundingSphere{physicsBody.Position, physicsSphere.getRadius()};

        float prevVelY = physicsBody.Velocity.y;
        engine.ResolvePlaneCollision(&physicsBody, physicsSphere, ground);
        if (prevVelY < 0 && physicsBody.Velocity.y > 0)
            bounceTimer = 3.0f;

        physicsSphere = BoundingSphere{physicsBody.Position, physicsSphere.getRadius()};

        // === Test 1: Sphere vs Sphere ===
        sphereB.x = -8.0f + 2.5f * sinf(t * 1.5f);

        IntersectionData sphereResult;
        engine.TestSphereVsSphere(
            BoundingSphere{sphereA, 0.8f},
            BoundingSphere{sphereB, 0.8f},
            sphereResult
        );

        // === Test 2: AABB vs AABB ===
        float boxShift = 3.0f * sinf(t * 1.2f);
        boxBMin = Vec3{2.0f + boxShift, 0.5f, -1};
        boxBMax = Vec3{4.0f + boxShift, 2.5f, 1};

        IntersectionData aabbResult;
        engine.TestAABBvsAABB(
            AABB{boxAMin, boxAMax},
            AABB{boxBMin, boxBMax},
            aabbResult
        );

        // === Test 3: Sphere vs AABB ===
        sphereC.x = 8.0f + 3.0f * sinf(t * 1.3f);

        IntersectionData sphereAABBResult;
        engine.TestSphereVsAABB(
            BoundingSphere{sphereC, 0.8f},
            AABB{boxCMin, boxCMax},
            sphereAABBResult
        );

        // === Test 4: Plane vs Sphere — sphere bobs down into the ground plane ===
        float sphereY4 = 1.5f + 1.8f * sinf(t * 1.4f);
        Vec3 planeTestSphere{20, sphereY4, 0};

        IntersectionData planeSphereResult;
        engine.TestPlaneVsSphere(
            ground,
            BoundingSphere{planeTestSphere, 0.8f},
            planeSphereResult
        );

        // === Test 5: Plane vs AABB — box bobs down into the ground plane ===
        float boxY5 = 1.2f + 1.5f * sinf(t * 1.1f);
        Vec3 boxEMin{29, boxY5 - 1.0f, -1};
        Vec3 boxEMax{31, boxY5 + 1.0f, 1};

        IntersectionData planeAABBResult;
        engine.TestPlaneVsAABB(
            ground,
            AABB{boxEMin, boxEMax},
            planeAABBResult
        );

        // --- Rendering ---
        visualizer.BeginRender();

        // Rigidbody bounce sphere (x = -20)
        visualizer.DrawSphereDebug(
            physicsSphere.getPosition(), physicsSphere.getRadius(), PURPLE);

        // Test 1: Sphere vs Sphere (x = -10)
        Color colA = sphereResult.hasCollided ? RED : GREEN;
        Color colB = sphereResult.hasCollided ? RED : BLUE;
        visualizer.DrawSphereDebug(sphereA, 0.8f, colA);
        visualizer.DrawSphereDebug(sphereB, 0.8f, colB);

        // Test 2: AABB vs AABB (x = 0)
        Color boxColA = aabbResult.hasCollided ? ORANGE : LIME;
        Color boxColB = aabbResult.hasCollided ? ORANGE : PINK;
        visualizer.DrawAABBDebug(boxAMin, boxAMax, boxColA);
        visualizer.DrawAABBDebug(boxBMin, boxBMax, boxColB);

        // Test 3: Sphere vs AABB (x = 10)
        Color sphAABBCol = sphereAABBResult.hasCollided ? RED : SKYBLUE;
        Color boxAABBCol = sphereAABBResult.hasCollided ? RED : YELLOW;
        visualizer.DrawSphereDebug(sphereC, 0.8f, sphAABBCol);
        visualizer.DrawAABBDebug(boxCMin, boxCMax, boxAABBCol);

        // Test 4: Plane vs Sphere (x = 20)
        Color planeSphCol = planeSphereResult.hasCollided ? RED : MAGENTA;
        visualizer.DrawSphereDebug(planeTestSphere, 0.8f, planeSphCol);

        // Test 5: Plane vs AABB (x = 30)
        Color planeBoxCol = planeAABBResult.hasCollided ? RED : BEIGE;
        visualizer.DrawAABBDebug(boxEMin, boxEMax, planeBoxCol);

        // Ground plane
        visualizer.DrawPlaneDebug(Vec3{0, 0, 0}, Vec3{0, 1, 0}, 80.0f, DARKGRAY);

        visualizer.EndRender();

        // --- HUD ---
        visualizer.DrawFPS(10, 10);

        int hudY = 40;
        if (bounceTimer > 0.0f) {
            visualizer.DrawText("RIGIDBODY BOUNCE", 10, hudY, 20, PURPLE);
            bounceTimer -= dt;
            hudY += 25;
        }
        if (sphereResult.hasCollided) {
            visualizer.DrawText("SPHERE vs SPHERE", 10, hudY, 20, MAROON);
            hudY += 25;
        }
        if (aabbResult.hasCollided) {
            visualizer.DrawText("AABB vs AABB", 10, hudY, 20, ORANGE);
            hudY += 25;
        }
        if (sphereAABBResult.hasCollided) {
            visualizer.DrawText("SPHERE vs AABB", 10, hudY, 20, RED);
            hudY += 25;
        }
        if (planeSphereResult.hasCollided) {
            visualizer.DrawText("PLANE vs SPHERE", 10, hudY, 20, MAGENTA);
            hudY += 25;
        }
        if (planeAABBResult.hasCollided) {
            visualizer.DrawText("PLANE vs AABB", 10, hudY, 20, GOLD);
            hudY += 25;
        }
    }

    return 0;
}
