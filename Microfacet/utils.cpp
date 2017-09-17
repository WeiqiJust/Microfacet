#include "utils.h"
#include "d3dcompiler.h"
#include "Magick++.h"
using namespace DirectX;

void matrix_to_XMMATRIX(DirectX::XMMATRIX &m, const Matrix4 &mat)
{
	for (int i = 0; i < 4; i++)
	{	
		for (int j = 0; j < 4; j++)
			m(i,j) = mat.m[i][j];
	}
}

void XMMATRIX_to_matrix(Matrix4 &m, const XMMATRIX &mat)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			m.m[i][j] = mat(i, j);
	}
}

void CatmullRom(Vector3 &q,
	const float t,
	const Vector3 &p0,
	const Vector3 &p1,
	const Vector3 &p2,
	const Vector3 &p3)
{
	//Q(s) = [(-s3 + 2s2 - s)p0 + (3s3 - 5s2 + 2)p1 + (-3s3 + 4s2 + s)p2 + (s3 - s2)p3] / 2
	float	t2 = t*t,
		t3 = t*t*t;

	q = ((-t3 + 2 * t2 - t)*p0 + (3 * t3 - 5 * t2 + 2)*p1 + (-3 * t3 + 4 * t2 + t)*p2 + (t3 - t2)*p3) / 2;
}

void projection_orthogonal(Matrix4& mat, float viewWidth, float viewHeight, float znear, float zfar)
{
	/*
	float m00 = 1.0f / ax;
	float m11 = 1.0f / ay;
	float m22 = -2.0f / (zfar - znear);
	float m23 = -(zfar + znear) / (zfar - znear);
	mat = Matrix4( m00, 0, 0, 0,
				   0, m11, 0, 0,
				   0, 0, m22, m23,
				   0, 0,  0,  1 );
	*/
	XMMATRIX g_ProjMatrix = XMMatrixOrthographicRH(viewWidth, viewHeight, znear, zfar);
	g_ProjMatrix = XMMatrixTranspose(g_ProjMatrix);
	XMMATRIX_to_matrix(mat, g_ProjMatrix);
}

void projection_perspective(Matrix4& mat, float FovAngleY, float height, float width, float znear, float zfar) //FovAngleY in degree
{
	/*
	float m00 = znear / ax;
	float m11 = znear / ay;
	float m22 = -(zfar + znear) / (zfar - znear);
	float m23 = -2.0f*zfar*znear / (zfar - znear);
	mat = Matrix4(m00, 0, 0, 0,
		0, m11, 0, 0,
		0, 0, m22, m23,
		0, 0, -1, 0);
	*/
	XMMATRIX g_ProjMatrix = XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(FovAngleY), width/height, znear, zfar);
	g_ProjMatrix = XMMatrixTranspose(g_ProjMatrix);
	XMMATRIX_to_matrix(mat, g_ProjMatrix);
}

void build_frame(Vector3 &t, Vector3 &b, const Vector3 &n)
{
	if (n.z != 1 && n.z != -1)
	{
		t.x = t.y = 0;
		t.z = 1;
	}
	else {
		t.x = 1;
		t.y = t.z = 0;
	}
	t = Cross(t, n);
	t = Normalize(t);
	b = Cross(t, n);
}

void matrix_lookat(Matrix4 &m, const Vector3 &eye, const Vector3 &lookat, const Vector3 &up)
{
	/*
	const Vector3	vat = Normalize(eye - lookat);
	const Vector3	vup = Normalize(up - (Dot(up, vat) * vat));
	const Vector3	vright = Cross(vup, vat);

	m = Matrix4(
		vright.x, vright.y, vright.z, Dot(-eye, vright),
		vup.x, vup.y, vup.z, Dot(-eye, vup),
		vat.x, vat.y, vat.z, Dot(-eye, vat),
		0, 0, 0, 1);
	*/
	XMVECTOR eyePosition = XMVectorSet(eye.x, eye.y, eye.z, 1);
	XMVECTOR focusPoint = XMVectorSet(lookat.x, lookat.y, lookat.z, 1);
	XMVECTOR upDirection = XMVectorSet(up.x, up.y, up.z, 0);
	XMMATRIX g_ViewMatrix = XMMatrixLookAtRH(eyePosition, focusPoint, upDirection);
	g_ViewMatrix = XMMatrixTranspose(g_ViewMatrix);
	XMMATRIX_to_matrix(m, g_ViewMatrix);

}

void uniform_disk_sampling(Vector2 & result, Vector2 rnd)
{
	result.x = sqrt(rnd.x)*cos(rnd.y);
	result.y = sqrt(rnd.x)*sin(rnd.y);
}

