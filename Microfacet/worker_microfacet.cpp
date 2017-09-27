#include "worker.h"
#include "microfacet_factory.h"
#include <DirectXMath.h>

#define MIN_WO_Z	0.1f

//void worker_microfacet::work_task_test(task_microfacet *t_org)
//{
//	task_test &t = t_org->tr;
//
//	Matrix4 matProjView = t_org->matProj*t_org->matView;
//	//float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f};
//	t.p_screen->clear_depth();
//	t.p_screen->set();
//	t.p_skybox->draw(matProjView);
//
//	Matrix4 mat;
//	r_blendstate	*p_blend_add = mff_singleton::get()->get_blend("add");
//	r_dsstate		*p_ds_lesseq = mff_singleton::get()->get_ds("lesseq");
//	r_shader_basic	*p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
//
//	mff_singleton::get()->get_layer("basic")->set();
//	for (int l = 0; l < t.p_shadow->lights.size(); l++)
//	{
//		//gen shadow map
//		Matrix4 matLightProjView = t.p_shadow->mat_light_proj * t.p_shadow->mat_light_view[l];
//
//		t.p_shadow->set_gen_shadow(0);
//		p_sh_shadow->set_shader_input(t.p_main->get_shader_input(), &matLightProjView);
//		t.p_main->draw_with_shader(p_sh_shadow);
//		for (int i = 0; i < t.p_background->size(); i++)
//		{
//			r_instance *pi = (*t.p_background)[i];
//			p_sh_shadow->set_shader_input(pi->get_shader_input(), &matLightProjView);
//			pi->draw_with_shader(p_sh_shadow);
//		}
//		t.p_shadow->unset(0, NULL);
//
//		//use shadow map
//		((r_shader_basic*)mff_singleton::get()->get_shader("diffuse"))->setup_cb_lights(&t.p_shadow->lights[l], &matLightProjView);
//		t.p_shadow->set_use_shadow(0, p_sh_shadow);
//		t.p_screen->set();
//		if (l > 0)
//		{
//			p_blend_add->set();
//			p_ds_lesseq->set();
//		}
//
//		//((r_shader_basic*)t.p_main->get_shader())->setup_cb_basic(t.p_main->get_shader_input(), matProjView);
//		t.p_main->get_shader()->set_shader_input(t.p_main->get_shader_input(), &matProjView);
//		t.p_main->draw();
//		for (int i = 0; i < t.p_background->size(); i++)
//		{
//			r_instance *pi = (*t.p_background)[i];
//			//((r_shader_basic*)pi->get_shader())->setup_cb_basic(pi->get_shader_input(), matProjView);
//			pi->get_shader()->set_shader_input(pi->get_shader_input(), &matProjView);
//			pi->draw();
//		}
//
//		if (l > 0)
//		{
//			p_blend_add->unset();
//			p_ds_lesseq->unset();
//		}
//		t.p_shadow->unset(0, p_sh_shadow);
//	}
//}

//codex::utils::timer tm;

void worker_microfacet::work_task_microfacet_changed(task_microfacet *t)
{
	task_microfacet_changed &tmc = t->tmc;
	if (tmc.sub_type & TASK_SUBTYPE_GEN_DISTR)
	{
		tmc.details->update_with_distr(*tmc.idx);
		tmc.details->generate_blocks(*tmc.idx, *tmc.blocks);
		tmc.details->sample_points(*tmc.idx, *tmc.blocks);
	}

	if (tmc.sub_type & TASK_SUBTYPE_VIS_NORMAL)
	{
		compute_vis_area_and_normal(
			*tmc.details, *tmc.idx, *tmc.blocks,
			*tmc.fr_Avis, *tmc.fr_vis,
			*tmc.avis_sampler, *tmc.vismask_sampler, *tmc.rp);
	}

	if (tmc.sub_type & TASK_SUBTYPE_BRDF_TRUTH)
	{
		compute_BRDF_truth(tmc);
	}

	if (tmc.sub_type & TASK_SUBTYPE_BRDF_NORMAL)
	{
		compute_BRDF_normal(tmc);
	}

	if (tmc.sub_type & TASK_SUBTYPE_BRDF_PSNR)
	{
		compute_PSNR(tmc);
	}
}

