#pragma once
#include "la_math.h"
#include "utils.h"

class rp_solver
{
private:
	la_matrix<float> Rt, Rt_inv, tempR;

public:
	rp_solver();
	void init(const int dim, const int reduced_dim);

	void process(la_matrix<float> &U, la_matrix<float> &Vt, la_vector<float> &avg_row,
		la_matrix<float> &C, 
		const float *single_ch_row_scalar, const float *inv_single_ch_row_scalar,
		const float percent);
};
