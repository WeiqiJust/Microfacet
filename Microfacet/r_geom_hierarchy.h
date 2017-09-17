#pragma once
#include "utils.h"
#include "r_geometry.h"

class point_sample_param
{
public:
	float	s;
	int		num;
};

class r_geom_hierarchy
{
public:
	r_geom_hierarchy();

	void set_geom(std::shared_ptr<r_geometry> &p);
	std::shared_ptr<r_geometry> get_geom();

	//find s in vector scales
	int scale_idx(const float s) const;

	//get sample point and normals from index 
	void get_samples(const int idx, const std::vector<Vector3> *&p, const std::vector<normal_weight> *&n);

	void set_scales(const std::vector<point_sample_param> &scales);

	void clear_samples();

	//call r_geometry sample_point iteratively
	void sample_points(const float density, const int num_area_hits_scalar);

private:
	// sampling points and normals
	std::vector<std::vector<std::vector<Vector3>>>			points;
	std::vector<std::vector<std::vector<normal_weight>>>	normals;
	std::vector<point_sample_param>							scales;
	random													rng;

	std::shared_ptr<r_geometry>	p_geom;
};