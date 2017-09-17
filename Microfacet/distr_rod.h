#pragma once
#include "distr.h"

class distr_rod : public microfacet_distr
{
private:
	distr_rod_param param;

public:
	void set_param(const distr_rod_param &p);
	virtual void* get_param();

	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors);
};