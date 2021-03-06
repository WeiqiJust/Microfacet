#include "microfacet.h"
#include "worker.h"
#include "trackball.h"

class MicrofacetEditor
{
public:
	MicrofacetEditor();
	~MicrofacetEditor();
	//void set_render_visualization(const int x, const int y);
	//void render_visualization();

	void load_scene(const string object);

	void load_cube_map(const string folder, const int num, const int mip_level, const Vector3 scale, const float dist, const Matrix4 mat);

	void load_sky_box(int idx, const string sample_file);

	void load_material(Vector3 albedo, const string basic_material, const string binder_id, const string dist_id);

	void load_material(Vector3 albedo_0, Vector3 albedo_1, const string basic_material, const string binder_id_0, const string binder_id_1, const string dist_id);

	void generate_distr_none() {};

	microfacet_distr* generate_distr_grid(const float x, const float y, const float z,
		const float scale, const float height, string& distr_name);

	microfacet_distr* generate_distr_low_discrp(const float density, const float scale, const float height, string& distr_name);

	microfacet_distr* generate_distr_low_discrp_3d(const float density, const float scale,
		const float height, const float relative_height_density, string& distr_name);

	microfacet_distr* generate_distr_rod(const float density, const float scale, const float random, string& distr_name);

	void generate_binder_none() {};

	microfacet_binder* generate_binder_plane(string& binder_name);

	microfacet_binder* generate_binder_groove(const float precent, const float height, string& binder_name);

	microfacet_binder* generate_binder_brick(const int x, const int y,
		const float bottom_x, const float bottom_y,
		const float top_x, const float top_y,
		const float height, string& binder_name);

	microfacet_binder* generate_binder_regpoly(const int edge, const float height,
		const float radius_top, const float radius_bottom, string& binder_name);

	microfacet_binder* generate_binder_dispmap(const int x, const int y, const float amp_, string& binder_name);

	microfacet_binder* generate_binder_random_surface(const int x, const int y, const float amp_, string& binder_name);

	microfacet_binder* generate_binder_woven(const int x, const int y,
		const int verts, const int path,
		const float w_x, const float h_x,
		const float hegith_x, const float w_y,
		const float h_y, const float height_y, string& binder_name);

	microfacet_binder* generate_binder_woven2(const int x, const int y,
		const int verts, const int path,
		const float w_x, const float h_x,
		const float hegith_x, const float w_y,
		const float h_y, const float height_y, string& binder_name);

	microfacet_binder* generate_binder_woven_threads(const int x, const int y,
		const int verts, const int path,
		const float w_x, const float h_x,
		const float w_y, const float h_y,
		const float height, const float r_x,
		const float r_y, string& binder_name);

	void generate_microfacet_details(microfacet_binder* binder, microfacet_distr* distr, int w, int h,
		float d, float dens, int num_area, string binder_name, string distr_name, bool save_mesh);

	void set_view_direction(const Vector3 eye, const Vector3 lookat, const Vector3 up) { tball_distant.init(eye, lookat, up); }

	void compute_microfacet_change();

	void render_buffer();

	void render();

	void create_reflectance_table();

	void debug_our_pixel();

	void compute_ground_truth_BRDF();

	void compute_normal_mapped_BRDF();

	void render_visualization();

	void render_ground_truth();

	void render_ref_BRDF(string roughness, Vector3 albedo = Vector3(1.0f, 1.0f, 1.0f));

	void render_measured_BRDF(string filename);

	void gen_anim_ours();

	void gen_anim_truth();

	void render_raytrace();

	void update_render();

	void update_render(const string filename);

private:

	void generate_mesh(const string filename, const Matrix4 mat);

	void generate_background_mesh(const string filename, const Matrix4 mat);

	void generate_init_matr(const Vector3 albedo, const string lib_matr_id, const string lib_matr);

	void save_details_as_obj(const char *filename, tri_mesh &mesh, string dist_name, string binder_name,
		const std::vector<microfacet_block> &result);

	// If sample_file not exist, sample lighting and save the sampled lights in sample file
	// Otherwise, read lightings from sample file
	void set_num_shadows(int num, const string sample_file); 

	void set_light_inten(int v);

	void set_envlight_inten(int v);

	void set_vis_light_inten(int v);

	void set_vis_mode();

	void render_image(task_microfacet *pt);

	void setup_p_wo(const double &ratio_w, const double &ratio_h, const double &z_near);

	void init_dirs(); //init distribution?

	void init_vars();

	void render_ground_truth_task(const int type, const int num_levels = 4);

	void gen_anim(const int subtype);

private:

	//zrt::ZRTInterface	izrt;
	int					render_width, render_height;
	int					sample_theta, sample_phi;
	bool				b_scene_ready;
	task_manager		*p_manager;
	worker_microfacet	*p_worker;
	GPGPU_env_D3D		gpu_env;
	Matrix4			mat_proj, mat_proj_vis_direct, mat_proj_vis_illu;
	Vector3			v_render_lookat, v_render_up, v_render_right;
	Vector3				v_global_eye, v_global_at;
	render_target		*p_target, *p_target_uv,
		*p_target_normal, *p_target_tangent,
		*p_target_vis, *p_target_test;
	render_output		*p_screen;
	int					visual_x, visual_y;

	//rendering buffers (CPU)
	int					vis_pixel_size;
	UINT				*p_screen_buffer, *p_background_buffer, *p_reflectance_table;
	float				*p_uv, *p_vis_buffer, *p_test;
	short				*p_normal, *p_tangent;
	std::vector<Vector3> p_wo;

	//core editing objects
	r_skybox			*p_skybox;
	vector<r_skybox*>	skyboxes;
	microfacet_factory	*p_factory;
	base_obj			*p_base;
	r_instance			*pi_base, *pi_base_vis, *pi_test;
	std::vector<base_obj*> p_background;
	std::vector<r_instance*> pi_background, pi_background_vis;
	Vector3				scene_center;
	float				scene_radius;

	std::vector<microfacet_block>
		final_details;
	microfacet_details	*details;

	std::vector<int>	selection, selection_compute;
	bool				b_microfacet_changed, b_buffer_ready, b_render_image;
	int					microfacet_ops;

	parab_frame			fr_Avis, fr_vis;
	rp_solver			rand_proj;
	//std::vector<vector3f>
	//					vis_dir, area_dir;
	//kd_tree_3d_vec		kd_vis_dir;
	vis_point_sampler	vispt_sampler, vismask_sampler;
	batch_area_sampler	avis_sampler;
	int					dbg_pixel_x, dbg_pixel_y;
	std::vector<disc>	discs;

	//cube_map* cubemap;


	float	prev_light_inten, envlight_inten, vis_light_inten;
	bool	b_light_changed;
	r_shadowmap
		shadow_maps, shadow_visual;

	trackball track, tball_distant;
};