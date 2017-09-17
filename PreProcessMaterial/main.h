#pragma once
#include "utils.h"
#include "BRDF.h"
#include "parab_map.h"
#include "math_rtn.h"
#define SPHERICAL_AREA_SAMPLES	16000000
#define SVD_QUALITY				0.99


void SVD(la_matrix<double> &result_U, la_matrix<double> &result_Vt,
	la_matrix<double> &C, const double percent, 
	la_vector<double> *output_lamda = NULL,
	const bool multiply_lambda_Vt = true);
void SVD_eigen_num(la_matrix<double> &result_U, la_matrix<double> &result_Vt,
	la_matrix<double> &C, const int num_eigen,
	la_vector<double> *output_lamda = NULL,
	const bool multiply_lambda_Vt = true);

void gen_random_matrix(la_matrix<double> &M, 
	const int r, const int c, const double &scalar);
void gen_random_element(double &result,
		random &rng);

double test_n_sampling_rate(const int dim_fr_n,
	const int samples_per_texel_fr_n,
	const int dim_fr_brdf,
	const int samples_per_texel_fr_brdf,
	const BRDF_interface *brdf,
	const Vector3 &Mc);
double test_brdf_sampling_rate(const int dim_fr_n,
	const int samples_per_texel_fr_n,
	const int dim_fr_brdf,
	const int samples_per_texel_fr_brdf,
	const BRDF_interface *brdf,
	const Vector3 &Mc);
void compute_sampling_rate(int &dim_fr_n,
	int &dim_fr_brdf,
	const int dim_n_lo,
	const int dim_n_up,
	const double snr_n,
	const int dim_brdf_lo,
	const int dim_brdf_up,
	const double snr_brdf,
	const BRDF_interface *brdf,
	const Vector3 &Mc);

void compute_color_matrix(Vector3 &Mc,
	const int dim_fr_n,
	const int samples_per_texel_fr_n,
	const int dim_fr_brdf,
	const int samples_per_texel_fr_brdf,
	const BRDF_interface *brdf);

//void preconvolve_material(
//	const char *filename,
//	const int dim_fr_n,
//	const int samples_per_texel_fr_n,
//	const int dim_fr_brdf,
//	const int samples_per_texel_fr_brdf,
//	const BRDF_interface *brdf);

float compute_avg(const BRDF_interface *brdf,
	const parab_frame &fr_brdf,
	const Vector3 &Mc);

void preconv_matr_rp_brdf(
	const char *filename,
	const int dim_fr_n,
	const int samples_per_texel_fr_n,
	const int dim_fr_brdf,
	const int samples_per_texel_fr_brdf,
	const int area_samples_fr_brdf,
	const BRDF_interface *brdf,
	const int reduced_dim,
	const double eigen_percent,
	const Vector3 &Mc);

void compute_nonzero_elements(const int dim_fr_n,
	const int samples_per_texel_fr_n,
	const int dim_fr_brdf,
	const int samples_per_texel_fr_brdf,
	const int area_samples_fr_brdf,
	const BRDF_interface *brdf);

void compute_SVD_scalar(
	const char *filename,
	const char *input_filename,
	const int dim_vis);

void resample_brdf(const char* filename,
	const BRDF_interface *brdf,
	const int dim_fr_brdf,
	const int samples_per_texel_fr_brdf,
	const int area_samples_fr_brdf);
	