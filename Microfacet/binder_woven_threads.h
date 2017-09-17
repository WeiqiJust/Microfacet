#pragma once
#include "binder.h"
#include "binder_woven.h"

class binder_woven_threads : public microfacet_binder
{
private:
	std::vector<Vector3> disp_x, disp_y;
	float find_equal_dist_point(const float theta,
		const float a,
		const float b,
		const float dist);

protected:
	binder_woven_threads_param param;

public:
	void set_param(const binder_woven_threads_param &p);
	virtual void* get_param();

	virtual void generate_geom(const float density, const int num_area_hits_scalar);
	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors);
};