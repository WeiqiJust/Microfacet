#pragma once
#include "binder.h"

class binder_woven : public microfacet_binder
{
protected:
	binder_woven_param param;

public:
	void set_param(const binder_woven_param &p);
	virtual void* get_param();

	virtual void generate_geom(const float density, const int num_area_hits_scalar);
	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors);
};