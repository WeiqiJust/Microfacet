#pragma once
#include "utils.h"
#include "GPGPU_env_D3D11.h"
#include "r_shader_area.h"

class batch_area_sampler
{
private:
	render_output	*p_screen;
	render_target	*p_output;
	r_shader_area	*p_sh;

	BYTE			*p_buffer;
	int				dim, reduced_dim;
	float			radius;

	Matrix4		matProj, matViewProj[NUM_AREA_DIR_PER_BATCH];

public:
	batch_area_sampler();
	~batch_area_sampler();
	void release();

	void init(D3D_dev_handle *pdev, 
		const int dim,
		const float r,
		r_shader_area *psh);

	void begin_sample(const Vector3 &v_lookat, const Vector3 *v_dir);
	void end_sample();

	r_shader_area* get_shader();
	void get_area(float *result);
};

