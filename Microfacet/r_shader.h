#pragma once
#include "mathutils.h"
#include "GPGPU_env_D3D11.h"
#include "utils.h"
class r_shader_input
{
public:
	Matrix4 matWorld; // transformation for geometry

	virtual ~r_shader_input() {}
};

class r_shader
{
public:
	r_shader();
	virtual ~r_shader();
	virtual void init(D3D_dev_handle* pdev, ID3D11VertexShader*	p_vs, ID3D11GeometryShader* p_gs, ID3D11PixelShader* p_ps);
	virtual void create_shader_input(r_shader_input *&p)
	{
		p = new r_shader_input;
	}

	virtual void set_shader_input(r_shader_input *p, void *param) {};
	virtual void setup_render();

	void set_shaders();
	void set_ps_resource(const int slot, ID3D11ShaderResourceView* &p_srv, const int num = 1);
	void set_ps_sampler(const int slot, ID3D11SamplerState* &p_sampler, const int num = 1);

protected:
	D3D_dev_handle		*pdev;
	ID3D11VertexShader	*p_vs;
	ID3D11GeometryShader*p_gs;
	ID3D11PixelShader	*p_ps;

	void update_cb(ID3D11Buffer *pcb, void *p);
	void create_cb(ID3D11Buffer* &pcb, const int sz);

};

void create_shader_from_file(r_shader* sh, char *filename,
	char *vsname, char *vsmodel,
	char *gsname, char *gsmodel,
	char *psname, char *psmodel,
	D3D_dev_handle* pdev,
	ID3DBlob** p_blob);

void create_shader_from_file_seperate(r_shader* sh,
	char *vertexfilename, char *pixelfilename, char *geomfilename,
	char *vsname, char *vsmodel,
	char *gsname, char *gsmodel,
	char *psname, char *psmodel,
	D3D_dev_handle* pdev,
	ID3DBlob** p_blob);