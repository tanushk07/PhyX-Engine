#include "Visualizer.h"
#include <algorithm>
#include <cmath>
#include <raymath.h>
#include <rlgl.h>
#include "LitShader.h"
#include "RaylibConvert.h"

static const Color SkyColor{ 226, 231, 238, 255 };

// Quaternion -> axis + angle (degrees), the inverse of Quat::fromAxisAngle. raylib draws with axis-angle.
static void ToAxisAngle(const Quat& q, Vector3& axis, float& angleDeg)
{
    const float w = std::clamp(q.w, -1.0f, 1.0f);       // clamp BEFORE acosf, or a rounding error gives NaN
    angleDeg = 2.0f * acosf(w) * RAD2DEG;
    const float s = sqrtf(1.0f - w * w);
    const Vec3 a = s > 1e-6f ? q.vector / s : Vec3{1.0f, 0.0f, 0.0f};
    axis = ToRaylib(a);
}

// Two-tone checker texture: white squares show the body's colour, grey ones a darker shade.
// The pattern is what makes spinning and rolling visible.
static Texture2D CheckerTexture(int width, int height, int squares, unsigned char grey)
{
    Image image = GenImageChecked(width, height, width / squares, width / squares, WHITE, Color{ grey, grey, grey, 255 });
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
    return texture;
}


Visualizer::Visualizer(int width, int height, const char* title) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);

    camera = { 0 };
    camera.position = Vector3{ 4.0f, 14.0f, 30.0f };
    camera.target = Vector3{ 3.0f, 2.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    litShader  = LoadShaderFromMemory(LitVertexShader, LitFragmentShader);
    viewPosLoc = GetShaderLocation(litShader, "viewPos");
    isFloorLoc = GetShaderLocation(litShader, "isFloor");
    washOutLoc = GetShaderLocation(litShader, "washOut");

    sphereModel = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 48));   // radius 1, scaled per body
    cubeModel   = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f)); // 1 x 1 x 1, scaled per body
    floorModel  = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
    sphereTexture = CheckerTexture(512, 256, 8, 200);   // 8 x 4 squares around the ball, like a beach ball
    boxTexture    = CheckerTexture(256, 256, 2, 215);   // 2 x 2 squares on every face

    for (Model* m : { &sphereModel, &cubeModel, &floorModel }) m->materials[0].shader = litShader;
    sphereModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sphereTexture;
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture   = boxTexture;
    SetTargetFPS(180);
}

