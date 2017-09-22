#include "r_skybox.h"
#include "hammersley.h"

struct SKYBOX_VERTEX
{
	SKYBOX_VERTEX() {};
	SKYBOX_VERTEX(float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) {};
	float x, y, z, w;
};

const D3D11_INPUT_ELEMENT_DESC g_aVertexLayout[] =
{
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
};




r_skybox::r_skybox(const Vector3 sscale, const float ddist, 
	const shared_ptr<cube_texture> & ttexture, const Matrix4 &mmat)
	:inten(sscale), dist(ddist), lightprobe(ttexture), mat(mmat), sh(NULL)
{
	pEnvironmentMap11 = NULL;
	pEnvironmentRV11 = NULL;
	pSam = NULL;
	pVertexLayout11 = NULL;
	pcbVSPerObject = NULL;
	pVB11 = NULL;
	pDepthStencilState11 = NULL;

	frame_o[0] = Vector3(-1, 0, 0);
	frame_x[0] = Vector3(0, 0, -1);
	frame_y[0] = Vector3(0, -1, 0);

	frame_o[1] = Vector3(1, 0, 0);
	frame_x[1] = Vector3(0, 0, 1);
	frame_y[1] = Vector3(0, -1, 0);

	frame_o[2] = Vector3(0, 1, 0);
	frame_x[2] = Vector3(1, 0, 0);
	frame_y[2] = Vector3(0, 0, -1);

	frame_o[3] = Vector3(0, -1, 0);
	frame_x[3] = Vector3(1, 0, 0);
	frame_y[3] = Vector3(0, 0, 1);

	frame_o[4] = Vector3(0, 0, 1);
	frame_x[4] = Vector3(1, 0, 0);
	frame_y[4] = Vector3(0, -1, 0);

	frame_o[5] = Vector3(0, 0, -1);
	frame_x[5] = Vector3(1, 0, 0);
	frame_y[5] = Vector3(0, -1, 0);


	lightprobe_sampler.address_mode = TXA_CLAMP;
	lightprobe_sampler.filter_mode = TXF_LINEAR;
}

r_skybox::~r_skybox()
{
	SAFE_RELEASE(pVB11);
	SAFE_RELEASE(pEnvironmentMap11);
	SAFE_RELEASE(pEnvironmentRV11);
	SAFE_RELEASE(pSam);
	SAFE_RELEASE(pVertexLayout11);
	SAFE_RELEASE(pcbVSPerObject);
	SAFE_RELEASE(pDepthStencilState11);
}


