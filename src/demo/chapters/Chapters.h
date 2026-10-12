#pragma once
#include <string>
#include <vector>
#include "Engine.h"

// One step of the walkthrough: a small scene that shows one idea from the engine.
struct Chapter
{
    const char*              Title;
    std::vector<const char*> Caption;                 // a few short lines shown on screen
    void (*Build)(PhysicsEngine&);                    // fills an empty engine with the scene

    // overlays switched on when the chapter starts (keys can still change them)
    bool Contacts, Forces, Velocities, PointVelocities, Axes, SlowMotion;

    Vec3 CameraPosition, CameraTarget;

    // optional live numbers for this chapter, called every frame; may be nullptr
    void (*Readout)(const PhysicsEngine&, std::vector<std::string>& lines);
};

const std::vector<Chapter>& GetChapters();