Visualizer::~Visualizer() {
    // The models share one shader and their textures are unloaded below: hand them back to
    // raylib's defaults first, so UnloadModel doesn't free them a second time.
    for (Model* m : { &sphereModel, &cubeModel, &floorModel })
    {
        m->materials[0].shader.id = rlGetShaderIdDefault();
        m->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture.id = rlGetTextureIdDefault();
        UnloadModel(*m);
    }
    UnloadTexture(sphereTexture);
    UnloadTexture(boxTexture);
    UnloadShader(litShader);
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


void Visualizer::SetCamera(Vec3 position, Vec3 target) {
    camera.position = ToRaylib(position);
    camera.target   = ToRaylib(target);
}

Ray Visualizer::MouseRay() const {
    return GetScreenToWorldRay(GetMousePosition(), camera);
}


void Visualizer::BeginRender() {
    BeginDrawing();
    ClearBackground(litLook ? SkyColor : RAYWHITE);
    const float eye[3] = { camera.position.x, camera.position.y, camera.position.z };
    SetShaderValue(litShader, viewPosLoc, eye, SHADER_UNIFORM_VEC3);
    BeginMode3D(camera);
}

void Visualizer::EndScene() {
    if (!litLook) DrawGrid(50, 1.0f);      // the lit look has its own checkerboard floor
    EndMode3D();
}

void Visualizer::SetWashOut(float amount) {
    SetShaderValue(litShader, washOutLoc, &amount, SHADER_UNIFORM_FLOAT);
}

void Visualizer::DrawSphereLit(Vec3 position, float radius, const Quat& orientation, Color color, float washOut) {
    Vector3 axis; float angle;
    ToAxisAngle(orientation, axis, angle);              // turn the mesh too, so the pattern rolls with the ball
    SetWashOut(washOut);
    DrawModelEx(sphereModel, ToRaylib(position), axis, angle,
                Vector3{ radius, radius, radius }, color);
    SetWashOut(0.0f);
}

void Visualizer::DrawBoxLit(Vec3 center, Vec3 halfExtents, const Quat& orientation, Color color, float washOut) {
    Vector3 axis; float angle;
    ToAxisAngle(orientation, axis, angle);
    const Vector3 size{ halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f };
    SetWashOut(washOut);
    DrawModelEx(cubeModel, ToRaylib(center), axis, angle, size, color);
    SetWashOut(0.0f);

    // thin dark edges, so boxes of the same colour stay readable when stacked
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(angle, axis.x, axis.y, axis.z);
    DrawCubeWiresV(Vector3{0, 0, 0}, Vector3{ size.x * 1.002f, size.y * 1.002f, size.z * 1.002f }, Fade(BLACK, 0.3f * (1.0f - 0.6f * washOut)));
    rlPopMatrix();
}

void Visualizer::DrawFloor(float size) {
    const int on = 1, off = 0;
    SetShaderValue(litShader, isFloorLoc, &on, SHADER_UNIFORM_INT);
    DrawModelEx(floorModel, Vector3{0, 0, 0}, Vector3{0, 1, 0}, 0.0f, Vector3{ size, 1.0f, size }, WHITE);
    SetShaderValue(litShader, isFloorLoc, &off, SHADER_UNIFORM_INT);
}

void Visualizer::DrawShadow(Vec3 position, float footprint, float heightAboveFloor) {
    // darker in the middle (three stacked discs), fainter the higher the body is
    const float fade = std::clamp(1.0f - heightAboveFloor / 6.0f, 0.0f, 1.0f);
    if (fade <= 0.0f) return;
    const Color shade = Fade(BLACK, 0.10f * fade);
    for (float k : { 1.0f, 0.75f, 0.5f })
        DrawCylinder(Vector3{ position.x, 0.02f, position.z }, footprint * k, footprint * k, 0.0f, 32, shade);
}

void Visualizer::EndFrame() {
    ::EndDrawing();
}

void Visualizer::BeginOverlay() {
    rlDrawRenderBatchActive();      // draw what is queued so far with depth testing still on
    rlDisableDepthTest();
}

void Visualizer::EndOverlay() {
    rlDrawRenderBatchActive();      // draw the overlay now, while depth testing is off
    rlEnableDepthTest();
}

void Visualizer::DrawArrow(Vec3 from, Vec3 to, Color color) {
    const Vec3 d = to - from;
    const float length = d.length();
    if (length < 1e-4f) return;
    const float head = std::min(0.25f, length * 0.4f);           // cone at the tip
    const Vec3 neck = to - d * (head / length);
    const Vector3 a = ToRaylib(from), b = ToRaylib(neck), tip = ToRaylib(to);
    DrawCylinderEx(a, b, 0.035f, 0.035f, 8, color);
    DrawCylinderEx(b, tip, head * 0.4f, 0.0f, 10, color);
}

void Visualizer::DrawLine(Vec3 from, Vec3 to, Color color) {
    DrawLine3D(ToRaylib(from), ToRaylib(to), color);
}

void Visualizer::DrawPoint(Vec3 position, float radius, Color color) {
    DrawSphere(ToRaylib(position), radius, color);
}

void Visualizer::DrawSphereDebug(Vec3 position, float radius, Color color) {
    DrawSphere(ToRaylib(position), radius, Fade(color, 0.5f));
    DrawSphereWires(ToRaylib(position), radius, 16, 16, color);
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
    const float d = std::clamp(defaultNormal.dot(normal), -1.0f, 1.0f);
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
    const Vector3 o = ToRaylib(origin);

    // Where the body's own x, y, z axes point in world space right now.
    const Vec3 ax = orientation.rotate(Vec3{1, 0, 0}) * length;
    const Vec3 ay = orientation.rotate(Vec3{0, 1, 0}) * length;
    const Vec3 az = orientation.rotate(Vec3{0, 0, 1}) * length;

    DrawLine3D(o, ToRaylib(origin + ax), RED);
    DrawLine3D(o, ToRaylib(origin + ay), GREEN);
    DrawLine3D(o, ToRaylib(origin + az), BLUE);
}

void Visualizer::DrawBoxDebug(Vec3 center, Vec3 halfExtents, const Quat& orientation, Color color)
{
    Vector3 axis; float angle;
    ToAxisAngle(orientation, axis, angle);
    const Vector3 size{ halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f };

    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(angle, axis.x, axis.y, axis.z);
    DrawCubeV(Vector3{0, 0, 0}, size, Fade(color, 0.5f));
    DrawCubeWiresV(Vector3{0, 0, 0}, size, color);
    rlPopMatrix();
}
