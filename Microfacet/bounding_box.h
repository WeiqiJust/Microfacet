#pragma once
#include "utils.h"
#include "tri_mesh.h"

class Triangle
{
public:
	Triangle() {};
	Triangle(Vector3 p1, Vector3 p2, Vector3 p3)
	{
		vertex[0] = p1;
		vertex[1] = p2;
		vertex[2] = p3;
	};

	Vector3 get_mid_point()
	{
		return (vertex[0] + vertex[1] + vertex[2]) / 3;
	};

	void calculate_normal()
	{
		Vector3 edge1 = vertex[1] - vertex[0];
		Vector3 edge2 = vertex[2] - vertex[0];
		normal = Normalize(Cross(edge1, edge2));
	}

	Vector3 vertex[3];
	Vector3 normal;
	int face_index;
	int material_index;
};

class bounding_box
{
public:
	bounding_box();

	void add_triangle(const Triangle tri); // add a single triangle

	int longest_axis(); // 0 -> x, 1 -> y, 2 -> z

	bool hit(const Ray &r, float t0, float t1) const;
private:
	Vector3 parameters[2];
};