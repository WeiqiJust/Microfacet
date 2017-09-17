#include "distr_grid.h"

void distr_grid::set_param(const distr_grid_param &p)
{
	param = p;
}

void* distr_grid::get_param()
{
	return &param;
}

void distr_grid::generate(std::vector<instance_property*> &results,
	const Vector3 &v_min, const Vector3 &v_max,
	const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	int x1 = int(v_min.x / param.x_space) + 1,
		x2 = int(v_max.x / param.x_space),
		y1 = int(v_min.y / param.y_space) + 1,
		y2 = int(v_max.y / param.y_space);//,
	//zz = int((v_max.z-v_min.z-param.height-param.scale*2)/param.z_space) - 1;

	for (int z = 0; z <= 0; z++)
		for (int y = y1; y <= y2; y++)
			for (int x = x1; x <= x2; x++)
			{
				instance_property *p = new instance_property;
				p->id = M_ID_TEST_PARTICLE;
				p->setup_matrix(Vector3((x - 0.5)*param.x_space, (y - 0.5)*param.y_space, v_min.z + param.height + param.scale + z*param.z_space),
					Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), param.scale);
				results.push_back(p);
			}
}