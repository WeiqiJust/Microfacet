#pragma once
#include "mathutils.h"
#include "utils.h"
#include "instance_property.h"

class microfacet_distr
{
public:
	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors)
	{}

	virtual void* get_param() { return NULL; }
};

class distr_grid_param
{
public:
	float	x_space, y_space, z_space,
		scale, height;
};

class distr_low_discrepancy_param
{
public:
	float	density, scale, height;
};

class distr_low_discrepancy_3d_param
{
public:
	float	density, relative_height_density,
		scale, height;
};


class distr_rod_param
{
public:
	float	density, randomness, scale,
		pd_phi, pd_theta;
};