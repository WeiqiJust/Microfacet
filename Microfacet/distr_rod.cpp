#include "distr_rod.h"
#include "hammersley.h"

void distr_rod::set_param(const distr_rod_param &p)
{
	param = p;
}

void* distr_rod::get_param()
{
	return &param;
}

void distr_rod::generate(std::vector<instance_property*> &results,
	const Vector3 &v_min, const Vector3 &v_max,
	const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	int sz = (v_max.x - v_min.x)*(v_max.y - v_min.y)*param.density;

	hammersley seq(sz, 2);
	random rng;

	double	phi = (param.pd_phi / 360) * 2 * PI,
		theta = (param.pd_theta / 90)*PI / 2;
	Vector3 pd, pd_t, pd_b;
	pd.x = cos(phi)*sin(theta);
	pd.y = sin(phi)*sin(theta);
	pd.z = cos(theta);
	build_frame(pd_t, pd_b, pd);

	for (int i = 0; i < sz; i++)
	{
		instance_property *p = new instance_property;

		Vector2 rv, v;
		float* samples = seq.get_sample();
		rv = Vector2(samples[0], samples[1]);
		uniform_disk_sampling(v, Vector2(rng.get_random_float(), rng.get_random_float()));
		v = v*param.randomness;

		Vector3 n, t, b;
		n.x = v.x;
		n.y = v.y;
		n.z = 1;
		n = Normalize(n);
		n = n.x*pd_t + n.y*pd_b + n.z*pd;
		build_frame(t, b, n);

		p->id = M_ID_TEST_ROD;
		p->setup_matrix(
			Vector3(v_min.x + rv.x*(v_max.x - v_min.x), v_min.y + rv.y*(v_max.y - v_min.y), 0),
			t, b, n, param.scale);
		results.push_back(p);
	}
}
