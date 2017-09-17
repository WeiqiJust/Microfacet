#include "worker.h"

// How it works
// For each pixel,
// 1. Compute visible samples for the current wo. (generate normal and id)
// 2. Compute shadow info for visible samples.
// 3. Light-up each sample using BRDFs.

#define GROUND_TRUTH_SCENE_R	1.73f

float gen_pixel(Vector3 &result,
			   task_render_truth &t,
			   const Vector3 &tangent,
			   const Vector3 &binormal,
			   const Vector3 &normal,
			   const Vector3 &global_wo,
			   const int block_idx,
			   r_shadowmap *p_shadow,
			   render_output *p_screen,
			   render_target *p_target_id,
			   float *p_id,
			   render_target *p_target_normal,
			   short *p_normal,
			   render_target *p_target_vis,
			   float *p_vis,
			   const std::vector<r_light_dir> &global_lights,
			   const std::vector<bool> vis_wi,
			   bool b_debug
			   )
{
	int block_x, block_y;
	t.details->compute_back_idx(block_x, block_y, block_idx);

	Matrix4	matView, matProj, matProjView;	
	Vector3	wo(global_wo*tangent, global_wo*binormal, global_wo*normal);

	if (b_debug)
	{
		printf_s("wo (%g %g %g)\n", wo.x, wo.y, wo.z);
	}
	
	Vector3		center(block_x+0.5f, block_y+0.5f, 0), 
		vwo(wo.x, wo.y, wo.z);
	matrix_lookat(matView, center+vwo*3.0, center, Vector3(0, 1, 0));
	float	z_near= 1e-5*GROUND_TRUTH_SCENE_R, 
			z_far = 5.0;
	projection_orthogonal(matProj,  GROUND_TRUTH_SCENE_R, GROUND_TRUTH_SCENE_R, -z_near, -z_far);
	matProjView = matProj*matView;

	float clear_color[4] = {0, 0, 0, 0};
	std::vector<int> addr;
	//**************************
	//#1 : Context pass (To generate id, and normal buffers)
	//**************************
	r_shader_vis	*p_sh_context = (r_shader_vis*)mff_singleton::get()->get_shader("ground_truth");

	p_screen->clear_mrt();
	p_screen->add_rt(p_target_id);
	p_screen->add_rt(p_target_normal);
	p_screen->clear_depth();
	p_screen->set();
	p_screen->get_rt(0)->clear(clear_color);

	mff_singleton::get()->get_layer("basic")->set();

	microfacet_block &block = (*t.blocks)[block_idx];
	for (int i = 0; i < block.per_mat_groups.insts.size(); i++)
	{
		int id = block.per_mat_groups.get_id(i);
		r_light_dir temp;

		p_sh_context->setup_cb_lights(&temp, id, &matProjView);
		for (int j = 0; j < block.per_mat_groups.insts[i].size(); j++)
		{
			p_sh_context->set_shader_input(block.per_mat_groups.insts[i][j]->get_shader_input(), (void*)&matProjView);
			block.per_mat_groups.insts[i][j]->draw_with_shader(p_sh_context);
		}
	}

	r_light_dir temp_light;
	p_sh_context->setup_cb_lights(&temp_light, 0, &matProjView);
	for (int i = 1; i < block.neighbors_and_self.size(); i++)
		block.neighbors_and_self[i]->draw_with_shader(p_sh_context, &matProjView);

	p_screen->get_rt(0)->get_result((BYTE*)p_id, 4);
	p_screen->get_rt(1)->get_result((BYTE*)p_normal, 8);
	for (int i = 0; i < DIM_GROUND_TRUTH*DIM_GROUND_TRUTH; i++)
		if (p_id[i] != 0.0f)
			addr.push_back(i);

	//**************************
	//#2 : Shadow and Shading pass
	//**************************
	p_screen->clear_mrt();
	p_screen->add_rt(p_target_vis);
	//p_screen->clear_depth();

	std::vector<r_light_dir> lights;
	for (int i = 0; i < global_lights.size(); i++)
	{
		r_light_dir lt;
		Vector3 dir = global_lights[i].dir;
		lt.dir	= Vector3(dir*tangent, dir*binormal, dir*normal);
		lt.c	= global_lights[i].c;
		lights.push_back(lt);

		//DEBUG
		//printf_s("wi (%g %g %g)\n", lt.dir.x, lt.dir.y, lt.dir.z);
	}	
	p_shadow->set_lights(lights);
	p_shadow->compute_light_matrix(center);

	r_shader_basic	*p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
	r_shader_vis	*p_sh_use_shadow = (r_shader_vis*)mff_singleton::get()->get_shader("init_shadow");

	std::vector<float> img_dbg, n_distr;
	preconv_matr *p_basis_matr = mff_singleton::get()->get_basis_matr(BASIS_MATR_LAMBERTIAN);
	if (b_debug)
	{
		img_dbg.resize(DIM_GROUND_TRUTH*DIM_GROUND_TRUTH*3, 0.5f);
		for (int i = 0; i < addr.size(); i++)
			img_dbg[addr[i]*3+0] =
			img_dbg[addr[i]*3+1] =
			img_dbg[addr[i]*3+2] = 0.0f;
		n_distr.resize(p_basis_matr->fr_n.get_back_idx().size()*3, 0.0f);
	}

	mff_singleton::get()->get_layer("init_shadow")->set();
	for (int i = 0; i < lights.size(); i++)
		if (vis_wi[i])
		{
			Vector3 wi = lights[i].dir;

			if (wi.z > 0)
			{
				p_screen->get_rt(0)->clear(clear_color);
				p_screen->clear_depth();
				//gen shadow
				Matrix4 matLightProjView = p_shadow->mat_light_proj * p_shadow->mat_light_view[i];
				p_shadow->set_gen_shadow(0);
				for (int j = 0; j < block.neighbors_and_self.size(); j++)
				{
					block.neighbors_and_self[j]->draw_with_shader(p_sh_shadow, &matLightProjView);
				}
				p_shadow->unset(0, NULL);

				//use shadow
				int vis_mask = 1;
				p_sh_use_shadow->setup_cb_lights(&p_shadow->lights[i], vis_mask, &matLightProjView);
				p_shadow->set_use_shadow(0, p_sh_use_shadow);
				p_screen->set();
				block.draw_with_shader(p_sh_use_shadow, &matProjView);
				p_shadow->unset(0, p_sh_shadow);

				p_screen->get_rt(0)->get_result((BYTE*)p_vis, 4);

				//shading
				for (int j = 0; j < addr.size(); j++)
				{
					int idx = addr[j];
					if (p_vis[idx] != 0.0f)
					{
						//if (b_debug)
						//	printf_s("[#%d] ", i);

						matr* p_matr = mff_singleton::get()->get_matr((int)p_id[idx]);
						preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

						Vector3 n;
						n.x	= snorm2float(p_normal[idx*4]);
						n.y	= snorm2float(p_normal[idx*4+1]);
						n.z	= snorm2float(p_normal[idx*4+2]);
						n.normalize();
						//printf_s("n %g %g %g wi*n(%g) wo*n(%g)\n", n.x, n.y, n.z, wi*n, wo*n);

						Vector3 refl;
						basis_matr->pBRDF->sample(refl, wi, wo, n);
						
						if (b_debug && idx == 56+51*256)
							printf_s("n(%g %g %g)\nbrdf(%g %g %g)\nalbedo(%g %g %g)\nn*wo(%g)\nlight(%g %g %g)\n", 
								n.x, n.y, n.z,
								refl.x, refl.y, refl.z, p_matr->albedo.x, p_matr->albedo.y, p_matr->albedo.z, n*wo,
								lights[i].c.x, lights[i].c.y, lights[i].c.z);

						refl.x *= p_matr->albedo.x;
						refl.y *= p_matr->albedo.y;
						refl.z *= p_matr->albedo.z;
#ifdef LOG_MATERIAL
						refl.x = exp(refl.x)-1.0f;
						refl.y = exp(refl.y)-1.0f;
						refl.z = exp(refl.z)-1.0f;
#endif // LOG_MATERIAL
						refl *= basis_matr->inv_avg/(n*wo);
						Vector3 c;
						c.x = refl.x*lights[i].c.x;
						c.y = refl.y*lights[i].c.y;
						c.z = refl.z*lights[i].c.z;
						result += c;

						if (b_debug)
						{
							if (idx == 56+51*256)
								printf_s("final(%g %g %g)\n", c.x, c.y, c.z);

							img_dbg[idx*3+0] += c.x;
							img_dbg[idx*3+1] += c.y;
							img_dbg[idx*3+2] += c.z;
							//img_dbg[idx*3+0] = (n.x+1)/2;
							//img_dbg[idx*3+1] = (n.y+1)/2;
							//img_dbg[idx*3+2] = (n.z+1)/2;

							if (refl.LengthSquared() > 0.0f)
							{
								int idx = p_basis_matr->fr_n.map(n);
								n_distr[idx*3+0] += p_matr->albedo.x;
								n_distr[idx*3+1] += p_matr->albedo.y;
								n_distr[idx*3+2] += p_matr->albedo.z;
							}
						}
					}
				}
			}
		}
	result /= addr.size();

	float area = GROUND_TRUTH_SCENE_R*GROUND_TRUTH_SCENE_R/(DIM_GROUND_TRUTH*DIM_GROUND_TRUTH)*addr.size();
	if (b_debug)
	{
		printf_s("area = %g\n", area);
		save_image_color("d:/temp/Microfacet/debug_pixel.png", img_dbg, DIM_GROUND_TRUTH, DIM_GROUND_TRUTH);

		float scalar = GROUND_TRUTH_SCENE_R*GROUND_TRUTH_SCENE_R/(DIM_GROUND_TRUTH*DIM_GROUND_TRUTH)*128;

		std::vector<float> img;
		img.resize(p_basis_matr->fr_n.get_idx().size()*3, 0.5);
		for (int i = 0; i < p_basis_matr->fr_n.get_idx().size(); i++)
		{
			int ii = p_basis_matr->fr_n.get_idx()[i];
			if (ii >= 0)
			{
				img[i*3+0] = n_distr[ii*3+0]*scalar;
				img[i*3+1] = n_distr[ii*3+1]*scalar;
				img[i*3+2] = n_distr[ii*3+2]*scalar;
			}
		}
		save_image_color("d:/temp/Microfacet/avg_n_truth.png", img, 
			p_basis_matr->fr_n.get_actual_dim(), p_basis_matr->fr_n.get_actual_dim()*2);

		printf_s("result(lighted) : %g %g %g\n", result.x, result.y, result.z);
	}
	return area;
}