void worker_microfacet::work_task_buffer(task_microfacet *t_org)
{
	//tm.update();
	task_buffer &t = t_org->tb;
	Matrix4 matProjView = t_org->matProj*t_org->matView;

	r_blendstate	*p_blend_add = mff_singleton::get()->get_blend("add");
	r_dsstate		*p_ds_lesseq = mff_singleton::get()->get_ds("lesseq");
	r_shader_basic	*p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
	r_shader_vis	*p_sh_use_shadow = (r_shader_vis*)mff_singleton::get()->get_shader("init_shadow");

//**************************
//#1 : Context pass (To generate uv, normal and tangent buffers)
//**************************

	t.p_screen->clear_mrt();
	//t.p_screen->add_rt(t.p_target_test);
	t.p_screen->add_rt(t.p_target_uv);
	t.p_screen->add_rt(t.p_target_normal);
	t.p_screen->add_rt(t.p_target_tangent);
	
	t.p_screen->clear_depth();
	t.p_screen->set();
	float clearUV[4] = { -1.0f, -1.0f, -1.0f, -1.0f };
	t.p_screen->get_rt(0)->clear(clearUV);
	mff_singleton::get()->get_layer("init_context")->set();
	//mff_singleton::get()->get_layer("test_shader")->set();

	DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);

	DirectX::XMMATRIX g_WorldMatrix = DirectX::XMMatrixTranspose(XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(0)));
	Matrix4 world;
	XMMATRIX_to_matrix(world, g_WorldMatrix);
	/************test code end************/
	//cout << "int test code attr = " << t.p_main_test->get_geom()->get_mesh()->get_attr() << " v = " << t.p_main_test->get_geom()->get_mesh()->get_vertex_number()<<
	//	" n = " << t.p_main_test->get_geom()->get_mesh()->get_normal_number() << 
	//	" uv = " << t.p_main_test->get_geom()->get_mesh()->get_uv_number() << 
	//	" tangent = " << t.p_main_test->get_geom()->get_mesh()->get_tangent_number() << endl;
	//t.p_main_test->get_shader()->set_shader_input(t.p_main_test->get_shader_input(), &matProjView);
	//t.p_main_test->draw();
	/************test code end************/

	r_shader_input* input = new r_shader_input();
	input->matWorld = world;
	//t.p_main->get_shader()->set_shader_input(input, &matProjView);
	t.p_main->get_shader()->set_shader_input(t.p_main->get_shader_input(), &matProjView);
	t.p_main->draw();
	//for (int i = 0; i < t.p_main->get_geom()->get_mesh()->get_vertex_number(); i++)
		//cout << t.p_main->get_geom()->get_mesh()->vertices[i] << endl;
	/*
	for (int i = 0; i < t.p_background->size(); i++)
	{
		r_instance *pi = (*t.p_background)[i];
		pi->get_shader()->set_shader_input(pi->get_shader_input(), &matProjView);
		pi->draw();
	}*/
	/*
	t.p_screen->get_rt(0)->get_result((BYTE*)t.p_uv, 8);
	for (int i = 0; i < 256 * 256; i++)
	{
		if (t.p_uv[i * 2] != -1.0f)
			cout << "render result uv " << t.p_uv[i * 2] << " " << t.p_uv[i * 2 + 1] << endl;
	}*/
	
	t.p_screen->get_rt(0)->get_result((BYTE*)t.p_uv, 8);
	/*
	int count_uv = 0, count_normal = 0, count_tangent = 0;
	for (int i = 0; i < 256 * 256; i++)
	{
		if (t.p_uv[i * 2] != -1.0f)
		{ 
			//cout << "render result test " << t.p_uv[i * 2] << " " << t.p_uv[i * 2 + 1] << endl;
			count_uv++;
		}
	}
	cout << "The numbe of non default pixel uv = " << count_uv << endl;*/
	
	t.p_screen->get_rt(1)->get_result((BYTE*)t.p_normal, 8);
	/*
	for (int i = 0; i < 256 * 256; i++)
	{
		if (t.p_normal[i * 4] != 0.0f ||  t.p_normal[i * 4 + 1] != 0.0f || t.p_normal[i * 4+2] != 0.0f)
			//cout << "render result test " << t.p_uv[i * 2] << " " << t.p_uv[i * 2 + 1] << endl;
			count_normal++;
	}
	cout << "The numbe of non default pixel normal = " << count_normal << endl;*/
	t.p_screen->get_rt(2)->get_result((BYTE*)t.p_tangent, 8);
	/*
	for (int i = 0; i < 256 * 256; i++)
	{
		if (t.p_tangent[i * 4] != 0.0f || t.p_tangent[i * 4+1] != 0.0f || t.p_tangent[i * 4+2] != 0.0f)
			//cout << "render result test " << t.p_uv[i * 2] << " " << t.p_uv[i * 2 + 1] << endl;
			count_tangent++;
	}
	cout << "The numbe of non default pixel tangent = " << count_tangent << endl;*/
	
	//tm.update();
	//printf_s("#1 : %7.4f ", tm.elapsed_time());
	//printf_s("work_task_buffer #1 \n");

//**************************
//#2 : Shadow pass
//**************************
	
	t.p_screen->clear_mrt();
	t.p_screen->add_rt(t.p_target_vis);
	float clearVis[4] = {0, 0, 0, 0};
	
	mff_singleton::get()->get_layer("init_shadow")->set();
	for (int batch = 0; batch < t.vis_pixel_size; batch++)
	{
		t.p_screen->get_rt(0)->clear(clearVis);
		for (int l = 0; l < min((t.p_shadow->lights.size()-batch*NUM_BITS_VIS), NUM_BITS_VIS); l++)
		{
			//gen shadow
			Matrix4 matLightProjView = t.p_shadow->mat_light_proj * t.p_shadow->mat_light_view[l+batch*NUM_BITS_VIS];

			t.p_shadow->set_gen_shadow(0);
			p_sh_shadow->set_shader_input(t.p_main->get_shader_input(), &matLightProjView);
			t.p_main->draw_with_shader(p_sh_shadow);
			for (int i = 0; i < t.p_background->size(); i++)
			{
				r_instance *pi = (*t.p_background)[i];
				p_sh_shadow->set_shader_input(pi->get_shader_input(), &matLightProjView);
				pi->draw_with_shader(p_sh_shadow);
			}

			t.p_shadow->unset(0, NULL);

			//use shadow
			int vis_mask = (1 << l);
			p_sh_use_shadow->setup_cb_lights(&t.p_shadow->lights[l+batch*NUM_BITS_VIS], vis_mask, &matLightProjView);
			t.p_shadow->set_use_shadow(0, p_sh_use_shadow);
			t.p_screen->set();
			p_blend_add->set();
			p_ds_lesseq->set();

			p_sh_use_shadow->set_shader_input(t.p_main->get_shader_input(), &matProjView);
			t.p_main->draw_with_shader(p_sh_use_shadow);

			p_blend_add->unset();
			p_ds_lesseq->unset();
			t.p_shadow->unset(0, p_sh_shadow);
		}

		//This read-in is causing a lot of delay. Try to avoid this.
		t.p_screen->get_rt(0)->get_result((BYTE*)&t.p_vis_buffer[t_org->width*t_org->height*batch], 4);
	}

	//tm.update();
	//printf_s("#2 : %7.4f ", tm.elapsed_time());
	//printf_s("work_task_buffer #2 \n");
	
