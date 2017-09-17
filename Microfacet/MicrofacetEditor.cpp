#include "MicrofacetEditor.h"
#include "distr_grid.h"
#include "binder_plane.h"
#include "binder_groove.h"
#include "distr_rod.h"
#include "distr_low_discrepancy.h"

#define MAX_BACKGROUND_OBJ	10

MicrofacetEditor::MicrofacetEditor()
: p_manager(NULL), p_base(NULL), p_skybox(NULL), p_screen(NULL),
pi_base(NULL), pi_background(NULL), pi_base_vis(NULL), pi_background_vis(NULL),
p_screen_buffer(NULL), p_background_buffer(NULL), p_vis_buffer(NULL),
dbg_pixel_x(0), dbg_pixel_y(0), details(NULL), b_scene_ready(false),
b_microfacet_changed(true), b_buffer_ready(false), b_render_image(false),
visual_x(0), visual_y(0)
{
	mff_singleton::init();
	//p_editor = this;
	//UI Initialization
	
	gpu_env.init();

#ifndef SUPER_RESOLUTION
	render_width = RENDER_WIDTH;
	render_height = RENDER_HEIGHT;
#else
	render_width = SUPER_RES_X,
		render_height = SUPER_RES_Y;
#endif
	visual_x = render_width / 2;
	visual_y = render_height / 2;

	p_screen_buffer = new UINT[render_width*render_height];
	p_background_buffer
		= new UINT[render_width*render_height];
	p_uv = new float[render_width*render_height * 2];
	p_normal = new short[render_width*render_height * 4];
	p_tangent = new short[render_width*render_height * 4];
	p_test = new float[render_width*render_height * 2];

	p_target = new render_target(gpu_env.get_handle());
	p_target->init(render_width, render_height, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, R_TARGET_CPU_READ);

	p_target_uv = new render_target(gpu_env.get_handle());
	p_target_uv->init(render_width, render_height, 1, 1, DXGI_FORMAT_R32G32_FLOAT, R_TARGET_CPU_READ);
	p_target_normal = new render_target(gpu_env.get_handle());
	p_target_normal->init(render_width, render_height, 1, 1, DXGI_FORMAT_R16G16B16A16_SNORM, R_TARGET_CPU_READ);
	p_target_tangent = new render_target(gpu_env.get_handle());
	p_target_tangent->init(render_width, render_height, 1, 1, DXGI_FORMAT_R16G16B16A16_SNORM, R_TARGET_CPU_READ);
	p_target_vis = new render_target(gpu_env.get_handle());
	p_target_vis->init(render_width, render_height, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);

	p_target_test = new render_target(gpu_env.get_handle());
	p_target_test->init(render_width, render_height, 1, 1, DXGI_FORMAT_R32G32_FLOAT, R_TARGET_CPU_READ);

	p_screen = new render_output(gpu_env.get_handle());
	p_screen->init(render_width, render_height, 1);
	p_screen->add_rt(p_target);

	float z_near = 1e-3f, z_far = 100.0f;
	projection_perspective(mat_proj, 45.0f, render_width, render_height, z_near, z_far);

	mat_proj_vis_illu = mat_proj;
	setup_p_wo(2.0, 2.0, z_near);

	float r = 10.0f;
	z_near = 1e-3*r;
	z_far = 20 * r;
	projection_orthogonal(mat_proj_vis_direct, r, r, z_near, z_far);

	p_factory = new microfacet_factory(gpu_env.get_handle());
	mff_singleton::set(p_factory);

	p_manager = new task_manager(4, 1);
	p_worker = new worker_microfacet();

	scene_center = Vector3(0, 0, 0);
	scene_radius = 4.0;
	shadow_maps.init(gpu_env.get_handle(), mff_singleton::get()->get_sampler("shadow"),
		1, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT, scene_radius);
	shadow_visual.init(gpu_env.get_handle(), mff_singleton::get()->get_sampler("shadow"),
		1, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT, scene_radius);

	init_dirs();
	load_scene();
	vector<sample_group> samples;
	//vismask_sampler.compute_vis_masks(samples, 0*NUM_VIS_PER_BATCH, NUM_VIS_PER_BATCH);
	//cout << "after constructe" << endl;

	//timer = new QTimer(this);
	//connect_actions();
	//timer->start(1);

	//read_settings();
}

