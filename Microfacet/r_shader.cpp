#include "r_shader.h"

r_shader::r_shader()
: p_vs(NULL), p_gs(NULL), p_ps(NULL)
{
}

void r_shader::init(D3D_dev_handle* pd, 
					ID3D11VertexShader* p_v, 
					ID3D11GeometryShader* p_g,
					ID3D11PixelShader* p_p)
{
	pdev = pd;
	p_vs = p_v;
	p_ps = p_p;
	p_gs = p_g;
}

r_shader::~r_shader()
{
	SAFE_RELEASE(p_vs);
	SAFE_RELEASE(p_gs);
	SAFE_RELEASE(p_ps);
}

void r_shader::create_cb(ID3D11Buffer* &pcb, const int sz)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sz;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HRESULT hr = pdev->get_device()->CreateBuffer(&bd, NULL, &pcb);

	if (hr == E_INVALIDARG) {
		MessageBox(0, L"[CBPEROBJECTBUFFER] An invalid parameter was passed to the returning function.", L"Error", MB_OK);
		return;
	}
	else if (hr == E_OUTOFMEMORY) {
		MessageBox(0, L"[CBPEROBJECTBUFFER] Out of memory", L"Error", MB_OK);
		return;
	}
	else if (FAILED(hr)) {
		MessageBox(0, L"[CBPEROBJECTBUFFER] An unknown error occured", L"Error", MB_OK);
		return;
	}
}

void r_shader::setup_render()
{
	set_shaders();
}

void r_shader::update_cb(ID3D11Buffer *pcb, void *p)
{
	pdev->get_context()->UpdateSubresource(pcb, 0, NULL, p, 0, 0);
}

void r_shader::set_shaders()
{
	pdev->get_context()->VSSetShader(p_vs, NULL, 0);
	pdev->get_context()->GSSetShader(p_gs, NULL, 0);
	pdev->get_context()->PSSetShader(p_ps, NULL, 0);
}

void r_shader::set_ps_resource(const int slot, ID3D11ShaderResourceView* &p_srv, const int num)
{
	pdev->get_context()->PSSetShaderResources(slot, num, &p_srv);
}

void r_shader::set_ps_sampler(const int slot, ID3D11SamplerState* &p_sampler, const int num)
{
	pdev->get_context()->PSSetSamplers(slot, num, &p_sampler);
}

void create_shader_from_file(r_shader* sh,
	char *filename,
	char *vsname, char *vsmodel,
	char *gsname, char *gsmodel,
	char *psname, char *psmodel,
	D3D_dev_handle* pdev,
	ID3DBlob** p_blob)
{
	HRESULT hr;
	ID3D11VertexShader*		p_vs = NULL;
	ID3D11PixelShader*		p_ps = NULL;
	ID3D11GeometryShader*	p_gs = NULL;

	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(filename, vsname, vsmodel, &pVSBlob);
	hr = pdev->get_device()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &p_vs);
	if (p_blob == NULL)
		pVSBlob->Release();
	else
		*p_blob = pVSBlob;

	if (strlen(gsname) > 0)
	{
		ID3DBlob* pGSBlob = NULL;
		hr = CompileShaderFromFile(filename, gsname, gsmodel, &pGSBlob);
		hr = pdev->get_device()->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &p_gs);
		pGSBlob->Release();
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(filename, psname, psmodel, &pPSBlob);
	hr = pdev->get_device()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &p_ps);
	pPSBlob->Release();

	sh->init(pdev, p_vs, p_gs, p_ps);
}


void create_shader_from_file_seperate(r_shader* sh,
	char *vertexfilename, char *pixelfilename, char *geomfilename,
	char *vsname, char *vsmodel,
	char *gsname, char *gsmodel,
	char *psname, char *psmodel,
	D3D_dev_handle* pdev,
	ID3DBlob** p_blob)
{
	HRESULT hr;
	ID3D11VertexShader*		p_vs = NULL;
	ID3D11PixelShader*		p_ps = NULL;
	ID3D11GeometryShader*	p_gs = NULL;

	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(vertexfilename, vsname, vsmodel, &pVSBlob);
	hr = pdev->get_device()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &p_vs);
	if (p_blob == NULL)
		pVSBlob->Release();
	else
		*p_blob = pVSBlob;

	if (strlen(gsname) > 0)
	{
		ID3DBlob* pGSBlob = NULL;
		hr = CompileShaderFromFile(geomfilename, gsname, gsmodel, &pGSBlob);
		hr = pdev->get_device()->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &p_gs);
		pGSBlob->Release();
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(pixelfilename, psname, psmodel, &pPSBlob);
	hr = pdev->get_device()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &p_ps);
	pPSBlob->Release();

	sh->init(pdev, p_vs, p_gs, p_ps);
}
