#include "rp_solver.h"
#include "math_rtn.h"
rp_solver::rp_solver()
{

}

void rp_solver::init(const int dim, const int reduced_dim)
{
	gen_random_matrix(Rt, reduced_dim, dim, sqrt(1.0f/(float)reduced_dim));
	inverse_matrix(Rt_inv, Rt, 0.0001f);
	tempR.init(Rt.row, Rt.column);
}

void rp_solver::process(la_matrix<float> &U, la_matrix<float> &Vt, la_vector<float> &avg_row,
						la_matrix<float> &C, 
						const float *single_ch_row_scalar,
						const float *inv_single_ch_row_scalar,
						const float percent)
{
	int inc_one = 1, inc_zero = 0;
	float one = 1.0f, zero = 0.0f, alpha;
	//prepare tempR
	memcpy(tempR.m, Rt.m, sizeof(float)*Rt.row*Rt.column);
	int len = tempR.row*3;
	for (int i = 0; i < tempR.column/3; i++)
		sscal(&len, &single_ch_row_scalar[i], &tempR.m[i*len], &inc_one);

	//RtC = Rt * C
	la_matrix<float> RtC(Rt.row, C.column);
	sgemm("N", "N", &Rt.row, &C.column, &C.row,
		&one, tempR.m, &tempR.row, C.m, &C.row, 
		&zero, RtC.m, &RtC.row);
	//C.save_txt("T:/Microfacet/output/C.txt");
	//Rt.save_txt("T:/Microfacet/output/Rt.txt");
	//RtC.save_txt("T:/Microfacet/output/RtC.txt");
	//compute avg_row
	la_vector<float> alpha_vec(C.row);
	avg_row.init(C.column);
	for (int i = 0; i < alpha_vec.length; i++)
		alpha_vec.v[i] = single_ch_row_scalar[i/3];
	alpha = 1.0f / C.row;
	sgemm("N", "N", &inc_one, &C.column, &C.row,
		&alpha, alpha_vec.v, &inc_one, C.m, &C.row, 
		&zero, avg_row.v, &inc_one);

	//The following code is not working!!!
	////compute avg_row
	//la_matrix<float> S(1, Rt.row);
	//la_vector<float> alpha_vec(Rt.column);
	//alpha = 1.0f / C.row;
	//for (int i = 0; i < alpha_vec.length; i++)
	//	alpha_vec.v[i] = alpha;
	//sgemm("N", "N", &S.row, &S.column, &Rt.column, 
	//	&one, alpha_vec.v, &inc_one, Rt_inv.m, &Rt_inv.row, &zero, S.m, &S.row);

	//avg_row.init(C.column);
	//sgemm("N", "N", &S.row, &C.column, &S.column,
	//	&one, S.m, &S.row, RtC.m, &RtC.row, 
	//	&zero, avg_row.v, &inc_one);

	//center RtC
	la_matrix<float> avg_scalar(Rt.row, 1);
	alpha = 1.0f;
	for (int i = 0; i < alpha_vec.length; i++)
		alpha_vec.v[i] = alpha;	
	sgemm("N", "N", &avg_scalar.row, &avg_scalar.column, &Rt.column,
		&one, Rt.m, &Rt.row, alpha_vec.v, &alpha_vec.length, 
		&zero, avg_scalar.m, &avg_scalar.row);
	//avg_scalar.save_txt("T:/Microfacet/output/avg_scalar.txt");
	for (int i = 0; i < RtC.row; i++)
	{
		alpha = -avg_scalar.m[i];
		saxpy(&RtC.column, &alpha, avg_row.v, &inc_one, &RtC.m[i], &RtC.row);
	}

	//codex::utils::timer t;
	SVD_Vt_L1(Vt, RtC, percent);
	//t.update();
	//printf_s("SVD_L1 %gsecs\n", t.elapsed_time());
	//printf_s("SVD_L1 finished\n");
	U.init(Rt_inv.row, Vt.row);
	
	//U = center(C)*Vt
	sgemm("N", "T", &U.row, &U.column, &C.column, 
		&one, C.m, &C.row, Vt.m, &Vt.row, 
		&zero, U.m, &U.row);
	//U.save_txt("T:/Microfacet/output/U.txt");

	la_matrix <float> avgVt(1, Vt.row);
	sgemm("N", "T", &avgVt.row, &avgVt.column, &C.column, 
		&one, avg_row.v, &inc_one, Vt.m, &Vt.row, 
		&zero, avgVt.m, &avgVt.row);
	//avgVt.save_txt("T:/Microfacet/output/avgVt.txt");
	for (int i = 0; i < U.row; i++)
	{
		alpha = -inv_single_ch_row_scalar[i/3];
		saxpy(&U.column, &alpha, avgVt.m, &avgVt.row, &U.m[i], &U.row);
	}
}
