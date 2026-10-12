#pragma once
#include <raylib.h>
#include "Vector.h"

// phyX uses Vec3, raylib uses Vector3: the same three floats, two names.
inline Vector3 ToRaylib(Vec3 v)    { return Vector3{ v.x, v.y, v.z }; }
inline Vec3    ToVec3(Vector3 v)   { return Vec3{ v.x, v.y, v.z }; }