void worker_microfacet::work_task_ground_truth_direct(task_microfacet *t_org)
{
	task_render_truth &t = t_org->trt;
	Vector3 v_right = Cross((*t.v_up), (*t.v_lookat));
	v_right.normalize();

	short *p_normal;
	float *p_vis, *p_id;
	p_normal	= new short[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH*4];
	p_vis		= new float[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH];
	p_id		= new float[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH];

	render_target	*p_target_normal,
		*p_target_vis,
		*p_target_id;
	p_target_normal = new render_target(mff_singleton::get()->get_handle());
	p_target_normal->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R16G16B16A16_SNORM, R_TARGET_CPU_READ);
	p_target_vis = new render_target(mff_singleton::get()->get_handle());
	p_target_vis->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);
	p_target_id = new render_target(mff_singleton::get()->get_handle());
	p_target_id->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);

	render_output *p_screen;
	p_screen = new render_output(mff_singleton::get()->get_handle());
	p_screen->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1);

	r_shadowmap shadowmap;
	shadowmap.init(mff_singleton::get()->get_handle(), mff_singleton::get()->get_sampler("shadow"), 
		1, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT, GROUND_TRUTH_SCENE_R);

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;
	//int num_area;
	//int idx_area[4];
	//float weight_area[4];
	for (int y = 0; y < t_org->height; y++)
		for (int x = 0; x < t_org->width; x++)
		{
			int		idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf_s("(%d/%d) ", x+y*t_org->width+1, t_org->width*t_org->height);

				//vector2 uv;
				Vector3 normal, tangent, binormal, global_wo;

				//uv.x = t.p_uv[idx*2];
				//uv.y = t.p_uv[idx*2+1];

				normal.x	= snorm2float(t.p_normal[idx*4]);
				normal.y	= snorm2float(t.p_normal[idx*4+1]);
				normal.z	= snorm2float(t.p_normal[idx*4+2]);
				tangent.x	= snorm2float(t.p_tangent[idx*4]);
				tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
				tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

				binormal	= Cross(normal, tangent);
				global_wo	=	(*t.p_wo)[idx].x*v_right +
					(*t.p_wo)[idx].y*(*t.v_up) +
					(*t.p_wo)[idx].z*(*t.v_lookat);

				//FIX ME: huge bug here!
				int idx_block = t.details->compute_idx(0, 0);

				std::vector<bool> vis_wi;

				for (int batch = 0; batch < t.vis_pixel_size; batch++)
				{
					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
					for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
						if (mask & (1<<l))
						{
							vis_wi.push_back(true);
						} else {
							vis_wi.push_back(false);
						}
				}

				Vector3 c;
				float area = gen_pixel(c, t, tangent, binormal, normal, global_wo, idx_block, 
					&shadowmap, p_screen, p_target_id, p_id, p_target_normal, p_normal, p_target_vis, p_vis, lights, vis_wi,
					false);

				//DEBUG
				//c = shadowmap.lights[0].c/10;
				//c /= area;
				//t.fr_Avis->lerp(num_area, idx_area, weight_area, wo, false, false);
				//c /= (*t.blocks)[idx_block].Avis.get_area(num_area, idx_area, weight_area);

				BYTE r, g, b;
				r = min(max(c.x, 0), 1)*255;
				g = min(max(c.y, 0), 1)*255;
				b = min(max(c.z, 0), 1)*255;
				UINT pixel = (r << 16) + (g << 8) + (b);
				t.result[idx] = pixel;
			}
		}
	printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf_s("done\n");

	SAFE_DELETE_ARR(p_normal);
	SAFE_DELETE_ARR(p_vis);
	SAFE_DELETE_ARR(p_id);
	SAFE_DELETE(p_target_normal);
	SAFE_DELETE(p_target_vis);
	SAFE_DELETE(p_target_id);
	SAFE_DELETE(p_screen);
}

