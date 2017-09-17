#pragma once
#include "utils.h"
#include "r_instance.h"
#include "vis_normal_distr.h"
#include "preconv_matr.h"
#include "BRDF.h"

class inst_group
{
private:
	std::vector<int> mat_ids;
public:
	std::vector<std::vector<r_instance*>> insts;

	void clear();
	int idx(const int mat_id);
	int get_id(const int i) const;
};


class vis_area
{
private:
	std::vector<float> area;

public:
	void set_area(const std::vector<float> &v);
	float get_area(const int num,
		const int *idx,
		const float *weight) const;
};

class microfacet_block
{
public:
	std::vector<r_instance*>		insts;
	std::vector<microfacet_block*>	neighbors_and_self;
	inst_group						per_mat_groups;

	//for rendering
	vis_area Avis;
#ifndef USE_SVD_SCALAR
	//std::vector<float> sample_Avis;
	std::vector<float> inv_SVD_scalar;
#endif
	std::vector<vis_normal_distr> vis_normal;

	void draw_with_shader(r_shader *sh, void* mat);
	void release();

	void get_reflectance(Vector3 &result, const Vector3 &wi, const Vector3 &wo,
		const parab_frame &fr_vis);
	void get_reflectance_BRDF(Vector3 &result, const Vector3 &wi, const Vector3 &wo);

	//DEBUG
	void debug_BRDF_no_lerp(Vector3 &result, Vector3 &result_wi, Vector3 &result_wo,
		const Vector3 &wi, const Vector3 &wo);

	BRDF_interface		*BRDF_ref;
	std::vector<float>	BRDF_truth;
	void save_BRDF_truth(const char *filename);
	void save_BRDF_for_fit(const char *filename, const parab_frame &fr_Avis);
	void load_BRDF_truth(const char *filename);

	void get_n_distr_points(la_vector<float> &result,
		const int ch,
		const double_parab_frame &fr_n,
		const parab_frame &fr_vis,
		const int num_vis_wi,
		const int *idx_vis_wi,
		const float *weight_vis_wi,
		const int num_vis_wo,
		const int *idx_vis_wo,
		const float *weight_vis_wo,
		vis_normal_distr &vis_n);

	void get_n_distr_ours(la_vector<float> &result,
		const int ch,
		const parab_frame &fr_vis,
		const int num_vis_wi,
		const int *idx_vis_wi,
		const float *weight_vis_wi,
		const int num_vis_wo,
		const int *idx_vis_wo,
		const float *weight_vis_wo,
		vis_normal_distr &vis_n);

	float n_distr_to_reflectance_ours(const Vector3 &wi, const Vector3 &wo,
		const la_vector<float> &n_distr,
		preconv_matr *p_basis_matr);

	void get_reflectance_debug(Vector3 &result,
		const Vector3 &wi, const Vector3 &wo,
		const parab_frame &fr_vis, const bool b_debug);
	void get_reflectance_assembler(Vector3 &result,
		const Vector3 &wi, const Vector3 &wo,
		const parab_frame &fr_vis, const bool b_debug);
};

/*
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
*/