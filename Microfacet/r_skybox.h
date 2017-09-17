#pragma once
#include "utils.h"
#include "GPGPU_env_D3D11.h"
#include "r_shader_basic.h"
#include "cube_texture.h"


class face_prob_distribution
{
public:
	step_1D_prob prob_y;
	std::vector<step_1D_prob>  prob_x;
};

class r_skybox
{
protected:
	struct CB_VS_SKYBOX
	{
		DirectX::XMMATRIX mWorldViewProj;
	};    

	ID3D11Texture2D*			pEnvironmentMap11;
	ID3D11ShaderResourceView*	pEnvironmentRV11;

	ID3D11SamplerState*			pSam;
	ID3D11InputLayout*			pVertexLayout11;
	ID3D11Buffer*				pcbVSPerObject;
	ID3D11Buffer*				pVB11;
	ID3D11DepthStencilState*	pDepthStencilState11;

	int		screen_w, screen_h;
	D3D_dev_handle* pdev;
	r_shader* sh;

	shared_ptr<cube_texture> lightprobe;
	Vector3 scale;
	float dist;
	Matrix4 mat;

public:
	r_skybox(const Vector3 sscale, const float ddist, 
		const shared_ptr<cube_texture> & ttexture, const Matrix4 &mmat);
	~r_skybox();

	void config_scene(const Vector3 &center, const float radius);

	void update_cube_tex();
	void sample_lights(std::vector<r_light_dir> &samples, const int num) const;

	void init(const int scr_w, const int scr_h, D3D_dev_handle* pdev);
	void draw(const Matrix4 &mat);

	void get_energy(Vector3 &c, const geometry_differential &ir) const;
	void sample(geometry_differential &d_geom, const Vector3 &rv) const;

	void set_intensity(const Vector3 intensity);

	Vector3 get_intensity();
private:
	Vector3		frame_x[6], frame_y[6], frame_o[6];
	Vector3			scene_center;
	float			scene_radius;

	Vector3 power, inten;

	face_prob_distribution prob_per_face[6], mix_prob_per_face[6];

	step_1D_prob prob_face, mix_prob_face;

	single_texture_sampler lightprobe_sampler;

	Matrix4	world_to_direct, direct_to_world, direct_to_world_normal;
};