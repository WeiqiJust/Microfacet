#pragma once
//#define LOG_MATERIAL

//#define NUM_PREVIS_DIR		16
//#define NUM_VIS_DIR			64
#include <vector>
#include <queue>
#include <iostream>
#include <random>
#include <limits>
#include <numeric>
#include <ctime>
#include <memory>
#include <chrono>
#include <DirectXMath.h>
#include <windows.h>
#include <D3Dcommon.h>
#include <stdio.h>
#include "mathutils.h"

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_DELETE(p) { if(p) { delete p; (p)=NULL; } }
#define SAFE_DELETE_ARR(p) {if (p) {delete []p; (p)=NULL;}}
#define FOPEN(fp, name, mode)	{ fopen_s(&fp, name, mode); if (fp == NULL) cout << "failed to open file " << name << endl; }

#define NUM_VIS_PER_BATCH		8
#define NUM_AREA_DIR			64
#define NUM_AREA_DIR_PER_BATCH	4

#define DIM_VIS					10
#define DIM_AREA				16
#define DEFAULT_AREA_SAMPLES	4096

#define DIM_GROUND_TRUTH		256

#define MAX_VIS_SAMPLES			64
#define NUM_BITS_VIS			24
#define NORMAL_DISTR_REDUCED	50

#define M_ID_TEST_PARTICLE		0xF0000001
#define M_ID_TEST_ROD			0xF0000002
#define M_TEST					0xF0000004

#define M_ID_BINDER					0x10000000
#define M_ID_BINDER_PLANE			0x10000001
#define M_ID_BINDER_GROOVE_PLANE	0x10000002
#define M_ID_BINDER_GROOVE_HILL		0x10000003
#define M_ID_BINDER_BRICK_BOTTOM	0x10000004
#define M_ID_BINDER_BRICK_SLOPE		0x10000005
#define M_ID_BINDER_BRICK_TOP		0x10000006
#define M_ID_BINDER_DISPMAP			0x10000007
#define M_ID_BINDER_WOVEN_Y			0x10000008
#define M_ID_BINDER_WOVEN_X			0x10000009
#define M_ID_BINDER_WOVEN_THREAD_Y	0x1000000A
#define M_ID_BINDER_WOVEN_THREAD_X	0x1000000B
#define M_ID_BINDER_REGPOLY			0x1000000C

#define M_ID_DISTR					0x20000000

//Basis Material ID
#define BASIS_MATR_LAMBERTIAN		0x00000001
#define BASIS_MATR_BLUE_RUBBER		0x00000003
#define BASIS_MATR_POLYETHYLENE		0x00000004
#define BASIS_MATR_BEIGE_FABRIC		0x00000005
#define BASIS_MATR_LIGHT_RED_PAINT	0x00000006
#define BASIS_MATR_GOLD_METALLIC	0x00000007
#define BASIS_MATR_GOLD_PAINT		0x00000008
#define BASIS_MATR_SILVER_PAINT		0x00000009
#define BASIS_MATR_PEARL_PAINT		0x0000000A
#define BASIS_MATR_GREEN_METALLIC	0x0000000B
#define BASIS_MATR_WHITE_FABRIC		0x0000000C
#define BASIS_MATR_WHITE_FABRIC2	0x0000000D
#define BASIS_MATR_SILVER_METALLIC	0x0000000E
#define BASIS_MATR_SILVER_METALLIC2	0x0000000F
#define BASIS_MATR_BLUE_METALLIC	0x00000010
#define BASIS_MATR_MAPLE_223		0x00000011
#define BASIS_MATR_WALNUT_224		0x00000012
#define BASIS_MATR_OAK_260			0x00000013
#define BASIS_MATR_BLACK_SOFT_PLASTIC	0x00000014
#define BASIS_MATR_WHITE_DIFFUSE	0x00000015
#define BASIS_MATR_NEOPRENE_RUBBER	0x00000016
#define BASIS_MATR_TEFLON			0x00000017
#define BASIS_MATR_DARK_RED_PAINT	0x00000018

#define BASIS_MATR_PHONG_2			0x00001001
#define BASIS_MATR_PHONG_4			0x00001002
#define BASIS_MATR_PHONG_8			0x00001003
#define BASIS_MATR_PHONG_16			0x00001004
#define BASIS_MATR_PHONG_32			0x00001005

