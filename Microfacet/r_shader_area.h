#pragma once
#include "r_shader.h"
class r_cb_matrix_arr
{
public:
	DirectX::XMMATRIX mViewProj[NUM_AREA_DIR_PER_BATCH];
};

class r_shader_area : public r_shader
{
protected:
	ID3D11Buffer	*p_cb_attr, *p_cb_viewproj, *p_cb_world;
	r_cb_matrix_arr	cb_viewproj;

public:
	r_shader_area();
	virtual ~r_shader_area();
	virtual void init(D3D_dev_handle* pdev,
		ID3D11VertexShader*	p_vs, ID3D11GeometryShader* p_gs, ID3D11PixelShader* p_ps);
	virtual void setup_render();
	virtual void set_shader_input(r_shader_input *p, void *param);

	void setup_cb_viewproj(const Matrix4 *mat);
	void setup_cb_world(const Matrix4 *mat);
	void setup_cb_attr(const float attr);
};
