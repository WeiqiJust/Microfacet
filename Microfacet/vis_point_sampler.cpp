#include "vis_point_sampler.h"
#include "microfacet_factory.h"

struct Vector4
{
	float x, y, z, w;
};

vis_point_sampler::vis_point_sampler()
	:p_screen(NULL), p_shadow(NULL), p_pos(NULL), p_normal(NULL), p_output(NULL),
	p_sh_srv(NULL)
{

}

vis_point_sampler::~vis_point_sampler()
{
	release();
}

void vis_point_sampler::release()
{
	SAFE_DELETE(p_screen);
	SAFE_DELETE(p_shadow);
	SAFE_DELETE(p_pos);
	SAFE_DELETE(p_normal);
	SAFE_DELETE(p_output);
	SAFE_DELETE_ARR(p_sh_srv);
}

void vis_point_sampler::init(D3D_dev_handle *pdev, const int num_views)
{
	vis.resize(VIS_POINT_BUFFER_DIM*VIS_POINT_BUFFER_DIM);

	p_screen = new render_output(pdev);
	p_screen->init(VIS_POINT_BUFFER_DIM, VIS_POINT_BUFFER_DIM, 1);
	p_output = new render_target(pdev);
	p_output->init(VIS_POINT_BUFFER_DIM, VIS_POINT_BUFFER_DIM, 1, 1,
		DXGI_FORMAT_R8_UINT, R_TARGET_CPU_READ);

	p_screen->clear_mrt();
	p_screen->add_rt(p_output);

	p_shadow = new r_shadowmap();
	p_shadow->init(pdev, mff_singleton::get()->get_sampler("shadow"),
		num_views, VIS_POINT_SHADOW_DIM,
		DXGI_FORMAT_R32_FLOAT, 2.5);

	p_sh_srv = new ID3D11ShaderResourceView*[num_views];
	for (int i = 0; i < num_views; i++)
		p_sh_srv[i] = p_shadow->get_srv(i);

	p_pos = new render_target(pdev);
	p_pos->init(VIS_POINT_BUFFER_DIM, VIS_POINT_BUFFER_DIM, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT, R_TARGET_NO_RTV | R_TARGET_CREATE_SRV | R_TARGET_CPU_WRITE);
	p_normal = new render_target(pdev);
	p_normal->init(VIS_POINT_BUFFER_DIM, VIS_POINT_BUFFER_DIM, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT, R_TARGET_NO_RTV | R_TARGET_CREATE_SRV | R_TARGET_CPU_WRITE);
}

void vis_point_sampler::set_views(const std::vector<Vector3> &views)
{
	view_samples = views;

	std::vector<r_light_dir> lights;
	lights.resize(view_samples.size());
	for (int i = 0; i < view_samples.size(); i++)
		lights[i].dir = view_samples[i];
	p_shadow->set_lights(lights);
}

void vis_point_sampler::set_scene_center(const Vector3 &center)
{
	p_shadow->compute_light_matrix(Vector3(center.x, center.y, center.z));
}

int vis_point_sampler::num_views() const
{
	return view_samples.size();
}

void vis_point_sampler::begin_gen_shadow(const int idx, Matrix4 &matLight)
{
	p_shadow->set_gen_shadow(idx);
	matLight = p_shadow->mat_light_proj * p_shadow->mat_light_view[idx];
}

void vis_point_sampler::end_gen_shadow(const int idx)
{
	p_shadow->unset(idx, NULL);
}

