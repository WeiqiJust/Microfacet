#include "MicrofacetEditor.h"

void MicrofacetEditor::render_image(task_microfacet *pt)
{
	shared_ptr<task> t(pt);

	pt->b_async = true;
	pt->per_thread_global_var
		= NULL;
	pt->p_worker = p_worker;
	p_manager->start(t);
}


void MicrofacetEditor::compute_microfacet_change()
{
	task_microfacet *t = new task_microfacet;
	t->type = TASK_TYPE_MICROFACET_CHANGED;

	selection_compute = selection;
	//cout << "compute microfacet change:: sube type = " << microfacet_ops << endl;
	t->tmc.sub_type = microfacet_ops;
	t->tmc.details = details;
	t->tmc.idx = &selection_compute;
	t->tmc.blocks = &final_details;
	t->tmc.fr_Avis = &fr_Avis;
	t->tmc.fr_vis = &fr_vis;
	t->tmc.avis_sampler = &avis_sampler;
	t->tmc.vismask_sampler
		= &vismask_sampler;
	t->tmc.rp = &rand_proj;
	render_image(t);

	microfacet_ops = 0;
	b_microfacet_changed = false;
}

void MicrofacetEditor::render_buffer()
{

	task_microfacet *t = new task_microfacet;
	shared_ptr<task> t_done;
	/*************test code****************/
	while (!p_manager->get_result(t_done))
	{
		Sleep(1);
	}
	t->type = TASK_TYPE_BUFFER;
	t->id = 0;
	t->width = render_width;
	t->height = render_height;
	t->tb.p_background = &pi_background;
	t->tb.p_main = pi_base;
	t->tb.p_background_vis = &pi_background_vis;
	t->tb.p_main_vis = pi_base_vis;
	t->tb.p_skybox = p_skybox;
	t->tb.p_shadow = &shadow_maps;
	t->tb.p_target = p_target;
	t->tb.p_target_uv = p_target_uv;
	t->tb.p_target_normal = p_target_normal;
	t->tb.p_target_tangent = p_target_tangent;
	t->tb.p_target_vis = p_target_vis;
	t->tb.p_screen = p_screen;
	t->tb.p_uv = p_uv;
	t->tb.p_normal = p_normal;
	t->tb.p_tangent = p_tangent;
	t->tb.vis_pixel_size = vis_pixel_size;
	t->tb.p_vis_buffer = p_vis_buffer;
	t->tb.result = p_background_buffer;
	t->tb.envlight_inten = envlight_inten;

	t->tb.p_target_test = p_target_test;
	t->tb.p_test = p_test;
	t->tb.p_main_test = pi_test;

	trackball_param param;
	t->matProj = mat_proj;

	param = tball_distant.get_params();
	matrix_lookat(t->matView, param.eye, param.lookat, Vector3(0, 1, 0));
	
	/*
	for (int i = 0; i < 4; i++)
	{
	for (int j = 0; j < 4; j++)
	{
	cout << t->matView.m[i][j] << " ";
	}
	cout << endl;
	}*/

	render_image(t);
}

void MicrofacetEditor::render()
{
	task_microfacet *t = new task_microfacet;
	shared_ptr<task> t_done;
	while (!p_manager->get_result(t_done))
	{
		Sleep(1);
	}
	t->type = TASK_TYPE_RENDER;
	t->id = 0;
	t->width = render_width;
	t->height = render_height;
	t->r.blocks = &final_details;
	t->r.details = details;
	t->r.result = p_background_buffer;
	t->r.vis_pixel_size
		= vis_pixel_size;
	t->r.fr_Avis = &fr_Avis;
	t->r.fr_vis = &fr_vis;

	t->r.p_shadow = &shadow_maps;
	t->r.p_normal = p_normal;
	t->r.p_tangent = p_tangent;

	t->r.p_uv = p_uv;
	t->r.p_vis_buffer = p_vis_buffer;
	t->r.p_wo = &p_wo;
	t->r.dbg_pixel_x = -1;
	t->r.dbg_pixel_y = -1;

	trackball_param param;
	param = tball_distant.get_params();
	Vector3 v = param.eye - param.lookat;
	v.normalize();
	v_render_lookat.x = (float)v.x;
	v_render_lookat.y = (float)v.y;
	v_render_lookat.z = (float)v.z;
	v_render_up = Vector3(0, 1, 0);
	v_render_up -= (v_render_up*v_render_lookat)*v_render_lookat;
	v_render_up.normalize();
	v_render_right = Cross(v_render_up, v_render_lookat);
	v_render_right.normalize();
	t->r.v_lookat = &v_render_lookat;
	t->r.v_up = &v_render_up;
	t->r.v_right = &v_render_right;

	render_image(t);
}

