#include "microfacet.h"
#include "worker.h"
#define GROUND_TRUTH_SCENE_R	1.73

float compute_BRDF_wo(float *result,
					  microfacet_details *details,
					  std::vector<microfacet_block> *blocks,
					  const Vector3 &wo,
					  const parab_frame &fr_wi,
					  const int block_idx,
					  r_shadowmap *p_shadow,
					  render_output *p_screen,
					  render_target *p_target_id,
					  float *p_id,
					  render_target *p_target_normal,
					  short *p_normal,
					  render_target *p_target_vis,
					  float *p_vis)
{
	int block_x, block_y;
	details->compute_back_idx(block_x, block_y, block_idx);

	Matrix4	matView, matProj, matProjView;
	Vector3		center(block_x+0.5, block_y+0.5, 0), 
				vwo(wo.x, wo.y, wo.z);
	matrix_lookat(matView, center+vwo*3.0, center, Vector3(0, 1, 0));
	float	z_near= 1e-5*GROUND_TRUTH_SCENE_R, 
			z_far = 5.0;
	projection_orthogonal(matProj, GROUND_TRUTH_SCENE_R, GROUND_TRUTH_SCENE_R, z_near, z_far);
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

	microfacet_block &block = (*blocks)[block_idx];
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
	p_screen->clear_depth();

	r_shader_basic	*p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
	r_shader_vis	*p_sh_use_shadow = (r_shader_vis*)mff_singleton::get()->get_shader("init_shadow");

	mff_singleton::get()->get_layer("init_shadow")->set();
	for (int i = 0; i < fr_wi.get_n().size(); i++)
	{
		const Vector3 &wi = fr_wi.get_n()[i][0];
		r_light_dir lt;
		lt.dir	= wi;
		lt.c	= Vector3(1,1,1);

		std::vector<r_light_dir> lights;
		lights.push_back(lt);
		p_shadow->set_lights(lights);
		p_shadow->compute_light_matrix(center);

		Vector3 brdf;
		if (wi.z > 0)
		{
			p_screen->clear_depth();
			p_screen->get_rt(0)->clear(clear_color);
			//gen shadow
			Matrix4 matLightProjView = p_shadow->mat_light_proj * p_shadow->mat_light_view[0];
			p_shadow->set_gen_shadow(0);
			for (int j = 0; j < block.neighbors_and_self.size(); j++)
			{
				block.neighbors_and_self[j]->draw_with_shader(p_sh_shadow, &matLightProjView);
			}
			p_shadow->unset(0, NULL);

			//use shadow
			float vis_mask = 1.0f;
			p_sh_use_shadow->setup_cb_lights(&p_shadow->lights[0], vis_mask, &matLightProjView);
			p_shadow->set_use_shadow(0, p_sh_use_shadow);
			p_screen->set();
			block.draw_with_shader(p_sh_use_shadow, &matProjView);
			p_shadow->unset(0, p_sh_use_shadow);

			p_screen->get_rt(0)->get_result((BYTE*)p_vis, 4);

			//shading
			for (int j = 0; j < addr.size(); j++)
			{
				int idx = addr[j];
				if (p_vis[idx] != 0.0f)
				{
					matr* p_matr = mff_singleton::get()->get_matr((int)p_id[idx]);
					preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

					Vector3 n;
					n.x	= snorm2float(p_normal[idx*4]);
					n.y	= snorm2float(p_normal[idx*4+1]);
					n.z	= snorm2float(p_normal[idx*4+2]);
					n = Normalize(n);

					Vector3 refl;
					basis_matr->pBRDF->sample(refl, wi, wo, n);
					refl.x *= p_matr->albedo.x;
					refl.y *= p_matr->albedo.y;
					refl.z *= p_matr->albedo.z;
#ifdef LOG_MATERIAL
					refl.x = exp(refl.x)-1.0f;
					refl.y = exp(refl.y)-1.0f;
					refl.z = exp(refl.z)-1.0f;
#endif // LOG_MATERIAL
					brdf += refl * (basis_matr->inv_avg/(Dot(n, wo)));
				}
			}
			brdf /= addr.size();
		}

		result[i*3+0] = brdf.x;
		result[i*3+1] = brdf.y;
		result[i*3+2] = brdf.z;
	}

	float area = GROUND_TRUTH_SCENE_R*GROUND_TRUTH_SCENE_R/(DIM_GROUND_TRUTH*DIM_GROUND_TRUTH)*addr.size();
	return area;
}

