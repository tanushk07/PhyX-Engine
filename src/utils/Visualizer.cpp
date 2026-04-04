#include "Visualizer.h"
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
    float angle = acosf(defaultNormal.dot(normal)) * RAD2DEG;

    if (rotationAxis.length() < 0.001f) {
        rotationAxis = {1.0f, 0.0f, 0.0f};
        angle = (normal.y < 0.0f) ? 180.0f : 0.0f;
    } else {
        rotationAxis = rotationAxis.normalize();
    }

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlRotatef(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
        DrawPlane(Vector3{0, 0, 0}, Vector2{size, size}, Fade(color, 0.5f));
    rlPopMatrix();
}