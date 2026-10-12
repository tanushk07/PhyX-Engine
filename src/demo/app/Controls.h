#pragma once
#include "Engine.h"
#include "DemoState.h"
#include "Visualizer.h"

// Rebuilds the engine with chapter `index` (wraps around) and applies its overlays and camera.
void StartChapter(int index, DemoState& state, PhysicsEngine& engine, Visualizer& vis, Grab& grab);

// Reads the keyboard and mouse once per frame.
void HandleInput(DemoState& state, PhysicsEngine& engine, Visualizer& vis, Grab& grab);
