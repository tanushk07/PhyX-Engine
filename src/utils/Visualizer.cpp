#include "Visualizer.h"
#include <algorithm>
#include <cmath>
#include <raymath.h>
#include <rlgl.h>

Visualizer::Visualizer(int width, int height, const char* title) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);

    camera = { 0 };
    camera.position = Vector3{ 0.0f, 5.0f, 6.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(180);
}

Visualizer::~Visualizer() {
    CloseWindow();
}

bool Visualizer::ShouldClose() const {
    return WindowShouldClose();
}

void Visualizer::UpdateCamera() {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        HideCursor();
        Vector2 mouseDelta = GetMouseDelta();
        Vector3 movement = { 0 };
        if (IsKeyDown(KEY_W)) movement.x += 0.2f;
        if (IsKeyDown(KEY_S)) movement.x -= 0.2f;
        if (IsKeyDown(KEY_D)) movement.y += 0.2f;
        if (IsKeyDown(KEY_A)) movement.y -= 0.2f;
        if (IsKeyDown(KEY_E)) movement.z += 0.2f;
        if (IsKeyDown(KEY_Q)) movement.z -= 0.2f;

        Vector3 rotation = { mouseDelta.x * 0.2f, mouseDelta.y * 0.2f, 0.0f };
        UpdateCameraPro(&camera, movement, rotation, -GetMouseWheelMove() * 2.0f);
    } else {
        ShowCursor();
        UpdateCameraPro(&camera, Vector3{0, 0, 0}, Vector3{0, 0, 0}, -GetMouseWheelMove() * 2.0f);
    }
}

void Visualizer::BeginRender() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);
}

void Visualizer::EndRender() {
    DrawGrid(50, 1.0f);
    EndMode3D();
    ::DrawFPS(10, 10);
    ::EndDrawing();
}

void Visualizer::DrawSphereDebug(Vec3 position, float radius, Color color) {
    DrawSphere(Vector3{position.x, position.y, position.z}, radius, Fade(color, 0.5f));
    DrawSphereWires(Vector3{position.x, position.y, position.z}, radius, 16, 16, color);
}

void Visualizer::DrawAABBDebug(Vec3 minExt, Vec3 maxExt, Color color) {
    Vector3 center = {(maxExt.x + minExt.x)/2.0f, (maxExt.y + minExt.y)/2.0f, (maxExt.z + minExt.z)/2.0f};
    Vector3 size = {(maxExt.x - minExt.x), (maxExt.y - minExt.y), (maxExt.z - minExt.z)};
    DrawCubeV(center, size, Fade(color, 0.5f));
    DrawCubeWiresV(center, size, color);
}

void Visualizer::DrawText(const char* text, int x, int y, int fontSize, Color color) {
    ::DrawText(text, x, y, fontSize, color);
}

void Visualizer::DrawFPS(int x, int y) {
    ::DrawFPS(x, y);
}

void Visualizer::DrawPlaneDebug(Vec3 position, Vec3 normal, float size, Color color) {
    Vec3 defaultNormal = { 0.0f, 1.0f, 0.0f };

    Vec3 rotationAxis = defaultNormal.cross(normal);
    float d = defaultNormal.dot(normal);
    d = std::min(d, 1.0f);
    d = std::max(d, -1.0f);
    float angle = acosf(d) * RAD2DEG;

    if (rotationAxis.length() < 0.001f) {
        rotationAxis = {1.0f, 0.0f, 0.0f};
        angle = (normal.y < 0.0f) ? 180.0f : 0.0f;
    } else {
        rotationAxis = rotationAxis.normalize();
    }

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlRotatef(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
        DrawPlane(Vector3{0, 0, 0}, Vector2{size, size}, color);

        // Wire grid, drawn in the plane's own local space (its surface is local XZ).
        // Lifted a hair along local +y so it does not z-fight with the fill.
        const float half  = size * 0.5f;
        const int   lines = 10;
        const Color wire  = Fade(color, 0.6f);
        for (int i = 0; i <= lines; ++i)
        {
            const float t = -half + (size * i) / lines;
            DrawLine3D(Vector3{t, 0.01f, -half}, Vector3{t, 0.01f, half}, wire);
            DrawLine3D(Vector3{-half, 0.01f, t}, Vector3{half, 0.01f, t}, wire);
        }
    rlPopMatrix();
}

void Visualizer::DrawBodyAxes(Vec3 origin, const Quat& orientation, float length)
{
    const Vector3 o{ origin.x, origin.y, origin.z };

    // Where the body's own x, y, z axes point in world space right now.
    const Vec3 ax = orientation.rotate(Vec3{1, 0, 0}) * length;
    const Vec3 ay = orientation.rotate(Vec3{0, 1, 0}) * length;
    const Vec3 az = orientation.rotate(Vec3{0, 0, 1}) * length;

    DrawLine3D(o, Vector3{ o.x + ax.x, o.y + ax.y, o.z + ax.z }, RED);
    DrawLine3D(o, Vector3{ o.x + ay.x, o.y + ay.y, o.z + ay.z }, GREEN);
    DrawLine3D(o, Vector3{ o.x + az.x, o.y + az.y, o.z + az.z }, BLUE);
}

void Visualizer::DrawBoxDebug(Vec3 center, Vec3 halfExtents, const Quat& orientation, Color color)
{
    // Quaternion -> axis + angle: the exact inverse of Quat::fromAxisAngle.
    float w = orientation.w;
    if (w >  1.0f) w =  1.0f;        // clamp BEFORE acosf, or a rounding error gives NaN
    if (w < -1.0f) w = -1.0f;

    const float angleDeg = 2.0f * acosf(w) * RAD2DEG;   // w = cos(theta/2)
    const float s        = sqrtf(1.0f - w * w);         //     = sin(theta/2)

    Vec3 axis{1.0f, 0.0f, 0.0f};                        // any axis is fine at zero angle
    if (s > 1e-6f) axis = orientation.vector / s;       // vector part stores axis * sin(theta/2)

    const Vector3 size{ halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f };

    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(angleDeg, axis.x, axis.y, axis.z);
    DrawCubeV(Vector3{0, 0, 0}, size, Fade(color, 0.5f));
    DrawCubeWiresV(Vector3{0, 0, 0}, size, color);
    rlPopMatrix();
}