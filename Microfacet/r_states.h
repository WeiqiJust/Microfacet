#pragma once
#include "utils.h"
#include "GPGPU_env_D3D11.h"

class r_blendstate
{
private:
	D3D_dev_handle		*pdev;
	ID3D11BlendState	*p_state, *p_save;
	float				blend_factor[4];
	UINT				sample_mask;

public:
	r_blendstate(D3D_dev_handle* pdev, D3D11_BLEND_DESC &desc);
	~r_blendstate();

	void set();
	void unset();
};

class r_dsstate
{
private:
	D3D_dev_handle		*pdev;
	ID3D11DepthStencilState
						*p_state, *p_save;
	UINT				stencil_ref;

public:
	r_dsstate(D3D_dev_handle* pdev, D3D11_DEPTH_STENCIL_DESC &desc);
	~r_dsstate();

	void set();
	void unset();
};

class r_samplerstate
{
private:
	D3D_dev_handle		*pdev;
	ID3D11SamplerState	*p_state;

public:
	r_samplerstate(D3D_dev_handle* pdev, D3D11_SAMPLER_DESC &desc);
	~r_samplerstate();

	ID3D11SamplerState *&get_state();
};