void worker_microfacet::compute_BRDF_truth(task_microfacet_changed &tmc)
{
	//codex::utils::timer t;
	printf_s("\ncomputing truth BRDF...");

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

	for (int xx = 0; xx < tmc.idx->size(); xx++)
		if (tmc.details->in_boundary((*tmc.idx)[xx]))
		{
			microfacet_block &block = (*tmc.blocks)[(*tmc.idx)[xx]];

			preconv_matr *spec_basis_matr = NULL;
			for (int i = 0; i < block.per_mat_groups.insts.size(); i++)
			{
				int id = block.per_mat_groups.get_id(i);
				matr* p_matr = mff_singleton::get()->get_matr(id);
				preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

				if (spec_basis_matr == NULL ||
					basis_matr->fr_brdf.get_n().size() > spec_basis_matr->fr_brdf.get_n().size())
					spec_basis_matr = basis_matr;
			}

			int brdf_dim = spec_basis_matr->fr_brdf.get_n().size();
			block.BRDF_truth.resize(3*brdf_dim*brdf_dim);
			const parab_frame &fr = spec_basis_matr->fr_brdf;

			printf_s("\n");
			for (int i_wo = 0; i_wo < brdf_dim; i_wo++)
			{
				printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf_s("(%d/%d) ", i_wo+1, brdf_dim);

				compute_BRDF_wo(&block.BRDF_truth[3*brdf_dim*i_wo], tmc.details, tmc.blocks, 
					fr.get_n()[i_wo][0], fr, (*tmc.idx)[xx], 
					&shadowmap, p_screen, p_target_id, p_id, p_target_normal, p_normal, p_target_vis, p_vis);
			}
			printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf_s("done.          ");
		}

	SAFE_DELETE_ARR(p_normal);
	SAFE_DELETE_ARR(p_vis);
	SAFE_DELETE_ARR(p_id);
	SAFE_DELETE(p_target_normal);
	SAFE_DELETE(p_target_vis);
	SAFE_DELETE(p_target_id);
	SAFE_DELETE(p_screen);

	//t.update();
	//printf_s("done. %g secs\n", t.elapsed_time());
	printf_s("done. \n");
}

void worker_microfacet::compute_PSNR(task_microfacet_changed &tmc)
{
	printf_s("\ncomputing PSNR...\n");

	for (int xx = 0; xx < tmc.idx->size(); xx++)
		if (tmc.details->in_boundary((*tmc.idx)[xx]))
		{
			microfacet_block &block = (*tmc.blocks)[(*tmc.idx)[xx]];

			preconv_matr *spec_basis_matr = NULL;
			for (int i = 0; i < block.per_mat_groups.insts.size(); i++)
			{
				int id = block.per_mat_groups.get_id(i);
				matr* p_matr = mff_singleton::get()->get_matr(id);
				preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

				if (spec_basis_matr == NULL ||
					basis_matr->fr_brdf.get_n().size() > spec_basis_matr->fr_brdf.get_n().size())
					spec_basis_matr = basis_matr;
			}

			int brdf_dim = spec_basis_matr->fr_brdf.get_n().size();
			
			int num_area;
			int idx_area[4];
			float weight_area[4];
			const parab_frame &fr = spec_basis_matr->fr_brdf;

			double max_i2 = 0, i2 = 0, mse = 0;
			for (int i_wo = 0; i_wo < brdf_dim; i_wo++)
			{
				tmc.fr_Avis->lerp(num_area, idx_area, weight_area, fr.get_n()[i_wo][0], true, false);
				float area = block.Avis.get_area(num_area, idx_area, weight_area);

				double albedo = 0;
				for (int i_wi = 0; i_wi < brdf_dim; i_wi++)
				{
					double	r = block.BRDF_truth[3*(brdf_dim*i_wo+i_wi)+0]*area,
							g = block.BRDF_truth[3*(brdf_dim*i_wo+i_wi)+1]*area,
							b = block.BRDF_truth[3*(brdf_dim*i_wo+i_wi)+2]*area;

					Vector3 truth_c, our_c;
					truth_c.x = r;
					truth_c.y = g;
					truth_c.z = b;
					block.get_reflectance_BRDF(our_c, fr.get_n()[i_wi][0], fr.get_n()[i_wo][0]);
					
					albedo	+= r*r+g*g+b*b;
					i2		+= r*r+g*g+b*b;
					mse		+= (truth_c-our_c).LengthSquared();
				}

				max_i2 = max(max_i2, albedo/brdf_dim);
			}
			mse /= brdf_dim*brdf_dim;
			i2 /= brdf_dim*brdf_dim;

			double PSNR = 10*log10(max_i2/mse);
			double SNR = 10*log10(i2/mse);
			printf_s("PSNR = %gdb\n", PSNR);
			printf_s("SNR = %gdb\n", SNR);
			printf_s("RMSE = %g\n", sqrt(mse));
		}
}