void MicrofacetEditor::debug_our_pixel()
{
	task_microfacet *t = new task_microfacet;
	shared_ptr<task> t_done;
	while (!p_manager->get_result(t_done))
	{
		Sleep(1);
	}
	if (b_scene_ready && p_manager->is_free())
	{
		printf_s("debug our pixel (%d %d)...\n", dbg_pixel_x, dbg_pixel_y);
		task_microfacet *t = new task_microfacet;

		t->type = TASK_TYPE_RENDER_DEBUG;
		t->id = 0;
		t->width = render_width;
		t->height = render_height;
		t->r.blocks = &final_details;
		t->r.details = details;
		t->r.result = p_background_buffer;
		t->r.vis_pixel_size
			= vis_pixel_size;
		t->r.fr_Avis = &fr_Avis;
		t->r.fr_vis = &fr_vis;

		t->r.p_shadow = &shadow_maps;
		t->r.p_normal = p_normal;
		t->r.p_tangent = p_tangent;
		t->r.p_uv = p_uv;
		t->r.p_vis_buffer = p_vis_buffer;
		t->r.p_wo = &p_wo;
		t->r.dbg_pixel_x = 128;
		t->r.dbg_pixel_y = 128;

		trackball_param param;
		param = tball_distant.get_params();
		Vector3 v = param.eye - param.lookat;
		v.normalize();
		v_render_lookat.x = (float)v.x;
		v_render_lookat.y = (float)v.y;
		v_render_lookat.z = (float)v.z;
		v_render_up = Vector3(0, 1, 0);
		v_render_up -= (v_render_up*v_render_lookat)*v_render_lookat;
		v_render_up.normalize();
		v_render_right = Cross(v_render_up ,v_render_lookat);
		v_render_right.normalize();
		t->r.v_lookat = &v_render_lookat;
		t->r.v_up = &v_render_up;
		t->r.v_right = &v_render_right;

		render_image(t);
	}
}

void MicrofacetEditor::compute_ground_truth_BRDF()
{
	task_microfacet *t = new task_microfacet;
	t->type = TASK_TYPE_MICROFACET_CHANGED;

	selection_compute = selection;
	t->tmc.sub_type = TASK_SUBTYPE_BRDF_TRUTH;
	t->tmc.details = details;
	t->tmc.idx = &selection_compute;
	t->tmc.blocks = &final_details;
	t->tmc.fr_Avis = &fr_Avis;
	t->tmc.fr_vis = &fr_vis;
	t->tmc.avis_sampler = &avis_sampler;
	t->tmc.vismask_sampler
		= &vismask_sampler;
	t->tmc.rp = &rand_proj;
	render_image(t);
}

void MicrofacetEditor::render_ground_truth()
{
	render_ground_truth_task(TASK_TYPE_GROUND_TRUTH);
}


void MicrofacetEditor::render_ref_BRDF()
{
	render_ground_truth_task(TASK_TYPE_RENDER_REF_BRDF);
}


