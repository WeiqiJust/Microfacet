#include "r_shader_basic.h"

//shader_basic_input
//void r_shader_basic_input::get_cb_basic(r_cb_basic &cb, const matrix4x4 &matProjView) const
//{
//	matrix_to_XMMATRIX(cb.mWorld, matWorld);
//	matrix4x4 mat = matProjView*matWorld;
//	matrix_to_XMMATRIX(cb.mWorldViewProj, mat);
//}

//void r_shader_basic_input::set(r_shader* psh, void* param)
//{
//	r_shader_basic *p_sh = (r_shader_basic*)psh;
//
//	p_sh->setup_cb_basic(&matWorld, (const matrix4x4*)param);
//}

//shader_basic
r_shader_basic::r_shader_basic()
:p_cb_basic(NULL), p_cb_lights(NULL)
{
}

void r_shader_basic::init(D3D_dev_handle* pd, 
						  ID3D11VertexShader* p_v, 
						  ID3D11GeometryShader* p_g,
						  ID3D11PixelShader* p_p)
{
	r_shader::init(pd, p_v, p_g, p_p);

	create_cb(p_cb_basic, sizeof(r_cb_basic));
	create_cb(p_cb_lights, sizeof(r_cb_lights));
}

r_shader_basic::~r_shader_basic()
{
	SAFE_RELEASE(p_cb_basic);
	SAFE_RELEASE(p_cb_lights);
}

void r_shader_basic::setup_render()
{
	set_shaders();
	pdev->get_context()->VSSetConstantBuffers(0, 1, &p_cb_basic);
	pdev->get_context()->VSSetConstantBuffers(1, 1, &p_cb_lights);
	pdev->get_context()->PSSetConstantBuffers(1, 1, &p_cb_lights);
	
}

void r_shader_basic::setup_cb_basic(const Matrix4 *matWorld, const Matrix4 *matProjView)
{
	//const r_shader_basic_input* p_shinput = (const r_shader_basic_input*)pshinput;
	//p_shinput->get_cb_basic(cb_basic, matProjView);

	matrix_to_XMMATRIX(cb_basic.mWorld, *matWorld);
	Matrix4 mat = (*matProjView)*(*matWorld); //Notice: should be column major, if is calculated from XMMath, need to be transposed! 
	matrix_to_XMMATRIX(cb_basic.mWorldViewProj, mat);

	//matrix_to_XMMATRIX(cb_basic.mWorld, Identity());
	//matrix_to_XMMATRIX(cb_basic.mWorldViewProj, Identity());

	update_cb(p_cb_basic, &cb_basic);
	
}

void r_shader_basic::setup_cb_lights(const r_light_dir *light, const Matrix4 *matProjView)
{
	cb_lights.dir.x = light->dir.x;
	cb_lights.dir.y = light->dir.y;
	cb_lights.dir.z = light->dir.z;
	cb_lights.inten.x = light->c.x;
	cb_lights.inten.y = light->c.y;
	cb_lights.inten.z = light->c.z;
	matrix_to_XMMATRIX(cb_lights.mLightViewProj, *matProjView);
	update_cb(p_cb_lights, &cb_lights);
}

void r_shader_basic::set_shader_input(r_shader_input *p, void *param)
{
	setup_cb_basic(&p->matWorld, (const Matrix4*)param);
}

//r_shader_vis
void r_shader_vis::setup_cb_lights(const r_light_dir *light, const float vis_mask, const Matrix4 *matProjView)
{
	cb_lights.dir.x = light->dir.x;
	cb_lights.dir.y = light->dir.y;
	cb_lights.dir.z = light->dir.z;
	cb_lights.inten.x = light->c.x;
	cb_lights.inten.y = light->c.y;
	cb_lights.inten.z = light->c.z;
	cb_lights.inten.w = vis_mask;
	matrix_to_XMMATRIX(cb_lights.mLightViewProj, *matProjView);
	update_cb(p_cb_lights, &cb_lights);
}