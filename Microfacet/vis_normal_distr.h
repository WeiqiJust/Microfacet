#pragma once
#include "utils.h"
#include "la_math.h"
#include "SSE_math_class.h"
#include "parab_map.h"

class vis_normal_distr
{
public:
	la_vector<float>	BRDF;

	int basis_id;

	//Result: Failed. Stick to the original method.
	////direct handling of mat_vis_n
	////if it turns out to be better, there is no need to use SVD scalar at all!!!
	////then we should delete all USE_SVD_SCALAR
	//la_matrix<float>	MnVm;
	//la_vector<float>	Mnavgm_scalar;
	//SSE_matrix			MnVm_SSE;
	//SSE_vector			Mnavgm_scalar_SSE;


	SSE_matrix			Un_SSE, VntVmUmt_SSE;//VntVm_SSE;
	SSE_vector			avgnVmUmt_SSE, //avgnVm_SSE, 
		Vntavgm_scalar_SSE;

	la_matrix<float>	Un, Vnt;
	la_vector<float>	avgn;

	la_matrix<float>	VntVmUmt;//VntVm;
	la_vector<float>	avgnVmUmt,//avgnVm, 
		Vntavgm_scalar;
	float				sum_avgn;
#ifdef USE_SVD_SCALAR
	la_vector<float>	inv_sUn;
#endif
	//DEBUG
	std::vector<float>	double_vis_matr;
	std::vector<normal_weight>
		pts;
	std::vector<Vector3>
		pos;

	void compute_BRDF(const parab_frame &fr_vis,
		const float* inv_SVD_scalar);

	void release();
};
