#include "r_shadowmap.h"
r_shadowmap::r_shadowmap()
	:p_samp_shadow(NULL), p_output(NULL)
{
}

r_shadowmap::~r_shadowmap()
{
	for (int i = 0; i < p_depth_map.size(); i++)
		SAFE_DELETE(p_depth_map[i]);
	SAFE_DELETE(p_output);
}

void r_shadowmap::init(D3D_dev_handle *pdev,
	r_samplerstate *p_samp,
	const int num_shadow_buffers,
	const int dim,
	const DXGI_FORMAT format,
	const float &s_radius)
{
	p_samp_shadow = p_samp;
	scene_radius = s_radius;

	p_output = new render_output(pdev);
	p_output->init(dim, dim, 1);

	p_depth_map.resize(num_shadow_buffers);
	for (int i = 0; i < p_depth_map.size(); i++)
	{
		p_depth_map[i] = new render_target(pdev);
		p_depth_map[i]->init(dim, dim, 1, 1, format,
			//R_OUTPUT_CPU|
			R_TARGET_CREATE_SRV);
	}

	float	z_near = 1e-3f,
		z_far = s_radius * 2;
	/***********test code***********/
	////projection_orthogonal(mat_light_proj, scene_radius, scene_radius, -z_near, -z_far);  // Weiqi: change orthogonal projection
	projection_orthogonal(mat_light_proj, scene_radius, scene_radius, z_near, z_far);

}

void r_shadowmap::set_lights(const std::vector<r_light_dir> &lts)
{
	lights = lts;
}

void r_shadowmap::compute_light_matrix(const Vector3 &s_center)
{
	mat_light_view.resize(lights.size());
	for (int i = 0; i < lights.size(); i++)
	{
		Vector3 n(-lights[i].dir.x, -lights[i].dir.y, -lights[i].dir.z), t, b;
		build_frame(t, b, n);
		matrix_lookat(mat_light_view[i], s_center - n*scene_radius, s_center, b);
	}
}

float depth[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void r_shadowmap::set_gen_shadow(const int idx)
{
	p_depth_map[idx]->clear(depth);

	p_output->clear_mrt();
	p_output->add_rt(p_depth_map[idx]);
	p_output->clear_depth();
	p_output->set();
}

void r_shadowmap::set_use_shadow(const int idx, r_shader *sh)
{
	sh->set_ps_resource(0, p_depth_map[idx]->get_target_srv());
	sh->set_ps_sampler(0, p_samp_shadow->get_state());
}

void r_shadowmap::unset(const int idx, r_shader *sh)
{
	p_output->unset();
	if (sh)
	{
		ID3D11ShaderResourceView* nullViews[] = { 0 };
		sh->set_ps_resource(0, nullViews[0]);
	}
}

ID3D11ShaderResourceView* r_shadowmap::get_srv(const int idx)
{
	return p_depth_map[idx]->get_target_srv();
}
