#pragma once
#include "Vector.h"
#include "IntersectionData.h"
class AABB
{
	Vec3 minExtend, maxExtend;
	
	public:
		AABB(Vec3 minE, Vec3 maxE) : minExtend(minE), maxExtend(maxE) {}
		void Intersect(AABB other, IntersectionData &Data);

		inline const Vec3& getMinExtend() const { return minExtend; }
		inline const Vec3& getMaxExtend() const { return maxExtend; }
};