#pragma once
#include "Vector.h"
#include "IntersectionData.h"

class BoundingSphere
{
	float Radius;
	Vec3 Position;

public:
	BoundingSphere(Vec3 Position, float Radius)
	{
		this->Radius=Radius;
		this->Position=Position;
	}

	void intersection(BoundingSphere other, IntersectionData& Data);
	void IntersectWithAABB(const class AABB& other, IntersectionData& Data);

	inline const float getRadius() const { return Radius; }
	inline const Vec3 getPosition() const { return Position; }
};

