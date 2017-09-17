#include "bounding_box.h"

bounding_box::bounding_box()
{
	parameters[0].x = (std::numeric_limits<float>::max)();
	parameters[1].x = (std::numeric_limits<float>::lowest)();
	parameters[0].y = (std::numeric_limits<float>::max)();
	parameters[1].y = (std::numeric_limits<float>::lowest)();
	parameters[0].z = (std::numeric_limits<float>::max)();
	parameters[1].z = (std::numeric_limits<float>::lowest)();
}

void bounding_box::add_triangle(const Triangle tri)
{
	for (int i = 0; i < 3; i++)
	{
		Vector3 point = tri.vertex[i];
		parameters[0].x = (std::min)(parameters[0].x, point.x);
		parameters[1].x = (std::max)(parameters[1].x, point.x);
		parameters[0].y = (std::min)(parameters[0].y, point.y);
		parameters[1].y = (std::max)(parameters[1].y, point.y);
		parameters[0].z = (std::min)(parameters[0].z, point.z);
		parameters[1].z = (std::max)(parameters[1].z, point.z);
	}
}

int bounding_box::longest_axis()
{
	float x = parameters[1].x - parameters[0].x;
	float y = parameters[1].y - parameters[0].y;
	float z = parameters[1].z - parameters[0].z;
	float max_v = (std::max)((std::max)(x, y), z);
	if (max_v == x)
		return 0;
	else if (max_v == y)
		return 1;
	else
		return 2;
}

bool bounding_box::hit(const Ray &r, float t0, float t1) const 
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (parameters[r.sign[0]].x - r.origin.x) * r.inv_direction.x;
	tmax = (parameters[1 - r.sign[0]].x - r.origin.x) * r.inv_direction.x;
	tymin = (parameters[r.sign[1]].y - r.origin.y) * r.inv_direction.y;
	tymax = (parameters[1 - r.sign[1]].y - r.origin.y) * r.inv_direction.y;
	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (parameters[r.sign[2]].z - r.origin.z) * r.inv_direction.z;
	tzmax = (parameters[1 - r.sign[2]].z - r.origin.z) * r.inv_direction.z;
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	return ((tmin < t1) && (tmax > t0));
}
