#include "r_states.h"

r_blendstate::r_blendstate(D3D_dev_handle* pdev, D3D11_BLEND_DESC &desc)
:pdev(pdev), p_state(NULL), p_save(NULL)
{
	pdev->get_device()->CreateBlendState(&desc, &p_state);
}

r_blendstate::~r_blendstate()
{
	SAFE_RELEASE(p_state);
}

void r_blendstate::set()
{
	pdev->get_context()->OMGetBlendState(&p_save, blend_factor, &sample_mask);
	pdev->get_context()->OMSetBlendState(p_state, NULL, 0xFFFFFFFF);
}

void r_blendstate::unset()
{
	pdev->get_context()->OMSetBlendState(p_save, blend_factor, sample_mask);
}


r_dsstate::r_dsstate(D3D_dev_handle* pdev, D3D11_DEPTH_STENCIL_DESC &desc)
:pdev(pdev), p_state(NULL), p_save(NULL)
{
	pdev->get_device()->CreateDepthStencilState(&desc, &p_state);
}

r_dsstate::~r_dsstate()
{
	SAFE_RELEASE(p_state);
}

void r_dsstate::set()
{
	pdev->get_context()->OMGetDepthStencilState(&p_save, &stencil_ref);
	pdev->get_context()->OMSetDepthStencilState(p_state, 0);
}

void r_dsstate::unset()
{
	pdev->get_context()->OMSetDepthStencilState(p_save, stencil_ref);
}

r_samplerstate::r_samplerstate(D3D_dev_handle* pdev, D3D11_SAMPLER_DESC &desc)
:pdev(pdev), p_state(NULL)
{
	pdev->get_device()->CreateSamplerState(&desc, &p_state);
}

r_samplerstate::~r_samplerstate()
{
	SAFE_RELEASE(p_state);
}

ID3D11SamplerState *&r_samplerstate::get_state()
{
	return p_state;
}
