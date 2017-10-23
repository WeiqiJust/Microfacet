#pragma once

#include "microfacet.h"
#include "task.h"

//const int TASK_TYPE_INITIAL				= 1;
const int TASK_TYPE_MICROFACET_CHANGED	= 2;
const int TASK_TYPE_BUFFER				= 3;
const int TASK_TYPE_RENDER				= 4;
const int TASK_TYPE_GROUND_TRUTH		= 5;
const int TASK_TYPE_VISUALIZATION		= 6;
const int TASK_TYPE_GROUND_TRUTH_DEBUG_PIXEL
										= 7;
const int TASK_TYPE_GROUND_TRUTH_DIRECT	= 8;
const int TASK_TYPE_RENDER_DEBUG		= 9;
const int TASK_TYPE_RENDER_BEFORE_SVD	= 10;
const int TASK_TYPE_GEN_ANIMATION		= 11;
const int TASK_TYPE_RENDER_REF_BRDF		= 12;
const int TASK_TYPE_RENDER_REF_BRDF_DEBUG_PIXEL
										= 13;

const int TASK_TYPE_DEBUG_RAYTRACE_PIXEL= 14;
const int TASK_TYPE_DEBUG_RAYTRACE		= 15;

const int TASK_TYPE_CREATE_REFLECTANCE_TABLE = 16;

const int TASK_SUBTYPE_GEN_DISTR		= 0x0001;
const int TASK_SUBTYPE_VIS_NORMAL		= 0x0002;
const int TASK_SUBTYPE_BRDF_TRUTH		= 0x0004;
const int TASK_SUBTYPE_BRDF_NORMAL		= 0x0008;
const int TASK_SUBTYPE_BRDF_PSNR		= 0x0010;

const int TASK_SUBTYPE_ANIM_OURS		= 0x0001;
const int TASK_SUBTYPE_ANIM_TRUTH		= 0x0002;

//class task_test
//{
//public:
//	r_instance		*p_main;
//	std::vector<r_instance*>
//					*p_background;
//	r_shadowmap		*p_shadow;
//	r_skybox		*p_skybox;
//	render_output	*p_screen;
//	UINT			*result;
//};

class task_buffer
{
public:
	r_skybox		*p_skybox;
	r_instance		*p_main, *p_main_vis, *p_main_test;
	std::vector<r_instance*>
					*p_background, *p_background_vis;
	r_shadowmap		*p_shadow;
		
	render_output	*p_screen;
	render_target	*p_target,
		*p_target_uv,
		*p_target_normal,
		*p_target_tangent,
		*p_target_vis,
		*p_target_test;

	float			*p_uv, *p_vis_buffer, *p_test;
	short			*p_normal, *p_tangent;
	int				vis_pixel_size;
	UINT			*result; 
	//float			*result; // original code
	float			envlight_inten;
};

class microfacet_details;
class kd_tree_3d_vec;
class parab_frame;
class vis_point_sampler;
class batch_area_sampler;

class task_microfacet_changed
{
public:
	int						sub_type;
	microfacet_details		*details;
	std::vector<int>		*idx;
	std::vector<microfacet_block> *blocks;
	parab_frame	 			*fr_Avis, *fr_vis;
	batch_area_sampler		*avis_sampler;
	vis_point_sampler		*vismask_sampler;
	rp_solver				*rp;
};

class task_render
{
public:
	r_shadowmap		*p_shadow;
	float			*p_uv, *p_vis_buffer;
	short			*p_normal, *p_tangent;
	int				vis_pixel_size;
	parab_frame		*fr_Avis, *fr_vis;

	microfacet_details
					*details;
	std::vector<microfacet_block> 
					*blocks;

	UINT			*result;
	//float			*result; // original code

	Vector3		*v_lookat, *v_up, *v_right;
	std::vector<Vector3>
					*p_wo;

	//DEBUG only
	int				dbg_pixel_x, dbg_pixel_y;
};

class task_create_reflectance
{
public:

	parab_frame		*fr_Avis, *fr_vis;
	int			sample_theta, sample_phi;
	microfacet_details
		*details;
	std::vector<microfacet_block>
		*blocks;

	UINT* result;
};



class task_render_truth
{
public:
	r_shadowmap		*p_shadow;
	float			*p_uv, *p_vis_buffer;
	short			*p_normal, *p_tangent;
	int				vis_pixel_size;

	microfacet_details
					*details;
	std::vector<microfacet_block> 
					*blocks;

	UINT			*result;
	//float			*result; // final render result // original code

	Vector3		*v_lookat, *v_up;
	std::vector<Vector3>
					*p_wo;