void r_skybox::config_scene(const Vector3 &center, const float radius)
{
	scene_center = center;
	scene_radius = radius;

	Vector3			texel;
	std::vector<float>	face_lumin;
	int	width = lightprobe->get_dim();
	int height = width;

	face_lumin.resize(6, 0);

	for (int fidx = 0; fidx < 6; fidx++)
	{
		float		dA;
		Vector3	face_power;

		std::vector<float>	y_lumin, x_lumin;

		dA = (2.0 / width) * (2.0 / height);

		y_lumin.resize(height, 0);
		x_lumin.resize(width);

		for (int iy = 0; iy < height; iy++)
		{
			for (int ix = 0; ix < width; ix++)
			{
				float x, y, r;

				x = (ix + 0.5) / width * 2 - 1;
				y = (iy + 0.5) / height * 2 - 1;
				r = sqrt(x*x + y*y + 1);

				texel = lightprobe->get_texel(fidx, ix, iy);
				texel *= dA / (r*r*r);

				face_power += texel;
				face_lumin[fidx] += (texel.x + texel.y + texel.z) / 3;//texel.luminance();

				x_lumin[ix] = (texel.x + texel.y + texel.z) / 3;//texel.luminance();
				y_lumin[iy] += (texel.x + texel.y + texel.z) / 3;//texel.luminance();
			}

			step_1D_prob prob_x(x_lumin);
			prob_per_face[fidx].prob_x.push_back(prob_x);
		}

		step_1D_prob prob_y(y_lumin);
		prob_per_face[fidx].prob_y = prob_y;

		power += face_power;
	}

	power = power * inten;
	power *= (PI * scene_radius * scene_radius);

	//init prob_face
	step_1D_prob prob(face_lumin);
	prob_face = prob;

	//mix prob
	float avg_lumin = 0;
	for (int i = 0; i < face_lumin.size(); i++)
		avg_lumin += face_lumin[i];
	avg_lumin /= face_lumin.size()*width*height;

	for (int i = 0; i < face_lumin.size(); i++)
		face_lumin[i] += avg_lumin*width*height;
	step_1D_prob mix_prob(face_lumin);
	mix_prob_face = mix_prob;

	for (int fidx = 0; fidx < 6; fidx++)
	{
		//((cube_texture*)lightprobe)->get_face(fidx, face);
		float		dA;
		std::vector<float>	y_lumin, x_lumin;

		dA = (2.0 / width) * (2.0 / height);

		y_lumin.resize(height, 0);
		x_lumin.resize(width);

		for (int iy = 0; iy < height; iy++)
		{
			for (int ix = 0; ix < width; ix++)
			{
				float x, y, r;

				x = (ix + 0.5) / width * 2 - 1;
				y = (iy + 0.5) / height * 2 - 1;
				r = sqrt(x*x + y*y + 1);

				texel = lightprobe->get_texel(fidx, ix, iy);
				texel *= dA / (r*r*r);

				x_lumin[ix] = (texel.x + texel.y + texel.z) / 3 + avg_lumin;
				y_lumin[iy] += (texel.x + texel.y + texel.z) / 3 + avg_lumin;
			}

			step_1D_prob prob_x(x_lumin);
			mix_prob_per_face[fidx].prob_x.push_back(prob_x);
		}

		step_1D_prob prob_y(y_lumin);
		mix_prob_per_face[fidx].prob_y = prob_y;
	}

	//set direct transform
	mat.m[0][3] = scene_center.x;
	mat.m[1][3] = scene_center.y;
	mat.m[2][3] = scene_center.z;

	direct_to_world = mat;

	world_to_direct = mat;
	world_to_direct = Inverse(world_to_direct);

	direct_to_world_normal = world_to_direct;
	direct_to_world_normal.m[0][3] =
		direct_to_world_normal.m[1][3] =
		direct_to_world_normal.m[2][3] = 0;
	direct_to_world_normal = Transpose(direct_to_world_normal);
}

int face_idx[] = {1, 0, 2, 3, 5, 4};

void r_skybox::update_cube_tex()
{
	//texture_2d *p_tex2d;
	//lightprobe.get_face(0, p_tex2d);

	int	width = lightprobe->get_dim(),
		height	= width;

	float *pdata = new float[width*height*4];
	for (int face = 0; face < 6; face++)
	{
		//float* p_tex2d = lightprobe->get_face(face_idx[face]);

		Vector3 c;
		for (int i = 0; i < width*height; i++)
		{
			int x = i % width, 
				y = i / width;

			if (face == 5)
			{
				x = width-1 - x;
				y = height-1 - y;
			}
			c = lightprobe->get_texel(face, x, y);
			c = Vector3(c.x*inten.x, c.y*inten.y, c.z*inten.z);
			pdata[i*4+0] = c.x;
			pdata[i*4+1] = c.y;
			pdata[i*4+2] = c.z;
			pdata[i * 4 + 3] = 1.0f;
		}

		pdev->get_context()->UpdateSubresource(pEnvironmentMap11, D3D11CalcSubresource(0, face, 1), NULL, pdata, 
			width*sizeof(float)*4, 0);
	}
	SAFE_DELETE_ARR(pdata);
	//SAFE_DELETE(pdata);
}

