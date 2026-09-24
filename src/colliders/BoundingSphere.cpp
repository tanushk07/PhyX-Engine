#include "BoundingSphere.h"
#include "AABB.h"

void BoundingSphere::Intersect(BoundingSphere otherSphere, IntersectionData& Data) const
{
	Vec3 otherP = otherSphere.Position;
	float otherR = otherSphere.Radius;
	float radiusDistance = otherR + Radius;

	float centerDistance = (Position - otherP).length();
	Data.hasCollided = centerDistance < radiusDistance;
	if (Data.hasCollided) 
	{
		Data.IntersectionDepth = radiusDistance - centerDistance;
		Data.IntersectionNormal = centerDistance<1e-6f? Vec3{0,1,0} : -(Position - otherP).normalize();
		Data.IntersectionPoint = Position + Data.IntersectionNormal*(Radius - Data.IntersectionDepth*.5f);
	}
}