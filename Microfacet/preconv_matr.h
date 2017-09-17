#pragma once
#include "mkl.h"
#include "la_math.h"
#include "utils.h"
#include "BRDF.h"
#include "parab_map.h"

#define NUM_MATR_ATTR	1
#define MATR_ATTR_SPEC	0

const char matr_attr_name[][64] = {
	"specularity",
};

class matr_attr
{
public:
	float attr[NUM_MATR_ATTR];
};


class BRDF_sampler
{
private:
	std::vector<step_1D_prob> cdf;

public:
	void init(const BRDF_interface* pBRDF);
	void sample_dir(Vector3 &wi, float &pdf, 
		const Vector3 &wo, random &rng);
};

class preconv_matr
{
public:
	BRDF_interface		*pBRDF;
	BRDF_sampler		*p_sampler;
	matr_attr			attr;

	double_parab_frame	fr_n;
	parab_frame			fr_brdf;

	//SSE_matrix			Umt_SSE;
	la_matrix<float>	Umt, Vm;
	la_vector<float>	avgm, inv_n_area;
#ifdef USE_SVD_SCALAR
	la_vector<float>	sUn, sVnt, inv_sUn, inv_sVnt;
#endif
	Vector3			Mc;
	float				inv_avg;		

	//for random projection
	la_matrix<float>	R;

	virtual ~preconv_matr();

	void load(const char *filename);
	void init_R(const int dim_reduced);

	int compute_idx_brdf(const Vector3 &wi, const Vector3 &wo) const;
	void lerp_brdf(int &num, 
		int *idx, float *weight,
		const Vector3 &wi, const Vector3 &wo) const;

	void SVD_rp(la_matrix<number> &U, la_matrix<number> &Vt, 
		la_matrix<number> &C, la_matrix<number> &CR, const number percent);
};

class matr
{
public:
	int			basis_id;
	Vector3	albedo;
};