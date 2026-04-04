#pragma once
#include <raylib.h>
#include "Vector.h"

class Visualizer {
public:
    Visualizer(int width, int height, const char* title);
    ~Visualizer();

    void UpdateCamera();
    void BeginRender();
    void EndRender();

    void DrawSphereDebug(Vec3 position, float radius, Color color);
    void DrawAABBDebug(Vec3 minExt, Vec3 maxExt, Color color);
    void DrawPlaneDebug(Vec3 position, Vec3 normal, float size, Color color);

    void DrawText(const char* text, int x, int y, int fontSize, Color color);
    void DrawFPS(int x, int y);

    bool ShouldClose() const;

private:
    Camera3D camera;
};
