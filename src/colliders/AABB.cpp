#include "AABB.h"

#include <algorithm>

#include "BoundingSphere.h"

void AABB::Intersect(const AABB& other, IntersectionData& Data) const
{
	const Vec3 distance1 = other.minExtend - maxExtend;
	const Vec3 distance2 = minExtend - other.maxExtend;
	Vec3 BoxACenter = (maxExtend + minExtend)/2;
	Vec3 BoxBCenter = (other.minExtend + other.maxExtend)/2;
	auto maxDistance = distance1.Max(distance2);
	
	
	if (maxDistance.Max() < 0)
	{
		Data.hasCollided = true ;
		Data.IntersectionDepth = fabs(maxDistance.Max());
		const Vec3 lo(	std::max(minExtend.x, other.minExtend.x),
						std::max(minExtend.y, other.minExtend.y),
						std::max(minExtend.z, other.minExtend.z));
		const Vec3 hi(	std::min(maxExtend.x, other.maxExtend.x),
						std::min(maxExtend.y, other.maxExtend.y),
						std::min(maxExtend.z, other.maxExtend.z));
		Data.IntersectionPoint = (lo + hi) * 0.5f;
		
		switch (maxDistance.MaxAxis())
		{
		case x:
			if (BoxACenter.x < BoxBCenter.x) Data.IntersectionNormal = {1,0,0};
			else Data.IntersectionNormal = {-1,0,0};
			break;
		case y:
			if (BoxACenter.y<BoxBCenter.y) Data.IntersectionNormal = {0,1,0};
			else Data.IntersectionNormal = {0,-1,0};
			break;
		case z:
			if (BoxACenter.z < BoxBCenter.z) Data.IntersectionNormal = {0,0,1};
			else Data.IntersectionNormal = {0,0,-1};
			break;
		default:  // NOLINT(clang-diagnostic-covered-switch-default)
			break;
		}
	}
	else
	{
		Data.hasCollided = false;
		Data.IntersectionDepth = 0.0f;
	}
}

void AABB::Intersect(const BoundingSphere& other, IntersectionData& Data) const
{
	Vec3 Position = other.getPosition();
	float Radius = other.getRadius();
	
	bool inside = Position.x >= minExtend.x && Position.x <= maxExtend.x && 
				  Position.y >= minExtend.y && Position.y <= maxExtend.y && 
				  Position.z >= minExtend.z && Position.z <= maxExtend.z;
	if (inside)
	{
		// distance from the centre out to each of the six faces; the smallest is the way out
		float best = Position.x - minExtend.x;   Vec3 n{-1, 0, 0};
		if (maxExtend.x - Position.x < best) { best = maxExtend.x - Position.x; n = Vec3{ 1, 0, 0}; }
		if (Position.y - minExtend.y  < best) { best = Position.y - minExtend.y; n = Vec3{ 0,-1, 0}; }
		if (maxExtend.y - Position.y  < best) { best = maxExtend.y - Position.y; n = Vec3{ 0, 1, 0}; }
		if (Position.z - minExtend.z  < best) { best = Position.z - minExtend.z; n = Vec3{ 0, 0,-1}; }
		if (maxExtend.z - Position.z  < best) { best = maxExtend.z - Position.z; n = Vec3{ 0, 0, 1}; }

		Data.hasCollided        = true;
		Data.IntersectionNormal = n;                  // box -> sphere, matches "this -> other"
		Data.IntersectionDepth  = Radius + best;      // out of the box, then clear its own radius
		Data.IntersectionPoint  = Position + n * best;  // the centre projected onto that face
		return;
	}
	
	
	Vec3 closestPoint(
		std::max(getMinExtend().x, std::min(Position.x, getMaxExtend().x)),
		std::max(getMinExtend().y, std::min(Position.y, getMaxExtend().y)),
		std::max(getMinExtend().z, std::min(Position.z, getMaxExtend().z))
	);
	
	Vec3 diff = Position - closestPoint;
	float distance = diff.length();
	
	if (distance < Radius)
	{
		Data.hasCollided = true;
		Data.IntersectionDepth = Radius - distance;
		Data.IntersectionNormal = diff.normalize();
		Data.IntersectionPoint = closestPoint;
	}
	else
	{
		Data.hasCollided = false;
		Data.IntersectionDepth = 0.0f;
	}
}