MicrofacetEditor::~MicrofacetEditor()
{
	SAFE_DELETE(p_manager);
	SAFE_DELETE(p_base);
	SAFE_DELETE(p_factory);
	SAFE_DELETE(p_skybox);
	SAFE_DELETE(p_screen);
	SAFE_DELETE_ARR(p_screen_buffer);
	SAFE_DELETE(pi_base);
	SAFE_DELETE(pi_base_vis);
	for (int i = 0; i < pi_background.size(); i++)
		SAFE_DELETE(pi_background[i]);
	for (int i = 0; i < pi_background_vis.size(); i++)
		SAFE_DELETE(pi_background_vis[i]);
}

void MicrofacetEditor::setup_p_wo(const double &ratio_w, const double &ratio_h, const double &z_near)
{
	p_wo.resize(render_width*render_height);

	double	w_near = z_near*ratio_w,
		h_near = z_near*ratio_h;

	for (int y = 0; y < render_height; y++)
		for (int x = 0; x < render_width; x++)
		{
			double	fx = (double)x / (render_width - 1),
				fy = (double)y / (render_height - 1);
			fx = fx * 2 - 1;
			fy = 1 - fy * 2;

			Vector3 v;
			v.x = (float)fx*w_near / 2;
			v.y = (float)fy*h_near / 2;
			v.z = -(float)z_near;
			v = -v;
			v.normalize();

			p_wo[x + y*render_width] = v;
		}
}