void vis_point_sampler::sample_vis_points(std::vector<Vector3> &output_p,
	std::vector<normal_weight> &output_n,
	const int num_points,
	const std::vector<Vector3> &input_p,
	const std::vector<normal_weight> &input_n)
{
	//codex::utils::timer t;
	p_pos->set_content((BYTE*)&input_p[0], input_p.size(), 4 * 4);
	p_normal->set_content((BYTE*)&input_n[0], input_n.size(), 4 * 4);
	//t.update();
	//printf_s("%gsecs\n", t.elapsed_time());

	r_dsstate *p_ds = mff_singleton::get()->get_ds("no");
	r_shader_vis_point *p_sh = (r_shader_vis_point*)mff_singleton::get()->get_shader("vis_point");
	r_samplerstate *p_ss = mff_singleton::get()->get_sampler("point");
	r_samplerstate *p_ss_sh = mff_singleton::get()->get_sampler("shadow");
	shared_ptr<r_geometry> p_geom = mff_singleton::get()->get_geom("scr_quad");

	mff_singleton::get()->get_layer("vis_point")->set();
	p_ds->set();
	p_screen->set();

	p_sh->set_ps_resource(0, p_pos->get_target_srv());
	p_sh->set_ps_resource(1, p_normal->get_target_srv());
	p_sh->set_ps_resource(2, *p_sh_srv, view_samples.size());
	p_sh->set_ps_sampler(0, p_ss->get_state());
	p_sh->set_ps_sampler(1, p_ss_sh->get_state());

	p_sh->setup_cb_lights(view_samples.size(), &view_samples[0], p_shadow->mat_light_proj, &p_shadow->mat_light_view[0]);
	p_sh->setup_render();
	p_geom->set();
	p_geom->draw();

	ID3D11ShaderResourceView* nullViews[MAX_VIS_SAMPLES] = { 0 };
	p_sh->set_ps_resource(0, nullViews[0]);
	p_sh->set_ps_resource(1, nullViews[0]);
	p_sh->set_ps_resource(2, nullViews[0], view_samples.size());

	p_screen->unset();
	p_ds->unset();
	//t.update();
	//printf_s("%gsecs\n", t.elapsed_time());

	output_p.clear();
	output_n.clear();

	p_output->get_result(&vis[0], 1);
	//t.update();
	//printf_s("%gsecs\n", t.elapsed_time());

	int num_vis = 0;
	for (int i = 0; i < num_points; i++)
		if (vis[i] != 0)
			num_vis++;
	output_p.resize(num_vis);
	output_n.resize(num_vis);

	int ii = 0;
	for (int i = 0; i < num_points; i++)
		if (vis[i] != 0)
		{
			output_p[ii] = input_p[i];
			output_n[ii] = input_n[i];
			ii++;
		}
	//t.update();
	//printf_s("%gsecs\n", t.elapsed_time());
}

void vis_point_sampler::compute_vis_masks(const std::vector<sample_group> &samples,
	const int shadow_start,
	const int shadow_size)
{
	p_pos->map();
	for (int i = 0; i < samples.size(); i++)
		if (samples[i].p.size() > 0)
		{
			vector<Vector4> vec;
			for (int j = 0; j < samples[i].p.size(); j++)
			{
				Vector4 tempt;
				tempt.x = samples[i].p[j].x;
				tempt.y = samples[i].p[j].y;
				tempt.z = samples[i].p[j].z;
				tempt.w = 1.0f;
				vec.push_back(tempt);
			}
			p_pos->append((BYTE*)&vec[0], vec.size(), 4 * 4);
			//p_pos->append((BYTE*)&samples[i].p[0], samples[i].p.size(), 4 * 3);
		}
	p_pos->unmap();

	p_normal->map();
	for (int i = 0; i < samples.size(); i++)
		if (samples[i].n.size() > 0)
		{
			p_normal->append((BYTE*)&samples[i].n[0], samples[i].n.size(), 4 * 4);
		}
	p_normal->unmap();

	r_dsstate *p_ds = mff_singleton::get()->get_ds("no");
	r_shader_vis_point *p_sh = (r_shader_vis_point*)mff_singleton::get()->get_shader("vis_point_mask");
	r_samplerstate *p_ss = mff_singleton::get()->get_sampler("point");
	r_samplerstate *p_ss_sh = mff_singleton::get()->get_sampler("shadow");
	shared_ptr<r_geometry> p_geom = mff_singleton::get()->get_geom("scr_quad");

	mff_singleton::get()->get_layer("vis_point")->set();
	p_ds->set();
	p_screen->set();

	p_sh->set_ps_resource(0, p_pos->get_target_srv());
	p_sh->set_ps_resource(1, p_normal->get_target_srv());
	p_sh->set_ps_resource(2, p_sh_srv[shadow_start], shadow_size);
	p_sh->set_ps_sampler(0, p_ss->get_state());
	p_sh->set_ps_sampler(1, p_ss_sh->get_state());

	p_sh->setup_cb_lights(shadow_size, &view_samples[shadow_start], p_shadow->mat_light_proj, &p_shadow->mat_light_view[shadow_start]);
	p_sh->setup_render();
	p_geom->set();
	p_geom->draw();

	ID3D11ShaderResourceView* nullViews[MAX_VIS_SAMPLES] = { 0 };
	p_sh->set_ps_resource(0, nullViews[0]);
	p_sh->set_ps_resource(1, nullViews[0]);
	p_sh->set_ps_resource(2, nullViews[0], shadow_size);

	p_screen->unset();
	p_ds->unset();
}

void vis_point_sampler::get_vis_masks(BYTE *result, const int num_points)
{
	p_output->get_result(&vis[0], 1);
	memcpy(result, &vis[0], num_points);
}