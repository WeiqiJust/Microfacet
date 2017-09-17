#pragma once
#include "distr.h"

class microfacet_binder : public microfacet_distr
{
public:
	virtual void generate_geom(const float density, const int num_area_hits_scalar) {};
};

class binder_brick_param
{
public:
	int		x_num, y_num;
	float	bottom_percent_x, bottom_percent_y,
		top_percent_x, top_percent_y, height;
};

class binder_groove_param
{
public:
	float	plane_percent, height;
};

class binder_plane_param
{
public:
	int x_res, y_res;
};

class binder_dispmap_param
{
public:
	int		res_x, res_y;
	float	amp;
};

class binder_regpoly_param
{
public:
	int		num_edges;
	float	radius_top, radius_bottom, height;
};

class binder_woven_param
{
public:
	int		x_num, y_num, bisect_verts, total_path_verts;
	float	bisect_w_y, bisect_h_y, height_y,
		bisect_w_x, bisect_h_x, height_x;
};

class binder_woven_threads_param
{
public:
	int		x_num, y_num, bisect_verts, total_path_verts;
	float	bisect_w_y, bisect_h_y, bisect_w_x, bisect_h_x, height,
		thread_r_y, thread_r_x;
};