void MicrofacetEditor::init_dirs()
{
	random rng;
	Vector2 rv_offset;
	rv_offset = Vector2(rng.get_random_float(), rng.get_random_float());
	//uniform_hemi_direction_3d<float> prob;

	////vis_dir
	//codex::math::utils::Hammersley_Zaremba_seq<float> seq_vis(NUM_VIS_DIR);
	//for (int i = 0; i < NUM_VIS_DIR; i++)
	//{
	//	float pdf;
	//	Vector3 v, rv;

	//	seq_vis.get_sample(rv);
	//	rv.x = add_random_offset(rv.x, rv_offset.x);
	//	rv.y = add_random_offset(rv.y, rv_offset.y);
	//	prob.sample(v, pdf, rv);
	//	v.normalize();

	//	//v = Vector3(0, 0, 1);
	//	vis_dir.push_back(v);
	//}

	////reorganize vis_dir
	//int interval = NUM_VIS_DIR / NUM_PREVIS_DIR,
	//	start1 = 0, 
	//	start2 = NUM_PREVIS_DIR;
	//std::vector<Vector3> temp_dir;
	//temp_dir.resize(vis_dir.size());
	//for (int i = 0; i < NUM_VIS_DIR; i++)
	//	if ((i % interval) == 0)
	//	{
	//		temp_dir[start1] = vis_dir[i];
	//		start1++;
	//	} else {
	//		temp_dir[start2] = vis_dir[i];
	//		start2++;
	//	}
	//vis_dir = temp_dir;
	//kd_vis_dir.init(&vis_dir, vis_dir.size());

	////area_dir
	//rv_offset = Vector2(rng.rand_real_open(), rng.rand_real_open());
	//codex::math::utils::Hammersley_Zaremba_seq<float> seq_area(NUM_AREA_DIR);
	//for (int i = 0; i < NUM_AREA_DIR; i++)
	//{
	//	float pdf;
	//	Vector3 v, rv;

	//	seq_area.get_sample(rv);
	//	rv.x = add_random_offset(rv.x, rv_offset.x);
	//	rv.y = add_random_offset(rv.y, rv_offset.y);
	//	prob.sample(v, pdf, rv);
	//	v.normalize();

	//	area_dir.push_back(v);
	//}

	//init frames
	fr_Avis.init(DIM_AREA, 256, DEFAULT_AREA_SAMPLES);
	fr_Avis.normalize_n();
	fr_vis.init(DIM_VIS, 256, DEFAULT_AREA_SAMPLES);
	fr_vis.normalize_n();
	rand_proj.init(fr_vis.get_n().size()*(fr_vis.get_n().size()+1)*3/2, 50);
		//250);

	////DEBUG Output points
	//{
	//	FILE *fp;
	//	FOPEN(fp, "d:/temp/Microfacet/vis.txt", "wt");
	//	fprintf_s(fp, "%d\n", fr_vis.get_n().size());
	//	for (int i = 0; i < fr_vis.get_n().size(); i++)
	//		fprintf_s(fp, "%g %g %g\n", 
	//			fr_vis.get_n()[i][0].x, fr_vis.get_n()[i][0].y, fr_vis.get_n()[i][0].z);
	//	fclose(fp);
	//}

	//init samplers
	//std::vector<Vector3> previs_dir;
	//previs_dir.resize(NUM_PREVIS_DIR);
	//for (int i = 0; i < NUM_PREVIS_DIR; i++)
	//	previs_dir[i] = vis_dir[i];
	//vispt_sampler.init(gpu_env.get_handle(), previs_dir.size());
	//vispt_sampler.set_views(previs_dir);

	std::vector<Vector3> vis_dir;
	for (int i = 0; i < fr_vis.get_n().size(); i++)
		vis_dir.push_back(fr_vis.get_n()[i][0]);
	while ((vis_dir.size() % NUM_VIS_PER_BATCH) != 0)
		vis_dir.push_back(Vector3(0, 0, 0));

	vismask_sampler.init(gpu_env.get_handle(), vis_dir.size());
	vismask_sampler.set_views(vis_dir);

	avis_sampler.init(mff_singleton::get()->get_handle(), 512, 2.5, 
		//256, 2.0,
		(r_shader_area*)mff_singleton::get()->get_shader("area"));
}