//**************************
//#3 : Render background
//**************************
	
	t.p_screen->clear_mrt();
	t.p_screen->add_rt(t.p_target);

	t.p_screen->clear_depth();
	t.p_screen->set();

	//float ClearColor[4] = {1, 1, 1, 1};
	//t.p_screen->get_rt(0)->clear(ClearColor);
	t.p_skybox->draw(matProjView);// matProjView;
	
	mff_singleton::get()->get_layer("basic")->set();
	for (int l = 0; l < t.p_shadow->lights.size(); l++)
	{
		//gen shadow map
		Matrix4 matLightProjView = t.p_shadow->mat_light_proj * t.p_shadow->mat_light_view[l];

		t.p_shadow->set_gen_shadow(0);
		p_sh_shadow->set_shader_input(t.p_main->get_shader_input(), &matLightProjView);
		t.p_main_vis->draw_with_shader(p_sh_shadow);
		for (int i = 0; i < t.p_background_vis->size(); i++)
		{
			r_instance *pi = (*t.p_background_vis)[i];
			p_sh_shadow->set_shader_input(pi->get_shader_input(), &matLightProjView);
			pi->draw_with_shader(p_sh_shadow);
		}
		t.p_shadow->unset(0, NULL);

		//use shadow map
		r_light_dir lt = t.p_shadow->lights[l];
		lt.c *= t.envlight_inten;
		((r_shader_basic*)mff_singleton::get()->get_shader("diffuse"))->setup_cb_lights(&lt, &matLightProjView);
		t.p_shadow->set_use_shadow(0, p_sh_shadow);
		t.p_screen->set();
		if (l > 0)
		{
			p_blend_add->set();
			p_ds_lesseq->set();
		}

		for (int i = 0; i < t.p_background_vis->size(); i++)
		{
			r_instance *pi = (*t.p_background_vis)[i];
			pi->get_shader()->set_shader_input(pi->get_shader_input(), &matProjView);
			pi->draw();
		}

		if (l > 0)
		{
			p_blend_add->unset();
			p_ds_lesseq->unset();
		}
		t.p_shadow->unset(0, p_sh_shadow);
	}
	
	//tm.update();
	//printf_s("#3 : %7.4f ", tm.elapsed_time());
	//printf_s("work_task_buffer #3 \n");
}


extern double t_total, t_init, t_init_loop, t_basis_coef1, t_basis_coef2, t_basis_lerp, t_refl;
//extern codex::utils::timer t_frame;
//void worker_microfacet::work_task_render(task_microfacet *t_org)
//{
//	//t_frame.update();
//	//printf_s("before %7.4f\n", t_frame.elapsed_time());
//	//DEBUG
//	t_total = 0;
//	t_init = t_init_loop = t_basis_coef1 = t_basis_coef2 = t_basis_lerp = t_refl = 0;
//
//	task_render &t = t_org->r;
//
//	codex::utils::timer tt;
//	double t_get_refl = 0;
//
//	int num_area;
//	int idx_area[4];
//	float weight_area[4];
//	for (int y = 0; y < t_org->height; y++)
//		for (int x = 0; x < t_org->width; x++)
//		{
//			int		idx = x+y*t_org->width;
//			if (t.p_uv[idx*2] >= 0)
//			{
//				//vector2 uv;
//				Vector3 normal, tangent, binormal, global_wo;
//
//				//uv.x = t.p_uv[idx*2];
//				//uv.y = t.p_uv[idx*2+1];
//
//				normal.x	= snorm2float(t.p_normal[idx*4]);
//				normal.y	= snorm2float(t.p_normal[idx*4+1]);
//				normal.z	= snorm2float(t.p_normal[idx*4+2]);
//				tangent.x	= snorm2float(t.p_tangent[idx*4]);
//				tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
//				tangent.z	= snorm2float(t.p_tangent[idx*4+2]);
//
//				binormal	= normal ^ tangent;
//				global_wo	=	(*t.p_wo)[idx].x*(*t.v_right) +
//								(*t.p_wo)[idx].y*(*t.v_up) +
//								(*t.p_wo)[idx].z*(*t.v_lookat);
//				Vector3 wo(global_wo*tangent, global_wo*binormal, global_wo*normal);
//
//				//FIX ME: huge bug here!
//				int idx_block = t.details->compute_idx(0, 0);
//				Vector3 c;
//				for (int batch = 0; batch < t.vis_pixel_size; batch++)
//				{
//					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
//					for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
//						if (mask & (1<<l))
//						{
//							float dot = t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir * normal;
//							if (dot > 0)
//							{
//								Vector3 wi(t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir*tangent, 
//											t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir*binormal, dot);
//
//								tt.update();
//								Vector3 temp;
//								(*t.blocks)[idx_block].get_reflectance(temp, wi, wo, *t.fr_vis);
//								tt.update();
//								t_get_refl += tt.elapsed_time();
//								//(*t.blocks)[idx_block].get_reflectance_debug(temp, wi, wo, *t.fr_vis);
//								c.x += temp.x*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.x;
//								c.y += temp.y*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.y;
//								c.z += temp.z*t.p_shadow->lights[l+batch*NUM_BITS_VIS].c.z;
//							}
//						}
//				}
//
//				t.fr_Avis->lerp(num_area, idx_area, weight_area, wo, false, false);
//				c /= (*t.blocks)[idx_block].Avis.get_area(num_area, idx_area, weight_area);
//
//				BYTE r, g, b;
//				r = min(max(c.x, 0), 1)*255;
//				g = min(max(c.y, 0), 1)*255;
//				b = min(max(c.z, 0), 1)*255;
//				UINT pixel = (r << 16) + (g << 8) + (b);
//				t.result[idx] = pixel;
//			}
//		}
//	//printf_s("get_refl : %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n", t_init, t_init_loop, t_basis_coef1, t_basis_coef2, t_basis_lerp, t_refl);
//	//t_frame.update();
//	//double t_total = t_frame.elapsed_time();
//	//printf_s("#3 : (%7.4f %7.4f)\n", t_get_refl, t_total-t_get_refl);
//	//printf_s("get_refl %7.4f\n", t_get_refl);
//}