void worker_microfacet::work_task_ground_truth_debug_pixel(task_microfacet *t_org)
{
	task_render_truth &t = t_org->trt;
	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	short *p_normal;
	float *p_vis, *p_id;
	p_normal	= new short[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH*4];
	p_vis		= new float[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH];
	p_id		= new float[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH];

	render_target	*p_target_normal,
		*p_target_vis,
		*p_target_id;
	p_target_normal = new render_target(mff_singleton::get()->get_handle());
	p_target_normal->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R16G16B16A16_SNORM, R_TARGET_CPU_READ);
	p_target_vis = new render_target(mff_singleton::get()->get_handle());
	p_target_vis->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);
	p_target_id = new render_target(mff_singleton::get()->get_handle());
	p_target_id->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);

	render_output *p_screen;
	p_screen = new render_output(mff_singleton::get()->get_handle());
	p_screen->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1);

	r_shadowmap shadowmap;
	shadowmap.init(mff_singleton::get()->get_handle(), mff_singleton::get()->get_sampler("shadow"), 
		1, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT, GROUND_TRUTH_SCENE_R);

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;
	//int num_area;
	//int idx_area[4];
	//float weight_area[4];
	int x = t.dbg_pixel_x;
	int y = t.dbg_pixel_y;
	int	idx = x+y*t_org->width;

	if (t.p_uv[idx*2] >= 0)
	{
		//vector2 uv;
		Vector3 normal, tangent, binormal, global_wo;

		//uv.x = t.p_uv[idx*2];
		//uv.y = t.p_uv[idx*2+1];

		normal.x	= snorm2float(t.p_normal[idx*4]);
		normal.y	= snorm2float(t.p_normal[idx*4+1]);
		normal.z	= snorm2float(t.p_normal[idx*4+2]);
		tangent.x	= snorm2float(t.p_tangent[idx*4]);
		tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
		tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

		binormal	= normal ^ tangent;
		global_wo	=	(*t.p_wo)[idx].x*v_right +
						(*t.p_wo)[idx].y*(*t.v_up) +
						(*t.p_wo)[idx].z*(*t.v_lookat);
		//DEBUG
		//printf_s("gwo %g %g %g\n", global_wo.x, global_wo.y, global_wo.z);

		//FIX ME: huge bug here!
		int idx_block = t.details->compute_idx(0, 0);

		std::vector<bool> vis_wi;

		for (int batch = 0; batch < t.vis_pixel_size; batch++)
		{
			int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
			for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
				if (mask & (1<<l))
				{
					vis_wi.push_back(true);
				} else {
					vis_wi.push_back(false);
				}
		}

		Vector3 c;
		gen_pixel(c, t, tangent, binormal, normal, global_wo, idx_block, 
			&shadowmap, p_screen, p_target_id, p_id, p_target_normal, p_normal, p_target_vis, p_vis, lights, vis_wi,
			true);

		//t.fr_Avis->lerp(num_area, idx_area, weight_area, wo, false, false);
		//c /= (*t.blocks)[idx_block].Avis.get_area(num_area, idx_area, weight_area);

		BYTE r, g, b;
		r = min(max(c.x, 0), 1)*255;
		g = min(max(c.y, 0), 1)*255;
		b = min(max(c.z, 0), 1)*255;
		UINT pixel = (r << 16) + (g << 8) + (b);
		t.result[idx] = pixel;
	}

	SAFE_DELETE_ARR(p_normal);
	SAFE_DELETE_ARR(p_vis);
	SAFE_DELETE_ARR(p_id);
	SAFE_DELETE(p_target_normal);
	SAFE_DELETE(p_target_vis);
	SAFE_DELETE(p_target_id);
	SAFE_DELETE(p_screen);
}