void uniform_hemisphere_sample(Vector3& result, float u1, float u2)
{
	const float r = sqrt(1.0f - u1 * u1);
	const float phi = 2 * PI * u2;
	result = Vector3(cos(phi) * r, sin(phi) * r, u1);

}

void uniform_sphere_sample(Vector3& result, float u1, float u2)
{
	float theta = 2 * PI*u1;
	float phi = acosf(2 * u2 - 1);
	result = Vector3(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
}

void decompose_file_name(const char *str, char *path, char *filename, char *ext)
{
	int i, j;

	for (i = (int)strlen(str) - 1; i >= 0; i--)
	{
		if (str[i] == '\\' || str[i] == '/')
		{
			strcpy(filename, str + i + 1);
			strncpy(path, str, i + 1);
			path[i + 1] = 0;
			//strcpy(filename, str);
			strcpy(ext, "");
			return;
		}

		if (str[i] == '.')
		{
			strncpy(ext, str + i + 1, strlen(str) - i - 1);
			ext[strlen(str) - i - 1] = 0;

			for (j = i; j >= 0; j--)
				if (str[j] == '\\' || str[j] == '/')
				{
					strcpy(filename, str + j + 1);
					filename[strlen(filename) - strlen(ext) - 1] = 0;
					strncpy(path, str, j + 1);
					path[j + 1] = 0;
					return;
				}

			strcpy(filename, str);
			filename[strlen(filename) - strlen(ext) - 1] = 0;
			strcpy(path, "");
			//strncpy(filename, str, i);
			//filename[i] = 0;

			return;
		}
	}

	strcpy(path, "");
	strcpy(ext, "");
	strcpy(filename, str);
}

void save_image_color(const char *filename, std::vector<float> &img, int w, int h)
{
	Magick::Image im(Magick::Geometry(w, h), Magick::Color(0, 0, 0));

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			im.pixelColor(x, y,
				Magick::Color(
				img[(x + y*w) * 3] * (unsigned short)65535,
				img[(x + y*w) * 3 + 1] * (unsigned short)65535,
				img[(x + y*w) * 3 + 2] * (unsigned short)65535
				));
		}
	im.write(filename);
}

void save_image(const char *filename, UINT *data, int w, int h)
{
	Magick::Image im(Magick::Geometry(w, h), Magick::Color(0, 0, 0));

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			UINT	c = data[x + y*w];
			float	r = ((c >> 16) & 0xFF) / 255.0f,
				g = ((c >> 8) & 0xFF) / 255.0f,
				b = ((c)& 0xFF) / 255.0f;
			//cout << "r = " << r << " g = " << g << " b = " << b << endl;
			//cout << data[x + y*w] << endl;
			im.pixelColor(x, y, Magick::Color(r* (unsigned short)65535, g* (unsigned short)65535, b* (unsigned short)65535));
		}
	im.write(filename);
}

HRESULT CompileShaderFromFile(char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob = NULL;
	//hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		//dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	string file(szFileName);
	std::wstring file_wstring(file.length(), L' ');
	std::copy(file.begin(), file.end(), file_wstring.begin());
	hr = D3DCompileFromFile(file_wstring.c_str(), NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		cout << "Error from D3DCompileFromFile" << endl;
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		}
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();
	//cout << "after CompileShaderFromFile" << endl;
	return S_OK;
}

float snorm2float(short a)
{
	a = (std::max)(a, (short)-0x7FFF);
	return a / float(0x7FFF);
}


step_1D_prob::step_1D_prob(std::vector<float> samples)
{
	float sum_of_elemens = std::accumulate(samples.begin(), samples.end(), 0.0f);
	for (int i = 0; i < samples.size(); i++)
	{
		data.push_back((float)samples[i] / sum_of_elemens);
	}
}

void step_1D_prob::sample(int& index, float rnd) const
{
	float minimum = (std::numeric_limits<float>::max)();
	index = 0;
	for (int i = 0; i < data.size(); i++)
	{
		if (abs(rnd - data[i]) <= minimum)
		{
			minimum = abs(rnd - data[i]);
			index = i;
		}
	}
}

random::random()
{
	srand(static_cast <unsigned> (time(0)));
	std::mt19937_64 rng;
	// initialize the random number generator with time-dependent seed
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
	rng.seed(ss);
	// initialize a uniform distribution between 0 and 1
}

int random::get_random_int()
{
	return rand();
}

float random::get_random_float()
{
	
	std::uniform_real_distribution<float> unif(0, 1);
	// ready to generate random numbers
	return unif(rng);
}

double random::get_random_double()
{
	std::uniform_real_distribution<double> unif(0, 1);
	// ready to generate random numbers
	return unif(rng);
}