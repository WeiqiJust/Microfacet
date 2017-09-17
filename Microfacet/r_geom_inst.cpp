#include "r_geom_inst.h"

void r_geom_inst::init(std::shared_ptr<r_geom_hierarchy> &p)
{
	p_gh = p;
}

void r_geom_inst::set_scale(const float s)
{
	scale = s;
	if (p_gh != NULL)
		scale_idx = p_gh->scale_idx(s);
	else
		scale_idx = 0;
}

std::shared_ptr<r_geometry> r_geom_inst::get_geom()
{
	return p_gh->get_geom();
}

void r_geom_inst::get_samples(const std::vector<Vector3> *&p,
	const std::vector<normal_weight> *&n,
	float &s)
{
	p_gh->get_samples(scale_idx, p, n);
	s = scale;
}

void r_geom_inst::sample_points(const float density,
	const int num_area_hits_scalar)
{
	p_gh->sample_points(density, num_area_hits_scalar);
}
