#include "BoundingSphere.h"
#include "AABB.h"
#include <algorithm>

void BoundingSphere::intersection(BoundingSphere otherSphere, IntersectionData& Data)
{
	Vec3 otherP = otherSphere.Position;
	float otherR = otherSphere.Radius;
	float radiusDistance = otherR + Radius;

	float centerDistance = (Position - otherP).length();
	Data.hasCollided = centerDistance < radiusDistance;
	if (Data.hasCollided) Data.IntersectionRadius = radiusDistance - centerDistance;
}

void BoundingSphere::IntersectWithAABB(const AABB& other, IntersectionData& Data)
{
    Vec3 closestPoint(
        std::max(other.getMinExtend().x, std::min(Position.x, other.getMaxExtend().x)),
        std::max(other.getMinExtend().y, std::min(Position.y, other.getMaxExtend().y)),
        std::max(other.getMinExtend().z, std::min(Position.z, other.getMaxExtend().z))
    );

    Vec3 diff = Position - closestPoint;
    float distance = diff.length();

    if (distance < Radius)
    {
        Data.hasCollided = true;
        Data.IntersectionRadius = Radius - distance;
    }
    else
    {
        Data.hasCollided = false;
        Data.IntersectionRadius = 0.0f;
    }
}