void r_skybox::init(const int scr_w, const int scr_h, D3D_dev_handle* p)
{
	screen_w = scr_w;
	screen_h = scr_h;
	pdev = p;

	//create the cube texture
	//texture_2d *p_tex2d;
	//((cube_texture*)lightprobe)->get_face(0, p_tex2d);

	int	width = lightprobe->get_dim(),
		height = lightprobe->get_dim();
	D3D11_TEXTURE2D_DESC dstex;
	ZeroMemory(&dstex, sizeof(dstex));
	dstex.Width		= width;
	dstex.Height	= height;
	dstex.MipLevels	= 1;
	dstex.ArraySize	= 6;
	dstex.SampleDesc.Count	= 1;
	dstex.Usage		= D3D11_USAGE_DEFAULT;
	dstex.Format	= DXGI_FORMAT_R32G32B32A32_FLOAT; // Weiqi:: original DXGI_FORMAT_R32G32B32_FLOAT;
	dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dstex.MiscFlags	= D3D11_RESOURCE_MISC_TEXTURECUBE;
	pdev->get_device()->CreateTexture2D(&dstex, NULL, &pEnvironmentMap11);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = dstex.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.TextureCube.MipLevels = 1;
	pdev->get_device()->CreateShaderResourceView(pEnvironmentMap11, &SRVDesc, 
		&pEnvironmentRV11);

	update_cube_tex();

	// Create the shaders
	ID3DBlob* pBlobVS = NULL;
	sh = new r_shader;
	create_shader_from_file(sh, SHADER_PATH "skybox11.hlsl", "SkyboxVS", "vs_5_0", "", "", "SkyboxPS", "ps_5_0", pdev, &pBlobVS);

	// Create an input layout
	pdev->get_device()->CreateInputLayout(g_aVertexLayout, 1, 
		pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), &pVertexLayout11);
	SAFE_RELEASE(pBlobVS);

	// Query support for linear filtering on DXGI_FORMAT_R32G32B32A32
	UINT FormatSupport = 0;
	pdev->get_device()->CheckFormatSupport(DXGI_FORMAT_R32G32B32A32_FLOAT, &FormatSupport);

	// Setup linear or point sampler according to the format Query result
	D3D11_SAMPLER_DESC SamDesc;
	SamDesc.Filter = (FormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) > 0 ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.MipLODBias = 0.0f;
	SamDesc.MaxAnisotropy = 1;
	SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
	SamDesc.MinLOD = 0;
	SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pdev->get_device()->CreateSamplerState(&SamDesc, &pSam);

	// Setup constant buffer
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;
	Desc.ByteWidth = sizeof(CB_VS_SKYBOX);
	pdev->get_device()->CreateBuffer(&Desc, NULL, &pcbVSPerObject);

	// Depth stencil state
	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	DSDesc.DepthEnable = FALSE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	pdev->get_device()->CreateDepthStencilState(&DSDesc, &pDepthStencilState11);

	// Fill the vertex buffer
	SKYBOX_VERTEX* pVertex = new SKYBOX_VERTEX[4];

	// Map texels to pixels 
	float fHighW = -1.0f - ( 1.0f / (float)screen_w );
	float fHighH = -1.0f - ( 1.0f / (float)screen_h );
	float fLowW = 1.0f + ( 1.0f / (float)screen_w );
	float fLowH = 1.0f + ( 1.0f / (float)screen_h );

	pVertex[0] = SKYBOX_VERTEX(fLowW, fLowH, 1.0f, 1.0f);
	pVertex[1] = SKYBOX_VERTEX(fLowW, fHighH, 1.0f, 1.0f);
	pVertex[2] = SKYBOX_VERTEX(fHighW, fLowH, 1.0f, 1.0f);
	pVertex[3] = SKYBOX_VERTEX(fHighW, fHighH, 1.0f, 1.0f);

	UINT uiVertBufSize = 4 * sizeof( SKYBOX_VERTEX );
	//Vertex Buffer
	D3D11_BUFFER_DESC vbdesc;
	vbdesc.ByteWidth = uiVertBufSize;
	vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbdesc.CPUAccessFlags = 0;
	vbdesc.MiscFlags = 0;    

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pVertex;    
	pdev->get_device()->CreateBuffer(&vbdesc, &InitData, &pVB11);
	SAFE_DELETE_ARR(pVertex);
	//SAFE_DELETE(pVertex);
}

void r_skybox::draw(const Matrix4 &matWorldViewProj)
{
	pdev->get_context()->IASetInputLayout( pVertexLayout11 );

	UINT uStrides = sizeof( SKYBOX_VERTEX );
	UINT uOffsets = 0;
	ID3D11Buffer* pBuffers[1] = { pVB11 };
	pdev->get_context()->IASetVertexBuffers( 0, 1, pBuffers, &uStrides, &uOffsets );
	pdev->get_context()->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );
	pdev->get_context()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	sh->set_shaders();

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pdev->get_context()->Map( pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	CB_VS_SKYBOX* pVSPerObject = (CB_VS_SKYBOX*)MappedResource.pData;
	Matrix4 mat = direct_to_world;
	mat = matWorldViewProj*mat;
	matrix_to_XMMATRIX(pVSPerObject->mWorldViewProj, mat);
	pdev->get_context()->Unmap( pcbVSPerObject, 0 );
	pdev->get_context()->VSSetConstantBuffers( 0, 1, &pcbVSPerObject );

	pdev->get_context()->PSSetSamplers( 0, 1, &pSam );
	pdev->get_context()->PSSetShaderResources( 0, 1, &pEnvironmentRV11 );

	ID3D11DepthStencilState* pDepthStencilStateStored11 = NULL;
	UINT StencilRef;
	pdev->get_context()->OMGetDepthStencilState( &pDepthStencilStateStored11, &StencilRef );
	pdev->get_context()->OMSetDepthStencilState( pDepthStencilState11, 0 );

	pdev->get_context()->Draw( 4, 0 );

	pdev->get_context()->OMSetDepthStencilState( pDepthStencilStateStored11, StencilRef );
}