void worker_microfacet::work_task_render_debug(task_microfacet *t_org)
{
	task_render &t = t_org->r;

	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	//codex::utils::timer tm;
	//tm.update();

	printf_s("************************\n");
	int y = t.dbg_pixel_y;
	int x = t.dbg_pixel_x;
	{
		int	idx = x+y*t_org->width;
		if (t.p_uv[idx*2] >= 0)
		{
			//vector2 uv;
			Vector3 normal, tangent, binormal, global_wo;

			//uv.x = t.p_uv[idx*2];
			//uv.y = t.p_uv[idx*2+1];

			normal.x	= snorm2float(t.p_normal[idx*4]);
			normal.y	= snorm2float(t.p_normal[idx*4+1]);
			normal.z	= snorm2float(t.p_normal[idx*4+2]);
			tangent.x	= snorm2float(t.p_tangent[idx*4]);
			tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
			tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

			//Normalization code. Should be redundant.
			//normal.normalize();
			//tangent -= (tangent*normal)*normal;
			//tangent.normalize();
			binormal	= normal ^ tangent;
			global_wo	=	(*t.p_wo)[idx].x*v_right +
							(*t.p_wo)[idx].y*(*t.v_up) +
							(*t.p_wo)[idx].z*(*t.v_lookat);
			////DEBUG
			//printf_s("normal (%g %g %g)\n", normal.x, normal.y, normal.z);
			//printf_s("tangent(%g %g %g)\n", tangent.x, tangent.y, tangent.z);
			//printf_s("gwo    (%g %g %g)\n", global_wo.x, global_wo.y, global_wo.z);

			Vector3 wo(global_wo*tangent, global_wo*binormal, global_wo*normal);
			if (wo.z < 0.02)
			{
				wo.z = 0.02;
				wo.normalize();
			}

			//DEBUG
			printf_s("wo    (%g %g %g)\n", wo.x, wo.y, wo.z);

			//FIX ME: huge bug here!
			int idx_block = t.details->compute_idx(0, 0);
			Vector3 c;
			for (int batch = 0; batch < t.vis_pixel_size; batch++)
			{
				int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
				for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
					if (mask & (1<<l))
					{
						float dot = t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir * normal;
						if (dot > 0)
						{
							Vector3 wi(t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir*tangent, 
										t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir*binormal, 
										dot);

							Vector3 temp;
							//(*t.blocks)[idx_block].get_reflectance_debug(temp, wi, wo, *t.fr_vis, true);
							//(*t.blocks)[idx_block].get_reflectance_assembler(temp, wi, wo, *t.fr_vis, true);
							(*t.blocks)[idx_block].get_reflectance_BRDF(temp, wi, wo);
							c.x += temp.x*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.x;
							c.y += temp.y*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.y;
							c.z += temp.z*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.z;
							printf_s("resultBRDF: %g %g %g\n", temp.x, temp.y, temp.z);
							//printf_s("light     : %g %g %g\n", 
							//	t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.x,
							//	t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.y,
							//	t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.z);

							//DEBUG
							{
								Vector3 rwi, rwo;
								(*t.blocks)[idx_block].debug_BRDF_no_lerp(temp, rwi, rwo, wi, wo);
								printf_s("---------------------------\n");
								printf_s("wi : (%g %g %g)\n", rwi.x, rwi.y, rwi.z);
								printf_s("wo : (%g %g %g)\n", rwo.x, rwo.y, rwo.z);
								printf_s("c  : (%g %g %g)\n", temp.x, temp.y, temp.z);
								printf_s("---------------------------\n");
							}
						}
					}
			}

			int num_area;
			int idx_area[4];
			float weight_area[4];

			t.fr_Avis->lerp(num_area, idx_area, weight_area, wo, true, false);
			float area = (*t.blocks)[idx_block].Avis.get_area(num_area, idx_area, weight_area);
			c /= area;
			
			printf_s("area = %g\n", area);
			printf_s("result(lighted)= (%g %g %g)\n", c.x, c.y, c.z);

			BYTE r, g, b;
			r = min(max(c.x, 0), 1)*255;
			g = min(max(c.y, 0), 1)*255;
			b = min(max(c.z, 0), 1)*255;
			UINT pixel = (r << 16) + (g << 8) + (b);
			t.result[idx] = pixel;
		}
	}
	//tm.update();
	//printf_s("#3 : %7.4f\n", tm.elapsed_time());
	printf_s("work_task_render_debug done\n");
}

