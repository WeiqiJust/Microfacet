#include "distr_low_discrepancy.h"
#include "hammersley.h"

void distr_low_discrepancy::set_param(const distr_low_discrepancy_param &p)
{
	param = p;
}

void* distr_low_discrepancy::get_param()
{
	return &param;
}

void distr_low_discrepancy::generate(std::vector<instance_property*> &results,
	const Vector3 &v_min, const Vector3 &v_max,
	const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	int sz = (v_max.x - v_min.x)*(v_max.y - v_min.y)*param.density;

	hammersley seq(sz);
	for (int i = 0; i < sz; i++)
	{
		instance_property *p = new instance_property;
		
		float* result = seq.get_sample(2);
		Vector2 rv(result[0], result[1]);
		p->id = M_ID_TEST_PARTICLE;
		p->setup_matrix(
			Vector3(v_min.x + rv.x*(v_max.x - v_min.x),
			v_min.y + rv.y*(v_max.y - v_min.y),
			v_min.z + param.scale + param.height),
			Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), param.scale);
		results.push_back(p);
	}
}

void distr_low_discrepancy_3d::set_param(const distr_low_discrepancy_3d_param &p)
{
	param = p;
}

void* distr_low_discrepancy_3d::get_param()
{
	return &param;
}

void distr_low_discrepancy_3d::generate(std::vector<instance_property*> &results,
	const Vector3 &v_min, const Vector3 &v_max,
	const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	float avg_dim = (v_max.x - v_min.x + v_max.y - v_min.y) / 2;
	int sz = ceil((v_max.x - v_min.x)*(v_max.y - v_min.y)*avg_dim*param.relative_height_density*param.density);
	float height_den = (v_max.z - v_min.z - param.scale * 2 - param.height) / avg_dim*param.relative_height_density;

	hammersley seq(sz);

	for (int i = 0; i < sz; i++)
	{
		instance_property *p = new instance_property;

		
		float* result = seq.get_sample(3);
		Vector3 rv(result[0], result[1], result[2]);
		p->id = M_ID_TEST_PARTICLE;
		p->setup_matrix(
			Vector3(v_min.x + rv.x*(v_max.x - v_min.x),
			v_min.y + rv.y*(v_max.y - v_min.y),
			v_min.z + param.scale + param.height + rv.z*(v_max.z - v_min.z - param.scale * 2 - param.height)),
			Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), param.scale);
		results.push_back(p);
	}
}