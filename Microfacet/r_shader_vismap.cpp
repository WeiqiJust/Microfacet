#include "r_shader_vismap.h"

//void r_shader_vismap_input::set(r_shader* psh, void* param)
//{
//	r_shader_vismap *p_sh = (r_shader_vismap*)psh;
//
//	matrix4x4 mat;
//	mat = (*(const matrix4x4*)param) * matWorld;
//	p_sh->setup_cb_world(&mat);
//}

r_shader_vismap::r_shader_vismap()
	:p_cb_viewproj(NULL), p_cb_world(NULL)
{
}

r_shader_vismap::~r_shader_vismap()
{
	SAFE_RELEASE(p_cb_viewproj);
	SAFE_RELEASE(p_cb_world);
}

void r_shader_vismap::init(D3D_dev_handle* pd,
	ID3D11VertexShader* p_v,
	ID3D11GeometryShader* p_g,
	ID3D11PixelShader* p_p)
{
	r_shader::init(pd, p_v, p_g, p_p);

	create_cb(p_cb_viewproj, sizeof(cb_viewproj));
	create_cb(p_cb_world, sizeof(DirectX::XMMATRIX));
}

void r_shader_vismap::setup_render()
{
	pdev->get_context()->VSSetConstantBuffers(1, 1, &p_cb_world);
	pdev->get_context()->GSSetConstantBuffers(0, 1, &p_cb_viewproj);
	set_shaders();
}

void r_shader_vismap::setup_cb_viewproj(const Matrix4 *mat)
{
	for (int i = 0; i < 6; i++)
		matrix_to_XMMATRIX(cb_viewproj.mViewProj[i], mat[i]);

	update_cb(p_cb_viewproj, &cb_viewproj);
}

void r_shader_vismap::setup_cb_world(const Matrix4 *mat)
{
	DirectX::XMMATRIX m;
	matrix_to_XMMATRIX(m, *mat);
	update_cb(p_cb_world, &m);
}

void r_shader_vismap::set_shader_input(r_shader_input *p, void *param)
{
	Matrix4 mat;
	mat = (*(const Matrix4*)param) * p->matWorld;
	setup_cb_world(&mat);
}