void worker_microfacet::work_task_render_before_SVD(task_microfacet *t_org)
{
	task_render &t = t_org->r;

	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	int num_area;
	int idx_area[4];
	float weight_area[4];
	for (int y = 0; y < t_org->height; y++)
		for (int x = 0; x < t_org->width; x++)
		{
			int		idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				//vector2 uv;
				Vector3 normal, tangent, binormal, global_wo;

				//uv.x = t.p_uv[idx*2];
				//uv.y = t.p_uv[idx*2+1];

				normal.x	= snorm2float(t.p_normal[idx*4]);
				normal.y	= snorm2float(t.p_normal[idx*4+1]);
				normal.z	= snorm2float(t.p_normal[idx*4+2]);
				tangent.x	= snorm2float(t.p_tangent[idx*4]);
				tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
				tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

				//Normalization code. Should be redundant.
				//normal.normalize();
				//tangent -= (tangent*normal)*normal;
				//tangent.normalize();
				binormal	= normal ^ tangent;
				global_wo	=	(*t.p_wo)[idx].x*v_right +
								(*t.p_wo)[idx].y*(*t.v_up) +
								(*t.p_wo)[idx].z*(*t.v_lookat);
				Vector3 wo(global_wo*tangent, global_wo*binormal, global_wo*normal);
				if (wo.z < 0)
				{
					wo.z = 0;
					wo.normalize();
				}

				//FIX ME: huge bug here!
				int idx_block = t.details->compute_idx(0, 0);
				Vector3 c;
				for (int batch = 0; batch < t.vis_pixel_size; batch++)
				{
					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
					for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
						if (mask & (1<<l))
						{
							float dot = t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir * normal;
							if (dot > 0)
							{
								Vector3 wi(t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir*tangent, 
									t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir*binormal, dot);

								Vector3 temp;
								//(*t.blocks)[idx_block].get_reflectance(temp, wi, wo, *t.fr_vis);
								//(*t.blocks)[idx_block].get_reflectance_debug(temp, wi, wo, *t.fr_vis, false);
								(*t.blocks)[idx_block].get_reflectance_assembler(temp, wi, wo, *t.fr_vis, false);
								c.x += temp.x*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.x;
								c.y += temp.y*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.y;
								c.z += temp.z*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.z;
							}
						}
				}

				t.fr_Avis->lerp(num_area, idx_area, weight_area, wo, true, false);
				c /= (*t.blocks)[idx_block].Avis.get_area(num_area, idx_area, weight_area);

				BYTE r, g, b;
				r = min(max(c.x, 0), 1)*255;
				g = min(max(c.y, 0), 1)*255;
				b = min(max(c.z, 0), 1)*255;
				UINT pixel = (r << 16) + (g << 8) + (b);
				t.result[idx] = pixel;
			}
		}
}


