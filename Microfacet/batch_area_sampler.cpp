#include "batch_area_sampler.h"

batch_area_sampler::batch_area_sampler()
:p_screen(NULL), p_output(NULL), p_sh(NULL)//, p_buffer(NULL)
{
}

batch_area_sampler::~batch_area_sampler()
{
	release();
}

void batch_area_sampler::release()
{
	SAFE_DELETE(p_screen);
	SAFE_DELETE(p_output);
	SAFE_DELETE_ARR(p_buffer);
	//SAFE_DELETE(p_buffer);
}

void batch_area_sampler::init(D3D_dev_handle *pdev, 
							  const int d,
							  const float r,
							  r_shader_area *psh)
{
	dim		= d;
	reduced_dim
			= dim / 64;
	p_sh	= psh;
	radius	= r;

	p_buffer = new BYTE[reduced_dim*reduced_dim*NUM_AREA_DIR_PER_BATCH];

	p_screen = new render_output(pdev);
	p_screen->init(dim, dim, NUM_AREA_DIR_PER_BATCH);
	p_output = new render_target(pdev);
	p_output->init(dim, dim, NUM_AREA_DIR_PER_BATCH, 0, DXGI_FORMAT_R8_UNORM, R_TARGET_CPU_READ|R_TARGET_CREATE_SRV);

	p_screen->clear_mrt();
	p_screen->add_rt(p_output);

	float	z_near= 1e-5*radius, 
			z_far = radius*5;
	/*****************test code***********/
	projection_orthogonal(matProj, radius, radius, z_near, z_far);//Weiqi: change orthogonal projection
	///projection_orthogonal(matProj, radius, radius, z_near, z_far);
}

void batch_area_sampler::begin_sample(const Vector3 &lookat, const Vector3 *dir)
{
	Matrix4 matView;

	for (int i = 0; i < NUM_AREA_DIR_PER_BATCH; i++)
	{
		Vector3 v_lookat(lookat.x, lookat.y, lookat.z);
		Vector3 v_dir(dir[i].x, dir[i].y, dir[i].z);
		Vector3 v_up;
		Vector3 v_eye = v_lookat + v_dir*radius;
		if (abs(v_dir.z) == 1)
			v_up = Vector3(1, 0, 0);
		else
			v_up = Vector3(0, 1, 0);

		matrix_lookat(matView, v_eye, v_lookat, v_up);
		matViewProj[i] = matProj*matView;
	}

	float clear_color[4] = {0, 0, 0, 0};
	p_screen->get_rt(0)->clear(clear_color);
	p_screen->clear_depth();
	p_screen->set();
	p_sh->setup_cb_viewproj(matViewProj);
	p_sh->setup_render();
}

void batch_area_sampler::end_sample()
{
	p_screen->unset();
}

r_shader_area* batch_area_sampler::get_shader()
{
	return p_sh;
}

void batch_area_sampler::get_area(float *result)
{
	p_output->generate_mips();
	p_output->get_result(p_buffer, 1, 6);

	for (int i = 0; i < NUM_AREA_DIR_PER_BATCH; i++)
	{
		int v = 0,
			offset = reduced_dim*reduced_dim*i;
		for (int j = 0; j < reduced_dim*reduced_dim; j++)
			v += p_buffer[offset+j];
		result[i] = radius*radius * (float)v/255.0f / (reduced_dim*reduced_dim);
	}

	////DEBUG
	//p_output->get_result(p_buffer, 1, 0);

	//for (int i = 0; i < NUM_AREA_PER_BATCH; i++)
	//{
	//	int v = 0,
	//		offset = reduced_dim*reduced_dim*i;
	//	for (int j = 0; j < reduced_dim*reduced_dim; j++)
	//		v += p_buffer[offset+j];
	//	result[i] = radius*radius * (float)v/255.0f / (reduced_dim*reduced_dim);
	//}
}