void worker_microfacet::work_task_render_block(task_microfacet *t_org, qrender_block *pb)
{
	task_render &t = t_org->r;

	int num_area;
	int idx_area[4];
	float weight_area[4];
	for (int y = pb->y0; y < pb->y1; y++)
		for (int x = pb->x0; x < pb->x1; x++)
		{
			int	idx = x+y*t_org->width;
			//if (t.p_uv[idx * 2]!=-1.0f)
				//cout << "in work task render block t.p_uv[idx*2] = " << t.p_uv[idx * 2] << endl;
			if (t.p_uv[idx*2] >= 0)
			{
				//vector2 uv;
				Vector3 normal, tangent, binormal, global_wo;

				//uv.x = t.p_uv[idx*2];
				//uv.y = t.p_uv[idx*2+1];

				normal.x	= snorm2float(t.p_normal[idx*4]);
				normal.y	= snorm2float(t.p_normal[idx*4+1]);
				normal.z	= snorm2float(t.p_normal[idx*4+2]);
				tangent.x =  snorm2float(t.p_tangent[idx * 4]);
				tangent.y =  snorm2float(t.p_tangent[idx * 4 + 1]);
				tangent.z =  snorm2float(t.p_tangent[idx * 4 + 2]);
				
				binormal	= Cross(normal, tangent);
				global_wo	=	(*t.p_wo)[idx].x*(*t.v_right) +
								(*t.p_wo)[idx].y*(*t.v_up) +
								(*t.p_wo)[idx].z*(*t.v_lookat);
				Vector3 wo(Dot(global_wo, tangent), Dot(global_wo, binormal), Dot(global_wo, normal));

				// Weiqi: from original code, this will generate a black strip in the middle of image
				/*
				if (wo.z < MIN_WO_Z)
				{
					wo.z = MIN_WO_Z;
					wo = Normalize(wo);
				*/

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
								(*t.blocks)[idx_block].get_reflectance_BRDF(temp, wi, wo);
								
								c.x += temp.x *t.p_shadow->lights[l + batch*NUM_BITS_VIS].c.x;
								c.y += temp.y *t.p_shadow->lights[l + batch*NUM_BITS_VIS].c.y;
								c.z += temp.z *t.p_shadow->lights[l + batch*NUM_BITS_VIS].c.z;
							}
						}
				}

				t.fr_Avis->lerp(num_area, idx_area, weight_area, wo, true, false);
				float area = (*t.blocks)[idx_block].Avis.get_area(num_area, idx_area, weight_area);
				c /= area;

				//DEBUG
				if (x == 128 && y == 128)
				{
					//printf_s("area = %g\n", area);
					printf_s("result(lighted)= (%g %g %g)\n", c.x, c.y, c.z);
					printf_s("light= (%g %g %g)\n", t.p_shadow->lights[0].c.x, t.p_shadow->lights[0].c.y, t.p_shadow->lights[0].c.z);
				}
				//c = t.p_shadow->lights[0].c/10;
				//c = (wo+Vector3(1,1,1))/2;
				//if (c.x == 0.0f && c.y == 0.0f && c.z == 0.0f)
				//	cout << "x = " << x << " y = " << y << endl;
				BYTE r, g, b;
				r = min(max(c.x, 0), 1)*255;
				g = min(max(c.y, 0), 1)*255;
				b = min(max(c.z, 0), 1)*255;
				//cout << "r = " << c.x << " g = " << c.y << " b = " << c.z << endl;
				UINT pixel = (r << 16) + (g << 8) + (b);
				t.result[idx] = pixel;
			}
		}
}

