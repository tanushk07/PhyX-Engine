#include "AABB.h"

void AABB::Intersect(AABB other, IntersectionData& Data)
{
	const Vec3 distance1 = other.minExtend - maxExtend;
	const Vec3 distance2 = minExtend - other.maxExtend;
	auto maxDistance = distance1.Max(distance2);

	Data.hasCollided = maxDistance.Max() < 0;
	Data.IntersectionRadius = maxDistance.Max();
}