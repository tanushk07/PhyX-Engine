#pragma once
#include "Engine.h"
#include "DemoState.h"
#include "Theme.h"

// The 2D layer: two long, thin bars, both centred and the same width.
//   top:    status | chapter number + title | counters,  then captions | live readout
//   bottom: key help, then world keys | colour legend
void DrawHud(const Theme& theme, const PhysicsEngine& engine, const DemoState& state);
