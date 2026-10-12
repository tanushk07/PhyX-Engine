#pragma once
#include "Engine.h"

// What the demo is doing right now: changed by keys, read by the loop and the drawing.
struct DemoState
{
    int  chapter         = 0;
    bool paused          = false;
    int  stepsQueued     = 0;        // single steps asked for with N while paused
    bool slowMotion      = false;
    bool showContacts    = false;
    bool showForces      = false;
    bool showVelocities  = false;
    bool showPointVelocity = false;
    bool showAxes        = false;
    bool showHelp        = true;
};

// A body held by the mouse: a spring pulls the clicked point towards the mouse.
struct Grab
{
    RigidBody* body = nullptr;
    Vec3  localAnchor;            // the clicked point, in the body's own space
    float distance = 0.f;         // how far from the camera it was picked
};
