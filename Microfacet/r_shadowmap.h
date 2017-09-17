#pragma once
#include "r_states.h"
#include "r_shader_basic.h"
class r_shadowmap
{
private:
	r_samplerstate		*p_samp_shadow;
	float				scene_radius;

	//DEBUG
	std::vector<render_target*> p_depth_map;
	

public:
	std::vector<r_light_dir>	lights;
	std::vector<Matrix4>		mat_light_view;
	Matrix4						mat_light_proj;

	render_output*				p_output;

	r_shadowmap();
	~r_shadowmap();

	void init(D3D_dev_handle *pdev,
		r_samplerstate *p_samp,
		const int num_shadow_buffers,
		const int dim,
		const DXGI_FORMAT format,
		const float &s_radius);

	void set_lights(const std::vector<r_light_dir> &lts);
	void compute_light_matrix(const Vector3 &s_center);

	void set_gen_shadow(const int idx);
	void set_use_shadow(const int idx, r_shader *sh);
	void unset(const int idx, r_shader *sh);

	ID3D11ShaderResourceView* get_srv(const int idx);
};