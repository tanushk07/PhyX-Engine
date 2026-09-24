#pragma once
#include "Vector.h"

class IntersectionData
{
public:
	bool hasCollided = false;
	float IntersectionDepth = 0.f;
	Vec3 IntersectionPoint, IntersectionNormal;
};