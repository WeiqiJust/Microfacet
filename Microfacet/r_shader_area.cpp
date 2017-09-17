#include "r_shader_area.h"

r_shader_area::r_shader_area()
	:p_cb_viewproj(NULL), p_cb_world(NULL), p_cb_attr(NULL)
{
}

r_shader_area::~r_shader_area()
{
	SAFE_RELEASE(p_cb_viewproj);
	SAFE_RELEASE(p_cb_world);
	SAFE_RELEASE(p_cb_attr);
}

void r_shader_area::init(D3D_dev_handle* pd,
	ID3D11VertexShader* p_v,
	ID3D11GeometryShader* p_g,
	ID3D11PixelShader* p_p)
{
	r_shader::init(pd, p_v, p_g, p_p);

	create_cb(p_cb_viewproj, sizeof(cb_viewproj));
	create_cb(p_cb_world, sizeof(DirectX::XMMATRIX));
	create_cb(p_cb_attr, sizeof(DirectX::XMFLOAT4));
}

void r_shader_area::setup_render()
{
	pdev->get_context()->VSSetConstantBuffers(1, 1, &p_cb_world);
	pdev->get_context()->GSSetConstantBuffers(0, 1, &p_cb_viewproj);
	pdev->get_context()->PSSetConstantBuffers(2, 1, &p_cb_attr);
	set_shaders();
}

void r_shader_area::setup_cb_viewproj(const Matrix4 *mat)
{
	for (int i = 0; i < NUM_AREA_DIR_PER_BATCH; i++)
		matrix_to_XMMATRIX(cb_viewproj.mViewProj[i], mat[i]);

	update_cb(p_cb_viewproj, &cb_viewproj);
}

void r_shader_area::setup_cb_world(const Matrix4 *mat)
{
	DirectX::XMMATRIX m;
	matrix_to_XMMATRIX(m, *mat);
	update_cb(p_cb_world, &m);
}

void r_shader_area::setup_cb_attr(const float attr)
{
	DirectX::XMFLOAT4 v;
	v.x = v.y = v.z = v.w = attr;
	update_cb(p_cb_attr, &v);
}

void r_shader_area::set_shader_input(r_shader_input *p, void *param)
{
	setup_cb_world(&p->matWorld);
}
