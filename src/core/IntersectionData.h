#pragma once
#include <vector>

#include "Vector.h"

struct ContactPointData
{
	Vec3 IntersectionPoint;
	float IntersectionDepth;
	float TargetVn    = 0.f;
	float NormalTotal = 0.f;
	Vec3  FrictionTotal;
};

class IntersectionData
{
public:
	bool hasCollided = false;
	Vec3 IntersectionNormal;             // this -> other, as now
	ContactPointData Points[4];
	int  Count = 0;
	Vec3 RollingTotal;
	void AddPoint(Vec3 p, float depth)
	{
		if (Count < 4) Points[Count++] = { .IntersectionPoint = p, .IntersectionDepth = depth };
	}
};