#pragma once
#include "utils.h"
#include "r_geom_hierarchy.h"
class r_geom_inst
{
public:
	void init(std::shared_ptr<r_geom_hierarchy> &p);

	void set_scale(const float scale);

	std::shared_ptr<r_geometry> get_geom();

	// call r_geom_hierarchy to get sameples
	void get_samples(const std::vector<Vector3> *&p, const std::vector<normal_weight> *&n, float &s);

	// call r_geom_hierarchy to sample points
	void sample_points(const float density,const int num_area_hits_scalar);

private:
	std::shared_ptr<r_geom_hierarchy> p_gh;
	// scale value and its index in r_geom_hierarchy
	int		scale_idx;
	float	scale;
};