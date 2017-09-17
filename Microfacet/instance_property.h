#pragma once
#include "utils.h"
#include "tri_mesh.h"
#include "r_instance.h"

class base_obj
{
public:
	tri_mesh	mesh;
	//mapping_2d		*m;

	void apply_transform(const Matrix4 &mat);
	void convert_to_instance(r_instance* &result, const int id, D3D_dev_handle *pdev);
};

class instance_property
{
public:
	int			id;
	float		scale;
	Matrix4		mat;

	shared_ptr<r_geom_hierarchy> p_geom;

	void setup_matrix(const Vector3 &pos,
		const Vector3 &x, const Vector3 &y, const Vector3 &z,
		const float s);
};