void MicrofacetEditor::load_scene()
{
	/*
	for (int i = 0; i < MAX_BACKGROUND_OBJ; i++)
	{
		char str[64];
		sprintf_s(str, "background%d", i);
		if (izrt.var_get(str, v))
			p_background.push_back((base_obj*)v.ptr);
	}
	*/

	generate_mesh("T:/Microfacet/data/teapot.obj", Identity());
	p_base->convert_to_instance(pi_base, M_ID_BASE_OBJ, gpu_env.get_handle());
	cout << "position " << pi_base->get_geom()->get_mesh()->vertices.size() << " normal " << pi_base->get_geom()->get_mesh()->normals.size()
		<< " tangent " << pi_base->get_geom()->get_mesh()->tangents.size() << " uv " << pi_base->get_geom()->get_mesh()->uvs.size() << endl;
	//for (int i = 0; i <pi_base->get_geom()->get_mesh()->get_uv_number(); i++)
		//cout << "position " << pi_base->get_geom()->get_mesh()->vertices[i] << " normal " << pi_base->get_geom()->get_mesh()->normals[i] << " tangent " << pi_base->get_geom()->get_mesh()->tangents[i] << " uv " << pi_base->get_geom()->get_mesh()->uvs[i] << endl;
	p_base->convert_to_instance(pi_base_vis, M_ID_OBJ_VIS, gpu_env.get_handle());
	//p_base->mesh.uvs.clear();
	//p_base->mesh.normals.clear();
	//p_base->mesh.tangents.clear();
	//p_base->mesh.set_attribute(VERT_ATTR_POSITION);
	p_base->convert_to_instance(pi_test, M_TEST, gpu_env.get_handle());
	//pi_test->get_geom()->get_mesh()->set_attribute(attr);

	//generate_background_mesh("T:/Microfacet/data/cube.obj", Identity());
	pi_background.clear();
	pi_background_vis.clear();
	
	pi_background.clear();
	pi_background.resize(p_background.size());
	pi_background_vis.clear();
	pi_background_vis.resize(p_background.size());
	for (int i = 0; i < p_background.size(); i++)
	{
		p_background[i]->convert_to_instance(pi_background[i], M_ID_BACKGROUND_OBJ, gpu_env.get_handle());
		p_background[i]->convert_to_instance(pi_background_vis[i], M_ID_OBJ_VIS, gpu_env.get_handle());
	}

	//izrt.var_get("envmap", v);
	//p_skybox = (r_skybox*)v.ptr;
	generate_skybox("T:/Microfacet/data/cube_texture/cube", 2, Vector3(1.0f), 2, Identity());
	p_skybox->init(render_width, render_height, gpu_env.get_handle());
	p_skybox->config_scene(scene_center, scene_radius);

	//Debug
	{
		
		distr_grid* distr = new distr_grid();
		distr_grid_param param_grid;
		param_grid.x_space = 0.25;// 0.25;//10;//*3;
		param_grid.y_space = 0.25;
		param_grid.z_space =  0.25;//10;//*3;
		param_grid.scale = 0.1;// 0.1;//0.004;
		param_grid.height = 0.1;
		distr->set_param(param_grid);

		/*
		distr_rod* distr = new distr_rod();
		distr_rod_param param;
		param.scale		= 0.01;
		param.density	= 10;
		param.randomness= 0.1;
		distr->set_param(param);*/

		/*
		distr_low_discrepancy_3d* distr = new distr_low_discrepancy_3d();
		distr_low_discrepancy_3d_param param;
		param.height	= 0.5;
		param.scale		= 0.3;
		param.density	= 5;
		param.relative_height_density  = 1;
		distr->set_param(param);*/
		

		
		binder_plane* binder = new binder_plane();
		binder_plane_param param_plane;
		param_plane.x_res = param_plane.y_res = 1;
		binder->set_param(param_plane);
		

		/*
		binder_groove_param param_binder = generate_binder_groove(0.1, 0.1);
		binder_groove* binder = new binder_groove();
		binder->set_param(param_binder);*/



		details = new microfacet_details;
		details->init(1, 1, 1.0, 50, 2, binder, distr);
		details->init_blocks(final_details);

		details->idx_all(selection);
		details->update_with_distr(selection);
		details->generate_blocks(selection, final_details);

		printf_s("sampling points...");
		/***********test code************/
		details->sample_points(selection, final_details);
		printf_s("done.\n");

		init_vars();

		tri_mesh mesh;
		//save_details_as_obj("T:/Microfacet/test_mesh/", mesh, final_details);
		printf_s("finish save obj.\n");
	}

	//final_details[details->compute_idx(0, 0)].BRDF_ref = BRDF_factory::produce("BlinnPhong", "10");

	
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_0");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_1");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_2");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_3");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_0");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_1");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_2");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_3");

	/*
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_distr_0");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_distr_1");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_distr_2");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_distr_3");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_binder_0");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_binder_1");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_binder_2");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "ward_03", "matr_binder_3");*/

	set_num_shadows(32);
	set_light_inten(55);
	set_vis_light_inten(65);
	tball_distant.init(Vector3(0.0f, 0.0f, 2.0f), Vector3(0.0f));
	set_vis_mode();


	//izrt.var_get("details", v);
	/*
	details = new microfacet_details();
	generate_microfacet_details();
	details->init_blocks(final_details);

	details->idx_all(selection);
	details->update_with_distr(selection);
	details->generate_blocks(selection, final_details);
	printf_s("sampling points...");
	details->sample_points(selection, final_details);
	printf_s("done.\n");
	*/
}

