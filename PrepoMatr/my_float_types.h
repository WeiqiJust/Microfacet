#pragma once
#include "codex_math_utils.h"
#include "codex_math_vector2.h"
#include "codex_math_vector3.h"
#include "codex_math_vector4.h"
#include "codex_math_prob.h"
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
#include "mkl.h"

#define PI 3.1415926
#define DEFAULT_AREA_SAMPLES	4096

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_DELETE(p) { if(p) { delete p; (p)=NULL; } }
#define SAFE_DELETE_ARR(p) {if (p) {delete []p; (p)=NULL;}}
#define FOPEN(fp, name, mode)	{ fopen_s(&fp, name, mode); if (fp == NULL) cout << "failed to open file " << name << endl; }


typedef codex::math::vector::vector2<float>		vector2f;
typedef codex::math::vector::vector3<float>		vector3f;
typedef codex::math::vector::vector4<float>		vector4f;
//typedef codex::math::matrix::Matrix4<float>		Matrix4f;

inline int brdf_idx(int wi, int wo)
{
	return (wi <= wo) ? (wo + 1)*wo / 2 + wi : (wi + 1)*wi / 2 + wo;
}

inline double double2db(double a)
{
	return 10.0*log10(a);
}
