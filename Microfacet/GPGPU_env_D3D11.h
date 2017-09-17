#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "utils.h"

using namespace std;
class D3D_dev_handle
{
private:
	ID3D11Device*			pdevice;
	ID3D11DeviceContext*	pcontext;

public:
	D3D_dev_handle();
	virtual ~D3D_dev_handle();

	void set(ID3D11Device* pdev, ID3D11DeviceContext* pcon);
	ID3D11Device* get_device();
	ID3D11DeviceContext* get_context();
};

class GPGPU_env_D3D
{
private:
	HINSTANCE               g_hInst;
	HWND                    g_hWnd;
	D3D_DRIVER_TYPE         g_driverType;
	D3D_FEATURE_LEVEL		g_featureLevel;

	ID3D11Device*           g_pd3dDevice;
	ID3D11DeviceContext*	g_pcontext;
	IDXGISwapChain*         g_pSwapChain;

	D3D_dev_handle			dev_handle;

	void init_window(HINSTANCE hInstance);
	void init_d3d();

public:
	GPGPU_env_D3D();
	~GPGPU_env_D3D();

	void init();
	void release();

	D3D_dev_handle* get_handle();
};

#define R_TARGET_CPU_READ	0x0001
#define R_TARGET_CPU_WRITE	0x0002
#define R_TARGET_CREATE_SRV	0x0004
#define R_TARGET_NO_RTV		0x0008
#define R_TARGET_CUBEMAP	0x1000

class render_target
{
private:
	D3D_dev_handle			*pdev;
	int						width, height, array_size, mip_levels,
							flag;
	bool					b_init;

	ID3D11Texture2D			*p_rendertarget, *p_rendertarget_CPU;
	ID3D11ShaderResourceView*
							p_rendertarget_srv;
	ID3D11RenderTargetView	*p_rendertarget_view;

	//Below is for map
	BYTE						*p_texels;
	int							pos;
	D3D11_MAPPED_SUBRESOURCE	mapped_tex;


public:
	render_target(D3D_dev_handle *pdev);
	~render_target();

	void init(const int w, const int h, const int array_size,
		const int mip_levels,
		DXGI_FORMAT target_format,// = DXGI_FORMAT_R8G8B8A8_UNORM, 
		const int flg//= R_TARGET_CPU_READ
		);
	void release();

	int get_width(const int level) const;
	int get_height(const int level) const;

	void clear(float *ClearColor);
	ID3D11ShaderResourceView*& get_target_srv();
	ID3D11RenderTargetView* get_rtv();

	void generate_mips();
	void get_result(BYTE *result, const int size_per_pixel, const int miplevel = 0);
	void set_content(BYTE *src, const int num_pixels, const int size_per_pixel);

	void map();
	void append(BYTE *src, const int num_pixels, const int size_per_pixel);
	void unmap();

	//DEBUG
	void save_dds(const char *filename);
};

class render_output
{
private:
	D3D_dev_handle			*pdev;
	bool					b_init;
	D3D11_VIEWPORT			vp;

	ID3D11Texture2D			*p_depthstencil;
	ID3D11DepthStencilView	*p_depthstencil_view;

	std::vector<ID3D11RenderTargetView*> mrt;
	std::vector<render_target*> rts;

public:
	render_output(D3D_dev_handle *pdev);
	virtual ~render_output();

	void init(const int w, const int h, const int array_size, const bool b_cube = false);
	void release();

	void clear_mrt();
	void add_rt(render_target* p);
	render_target* get_rt(const int idx);

	void set();
	void unset();

	void clear_depth();

	//DEBUG
	//void save_depth(const char *filename);
};