	//for interreflection computation
	int				num_raytrace_level, num_rays, dim_raytrace;
	//DEBUG only
	int				dbg_pixel_x, dbg_pixel_y;
};

class task_animation
{
public:
	int				sub_type;

	int				angle_start, angle_finish, angle_num;

	r_skybox		*p_skybox;
	r_instance		*p_main, *p_main_vis;
	std::vector<r_instance*>
					*p_background, *p_background_vis;

	render_output	*p_screen;
	render_target	*p_target,
					*p_target_uv, 
					*p_target_normal,
					*p_target_tangent,
					*p_target_vis;

	r_shadowmap		*p_shadow;
	float			*p_uv, *p_vis_buffer;
	short			*p_normal, *p_tangent;
	int				vis_pixel_size;

	microfacet_details
					*details;
	std::vector<microfacet_block> 
					*blocks;
	Vector3			*lookat, *eye;
	std::vector<Vector3>
					*p_wo;

	parab_frame		*fr_Avis, *fr_vis;
	UINT			*result;
	//float			*result; // original code
};

class disc
{
public:
	Vector3 p, n, color;
};

class task_render_visual
{
public:
	microfacet_block
					*p_block;
	r_shadowmap		*p_shadow;
	render_output	*p_screen;
	render_target	*p_target;
	UINT			*result;
	//float			*result; // original code
	bool			b_vis_target, b_show_neighbors;
	float			vis_light_inten;

	std::vector<disc>
					*discs;
	float			disc_r;
};

class qrender_block : public subtask
{
public:
	BYTE option;
	int	x0, y0, x1, y1;
};

class task_microfacet : public task
{
public:
	int			type;
	int			id, width, height;
	Matrix4	matView, matProj;

	union {
		//task_test			tr;
		//task_initial_render	tir;
		task_buffer			tb;
		task_microfacet_changed
							tmc;
		task_render			r;
		task_render_truth	trt;
		task_render_visual	trv;
		task_animation		ta;
		task_create_reflectance tc;
	};
};

class worker_microfacet : public task_worker
{
private:
	bool		b_alloc;

	//For rendering task
	std::vector<bool>	alloc_table;
	int					num_pixels_computed,
						block_size_x, block_size_y,
						num_block_x, num_block_y;

	//DEBUG
	std::vector<Vector3> val; // final rendering result
		
public:
	worker_microfacet();

	virtual void work(shared_ptr<subtask> &t, void* p_per_thread_global_var);
		
	void work_task_test(task_microfacet *t);
	void work_task_initial_render(task_microfacet *t);//not used
	void work_task_microfacet_changed(task_microfacet *t);// Important see how microfacet changes
	void work_task_buffer(task_microfacet *t);
	void work_task_render(task_microfacet *t);
	void work_task_create_reflectance_table(task_microfacet *t);
	void work_task_render_block(task_microfacet *t, qrender_block *pb);
	void work_task_raytrace_block(task_microfacet *t, qrender_block *pb);
	void work_task_render_debug(task_microfacet *t);
	void work_task_render_before_SVD(task_microfacet *t);
	void work_task_ground_truth(task_microfacet *t);
	void work_task_ground_truth_debug_pixel(task_microfacet *t);
	void work_task_ground_truth_direct(task_microfacet *t_org);

	void work_task_debug_raytrace_pixel(task_microfacet *t);
	void work_task_debug_raytrace(task_microfacet *t);
	void work_task_raytrace_direct(task_microfacet *t_org);

	void work_task_render_ref_BRDF(task_microfacet *t);
	void work_task_render_ref_BRDF_debug_pixel(task_microfacet *t);
	void work_task_visual(task_microfacet *t);
	void work_task_animation(task_microfacet *t);
	void compute_BRDF_truth(task_microfacet_changed &tmc);
	void compute_BRDF_normal(task_microfacet_changed &tmc);
	void compute_PSNR(task_microfacet_changed &tmc);

	virtual void init_task(shared_ptr<task> &t);
	virtual void finish_task(shared_ptr<task> &t);
	virtual int get_subtask(shared_ptr<subtask> &st, shared_ptr<task> &t);
	virtual bool complete_subtask(shared_ptr<subtask> &t);
};

void compute_vis_area_and_normal(microfacet_details &details,
	const std::vector<int> &idx_blocks,
	std::vector<microfacet_block> &blocks,
	const parab_frame &fr_Avis,
	const parab_frame &fr_vis,
	batch_area_sampler &avis_sampler,
	vis_point_sampler &vismask_sampler,
	rp_solver &rp);

void compute_normal_mapped_BRDF(microfacet_details &details,
	const std::vector<int> &idx_blocks,
	std::vector<microfacet_block> &blocks);