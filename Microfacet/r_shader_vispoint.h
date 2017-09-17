#pragma once
#include "r_shader_basic.h"
//class r_cb_vis_point_lights
//{
//public:
//	XMFLOAT4	dir[NUM_VIEW_SAMPLES];
//	XMMATRIX	mLightViewProj[NUM_VIEW_SAMPLES];
//};

class single_vis_point_light
{
public:
	DirectX::XMFLOAT4	dir;
	DirectX::XMMATRIX	mLightViewProj;
};

class r_shader_vis_point : public r_shader
{
protected:
	ID3D11Buffer			*p_cb_lights;
	int						n_view_samples;
	std::vector<single_vis_point_light>
		cb_lights;
	//r_cb_vis_point_lights	cb_lights;

public:
	r_shader_vis_point(const int n);
	virtual ~r_shader_vis_point();
	virtual void init(D3D_dev_handle* pdev,
		ID3D11VertexShader*	p_vs, ID3D11GeometryShader* p_gs, ID3D11PixelShader* p_ps);
	virtual void setup_render();

	void setup_cb_lights(const int size, const Vector3 *light, const Matrix4 &matProj, const Matrix4 *matProjView);
};