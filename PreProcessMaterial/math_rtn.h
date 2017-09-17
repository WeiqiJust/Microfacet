#pragma once

#include "mkl.h"
#include "la_math.h"
#include "utils.h"
void SVD(la_matrix<number> &result_U, la_matrix<number> &result_Vt,
	la_matrix<number> &C, const number percent);
void SVD(la_matrix<number> &result_U, la_matrix<number> &C, const number percent);
void SVD_U_L1(la_matrix<number> &result_U, la_matrix<number> &C, const number percent);
void SVD_Vt_L1(la_matrix<number> &result_Vt, la_matrix<number> &C, const number percent);

void inverse_matrix(la_matrix<number> &A, const la_matrix<number> &org, const number percent);

void gen_random_element(number &result, random &rng);
void gen_random_matrix(la_matrix<number> &M, const int r, const int c, const number &scalar);
