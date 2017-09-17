#include "GPGPU_env_D3D11.h"
//#include "d3dx9tex.h"

//D3D_dev_handle
D3D_dev_handle::D3D_dev_handle()
:pdevice(NULL), pcontext(NULL)
{
}

D3D_dev_handle::~D3D_dev_handle()
{
}

void D3D_dev_handle::set(ID3D11Device* pdev, ID3D11DeviceContext* pcon)
{
	pdevice = pdev;
	pcontext = pcon;
}

ID3D11Device* D3D_dev_handle::get_device()
{
	return pdevice;
}

ID3D11DeviceContext* D3D_dev_handle::get_context()
{
	return pcontext;
}

//GPGPU_env_D3D
GPGPU_env_D3D::GPGPU_env_D3D()
:g_hInst(NULL), g_hWnd(NULL), g_driverType(D3D_DRIVER_TYPE_NULL),
g_pd3dDevice(NULL), g_pcontext(NULL), g_pSwapChain(NULL), g_featureLevel(D3D_FEATURE_LEVEL_11_0)
{
}

GPGPU_env_D3D::~GPGPU_env_D3D()
{
	release();
}

void GPGPU_env_D3D::init()
{
	init_window(NULL);
	init_d3d();
}

void GPGPU_env_D3D::release()
{
	if (g_pcontext) g_pcontext->ClearState();

	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pcontext);
	SAFE_RELEASE(g_pd3dDevice);
}

void GPGPU_env_D3D::init_window(HINSTANCE hInstance)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"GPGPU_env_D3D";
	wcex.hIconSm = NULL;
	if (!RegisterClassEx( &wcex ))
		cout << "failed to register window class.";

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, RENDER_WIDTH, RENDER_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"GPGPU_env_D3D", L"window for D3D11", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
		cout << "failed to create window.";

	//hide the window
	ShowWindow(g_hWnd, SW_HIDE);
	UpdateWindow(g_hWnd);
}