void MicrofacetEditor::render_ground_truth_task(const int type, const int num_levels)
{

	task_microfacet *t = new task_microfacet;
	shared_ptr<task> t_done;
	while (!p_manager->get_result(t_done))
	{
		Sleep(1);
	}
	if (b_scene_ready && p_manager->is_free())
	{
		printf_s("rendering ground truth...\n");

		task_microfacet *t = new task_microfacet;

		t->type = type;
		t->id = 2;
		t->width = render_width;
		t->height = render_height;
		t->trt.blocks = &final_details;
		t->trt.details = details;
		t->trt.result = p_background_buffer;
		t->trt.vis_pixel_size
			= vis_pixel_size;

		t->trt.p_shadow = &shadow_maps;
		t->trt.p_normal = p_normal;
		t->trt.p_tangent = p_tangent;
		t->trt.p_uv = p_uv;
		t->trt.p_vis_buffer = p_vis_buffer;
		t->trt.p_wo = &p_wo;
		t->trt.dbg_pixel_x = -1;
		t->trt.dbg_pixel_y = -1;
		t->trt.num_raytrace_level = num_levels;
		t->trt.num_rays = 16;// ui.spin_num_rays->value();
		t->trt.dim_raytrace = 5;// ui.spin_dim_raytrace->value();

		trackball_param param;
		param = tball_distant.get_params();
		Vector3 v = param.eye - param.lookat;
		v.normalize();
		v_render_lookat.x = (float)v.x;
		v_render_lookat.y = (float)v.y;
		v_render_lookat.z = (float)v.z;
		v_render_up = Vector3(0, 1, 0);
		v_render_up -= (v_render_up*v_render_lookat)*v_render_lookat;
		v_render_up.normalize();
		t->trt.v_lookat = &v_render_lookat;
		t->trt.v_up = &v_render_up;

		render_image(t);
	}
}

void MicrofacetEditor::compute_normal_mapped_BRDF()
{
	task_microfacet *t = new task_microfacet;
	t->type = TASK_TYPE_MICROFACET_CHANGED;

	selection_compute = selection;
	t->tmc.sub_type = TASK_SUBTYPE_BRDF_NORMAL;
	t->tmc.details = details;
	t->tmc.idx = &selection_compute;
	t->tmc.blocks = &final_details;
	t->tmc.fr_Avis = &fr_Avis;
	t->tmc.fr_vis = &fr_vis;
	t->tmc.avis_sampler = &avis_sampler;
	t->tmc.vismask_sampler
		= &vismask_sampler;
	t->tmc.rp = &rand_proj;
	render_image(t);
}


void MicrofacetEditor::render_visualization()
{
	/*task_microfacet *t = new task_microfacet;
	shared_ptr<task> t_done;
	while (!p_manager->get_result(t_done))
	{
	Sleep(1);
	}*/
	if (p_manager->is_free())
	{
		int idx = visual_x + visual_y*render_width;
		if (p_uv[idx * 2] >= 0)
		{
			printf_s("rendering visualization...\n");

			dbg_pixel_x = visual_x;
			dbg_pixel_y = visual_y;

			Vector3 normal, tangent, binormal, global_wo;
			normal.x = snorm2float(p_normal[idx * 4]);
			normal.y = snorm2float(p_normal[idx * 4 + 1]);
			normal.z = snorm2float(p_normal[idx * 4 + 2]);
			tangent.x = snorm2float(p_tangent[idx * 4]);
			tangent.y = snorm2float(p_tangent[idx * 4 + 1]);
			tangent.z = snorm2float(p_tangent[idx * 4 + 2]);
			binormal = normal ^ tangent;

			task_microfacet *t = new task_microfacet;
			t->type = TASK_TYPE_VISUALIZATION;
			t->id = 1;
			t->width = render_width;
			t->height = render_height;

			int block_x, block_y, block_idx;
			block_idx = details->compute_idx(0, 0);
			details->compute_back_idx(block_x, block_y, block_idx);


			trackball_param param;
			param = tball_distant.get_params();
			matrix_lookat(t->matView, Vector3(0, 0, 3), Vector3(0, 0, 0), Vector3(0, 1, 0));
			t->matProj = mat_proj_vis_illu;

			t->trv.p_block = &final_details[block_idx];
			t->trv.p_screen = p_screen;
			t->trv.p_shadow = &shadow_visual;
			t->trv.p_target = p_target;
			t->trv.result = p_screen_buffer;
			t->trv.b_vis_target = false;// true;// (ui.ck_vis_target->checkState() == Qt::Checked);
			t->trv.b_show_neighbors = true;// (ui.ck_show_neighbor_tiles->checkState() == Qt::Checked);
			t->trv.vis_light_inten = vis_light_inten;
			t->trv.disc_r = -1;

			std::vector<r_light_dir> lights;
			lights = shadow_maps.lights;
			for (int i = 0; i < lights.size(); i++)
			{
				lights[i].dir = Vector3(lights[i].dir*tangent, lights[i].dir*binormal, lights[i].dir*normal);
				if (lights[i].dir.z <= 0)
					lights[i].c = Vector3(0, 0, 0);
			}
			shadow_visual.set_lights(lights);
			shadow_visual.compute_light_matrix(Vector3(0, 0, 0));

			render_image(t);
			//DEBUG
		}
		else {
			printf_s("invalid pixel...\n");
		}
		//DEBUG
	}
	else {
		printf_s("busy...\n");
	}
}

