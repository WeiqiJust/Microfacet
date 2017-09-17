#pragma once
#include "utils.h"
#include "r_geometry.h"
#include "r_shader_vispoint.h"
#include "r_states.h"
#include "r_shadowmap.h"

class vis_point_sampler
{
public:
	vis_point_sampler();
	~vis_point_sampler();
	void release();

	void init(D3D_dev_handle *pdev, const int num_views);
	void set_scene_center(const Vector3 &center);
	void set_views(const std::vector<Vector3> &views);

	int num_views() const;
	void begin_gen_shadow(const int idx, Matrix4 &matLight);
	void end_gen_shadow(const int idx);

	void sample_vis_points(std::vector<Vector3> &output_p, 
		std::vector<normal_weight> &output_n,
		const int num_points,
		const std::vector<Vector3> &input_p, 
		const std::vector<normal_weight> &input_n);

	//void compute_vis_masks(BYTE *result,
	//	const int num_points,
	//	const std::vector<Vector3> &input_p, 
	//	const std::vector<normal_weight> &input_n,
	//	const int shadow_start,
	//	const int shadow_size);

	void compute_vis_masks(
		const std::vector<sample_group> &samples,
		const int shadow_start,
		const int shadow_size);
	void get_vis_masks(BYTE *result, const int num_points);

private:
	r_shadowmap		*p_shadow;
	ID3D11ShaderResourceView*
		*p_sh_srv;
	render_output	*p_screen;
	render_target	*p_pos, *p_normal, *p_output, * p_target_test;

	std::vector<Vector3>	view_samples;
	std::vector<BYTE>		vis;
};