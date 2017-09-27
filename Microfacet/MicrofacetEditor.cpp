#include "MicrofacetEditor.h"
#include "distr_grid.h"
#include "binder_plane.h"
#include "binder_groove.h"
#include "distr_rod.h"
#include "distr_low_discrepancy.h"
#include "binder_woven.h"
#include "binder_woven_threads.h"
#include "binder_brick.h"

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
	rv_offset = Vector2(rng.get_random_float_open(), rng.get_random_float_open());
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
	/*
	std::vector<Vector3> previs_dir;
	previs_dir.resize(NUM_PREVIS_DIR);
	for (int i = 0; i < NUM_PREVIS_DIR; i++)
		previs_dir[i] = vis_dir[i];
	vispt_sampler.init(gpu_env.get_handle(), previs_dir.size());
	vispt_sampler.set_views(previs_dir);*/

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

void MicrofacetEditor::load_cube_map(const string folder, const int num, const int mip_level, const Vector3 scale, const float dist, const Matrix4 mat)
{
	if (num == 0)
	{
		string texture_file = folder;
		shared_ptr<cube_texture> texture = make_shared<cube_texture>();
		texture->load_texture(texture_file, mip_level);
		r_skybox* skybox = new r_skybox(scale, dist, texture, mat);
		skybox->init(render_width, render_height, gpu_env.get_handle());
		skybox->config_scene(scene_center, scene_radius);
		skyboxes.push_back(skybox);
		return;
	}

	for (int i = 0; i < num; i++)
	{
		string texture_file = folder + to_string(i) + "/cube";
		shared_ptr<cube_texture> texture = make_shared<cube_texture>();
		texture->load_texture(texture_file, mip_level);
		r_skybox* skybox = new r_skybox(scale, dist, texture, mat);
		skybox->init(render_width, render_height, gpu_env.get_handle());
		skybox->config_scene(scene_center, scene_radius);
		skyboxes.push_back(skybox);
	}
}

void MicrofacetEditor::load_sky_box(int idx)
{
	p_skybox = skyboxes[idx];
	init_vars();
	set_num_shadows(32);
	set_light_inten(80);
	set_vis_light_inten(50);
	set_envlight_inten(50);
}

void MicrofacetEditor::load_material(Vector3 albedo, const string basic_material, const string binder_id, const string dist_id)
{
	//generate_init_matr(albedo, "Lambert", binder_id);
	generate_init_matr(albedo, basic_material, binder_id);
	generate_init_matr(albedo, basic_material, dist_id);
	/*
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_0");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_1");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_2");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_distr_3");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_0");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_1");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_2");
	generate_init_matr(Vector3(0.6f, 1.0f, 0.6f), "Lambert", "matr_binder_3");*/
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
	
	p_base->convert_to_instance(pi_base_vis, M_ID_OBJ_VIS, gpu_env.get_handle());
	
	p_base->convert_to_instance(pi_test, M_TEST, gpu_env.get_handle());
	
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

	//Debug
	{
		/*
		distr_grid* distr = new distr_grid();
		distr_grid_param param_grid;
		param_grid.x_space = 0.25;// 0.25;//10;//*3;
		param_grid.y_space = 0.25;
		param_grid.z_space =  0.25;//10;//*3;
		param_grid.scale = 0;// 0.1;//0.004;
		param_grid.height = 0.01;
		distr->set_param(param_grid);*/

		/*
		distr_rod* distr = new distr_rod();
		distr_rod_param param;
		param.scale		= 0.1;
		param.density	= 10;
		param.randomness= 0.1;
		param.name = "rod";
		distr->set_param(param);*/

		/*
		distr_low_discrepancy_3d* distr = new distr_low_discrepancy_3d();
		distr_low_discrepancy_3d_param param;
		param.height	= 0.05;
		param.scale = 0.1;
		param.density	= 50;
		param.relative_height_density  = 0.001;
		param.name = "ld3d";
		distr->set_param(param);*/
		
		/*
		binder_plane* binder = new binder_plane();
		binder_plane_param param_plane;
		param_plane.x_res = param_plane.y_res = 1;
		binder->set_param(param_plane);*/
		
		/*
		binder_groove_param param_binder = generate_binder_groove(0.1, 0.1);
		param_binder.name = "groove";
		binder_groove* binder = new binder_groove();
		binder->set_param(param_binder);*/

		/*
		binder_woven_param binder_param = generate_binder_woven(8, 8, 8, 32, 0.5, 0.5, 0.5, 0.5, 0.5, 0.1);
		binder_woven* binder = new binder_woven();
		binder->set_param(binder_param);*/

		/*
		binder_woven_threads_param binder_param = generate_binder_woven_threads(4, 4, 4, 64, 10, 10, 0.1, 0.1, 0.2, 0.1,1);
		binder_woven_threads* binder = new binder_woven_threads();
		binder->set_param(binder_param);*/

		/*
		binder_brick_param binder_param = generate_binder_brick(2, 2, 0.1, 0.1, 0.2, 0.2, 0.1);
		binder_brick* binder = new binder_brick();
		binder->set_param(binder_param);*/

	}
	
	tball_distant.init(Vector3(0.0f, 0.0f, -8.0f), Vector3(0));
	set_vis_mode();
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
	tri_mesh &mesh, string dist_name, string binder_name,
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
			mesh.merge_mesh(temp.vertices, temp.faces);
		}
	string file(filename);
	file += dist_name + "_" + binder_name + ".obj";
	//string file("T:/Microfacet/test_mesh/mesh.obj");
	mesh.save_obj(file);
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
