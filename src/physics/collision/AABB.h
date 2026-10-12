#pragma once
#include "BoundingSphere.h"
#include "Vector.h"
#include "IntersectionData.h"
class AABB
{
	Vec3 minExtend, maxExtend;
	
	public:
		AABB(Vec3 minE, Vec3 maxE) : minExtend(minE), maxExtend(maxE) {}
		void Intersect(const AABB& other, IntersectionData &Data) const;
		void Intersect(const BoundingSphere& other, IntersectionData& Data) const;

		inline const Vec3& getMinExtend() const { return minExtend; }
		inline const Vec3& getMaxExtend() const { return maxExtend; }
};