#define BASIS_MATR_CT_0_50_0_25		0x00001011
#define BASIS_MATR_CT_0_45_0_25		0x00001012
#define BASIS_MATR_CT_0_40_0_25		0x00001013
#define BASIS_MATR_CT_0_35_0_25		0x00001014
#define BASIS_MATR_CT_0_30_0_25		0x00001015
#define BASIS_MATR_CT_0_25_0_25		0x00001016
#define BASIS_MATR_CT_0_20_0_25		0x00001017

#define BASIS_MATR_WARD_0_20		0x00001021
#define BASIS_MATR_WARD_0_25		0x00001022
#define BASIS_MATR_WARD_0_30		0x00001023
#define BASIS_MATR_WARD_0_35		0x00001024
#define BASIS_MATR_WARD_0_40		0x00001025
#define BASIS_MATR_WARD_0_45		0x00001026
#define BASIS_MATR_WARD_0_50		0x00001027

//Material ID
#define MATR_LAMBERTIAN_0	1
#define MATR_SPHERE_0		11
#define MATR_ROD_0			21
#define MATR_PLANE_0		31

#define MATR_DISTR_0		1001
#define MATR_DISTR_1		1002
#define MATR_DISTR_2		1003
#define MATR_DISTR_3		1004

#define MATR_BINDER_0		2001
#define MATR_BINDER_1		2002
#define MATR_BINDER_2		2003
#define MATR_BINDER_3		2004


#define M_ID_BASE_OBJ			0x00000001
#define M_ID_BACKGROUND_OBJ		0x00000002
#define M_ID_OBJ_VIS			0x00000003

#define VIS_POINT_SHADOW_DIM	512
#define VIS_POINT_BUFFER_DIM	128

#define SHADOW_MAP_SIZE	512

#define BLOCK_X 16
#define BLOCK_Y 16

#define MaxRGB 256

#define MAX_MIP_LEVEL	16

#define RELATIVE_PRECISION 0.05f

#define RENDER_WIDTH	256
#define RENDER_HEIGHT	256

#define VERT_ATTR_POSITION	0x00000001
#define VERT_ATTR_NORMAL	0x00000010
#define VERT_ATTR_TANGENT	0x00000100
#define VERT_ATTR_UV1		0x00001000


#ifdef LOG_MATERIAL
#define MATR_PATH "T:/Microfacet/data/"
#else
//#define MATR_PATH "d:/temp/Microfacet/material/HQ/"
#define MATR_PATH "T:/Microfacet/data/"
#endif

//#define SHADER_PATH "E:/Code/Research/Microfacet/MicrofacetEditor/"
//#define GEOM_PATH	"D:/temp/Microfacet/"
//#define FRAME_PATH	"D:/temp/Microfacet/temp/"

#define SHADER_PATH "T:/Microfacet/shaders/"
#define GEOM_PATH	"misc/"
#define FRAME_PATH	"T:/Microfacet/frame/"

void matrix_to_XMMATRIX(DirectX::XMMATRIX &m, const Matrix4 &mat);

void XMMATRIX_to_matrix(Matrix4 &m, const DirectX::XMMATRIX &mat);

void CatmullRom(Vector3 &q, const float t, const Vector3 &p0, const Vector3 &p1,
	const Vector3 &p2, const Vector3 &p3);

void projection_orthogonal(Matrix4& mat, float viewWidth, float viewHeight, float znear, float zfar);

void projection_perspective(Matrix4& mat, float FovAngleY, float height, float width, float znear, float zfar);

void build_frame(Vector3 &t, Vector3 &b, const Vector3 &n);

void matrix_lookat(Matrix4 &m, const Vector3 &eye, const Vector3 &lookat, const Vector3 &up);

void uniform_disk_sampling(Vector2 & result, Vector2 rnd, float radius = 1.0f);

void uniform_hemisphere_sample(Vector3& result, float u1, float u2);

void uniform_sphere_sample(Vector3& result, float u1, float u2);

void decompose_file_name(const char *str, char *path, char *filename, char *ext);

void save_image_color(const char *filename, std::vector<float> &img, int w, int h);

void save_image(const char *filename, UINT *data, int w, int h);

