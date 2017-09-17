#pragma once
#include "r_shader.h"
#include "utils.h"
class r_cb_cube_matrix
{
public:
	DirectX::XMMATRIX mViewProj[6];
};

class r_shader_vismap : public r_shader
{
protected:
	ID3D11Buffer			*p_cb_viewproj, *p_cb_world;
	r_cb_cube_matrix		cb_viewproj;

public:
	r_shader_vismap();
	virtual ~r_shader_vismap();
	virtual void init(D3D_dev_handle* pdev,
		ID3D11VertexShader*	p_vs, ID3D11GeometryShader* p_gs, ID3D11PixelShader* p_ps);
	virtual void setup_render();
	virtual void set_shader_input(r_shader_input *p, void *param);

	void setup_cb_viewproj(const Matrix4 *mat);
	void setup_cb_world(const Matrix4 *mat);
};