void MicrofacetEditor::gen_anim(const int subtype)
{

	trackball_param param;
	param = tball_distant.get_params();
	v_global_eye =  Vector3(0, 0, -10.0f);
	v_global_at = Vector3(0.0f);

	task_microfacet *t = new task_microfacet;
	shared_ptr<task> t_done;
	/*************test code****************/
	while (!p_manager->get_result(t_done))
	{
		Sleep(1);
	}

	t->type = TASK_TYPE_GEN_ANIMATION;
	t->id = 0;
	t->width = render_width;
	t->height = render_height;
	t->matProj = mat_proj;

	t->ta.sub_type = subtype;
	t->ta.angle_start = 0;// ui.spin_anim_start->value();
	t->ta.angle_finish = 23360;// ui.spin_anim_finish->value();
	t->ta.angle_num = 50;// ui.spin_anim_num->value();

	t->ta.blocks = &final_details;
	t->ta.details = details;
	t->ta.result = p_background_buffer;
	t->ta.vis_pixel_size
		= vis_pixel_size;

	t->ta.p_shadow = &shadow_maps;
	t->ta.p_normal = p_normal;
	t->ta.p_tangent = p_tangent;
	t->ta.p_uv = p_uv;
	t->ta.p_vis_buffer = p_vis_buffer;
	t->ta.p_wo = &p_wo;
	t->ta.fr_Avis = &fr_Avis;
	t->ta.fr_vis = &fr_vis;
	t->ta.eye = &v_global_eye;
	t->ta.lookat = &v_global_at;

	t->ta.p_background = &pi_background;
	t->ta.p_main = pi_base;
	t->ta.p_background_vis = &pi_background_vis;
	t->ta.p_main_vis = pi_base_vis;
	t->ta.p_skybox = p_skybox;
	t->ta.p_target = p_target;
	t->ta.p_target_uv = p_target_uv;
	t->ta.p_target_normal = p_target_normal;
	t->ta.p_target_tangent = p_target_tangent;
	t->ta.p_target_vis = p_target_vis;
	t->ta.p_screen = p_screen;

	render_image(t);
}

void MicrofacetEditor::gen_anim_ours()
{
	gen_anim(TASK_SUBTYPE_ANIM_OURS);
}

void MicrofacetEditor::gen_anim_truth()
{
	gen_anim(TASK_SUBTYPE_ANIM_TRUTH);
}

void MicrofacetEditor::render_raytrace()
{
	render_ground_truth_task(TASK_TYPE_DEBUG_RAYTRACE);
}

void MicrofacetEditor::update_render()
{
	shared_ptr<task> t_done;
	while (!p_manager->get_result(t_done))
	{
		Sleep(1);
	}
	task_microfacet* p = (task_microfacet*)(t_done.get());
	switch (p->type)
	{
	case TASK_TYPE_RENDER:
	case TASK_TYPE_RENDER_BEFORE_SVD:

		//t_frame.update();
		cout << "finish rendering." << endl;
		save_image("T:/Microfacet/output/img_render.png", p->r.result, p->width, p->height);
		break;
	case TASK_TYPE_VISUALIZATION:
		save_image("T:/Microfacet/output/img_detail.png", p->trv.result, p->width, p->height);
		break;
	case TASK_TYPE_BUFFER:
		b_buffer_ready = true;
		b_render_image = true;
		break;
	case TASK_TYPE_GROUND_TRUTH:
		cout << "finish rendering." << endl;
		save_image("T:/Microfacet/output/img_ground_truth.png", p->trt.result, p->width, p->height);
		break;
	case TASK_TYPE_GROUND_TRUTH_DIRECT:
	case TASK_TYPE_RENDER_REF_BRDF:
	case TASK_TYPE_DEBUG_RAYTRACE:
		save_image("T:/Microfacet/output/img_ref_brdf.png", p->trt.result, p->width, p->height);
		break;
	}
}