HRESULT CompileShaderFromFile(char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

float snorm2float(short a);

inline float add_random_offset(const float v, const float offset)
{
	return (v + offset >= 1.0f) ? v + offset - 1.0f : v + offset;
}

inline int brdf_idx(int wi, int wo)
{
	return (wi <= wo) ? (wo + 1)*wo / 2 + wi : (wi + 1)*wi / 2 + wo;
}


class step_1D_prob
{
public:
	step_1D_prob() {};

	step_1D_prob(const std::vector<float>& samples);

	void sample(int& index, float rnd) const;

private:
	std::vector<float> data;
	std::vector<float>	CDF;
};

/*
class random
{
public:
	random();

	float get_random_float();

	int get_random_int();

	double get_random_double();
private:
	std::mt19937_64 rng;
};*/

class random_number_generator_state
{
public:
	static const int N = 624;

	unsigned long mt[N];	/* the array for the state vector  */
	int mti;				/* mti==N+1 means mt[N] is not initialized */
};

class random
{
private:
	static const int N = 624;
	static const int M = 397;
	static const unsigned long MATRIX_A = 0x9908b0dfUL;	/* constant vector a */
	static const unsigned long UPPER_MASK = 0x80000000UL;	/* most significant w-r bits */
	static const unsigned long LOWER_MASK = 0x7fffffffUL; /* least significant r bits */

	unsigned long mt[N];	/* the array for the state vector  */
	int mti;				/* mti==N+1 means mt[N] is not initialized */

	unsigned long	rand_int()
	{
		unsigned long y;
		static unsigned long mag01[2] = { 0x0UL, MATRIX_A };
		/* mag01[x] = x * MATRIX_A  for x=0,1 */

		if (mti >= N) { /* generate N words at one time */
			int kk;

			if (mti == N + 1)   /* if init_genrand() has not been called, */
				rand_seed(5489UL); /* default initial seed */

			for (kk = 0; kk<N - M; kk++) {
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			for (; kk<N - 1; kk++) {
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
			mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

			mti = 0;
		}

		y = mt[mti++];

		/* Tempering */
		y ^= (y >> 11);
		y ^= (y << 7) & 0x9d2c5680UL;
		y ^= (y << 15) & 0xefc60000UL;
		y ^= (y >> 18);

		return y;
	}

public:
	random(unsigned long seed = 0x0272408)
		: mti(N + 1)
	{
		rand_seed(seed);
	}

	void rand_seed(unsigned long seed)
	{
		mt[0] = seed & 0xffffffffUL;
		for (mti = 1; mti<N; mti++) {
			mt[mti] =
				(1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
			/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
			/* In the previous versions, MSBs of the seed affect   */
			/* only MSBs of the array mt[].                        */
			/* 2002/01/09 modified by Makoto Matsumoto             */
			mt[mti] &= 0xffffffffUL;
			/* for >32 bit machines */
		}
	}

	inline float get_random_float_open() // [0, 1)
	{
		return (float)rand_int() / (float)4294967296.0;
	}

	inline float get_random_float() // [0, 1]
	{
		return (float)rand_int() / (float)4294967295.0;
	}

	//for debug purpose only
	void load_state(const random_number_generator_state &state)
	{
		mti = state.mti;
		memcpy(mt, state.mt, sizeof(mt));
	}

	void save_state(random_number_generator_state &state)
	{
		state.mti = mti;
		memcpy(state.mt, mt, sizeof(mt));
	}
};

class normal_weight
{
public:
	Vector3		normal;
	float		weight;
};

class sample_group
{
public:
	int		mat_id;
	std::vector<Vector3> p;
	std::vector<normal_weight> n;
};

class Ray
{
public:
	Ray() { }
	Ray(Vector3 o, Vector3 d)
	{
		origin = o;
		direction = d;
		inv_direction = Vector3(1 / d.x, 1 / d.y, 1 / d.z);
		sign[0] = (inv_direction.x < 0.0f);
		sign[1] = (inv_direction.y < 0.0f);
		sign[2] = (inv_direction.z < 0.0f);
	}
	Ray(const Ray &r) 
	{
		origin = r.origin;
		direction = r.direction;
		inv_direction = r.inv_direction;
		sign[0] = r.sign[0]; sign[1] = r.sign[1]; sign[2] = r.sign[2];
	}

	Vector3 origin;
	Vector3 direction;
	Vector3 inv_direction;
	int sign[3];
};

class rt_ray : public Ray
{
public:
	bool		has_ray_differential;
	Ray			dr_dx, dr_dy;
	float		ray_length;

	//DEBUG
	//bool		b_hack;

	rt_ray(bool b_ray_diff = false) : ray_length(0), has_ray_differential(b_ray_diff)/*, b_hack(false)*/ {}
	rt_ray(Vector3 origin, Vector3 dir, bool b_ray_diff = false) : Ray(origin, dir),
		ray_length(0), has_ray_differential(b_ray_diff) {}
};

class shader_input
{
public:
	shader_input() {};
	virtual ~shader_input() {};
};

class geometry_differential
{
public:
	Vector3 p, normal;
	Vector2 uv;
	int face_index;
};