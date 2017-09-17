#pragma once
#include "r_shader.h"
#include <DirectXMath.h>

class r_cb_basic
{
public:
	DirectX::XMMATRIX mWorldViewProj;
	DirectX::XMMATRIX mWorld;
};

class r_cb_lights
{
public:
	DirectX::XMFLOAT4	dir;
	DirectX::XMFLOAT4	inten;
	DirectX::XMMATRIX	mLightViewProj;
};

class r_light_dir
{
public:
	Vector3 dir, c;
};

class r_shader_basic : public r_shader
{
protected:
	ID3D11Buffer	*p_cb_basic, *p_cb_lights;
	r_cb_basic		cb_basic;
	r_cb_lights		cb_lights;

public:
	r_shader_basic();
	virtual ~r_shader_basic();
	virtual void init(D3D_dev_handle* pdev,
		ID3D11VertexShader*	p_vs, ID3D11GeometryShader* p_gs, ID3D11PixelShader* p_ps);
	virtual void setup_render();
	virtual void set_shader_input(r_shader_input *p, void *param);

	void setup_cb_basic(const Matrix4 *matWorld, const Matrix4 *matProjView);
	void setup_cb_lights(const r_light_dir *light, const Matrix4 *matProjView);
};

class r_shader_vis : public r_shader_basic
{
public:
	void setup_cb_lights(const r_light_dir *light, const float vis_mask, const Matrix4 *matProjView);
};