#pragma once
#include "distr.h"



class distr_low_discrepancy : public microfacet_distr
{
private:
	distr_low_discrepancy_param param;

public:
	void set_param(const distr_low_discrepancy_param &p);
	virtual void* get_param();

	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors);
};

class distr_low_discrepancy_3d : public microfacet_distr
{
private:
	distr_low_discrepancy_3d_param param;

public:
	void set_param(const distr_low_discrepancy_3d_param &p);
	virtual void* get_param();

	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors);
};