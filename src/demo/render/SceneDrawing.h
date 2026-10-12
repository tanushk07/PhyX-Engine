#pragma once
#include "Engine.h"
#include "DemoState.h"
#include "Visualizer.h"

// Every body, in the lit look or the wireframe look (G switches).
void DrawBodies(Visualizer& vis, const PhysicsEngine& engine, const Grab& grab);

// What the solver saw and did this step (contacts, forces, velocities, axes), drawn on top of the bodies.
void DrawOverlays(Visualizer& vis, const PhysicsEngine& engine, const DemoState& state, float dt);

// The rubber band from the held point to the mouse.
void DrawGrab(Visualizer& vis, const Grab& grab, Vec3 target);