void worker_microfacet::work_task_visual(task_microfacet *t_org)
{
	task_render_visual &t = t_org->trv;

	Matrix4 matProjView = t_org->matProj*t_org->matView;
	
	float ClearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	t.p_screen->clear_mrt();
	t.p_screen->add_rt(t.p_target);
	t.p_target->clear(ClearColor);

	t.p_screen->clear_depth();
	t.p_screen->set();

	Matrix4 mat;
	r_blendstate	*p_blend_add = mff_singleton::get()->get_blend("add");
	r_dsstate		*p_ds_lesseq = mff_singleton::get()->get_ds("lesseq");
	r_shader_basic	*p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
	r_shader_basic	*p_sh_use_shadow = (r_shader_vis*)mff_singleton::get()->get_shader("diffuse");

	int obj_size = (t.b_show_neighbors ? t.p_block->neighbors_and_self.size() : 1);
	mff_singleton::get()->get_layer("basic")->set();

	//ordinary visualization
	for (int l = 0; l < t.p_shadow->lights.size(); l++)
	{
		//gen shadow map
		Matrix4 matLightProjView = t.p_shadow->mat_light_proj * t.p_shadow->mat_light_view[l];
		t.p_shadow->set_gen_shadow(0);
		//cout << "before draw with shader" << endl;
		for (int j = 0; j < t.p_block->neighbors_and_self.size(); j++)
		{	
			t.p_block->neighbors_and_self[j]->draw_with_shader(p_sh_shadow, &matLightProjView);
		}
		//cout << "after draw with shader" << endl;
		t.p_shadow->unset(0, NULL);

		//use shadow map
		t.p_shadow->set_use_shadow(0, p_sh_shadow);
		t.p_screen->set();
		if (l > 0)
		{
			p_blend_add->set();
			p_ds_lesseq->set();
		}

		for (int j = 0; j < obj_size; j++)
		{
			microfacet_block *p_block = t.p_block->neighbors_and_self[j];
			for (int k = 0; k < p_block->per_mat_groups.insts.size(); k++)
			{
				int id = p_block->per_mat_groups.get_id(k);
				matr* p_matr = mff_singleton::get()->get_matr(id);
				preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

				r_light_dir lt = t.p_shadow->lights[l];

				lt.c.x *= p_matr->albedo.x*abs(basis_matr->Mc.x)*t.vis_light_inten;
				lt.c.y *= p_matr->albedo.y*abs(basis_matr->Mc.y)*t.vis_light_inten;
				lt.c.z *= p_matr->albedo.z*abs(basis_matr->Mc.z)*t.vis_light_inten;
				p_sh_use_shadow->setup_cb_lights(&lt, &matLightProjView);

				for (int p = 0; p < p_block->per_mat_groups.insts[k].size(); p++)
				{
					p_sh_use_shadow->set_shader_input(p_block->per_mat_groups.insts[k][p]->get_shader_input(), (void*)&matProjView);
					p_block->per_mat_groups.insts[k][p]->draw_with_shader(p_sh_use_shadow);
				}
			}
		}
		//cout << "after object size" << endl;
		if (l > 0)
		{
			p_blend_add->unset();
			p_ds_lesseq->unset();
		}
		t.p_shadow->unset(0, p_sh_shadow);
	}
	//cout << "before disk" << endl;
	
	if (t.disc_r > 0)
	{
		//disc visualization
		r_shader_basic	*p_sh = (r_shader_vis*)mff_singleton::get()->get_shader("vis_disc");
		shared_ptr<r_geometry>	p_vis = mff_singleton::get()->get_geom("vis_disc");

		r_light_dir		lt;
		Matrix4		mat1, mat2, mat3;
		r_shader_input  input;
		Vector3			tangent, binormal;

		t.p_screen->clear_depth();
		t.p_screen->set();
		p_ds_lesseq->set();

		for (int j = 0; j < t.discs->size(); j++)
		{
			const disc &d = (*t.discs)[j];
			if (d.color.x <= 0 && d.color.y <= 0 && d.color.z <= 0)
				continue;
			
			lt.c = //Vector3(157.0f/255, 14.0f/255, 10.0f/255);
					Vector3(238.0f/255, 226.0f/255, 10.0f/255);
				//d.color;
			p_sh->setup_cb_lights(&lt, &mat1);

			//DEBUG
			//if (d.color.x <= 0 && d.color.y <= 0 && d.color.z <= 0)
			//{
			//	printf_s("\nc (%g %g %g)\n", d.color.x, d.color.y, d.color.z);
			//}

			mat1 = mat1 * Scale(t.disc_r, t.disc_r, t.disc_r);
			build_frame(tangent, binormal, Vector3(d.n.x, d.n.y, d.n.z));
			mat2 = Matrix4(	tangent.x, binormal.x, d.n.x, 0,
								tangent.y, binormal.y, d.n.y, 0,
								tangent.z, binormal.z, d.n.z, 0,
										0,			0,     0, 1);
			float scalar = 0.001f;
			mat3 = Translate(Vector3(d.p.x + scalar*d.n.x, d.p.y + scalar*d.n.y, d.p.z + scalar*d.n.z));

			input.matWorld = mat3*mat2*mat1;
			p_sh->set_shader_input(&input, (void*)&matProjView);

			p_vis->set();
			p_sh->setup_render();
			p_vis->draw();
		}
		p_ds_lesseq->unset();
	}
	//cout << "after disk" << endl;
	//draw the fence
	if (t.b_vis_target)
	{
		r_shader_basic *p_sh_vis = (r_shader_basic*)mff_singleton::get()->get_shader("vis_fence");
		shared_ptr<r_geometry> p_vis = mff_singleton::get()->get_geom("fence");
		r_blendstate *p_blend_alpha = mff_singleton::get()->get_blend("alpha");

		r_shader_input input;
		input.matWorld = Identity();
		p_sh_vis->set_shader_input(&input, (void*)&matProjView);

		t.p_screen->set();
		p_blend_alpha->set();
		p_vis->set();
		p_sh_vis->setup_render();
		p_vis->draw();
		p_blend_alpha->unset();
	}
}


