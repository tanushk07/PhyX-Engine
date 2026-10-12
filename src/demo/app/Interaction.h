#pragma once
#include <raylib.h>
#include "Engine.h"
#include "DemoState.h"

// Finds the nearest body under the mouse ray. hitPoint is where the ray first touches it.
RigidBody* PickBody(const PhysicsEngine& engine, Ray ray, Vec3& hitPoint, float& distance);

// The point `distance` metres along the ray.
Vec3 AlongRay(Ray ray, float distance);

// Picks the body under the ray and holds it (grab stays empty if nothing was hit).
void StartGrab(Grab& grab, const PhysicsEngine& engine, Ray ray);

// Where the held point is right now, in world space.
Vec3 GrabAnchor(const Grab& grab);

// The spring that drags a grabbed body towards target.
void ApplyGrabForce(const Grab& grab, Vec3 target);

void ThrowBall(PhysicsEngine& engine, Ray ray);   // B: a ball shot along the view
void DropBox(PhysicsEngine& engine, Ray ray);     // K: a box dropped above where you point