void GPGPU_env_D3D::init_d3d()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 16;
	sd.BufferDesc.Height = 16;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pcontext);
		// need to install DirectX to fix it
		if( SUCCEEDED( hr ) )
			break;
	}
	if( FAILED( hr ) )
		cout << "failed to create D3D11 device.";
	//cout << "device level = " << g_pd3dDevice->GetFeatureLevel() << endl;
	dev_handle.set(g_pd3dDevice, g_pcontext);



	ID3D11Debug *d3dDebug = nullptr;
	if (SUCCEEDED(g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
		{
			ID3D11InfoQueue *d3dInfoQueue = nullptr;
			if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
			{
#ifdef _DEBUG
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

				D3D11_MESSAGE_ID hide[] =
				{
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
					// Add more message IDs here as needed
				};

				D3D11_INFO_QUEUE_FILTER filter;
				memset(&filter, 0, sizeof(filter));
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
				d3dInfoQueue->Release();
			}
			d3dDebug->Release();
		}
}

D3D_dev_handle* GPGPU_env_D3D::get_handle()
{
	return &dev_handle;
}

render_target::render_target(D3D_dev_handle *pdev)
:pdev(pdev), width(0), height(0), b_init(false),
p_rendertarget_view(NULL), p_rendertarget(NULL), 
p_rendertarget_CPU(NULL), p_rendertarget_srv(NULL)
{
}

render_target::~render_target()
{
	release();
}

void render_target::init(const int w, const int h, const int arraysize,
						 const int miplevels,
						 DXGI_FORMAT target_format, const int flg)
{
	if (b_init) return;

	width		= w;
	height		= h;
	flag		= flg;
	array_size	= (flag & R_TARGET_CUBEMAP) ? 6 : arraysize;
	mip_levels	= miplevels;

	//create the render target
	D3D11_TEXTURE2D_DESC dstex;
	memset(&dstex, 0, sizeof(dstex));
	dstex.Width		= width;
	dstex.Height	= height;
	dstex.MipLevels = mip_levels;
	dstex.ArraySize = array_size;
	dstex.SampleDesc.Count = 1;
	dstex.SampleDesc.Quality = 0;
	dstex.Usage		= D3D11_USAGE_DEFAULT;
	dstex.Format	= target_format;
	if ((flag & R_TARGET_NO_RTV) == 0)
		dstex.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if (flag & R_TARGET_CREATE_SRV)
		dstex.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	if (flag & R_TARGET_CUBEMAP)
		dstex.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (mip_levels != 1)
		dstex.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	pdev->get_device()->CreateTexture2D(&dstex, NULL, &p_rendertarget);

	if (mip_levels == 0)
	{
		int dim = max(width, height);
		while (dim > 0)
		{
			dim >>= 1;
			mip_levels++;
		}
	}

	if ((flag & R_TARGET_NO_RTV) == 0)
	{
		//create the render target view
		D3D11_RENDER_TARGET_VIEW_DESC dsrtv;
		memset(&dsrtv, 0, sizeof(dsrtv));
		dsrtv.Format		= dstex.Format;
		if (array_size > 1)
		{
			dsrtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			dsrtv.Texture2DArray.ArraySize = dstex.ArraySize;
		} else {
			dsrtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			dsrtv.Texture2D.MipSlice = 0;
		}
		HRESULT hr = pdev->get_device()->CreateRenderTargetView(p_rendertarget, &dsrtv, &p_rendertarget_view);
		if (FAILED(hr))
		{
			cout << "Error::create render target failed!" << endl;
		}
	}

	//create the render target for CPU
	if (flag & (R_TARGET_CPU_READ|R_TARGET_CPU_WRITE))
	{
		dstex.Usage = D3D11_USAGE_STAGING;
		dstex.CPUAccessFlags = 0;
		if (flag & R_TARGET_CPU_READ)
			dstex.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		if (flag & R_TARGET_CPU_WRITE)
			dstex.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		dstex.BindFlags = 0;
		dstex.MiscFlags &= ~D3D11_RESOURCE_MISC_GENERATE_MIPS;
		pdev->get_device()->CreateTexture2D(&dstex, NULL, &p_rendertarget_CPU);
	}

	//create shader resource view
	if (flag & R_TARGET_CREATE_SRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		memset(&srvdesc, 0, sizeof(srvdesc));
		srvdesc.Format = dstex.Format;
		if (flag & R_TARGET_CUBEMAP)
		{
			srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvdesc.TextureCube.MipLevels = 1;
			srvdesc.TextureCube.MostDetailedMip = 0;
		} else if (array_size > 1) 
		{
			srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvdesc.Texture2DArray.ArraySize = array_size;
			srvdesc.Texture2DArray.MipLevels = mip_levels;			
		} else {
			srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvdesc.Texture2D.MipLevels = mip_levels;
		}
		pdev->get_device()->CreateShaderResourceView(p_rendertarget, 
			&srvdesc, &p_rendertarget_srv);
	}

	b_init = true;
}

void render_target::release()
{
	SAFE_RELEASE(p_rendertarget);
	SAFE_RELEASE(p_rendertarget_view);
	SAFE_RELEASE(p_rendertarget_CPU);
	SAFE_RELEASE(p_rendertarget_srv);
}

void render_target::generate_mips()
{
	if (mip_levels != 1)
		pdev->get_context()->GenerateMips(p_rendertarget_srv);
}

int render_target::get_width(const int level) const
{
	return max((width >> level), 1);
}

int render_target::get_height(const int level) const
{
	return max((height >> level), 1);
}

void render_target::get_result(BYTE *result, const int size_per_pixel, const int level)
{
	//if (array_size == 1)
	//pdev->get_context()->CopyResource(p_rendertarget_CPU, p_rendertarget);

	for (int f = 0; f < array_size; f++)
	{
		UINT subresource = D3D11CalcSubresource(level, f, mip_levels);

		pdev->get_context()->CopySubresourceRegion(p_rendertarget_CPU, subresource,
			0, 0, 0, p_rendertarget, subresource, NULL);

		D3D11_MAPPED_SUBRESOURCE mapped_tex;
		pdev->get_context()->Map(p_rendertarget_CPU, subresource, 
			D3D11_MAP_READ, 0, &mapped_tex);

		UINT	w = get_width(level),
				h = get_height(level);
		BYTE* pTexels = (BYTE*)mapped_tex.pData;
		for(UINT row = 0; row < h; row++)
		{
			memcpy(result, pTexels, size_per_pixel*w);
			pTexels += mapped_tex.RowPitch;
			result	+= size_per_pixel*w;
		}

		pdev->get_context()->Unmap(p_rendertarget_CPU, subresource);
	}

	//if (flag & R_TARGET_CUBEMAP)
	//{
	//	for (int f = 0; f < 6; f++)
	//	{
	//		D3D11_MAPPED_SUBRESOURCE mapped_tex;
	//		pdev->get_context()->Map(p_rendertarget_CPU, D3D11CalcSubresource(0, f, 1), 
	//			D3D11_MAP_READ, 0, &mapped_tex);

	//		BYTE* pTexels = (BYTE*)mapped_tex.pData;
	//		for(UINT row = 0; row < height; row++)
	//		{
	//			memcpy(result, pTexels, size_per_pixel*width);
	//			pTexels += mapped_tex.RowPitch;
	//			result	+= size_per_pixel*width;
	//		}

	//		pdev->get_context()->Unmap(p_rendertarget_CPU, D3D11CalcSubresource(0, f, 1));
	//	}
	//} else {
	//	D3D11_MAPPED_SUBRESOURCE mapped_tex;
	//	pdev->get_context()->Map(p_rendertarget_CPU, 0, D3D11_MAP_READ, 0, &mapped_tex);

	//	BYTE* pTexels = (BYTE*)mapped_tex.pData;
	//	for(UINT row = 0; row < height; row++)
	//	{
	//		memcpy(result, pTexels, size_per_pixel*width);
	//		pTexels += mapped_tex.RowPitch;
	//		result	+= size_per_pixel*width;
	//	}

	//	pdev->get_context()->Unmap(p_rendertarget_CPU, 0);
	//}
}

void render_target::set_content(BYTE *src, const int num_pixels, const int size_per_pixel)
{
	D3D11_MAPPED_SUBRESOURCE mapped_tex;

	if (flag & R_TARGET_CUBEMAP)
	{
		for (int f = 0; f < 6; f++)
		{
			pdev->get_context()->Map(p_rendertarget_CPU, 
				D3D11CalcSubresource(0, f, 1), D3D11_MAP_WRITE, 0, &mapped_tex);

			BYTE* pTexels = (BYTE*)mapped_tex.pData;
			for(UINT row = 0; row < height; row++)
			{
				memcpy(pTexels, src, size_per_pixel*width);
				pTexels += mapped_tex.RowPitch;
				src		+= size_per_pixel*width;
			}
			
			pdev->get_context()->Unmap(p_rendertarget_CPU, 
				D3D11CalcSubresource(0, f, 1));
		}
	} else {
		pdev->get_context()->Map(p_rendertarget_CPU, 0, D3D11_MAP_WRITE, 0, &mapped_tex);
		
		BYTE* pTexels = (BYTE*)mapped_tex.pData;
		UINT row = 0;
		while (row < height)
		{
			int w = min(num_pixels - row*width, width);
			memcpy(pTexels, src, size_per_pixel*w);
			if (num_pixels - row*width <= width)
				break;
			pTexels += mapped_tex.RowPitch;
			src		+= size_per_pixel*width;
			row++;
		}
		
		pdev->get_context()->Unmap(p_rendertarget_CPU, 0);
	}

	pdev->get_context()->CopyResource(p_rendertarget, p_rendertarget_CPU);
}

void render_target::map()
{
	pdev->get_context()->Map(p_rendertarget_CPU, 0, D3D11_MAP_WRITE, 0, &mapped_tex);

	p_texels = (BYTE*)mapped_tex.pData;
	pos = 0;
}

void render_target::append(BYTE *src, const int num_pixels, const int size_per_pixel)
{
	bool b_one_line_only = false;
	int len_first, row, col, num_left;

	row			= pos / width;
	col			= pos % width;
	num_left	= num_pixels;
	len_first	= (width - col) % width;
	if (len_first >= num_pixels)
	{
		len_first = num_pixels;
		b_one_line_only = true;
	}

	if (len_first > 0)
	{
		memcpy(&p_texels[row*mapped_tex.RowPitch+col*size_per_pixel], src, size_per_pixel*len_first);
		src			+= size_per_pixel*len_first;
		num_left	-= len_first;
		row++;
	}

	if (!b_one_line_only)
	{
		while (num_left > 0)
		{
			int sz = min(num_left, width);

			memcpy(&p_texels[row*mapped_tex.RowPitch], src, size_per_pixel*sz);
			src		+= size_per_pixel*sz;
			num_left-= sz;
			row++;
		}
	}

	pos += num_pixels;

	//len_last = (pos+num_pixels) % width;

	//if (column != 0)
	//{
	//	int w = min(num_pixels, width-column);
	//	memcpy(p_texels, src, size_per_pixel*w);
	//	if (num_pixels - (row*width+column) <= width-column)
	//		return;
	//	p_texels+= mapped_tex.RowPitch;
	//	src		+= size_per_pixel*(width-column);
	//	row++;
	//}
}

void render_target::unmap()
{
	pdev->get_context()->Unmap(p_rendertarget_CPU, 0);
	pdev->get_context()->CopyResource(p_rendertarget, p_rendertarget_CPU);
}


void render_target::save_dds(const char *filename)
{
	pdev->get_context()->CopyResource(p_rendertarget_CPU, p_rendertarget);
	//D3DX11SaveTextureToFileA(pdev->get_context(), p_rendertarget_CPU, D3DX11_IFF_DDS, filename);// get from original code, for debug use
	//D3DXSaveTextureToFile(filename, D3DXIFF_DDS, p_rendertarget_CPU);
}

void render_target::clear(float *ClearColor)
{
	pdev->get_context()->ClearRenderTargetView(p_rendertarget_view, ClearColor);
}

ID3D11ShaderResourceView*& render_target::get_target_srv()
{
	return p_rendertarget_srv;
}

ID3D11RenderTargetView* render_target::get_rtv()
{
	return p_rendertarget_view;
}

render_output::render_output(D3D_dev_handle *pdev)
:pdev(pdev), b_init(false),
p_depthstencil(NULL), p_depthstencil_view(NULL)
{
}

render_output::~render_output()
{
	release();
}

void render_output::init(const int w, const int h, const int array_size, const bool b_cube)
{
	if (b_init) return;

	vp.Width	= (FLOAT)w;
	vp.Height	= (FLOAT)h;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	//create the depth stencil texture.
	D3D11_TEXTURE2D_DESC dstex;
	memset(&dstex, 0, sizeof(dstex));
	dstex.Width		= w;
	dstex.Height	= h;
	dstex.MipLevels = 1;
	dstex.SampleDesc.Count = 1;
	dstex.SampleDesc.Quality = 0;
	dstex.Usage		= D3D11_USAGE_DEFAULT;
	dstex.Format	= DXGI_FORMAT_D32_FLOAT;
	dstex.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dstex.CPUAccessFlags = 0;
	if (b_cube)
	{
		dstex.ArraySize = 6;
		dstex.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	} else {
		dstex.ArraySize = array_size;
		dstex.MiscFlags = 0;
	}
	pdev->get_device()->CreateTexture2D(&dstex, NULL, &p_depthstencil);

	//create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsdsv;
	memset(&dsdsv, 0, sizeof(dsdsv));
	dsdsv.Format			= dstex.Format;
	if (b_cube || array_size > 1)
	{
		dsdsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsdsv.Texture2DArray.ArraySize = dstex.ArraySize;
	} else {
		dsdsv.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
		dsdsv.Texture2D.MipSlice= 0;
	}

	pdev->get_device()->CreateDepthStencilView(p_depthstencil, &dsdsv, &p_depthstencil_view);

	b_init = true;
}

void render_output::release()
{
	SAFE_RELEASE(p_depthstencil);
	SAFE_RELEASE(p_depthstencil_view);
}

void render_output::set()
{
	//set render targets
	pdev->get_context()->OMSetRenderTargets(mrt.size(), &mrt[0], p_depthstencil_view);

	//set up the viewport
	pdev->get_context()->RSSetViewports(1, &vp);
}

void render_output::unset()
{
	pdev->get_context()->OMSetRenderTargets(0, NULL, NULL);
}

void render_output::clear_depth()
{
	pdev->get_context()->ClearDepthStencilView(p_depthstencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void render_output::clear_mrt()
{
	mrt.clear();
	rts.clear();
}

void render_output::add_rt(render_target* p)
{
	mrt.push_back(p->get_rtv());
	rts.push_back(p);
}

render_target* render_output::get_rt(const int idx)
{
	return rts[idx];
}

//void render_output::save_depth(const char *filename)
//{
//	//This following code cannot output meaningful images. Don't know why.
//	ID3D11Texture2D	*p_rendertarget_CPU;
//
//	//create the render target
//	D3D11_TEXTURE2D_DESC dstex;
//	memset(&dstex, 0, sizeof(dstex));
//	dstex.Width		= (int)vp.Width;
//	dstex.Height	= (int)vp.Height;
//	dstex.MipLevels = 1;
//	dstex.ArraySize = 1;
//	dstex.SampleDesc.Count = 1;
//	dstex.SampleDesc.Quality = 0;
//	dstex.Format	= DXGI_FORMAT_R32_FLOAT;
//	dstex.Usage = D3D11_USAGE_STAGING;
//	dstex.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//	dstex.BindFlags = 0;
//	pdev->get_device()->CreateTexture2D(&dstex, NULL, &p_rendertarget_CPU);
//
//	pdev->get_context()->CopyResource(p_rendertarget_CPU, p_depthstencil);
//	D3DX11SaveTextureToFileA(pdev->get_context(), p_rendertarget_CPU, D3DX11_IFF_DDS, filename);
//
//	SAFE_RELEASE(p_rendertarget_CPU);
//}