void MicrofacetEditor::init_vars()
{
	prev_light_inten = vis_light_inten = 1.0f;
	envlight_inten = 1.0f;
	b_light_changed = false;
	b_scene_ready = true;
	microfacet_ops = TASK_SUBTYPE_VIS_NORMAL;
	b_microfacet_changed = true;
	b_buffer_ready = false;
}

void MicrofacetEditor::save_details_as_obj(const char *filename,
	tri_mesh &mesh,
	const std::vector<microfacet_block> &result)
{
	for (int j = 0; j < result[0].insts.size(); j++)
		for (int i = 0; i < result.size(); i++)
		{
			tri_mesh temp;
			tri_mesh_d3dx11 *m = result[i].insts[j]->get_geom()->get_mesh();

			for (int k = 0; k < m->get_vertex_number(); k++)
			{
				r_shader_input *p_shinput = result[i].insts[j]->get_shader_input();
				temp.vertices.push_back(p_shinput->matWorld*m->vertices[k]);
			}
			for (int k = 0; k < m->get_face_number(); k++)
				temp.faces.push_back(m->faces[k]);
			//temp.compute_normal();
			mesh.merge_mesh(temp.vertices, temp.faces);
			//string file = filename + string("_material_") + std::to_string(j) + string("_block_") + std::to_string(i) + string(".obj");
			//temp.save_obj(file);
		}
	string file("T:/Microfacet/test_mesh/mesh.obj");
	mesh.save_obj(file);
	/*
	std::vector<std::string> mtrl_names;
	for (int j = 0; j < result[0].insts.size(); j++)
	{
		char s[256];
		sprintf_s(s, "material_%d", j);
		mtrl_names.push_back(s);
	}
	//mesh.invert_all_faces();
	mesh.save_obj(filename, mtrl_names);
	*/
}

void MicrofacetEditor::set_num_shadows(int num)
{
	while (p_manager->is_working())
		Sleep(1);

	if (p_skybox)
	{
		std::vector<r_light_dir> lights;
		p_skybox->sample_lights(lights, num);
		for (int i = 0; i < lights.size(); i++)	
			//lights[i].c = Vector3(1);
			lights[i].c /= envlight_inten;
		/**********test code********/
		////DEBUG
		//lights.resize(1);
		//lights[0].dir	= Vector3(2, 1, 1);
		//lights[0].dir = lights[0].dir.normalize();
		//lights[0].c		= Vector3(1, 1, 1);

		shadow_maps.set_lights(lights);
		shadow_maps.compute_light_matrix(scene_center);

		vis_pixel_size = (int)ceil(num / float(NUM_BITS_VIS));
		SAFE_DELETE_ARR(p_vis_buffer);
		p_vis_buffer = new float[render_width*render_height*vis_pixel_size];

		b_light_changed = true;
	}
}

void MicrofacetEditor::set_light_inten(int v)
{
	//DEBUG
	float inten = max(v, 1) / 50.0f;
	inten = inten*inten*inten;

	for (int i = 0; i < shadow_maps.lights.size(); i++)
		shadow_maps.lights[i].c *= (inten / prev_light_inten);

	Vector3 s;
	s = p_skybox->get_intensity();
	s *= (inten / prev_light_inten);
	p_skybox->set_intensity(s);
	p_skybox->update_cube_tex();

	prev_light_inten = inten;
	b_light_changed = true;
}

void MicrofacetEditor::set_envlight_inten(int v)
{
	float inten = max(v, 1) / 50.0f;
	inten = inten*inten;

	Vector3 s;
	s = p_skybox->get_intensity();
	s *= (inten / envlight_inten);
	p_skybox->set_intensity(s);
	p_skybox->update_cube_tex();

	envlight_inten = inten;
	b_light_changed = true;
}