//void worker_microfacet::work_task_initial_render(task_microfacet *t_org)
//{
//	tm.update();
//	task_initial_render &t = t_org->tir;
//	Matrix4 matProjView = t_org->matProj*t_org->matView;
//
////**************************
////#1 : Context pass (To generate uv, normal and tangent buffers)
////**************************
//	t.p_screen->clear_mrt();
//	t.p_screen->add_rt(t.p_target_uv);
//	t.p_screen->add_rt(t.p_target_normal);
//	t.p_screen->add_rt(t.p_target_tangent);
//	t.p_screen->clear_depth();
//	t.p_screen->set();
//	float clearUV[4] = {-1, -1, -1, -1};
//	t.p_screen->get_rt(0)->clear(clearUV);
//	
//	mff_singleton::get()->get_layer("init_context")->set();
//	//((r_shader_basic*)t.p_main->get_shader())->setup_cb_basic(t.p_main->get_shader_input(), matProjView);
//	t.p_main->get_shader()->set_shader_input(t.p_main->get_shader_input(), &matProjView);
//	t.p_main->draw();
//	for (int i = 0; i < t.p_background->size(); i++)
//	{
//		r_instance *pi = (*t.p_background)[i];
//		//((r_shader_basic*)pi->get_shader())->setup_cb_basic(pi->get_shader_input(), matProjView);
//		pi->get_shader()->set_shader_input(pi->get_shader_input(), &matProjView);
//		pi->draw();
//	}
//
//	t.p_screen->get_rt(0)->get_result((BYTE*)t.p_uv, 8);
//	t.p_screen->get_rt(1)->get_result((BYTE*)t.p_normal, 8);
//	//t.p_screen->get_rt(2)->get_result((BYTE*)t.p_tangent, 8);
//	tm.update();
//	printf_s("#1 : %7.4f ", tm.elapsed_time());
//
////**************************
////#2 : Shadow pass
////**************************
//	t.p_screen->clear_mrt();
//	t.p_screen->add_rt(t.p_target_vis);
//	float clearVis[4] = {0, 0, 0, 0};
//
//	r_blendstate	*p_blend_add = mff_singleton::get()->get_blend("add");
//	r_dsstate		*p_ds_lesseq = mff_singleton::get()->get_ds("lesseq");
//	r_shader_basic	*p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
//	r_shader_vis	*p_sh_use_shadow = (r_shader_vis*)mff_singleton::get()->get_shader("init_shadow");
//
//	mff_singleton::get()->get_layer("init_shadow")->set();
//	for (int batch = 0; batch < t.vis_pixel_size; batch++)
//	{
//		t.p_screen->get_rt(0)->clear(clearVis);
//		for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
//		{
//			//gen shadow
//			Matrix4 matLightProjView = t.p_shadow->mat_light_proj * t.p_shadow->mat_light_view[l+batch*NUM_BITS_VIS];
//
//			t.p_shadow->set_gen_shadow(0);
//			//p_sh_shadow->setup_cb_basic(t.p_main->get_shader_input(), matLightProjView);
//			p_sh_shadow->set_shader_input(t.p_main->get_shader_input(), &matLightProjView);
//			t.p_main->draw_with_shader(p_sh_shadow);
//			for (int i = 0; i < t.p_background->size(); i++)
//			{
//				r_instance *pi = (*t.p_background)[i];
//				//p_sh_shadow->setup_cb_basic(pi->get_shader_input(), matLightProjView);
//				p_sh_shadow->set_shader_input(pi->get_shader_input(), &matLightProjView);
//				pi->draw_with_shader(p_sh_shadow);
//			}
//			t.p_shadow->unset(0, NULL);
//
//			//use shadow
//			int vis_mask = (1 << l);
//			p_sh_use_shadow->setup_cb_lights(&t.p_shadow->lights[l+batch*NUM_BITS_VIS], vis_mask, &matLightProjView);
//			t.p_shadow->set_use_shadow(0, p_sh_use_shadow);
//			t.p_screen->set();
//			p_blend_add->set();
//			p_ds_lesseq->set();
//
//			//p_sh_use_shadow->setup_cb_basic(t.p_main->get_shader_input(), matProjView);
//			p_sh_use_shadow->set_shader_input(t.p_main->get_shader_input(), &matProjView);
//			t.p_main->draw_with_shader(p_sh_use_shadow);
//
//			p_blend_add->unset();
//			p_ds_lesseq->unset();
//			t.p_shadow->unset(0, p_sh_shadow);
//		}
//
//		//This read-in is causing a lot of delay. Try to avoid this.
//		t.p_screen->get_rt(0)->get_result((BYTE*)&t.p_vis_buffer[t_org->width*t_org->height*batch], 4);
//	}
//	tm.update();
//	printf_s("#2 : %7.4f ", tm.elapsed_time());
//	
////**************************
////#3 : Test pass
////**************************
//	for (int y = 0; y < t_org->height; y++)
//		for (int x = 0; x < t_org->width; x++)
//		{
//			UINT	pixel = 0;
//			int		idx = x+y*t_org->width;
//			if (t.p_uv[idx*2] >= 0)
//			{
//				//vector2 uv;
//				Vector3 normal, tangent, binormal;
//
//				//uv.x = t.p_uv[idx*2];
//				//uv.y = t.p_uv[idx*2+1];
//
//				normal.x = snorm2float(t.p_normal[idx*4]);
//				normal.y = snorm2float(t.p_normal[idx*4+1]);
//				normal.z = snorm2float(t.p_normal[idx*4+2]);
//				//tangent.x = snorm2double(t.p_tangent[idx*4]);
//				//tangent.y = snorm2double(t.p_tangent[idx*4+1]);
//				//tangent.z = snorm2double(t.p_tangent[idx*4+2]);
//				//binormal = tangent ^ normal;
//
//				Vector3 c;
//				for (int batch = 0; batch < t.vis_pixel_size; batch++)
//				{
//					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
//					for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
//						if (mask & (1<<l))
//						{
//							float dot = t.p_shadow->lights[l+batch*NUM_BITS_VIS].dir * normal;
//							if (dot > 0)
//							{
//								c += dot * t.p_shadow->lights[l+batch*NUM_BITS_VIS].c;
//							}
//						}
//				}
//
//				BYTE r, g, b;
//				r = min(max(c.x, 0), 1)*255;
//				g = min(max(c.y, 0), 1)*255;
//				b = min(max(c.z, 0), 1)*255;
//				pixel = (b << 16) + (g << 8) + (r);
//			}
//			t.result[idx] = pixel;
//		}
//	tm.update();
//	printf_s("#3 : %7.4f\n", tm.elapsed_time());
//}
//

