#include "r_geom_hierarchy.h"


r_geom_hierarchy::r_geom_hierarchy()
{
	point_sample_param p;
	p.s		= 1;
	p.num	= 1;
	scales.push_back(p);
}

void r_geom_hierarchy::set_geom(std::shared_ptr<r_geometry> &p)
{
	p_geom = p;
}

std::shared_ptr<r_geometry> r_geom_hierarchy::get_geom()
{
	return p_geom;
}

int r_geom_hierarchy::scale_idx(const float s) const
{
	int l = 0, r = (int)scales.size()-1, m;

	while (l < r)
	{
		m = (l+r) / 2;

		if (s <= scales[m].s)
		{
			r = m-1;
		} else if (s > scales[m+1].s)
		{
			l = m+1;
		} else {			
			return m+1;
		}
	}

	return (l+r)/2;
}

void r_geom_hierarchy::get_samples(const int l, const std::vector<Vector3> *&p, const std::vector<normal_weight> *&n)
{
	int idx = rng.get_random_float_open()*points[l].size();
	p = &points[l][idx];
	n = &normals[l][idx];
}

void r_geom_hierarchy::set_scales(const std::vector<point_sample_param> &s)
{
	scales = s;
}

void r_geom_hierarchy::clear_samples()
{
	points.clear();
	normals.clear();
}

void r_geom_hierarchy::sample_points(const float density,
				   const int num_area_hits_scalar)
{
	if (points.size() != scales.size())
	{
		points.resize(scales.size());
		normals.resize(scales.size());

		for (int i = 0; i < scales.size(); i++)
		{
			points[i].resize(scales[i].num);
			normals[i].resize(scales[i].num);

			for (int j = 0; j < scales[i].num; j++)
					p_geom->sample_points(
					points[i][j], normals[i][j], 
					density*scales[i].s*scales[i].s, 
					num_area_hits_scalar,
					rng, Vector2(rng.get_random_float_open(), rng.get_random_float_open()));
		}
	}
}

