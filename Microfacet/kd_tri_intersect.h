#pragma once
#include "utils.h"
#include "bounding_box.h"

class intersection_result
{
public:
	Vector3 hit_point;
	Triangle* triangle;
	float distance;
};

class kd_tri_intersect
{
public:
	kd_tri_intersect();

	kd_tri_intersect* build(vector<Triangle*>& tris, int depth) const;

	bool hit(const kd_tri_intersect* node, const Ray& ray, intersection_result& result) const;

	bounding_box bbox;
	kd_tri_intersect *left, *right;
	vector<Triangle*> triangles;
};