#include "r_shader_vispoint.h"


r_shader_vis_point::r_shader_vis_point(const int n)
	:p_cb_lights(NULL), n_view_samples(n)
{
	cb_lights.resize(n_view_samples);
}

void r_shader_vis_point::init(D3D_dev_handle* pd,
	ID3D11VertexShader* p_v,
	ID3D11GeometryShader* p_g,
	ID3D11PixelShader* p_p)
{
	r_shader::init(pd, p_v, p_g, p_p);

	create_cb(p_cb_lights, sizeof(single_vis_point_light)*n_view_samples);
}

r_shader_vis_point::~r_shader_vis_point()
{
	SAFE_RELEASE(p_cb_lights);
}

void r_shader_vis_point::setup_render()
{
	pdev->get_context()->PSSetConstantBuffers(1, 1, &p_cb_lights);
	set_shaders();
}

void r_shader_vis_point::setup_cb_lights(const int size, const Vector3 *light, const Matrix4 &matProj, const Matrix4 *matView)
{
	for (int i = 0; i < size; i++)
	{
		cb_lights[i].dir.x = light[i].x;
		cb_lights[i].dir.y = light[i].y;
		cb_lights[i].dir.z = light[i].z;
		cb_lights[i].dir.w = 1;

		Matrix4 mat = matProj*matView[i];
		matrix_to_XMMATRIX(cb_lights[i].mLightViewProj, mat);
	}

	for (int i = size; i < n_view_samples; i++)
	{
		cb_lights[i].dir.w = 0;
	}

	update_cb(p_cb_lights, &cb_lights[0]);
}