//worker definition
void worker_microfacet::work(shared_ptr<subtask> &st, void* p_per_thread_global_var)
{
	task_microfacet *t = (task_microfacet*)(st->p_task);
	switch (t->type)
	{
	case TASK_TYPE_MICROFACET_CHANGED:
		work_task_microfacet_changed(t);
		break;
	case TASK_TYPE_BUFFER:
		work_task_buffer(t);
		break;
	case TASK_TYPE_RENDER:
		{
			qrender_block *pb = (qrender_block*)(st.get()); // Error may caused here!!!
			work_task_render_block(t, pb);
		}
		break;
	case TASK_TYPE_VISUALIZATION:
		work_task_visual(t);
		break;
	//for DEBUGGING only
	case TASK_TYPE_GROUND_TRUTH:
		work_task_ground_truth(t);
		break;
	case TASK_TYPE_GROUND_TRUTH_DIRECT:
		work_task_ground_truth_direct(t);
		break;
	case TASK_TYPE_GROUND_TRUTH_DEBUG_PIXEL:
		work_task_ground_truth_debug_pixel(t);
		break;
	case TASK_TYPE_RENDER_DEBUG:
		work_task_render_debug(t);
		break;
	case TASK_TYPE_RENDER_BEFORE_SVD:
		work_task_render_before_SVD(t);
		break;
	case TASK_TYPE_GEN_ANIMATION:
		work_task_animation(t);
		break;
	case TASK_TYPE_RENDER_REF_BRDF:
		work_task_render_ref_BRDF(t);
		break;
	case TASK_TYPE_RENDER_REF_BRDF_DEBUG_PIXEL:
		work_task_render_ref_BRDF_debug_pixel(t);
		break;
	case TASK_TYPE_DEBUG_RAYTRACE_PIXEL:
		work_task_debug_raytrace_pixel(t);
		break;
	case TASK_TYPE_DEBUG_RAYTRACE:
		{
			qrender_block *pb = (qrender_block*)(st.get());
			work_task_raytrace_block(t, pb);
		}
		break;
	}
}

//FOR RGBA method
//void worker_microfacet::work(shared_ptr<subtask*> &st, void* p_per_thread_global_var)
//{
//	//codex::utils::timer tm, tm_local, tm_draw;
//	float	t_gen_shadow, t_render_shadow, t_misc, 
//		t_draw, t_transform,
//		t_draw2, t_transform2;
//
//	t_gen_shadow = t_render_shadow = t_misc = 
//		t_draw = t_transform = 
//		t_draw2 = t_transform2 = 0;
//
//	task_microfacet *t = (task_microfacet*)(((subtask*)st)->p_task);
//	task_render &tr = t->tr;
//
//	//tm_local.update();
//	Matrix4 matProjView = t->matProj*t->matView;
//	float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f};
//	tr.p_screen->set();
//	tr.p_screen->clear_depth();
//	tr.p_skybox->draw(matProjView);
//
//	//tm_local.update();
//	//t_misc += tm_local.elapsed_time();
//
//	Matrix4 mat;
//	r_blendstate	*p_blend_add = t->mff_singleton::get()->get_blendstate_add();
//	r_dsstate		*p_ds_lesseq = t->mff_singleton::get()->get_dsstate_lesseq();
//	r_shader_basic	*p_sh_shadow = (r_shader_basic*)t->mff_singleton::get()->get_sh_shadow();
//
//	for (int batch = 0; batch < tr.p_shadow->lights.size()/4; batch++)
//	{
//		//gen shadow begin
//		//tm_local.update();
//		Matrix4 matLightProjView[4];
//		for (int l = 0; l < 4; l++)
//			matLightProjView[l] = tr.p_shadow->mat_light_proj * tr.p_shadow->mat_light_view[batch*4+l];
//
//		t->mff_singleton::get()->set_light(&tr.p_shadow->lights[batch*4], matLightProjView);
//
//		tr.p_shadow->set_gen_shadow(0);
//
//		//tm_draw.update();
//		for (int l = 0; l < 4; l++)
//		{
//			tr.p_shadow->clear_depth(0);
//			r_blendstate *p_blend = t->mff_singleton::get()->get_blendstate_shadow(l);
//			p_blend->set();
//			p_sh_shadow->setup_cb_basic(tr.p_main->get_shader_input(), matLightProjView[l]);
//			tr.p_main->draw_with_shader(p_sh_shadow);
//			p_sh_shadow->setup_cb_basic(tr.p_background->get_shader_input(), matLightProjView[l]);
//			tr.p_background->draw_with_shader(p_sh_shadow);
//			p_blend->unset();
//		}
//		//tm_draw.update();
//		//t_draw += tm_draw.elapsed_time();
//
//		tr.p_shadow->unset(0, NULL);
//		//tm_local.update();
//		//t_gen_shadow += tm_local.elapsed_time();
//
//		//gen shadow end
//
//		tr.p_shadow->set_use_shadow(0, p_sh_shadow);
//		tr.p_screen->set();
//		if (batch > 0)
//		{
//			p_blend_add->set();
//			p_ds_lesseq->set();
//		}
//
//		//tm_draw.update();
//		((r_shader_basic*)tr.p_main->get_shader())->setup_cb_basic(tr.p_main->get_shader_input(), matProjView);
//		tr.p_main->draw();
//		((r_shader_basic*)tr.p_background->get_shader())->setup_cb_basic(tr.p_background->get_shader_input(), matProjView);
//		tr.p_background->draw();
//		//tm_draw.update();
//		//t_draw2 += tm_draw.elapsed_time();
//
//		if (batch > 0)
//		{
//			p_blend_add->unset();
//			p_ds_lesseq->unset();
//		}
//		tr.p_shadow->unset(0, p_sh_shadow);
//		//tm_local.update();
//		//t_render_shadow += tm_local.elapsed_time();
//	}
//}

