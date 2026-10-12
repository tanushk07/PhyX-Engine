#pragma once
#include <raylib.h>
#include "Vector.h"
#include "Quaternion.h"

class Visualizer {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    Visualizer(int width, int height, const char* title);
    ~Visualizer();

    void UpdateCamera();
    void BeginRender();     // start the frame and the 3D scene
    void SetCamera(Vec3 position, Vec3 target);
    Ray  MouseRay() const;          // the line from the camera through the mouse pointer
    void EndScene();                // end the 3D scene; 2D text (the HUD) can be drawn after this
    void EndFrame();                // show the frame
    
    // Lit, textured look (the default). G in the demo switches to the wireframe look.
    void SetLitLook(bool lit) { litLook = lit; }
    bool IsLitLook() const { return litLook; }
    // washOut: 0 = normal, up to 1 = faded towards white (used for sleeping bodies)
    void DrawSphereLit(Vec3 position, float radius, const Quat& orientation, Color color, float washOut = 0.0f);
    void DrawBoxLit(Vec3 center, Vec3 halfExtents, const Quat& orientation, Color color, float washOut = 0.0f);
    void DrawFloor(float size);
    void DrawShadow(Vec3 position, float footprint, float heightAboveFloor);   // soft dark spot under a body
    
    void DrawSphereDebug(Vec3 position, float radius, Color color);
    void DrawAABBDebug(Vec3 minExt, Vec3 maxExt, Color color);
    void DrawPlaneDebug(Vec3 position, Vec3 normal, float size, Color color);
    void DrawBoxDebug(Vec3 center, Vec3 halfExtents, const Quat& orientation, Color color);
    void DrawBodyAxes(Vec3 origin, const Quat& orientation, float length);

    void BeginOverlay();            // everything drawn until EndOverlay() shows through the bodies
    void EndOverlay();
    void DrawArrow(Vec3 from, Vec3 to, Color color);
    void DrawPoint(Vec3 position, float radius, Color color);
    void DrawLine(Vec3 from, Vec3 to, Color color);
    
    void DrawText(const char* text, int x, int y, int fontSize, Color color);
    void DrawFPS(int x, int y);

    bool ShouldClose() const;

private:
    Camera3D camera;
    
    bool litLook = true;

    Shader    litShader;
    int       viewPosLoc = -1;
    int       isFloorLoc = -1;
    int       washOutLoc = -1;

    void SetWashOut(float amount);
    Model     sphereModel;
    Model     cubeModel;
    Model     floorModel;
    Texture2D sphereTexture;
    Texture2D boxTexture;
};