void r_skybox::sample(geometry_differential &d_geom, const Vector3 &rv) const
{
	//((geometry*)geom)->sample_by_area(d_geom, direct_to_world, direct_to_world_normal, rv);
	//float	pdf_face, pdf_x, pdf_y;
	int		fidx, iy, ix;

	prob_face.sample(fidx, rv.x);
	prob_per_face[fidx].prob_y.sample(iy, rv.y);
	prob_per_face[fidx].prob_x[iy].sample(ix, rv.z);

	//texture_2d*	face;

	//((cube_texture*)lightprobe)->get_face(fidx, face);
	//printf_s("[%d] ", fidx);

	int		width = lightprobe->get_dim(),
		height = lightprobe->get_dim();
	float	x, y, u, v;

	u = (ix + 0.01) / width;
	v = (iy + 0.01) / height;
	x = u * 2 - 1;
	y = v * 2 - 1;

	Vector3	pos;

	pos = (frame_o[fidx] + x * frame_x[fidx] + y * frame_y[fidx]) * dist + scene_center;

	d_geom.normal = -frame_o[fidx];
	d_geom.p = pos;

	d_geom.p = direct_to_world		* d_geom.p;
	d_geom.normal = direct_to_world_normal* d_geom.normal;
	d_geom.normal.normalize();

	d_geom.uv = Vector2(u, v);

	d_geom.face_index = fidx;
}

void r_skybox::get_energy(Vector3 &c, const geometry_differential &ir) const
{
	lightprobe->get_sample(c, ir.face_index, ir.uv,
		Vector2(RELATIVE_PRECISION, 0), Vector2(0, RELATIVE_PRECISION), lightprobe_sampler);

	c = c*inten;
}

void r_skybox::sample_lights(std::vector<r_light_dir> &samples, const int num) const
{
	hammersley seq(num);

	samples.clear();
	for (int i = 0; i < num; i++)
	{
		geometry_differential light_d_geom;
		//rt_ray	shadow_ray;
		
		float* re = seq.get_sample(3);
		Vector3	rv = Vector3(re[0], re[1], re[2]);
		rv += Vector3(1e-8f, 1e-8f, 1e-8f);
		rv.x = min(rv.x, 1);
		rv.y = min(rv.y, 1);
		rv.z = min(rv.z, 1);
		sample(light_d_geom, rv);

		//shadow_ray.origin	= light_d_geom.p + light_d_geom.normal;
		//shadow_ray.direction=-light_d_geom.normal;

		Vector3 light_energy;
		//intersection_result shadow_ir;

		//intersect(shadow_ir, shadow_ray);
		get_energy(light_energy, light_d_geom);

		r_light_dir s;
		light_d_geom.p.normalize();
		s.dir = Vector3(light_d_geom.p.x, light_d_geom.p.y, light_d_geom.p.z);
		s.dir.x = -s.dir.x;
		s.dir.z = -s.dir.z;
		Vector3 c = light_energy / PI / num;
		s.c = c;
		samples.push_back(s);
	}

	//DEBUG
	//samples.resize(1);
	//for (int i = 1; i < num; i++)
	//	samples.push_back(samples[0]);

	////DEBUG
	//if (num == 3)
	//{
	//	samples[0].c = Vector3(1, 0, 0);
	//	samples[1].c = Vector3(0, 1, 0);
	//	samples[2].c = Vector3(0, 0, 1);
	//}

	////DEBUG
	//samples.clear();
	//{
	//	light_dir s;
	//	s.dir = Vector3(0, -1, 0);
	//	s.dir.normalize();
	//	s.c(1) = 0;
	//	s.c(2) = 0;
	//	s.c(3) = 1;
	//	samples.push_back(s);
	//}
}

void r_skybox::set_intensity(const Vector3 intensity)
{
	inten = intensity;
}

Vector3 r_skybox::get_intensity()
{
	return inten;
}