void MicrofacetEditor::set_vis_light_inten(int v)
{
	vis_light_inten = max(v, 1) / 50.0f;
	//b_cam_changed_close = true;
}

void MicrofacetEditor::set_vis_mode()
{
	//if (v == VIS_MODE_ILLUSTRATION)
	//{
		int block_x, block_y;
		block_x = block_y = 0;
		Vector3 center(block_x + 0.5, block_y + 0.5, 0.5);
		Vector3 eye//(0.5, -0.318803, 1.02373);
			//(0.5, -0.0766627, 1.01095);
			(0.509798, 0.548086, 1.06437);
		track.init(eye, center);
	//}
	//else {
	//	b_cam_changed_close = true;
//	}
}

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
	/*************test code******************/
	/////matrix_lookat(t->matView, param.eye, param.lookat, Vector3(0, 1, 0));
	/*************test code******************/
	matrix_lookat(t->matView, Vector3(0, 0, 10), Vector3(0.0f, 0.0f, 0.0f), Vector3(0, 1, 0));
	/*
	for (int i = 0; i < 4; i++)
	{ 
		for (int j = 0; j < 4; j++)
		{
			cout << t->matView.m[i][j] << " ";
		}
		cout << endl;
	}

	const Vector3	vat = Normalize(Vector3(0, 0, -5) - Vector3(0.0f, 0.0f, 0.0f));
	const Vector3	vup = Normalize(Vector3(0, 1, 0) - (Dot(Vector3(0, 1, 0), vat) * vat));
	const Vector3	vright = Cross(vup, vat);

	Matrix4 m = Matrix4(
		vright.x, vright.y, vright.z, Dot(-Vector3(0, 0, -5), vright),
		vup.x, vup.y, vup.z, Dot(-Vector3(0, 0, -5), vup),
		vat.x, vat.y, vat.z, Dot(-Vector3(0, 0, -5), vat),
		0, 0, 0, 1);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cout << m.m[i][j] << " ";
		}
		cout << endl;
	}
	*/

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
	v_render_right = v_render_up ^ v_render_lookat;
	v_render_right.normalize();

	t->r.v_lookat = &v_render_lookat;
	t->r.v_up = &v_render_up;
	t->r.v_right = &v_render_right;

	render_image(t);
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

		//trackball_param param;
		//tball_distant.get_params(param);
		Vector3 eye(0.0f, 5.0f, -5.0f);
		Vector3 lookat(0.0f);
		Vector3 v = eye -lookat;
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

			matrix_lookat(t->matView, Vector3(0, 0, -5), Vector3(0, 0, 0), Vector3(0, 1, 0));
			t->matProj =  mat_proj_vis_illu;

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
	
	v_global_eye = Vector3(0,0,-10.0f);
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
	cout << "Update_render:: type " << p->type << endl;
	switch (p->type)
	{
	case TASK_TYPE_RENDER:
	case TASK_TYPE_RENDER_BEFORE_SVD:
	
		//t_frame.update();
		cout << "finish rendering." << endl;

		save_image("T:/Microfacet/img_render.png", p->r.result, p->width, p->height);
		break;
	case TASK_TYPE_VISUALIZATION:
		save_image("T:/Microfacet/img_detail.png", p->trv.result, p->width, p->height);
		break;
	case TASK_TYPE_BUFFER:
		b_buffer_ready = true;
		b_render_image = true;
		break;
	case TASK_TYPE_GROUND_TRUTH:
		cout << "finish rendering." << endl;
		save_image("T:/Microfacet/img_ours.png", p->trt.result, p->width, p->height);
		break;
	case TASK_TYPE_GROUND_TRUTH_DIRECT:
	case TASK_TYPE_RENDER_REF_BRDF:
	case TASK_TYPE_DEBUG_RAYTRACE:
		save_image("T:/Microfacet/img_ours.png", p->trt.result, p->width, p->height);
		break;
	}
}