void worker_microfacet::work_task_ground_truth(task_microfacet *t_org)
{
	task_render_truth &t = t_org->trt;
	Vector3 v_right = Cross((*t.v_up), (*t.v_lookat));
	v_right = Normalize(v_right);

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
	const parab_frame &fr = spec_basis_matr->fr_brdf;
	const std::vector<float> &BRDF = (*t.blocks)[idx_block].BRDF_truth;

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;
	//int num_area;
	//int idx_area[4];
	//float weight_area[4];
	for (int y = 0; y < t_org->height; y++)
		for (int x = 0; x < t_org->width; x++)
		{
			int	idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf_s("(%d/%d) ", x+y*t_org->width+1, t_org->width*t_org->height);

				//vector2 uv;
				//uv.x = t.p_uv[idx*2];
				//uv.y = t.p_uv[idx*2+1];

				Vector3 normal, tangent, binormal, global_wo;
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
				//int idx_block = t.details->compute_idx(0, 0);

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

				int num_brdf_i, num_brdf_o;
				int idx_brdf_i[4], idx_brdf_o[4];
				float weight_brdf_i[4], weight_brdf_o[4];

				Vector3 c;
				Vector3 wo(Dot(global_wo, tangent), Dot(global_wo, binormal), Dot(global_wo, normal));
				fr.lerp(num_brdf_o, idx_brdf_o, weight_brdf_o, wo, true, false);

				for (int l = 0; l < vis_wi.size(); l++)
					if (vis_wi[l])
					{
						Vector3 wi(Dot(lights[l].dir, tangent), Dot(lights[l].dir, binormal), Dot(lights[l].dir, normal));
						fr.lerp(num_brdf_i, idx_brdf_i, weight_brdf_i, wi, true, false);
						
						Vector3 refl;
						for (int oo = 0; oo < num_brdf_o; oo++)
							for (int ii = 0; ii < num_brdf_i; ii++)
							{
								float weight = weight_brdf_i[ii]*weight_brdf_o[oo];
								refl.x += BRDF[(idx_brdf_i[ii]+idx_brdf_o[oo]*fr.get_n().size())*3+0]*weight;
								refl.y += BRDF[(idx_brdf_i[ii]+idx_brdf_o[oo]*fr.get_n().size())*3+1]*weight;
								refl.z += BRDF[(idx_brdf_i[ii]+idx_brdf_o[oo]*fr.get_n().size())*3+2]*weight;
							}

						c.x += lights[l].c.x*refl.x;
						c.y += lights[l].c.y*refl.y;
						c.z += lights[l].c.z*refl.z;
					}

				if (x == 128 && y == 128)
				{
					//printf_s("area = %g\n", area);
					printf_s("result(lighted)= (%g %g %g)\n", c.x, c.y, c.z);
					printf_s("light= (%g %g %g)\n", lights[0].c.x, lights[0].c.y, lights[0].c.z);
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