void worker_microfacet::work_task_render_ref_BRDF(task_microfacet *t_org)
{
	task_render_truth &t = t_org->trt;
	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	preconv_matr *spec_basis_matr = NULL;
	int idx_block = t.details->compute_idx(0, 0);
	for (int i = 0; i < (*t.blocks)[idx_block].per_mat_groups.insts.size(); i++)
	{
		int id = (*t.blocks)[idx_block].per_mat_groups.get_id(i);
		matr* p_matr = mff_singleton::get()->get_matr(id);
		preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

		if (spec_basis_matr == NULL ||
			basis_matr->fr_brdf.get_n().size() > spec_basis_matr->fr_brdf.get_n().size())
			spec_basis_matr = basis_matr;
	}

	BRDF_interface *pBRDF = (*t.blocks)[idx_block].BRDF_ref;

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;

	for (int y = 0; y < t_org->height; y++)
		for (int x = 0; x < t_org->width; x++)
		{
			int	idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf_s("(%d/%d) ", x+y*t_org->width+1, t_org->width*t_org->height);

				Vector3 normal, tangent, binormal, global_wo;
				normal.x	= snorm2float(t.p_normal[idx*4]);
				normal.y	= snorm2float(t.p_normal[idx*4+1]);
				normal.z	= snorm2float(t.p_normal[idx*4+2]);
				tangent.x	= snorm2float(t.p_tangent[idx*4]);
				tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
				tangent.z	= snorm2float(t.p_tangent[idx*4+2]);
				binormal	= normal ^ tangent;
				global_wo	=	(*t.p_wo)[idx].x*v_right +
								(*t.p_wo)[idx].y*(*t.v_up) +
								(*t.p_wo)[idx].z*(*t.v_lookat);

				std::vector<bool> vis_wi;
				for (int batch = 0; batch < t.vis_pixel_size; batch++)
				{
					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
					for (int l = 0; l < min(lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
						if (mask & (1<<l))
						{
							vis_wi.push_back(true);
						} else {
							vis_wi.push_back(false);
						}
				}

				Vector3 c;
				Vector3 wo(global_wo*tangent, global_wo*binormal, global_wo*normal);

				for (int l = 0; l < vis_wi.size(); l++)
					if (vis_wi[l])
					{
						Vector3 wi(lights[l].dir*tangent, lights[l].dir*binormal, lights[l].dir*normal);						
						Vector3 refl;
						pBRDF->sample(refl, wi, wo, Vector3(0, 0, 1));
#ifdef LOG_MATERIAL
						refl.x = exp(refl.x)-1.0f;
						refl.y = exp(refl.y)-1.0f;
						refl.z = exp(refl.z)-1.0f;
#endif // LOG_MATERIAL
						//refl *= spec_basis_matr->inv_avg/wo.z;
						refl /= wo.z;

						c.x += lights[l].c.x*refl.x;
						c.y += lights[l].c.y*refl.y;
						c.z += lights[l].c.z*refl.z;
					}

				BYTE r, g, b;
				r = min(max(c.x, 0), 1)*255;
				g = min(max(c.y, 0), 1)*255;
				b = min(max(c.z, 0), 1)*255;
				UINT pixel = (r << 16) + (g << 8) + (b);
				t.result[idx] = pixel;
			}
		}
	printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf_s("done\n");
}

void worker_microfacet::work_task_render_ref_BRDF_debug_pixel(task_microfacet *t_org)
{
	cout << "in render refere brdf" << endl;
	task_render_truth &t = t_org->trt;
	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	preconv_matr *spec_basis_matr = NULL;
	int idx_block = t.details->compute_idx(0, 0);
	for (int i = 0; i < (*t.blocks)[idx_block].per_mat_groups.insts.size(); i++)
	{
		int id = (*t.blocks)[idx_block].per_mat_groups.get_id(i);
		matr* p_matr = mff_singleton::get()->get_matr(id);
		preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

		if (spec_basis_matr == NULL ||
			basis_matr->fr_brdf.get_n().size() > spec_basis_matr->fr_brdf.get_n().size())
			spec_basis_matr = basis_matr;
	}

	BRDF_interface *pBRDF = (*t.blocks)[idx_block].BRDF_ref;

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;

	int y = t.dbg_pixel_y;
	int x = t.dbg_pixel_x;
	int	idx = x+y*t_org->width;

	if (t.p_uv[idx*2] >= 0)
	{
		printf_s("************************\n");

		Vector3 normal, tangent, binormal, global_wo;
		normal.x	= snorm2float(t.p_normal[idx*4]);
		normal.y	= snorm2float(t.p_normal[idx*4+1]);
		normal.z	= snorm2float(t.p_normal[idx*4+2]);
		tangent.x	= snorm2float(t.p_tangent[idx*4]);
		tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
		tangent.z	= snorm2float(t.p_tangent[idx*4+2]);
		binormal	= normal ^ tangent;
		global_wo	=	(*t.p_wo)[idx].x*v_right +
						(*t.p_wo)[idx].y*(*t.v_up) +
						(*t.p_wo)[idx].z*(*t.v_lookat);

		std::vector<bool> vis_wi;
		for (int batch = 0; batch < t.vis_pixel_size; batch++)
		{
			int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
			for (int l = 0; l < min(lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
				if (mask & (1<<l))
				{
					vis_wi.push_back(true);
				} else {
					vis_wi.push_back(false);
				}
		}

		Vector3 c;
		Vector3 wo(global_wo*tangent, global_wo*binormal, global_wo*normal);
		printf_s("wo(%g %g %g)\n", wo.x, wo.y, wo.z);

		for (int l = 0; l < vis_wi.size(); l++)
			if (vis_wi[l])
			{
				Vector3 wi(lights[l].dir*tangent, lights[l].dir*binormal, lights[l].dir*normal);						
				Vector3 refl;
				pBRDF->sample(refl, wi, wo, Vector3(0, 0, 1));
#ifdef LOG_MATERIAL
				refl.x = exp(refl.x)-1.0f;
				refl.y = exp(refl.y)-1.0f;
				refl.z = exp(refl.z)-1.0f;
#endif // LOG_MATERIAL
				//refl *= spec_basis_matr->inv_avg/wo.z;
				refl /= wo.z;
				printf_s("BRDF : %g %g %g\n", refl.x, refl.y, refl.z);
				printf_s("light: %g %g %g\n", lights[l].c.x, lights[l].c.y, lights[l].c.z),

				c.x += lights[l].c.x*refl.x;
				c.y += lights[l].c.y*refl.y;
				c.z += lights[l].c.z*refl.z;

				//DEBUG
				{
					wi = Vector3(-0.575532f, -0.000498352f, 0.817779f);
					wo = Vector3(0.573084f, -0.000498839f, 0.819497f);
					pBRDF->sample(refl, wi, wo, Vector3(0, 0, 1));
					refl.x = exp(refl.x)-1.0f;
					refl.y = exp(refl.y)-1.0f;
					refl.z = exp(refl.z)-1.0f;
					//refl *= spec_basis_matr->inv_avg/wo.z;
					printf_s("---------------------------\n");
					printf_s("wi : (%g %g %g)\n", wi.x, wi.y, wi.z);
					printf_s("wo : (%g %g %g)\n", wo.x, wo.y, wo.z);
					printf_s("c  : (%g %g %g)\n", refl.x, refl.y, refl.z);
					printf_s("---------------------------\n");
				}
			}

		printf_s("result: %g %g %g\n", c.x, c.y, c.z);
	}
}