worker_microfacet::worker_microfacet()
:num_pixels_computed(0), block_size_x(BLOCK_X), block_size_y(BLOCK_Y)
{

}

void worker_microfacet::init_task(shared_ptr<task>& t_org)
{
	task_microfacet *t = (task_microfacet*)t_org.get();
	if (t->type == TASK_TYPE_RENDER || t->type == TASK_TYPE_DEBUG_RAYTRACE)
	{
		num_pixels_computed = 0;
		num_block_x = (t->width-1)  / block_size_x + 1;
		num_block_y = (t->height-1) / block_size_y + 1;
		alloc_table.resize(num_block_x*num_block_y);
		alloc_table.assign(alloc_table.size(), false);

		if (t->type == TASK_TYPE_DEBUG_RAYTRACE)
		{
			val.clear();
			val.resize(t->width*t->height);
		}
	} else {
		b_alloc = false;
	}
}

void worker_microfacet::finish_task(shared_ptr<task>& t_org)
{
	task_microfacet *t = (task_microfacet*)t_org.get();
	switch (t->type)
	{
	case TASK_TYPE_BUFFER:
		t->tb.p_screen->get_rt(0)->get_result((BYTE*)t->tb.result, 4);
		break;
	case TASK_TYPE_VISUALIZATION:
		t->trv.p_screen->get_rt(0)->get_result((BYTE*)t->trv.result, 4);
		break;
	}
}

int worker_microfacet::get_subtask(shared_ptr<subtask>& t, shared_ptr<task>& tk)
{
	task_microfacet *t_current = (task_microfacet*)tk.get();
	if (t_current->type == TASK_TYPE_RENDER || t_current->type == TASK_TYPE_DEBUG_RAYTRACE)
	{
		qrender_block *block = new qrender_block;
		t = shared_ptr<subtask>(block);

		for (int y = 0; y < num_block_y; y++)
			for (int x = 0; x < num_block_x; x++)
				if (alloc_table[x + y*num_block_x] == false)
				{
					alloc_table[x + y*num_block_x] = true;

					int x0, y0, x1, y1;
					x0 = x*block_size_x;
					y0 = y*block_size_y;
					x1 = x0 + min(block_size_x, t_current->width - x0);
					y1 = y0 + min(block_size_y, t_current->height - y0);

					block->x0 = x0;
					block->y0 = y0;
					block->x1 = x1;
					block->y1 = y1;

					if (block->x1 > block->x0 && block->y1 > block->y0)
						return 1;
				}
	} else {
		if (!b_alloc)
		{
			b_alloc = true;

			subtask* p = new subtask;
			t = shared_ptr<subtask>(p);
			return 1;
		}
	}

	return 0;
}

bool worker_microfacet::complete_subtask(shared_ptr<subtask>& t)
{
	task_microfacet *t_current = (task_microfacet*)t->p_task;
	if (t_current->type == TASK_TYPE_RENDER || t_current->type == TASK_TYPE_DEBUG_RAYTRACE)
	{
		qrender_block &block = *(qrender_block*)t.get();

		num_pixels_computed += (block.x1 - block.x0) * (block.y1 - block.y0);

		if (t_current->type == TASK_TYPE_DEBUG_RAYTRACE)
		{
			printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf_s("(%d/%d) ", num_pixels_computed+1, t_current->width*t_current->height);
			if (num_pixels_computed == t_current->width*t_current->height)
			{
				printf_s("\n");
				work_task_raytrace_direct(t_current);
			}
		}

		return (num_pixels_computed == t_current->width*t_current->height);
	} else {
		return true;
	}
}