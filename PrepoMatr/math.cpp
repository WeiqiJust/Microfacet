#include "main.h"

#include "codex_math_prob.h"
using codex::math::prob::normal_rng_pair;
using namespace std;

void SVD(la_matrix<double> &result_U, la_matrix<double> &result_Vt,
					 la_matrix<double> &C, const double percent,
					 la_vector<double> *output_lamda,
					 const bool multiply_lambda_Vt)
{
	la_matrix<double>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<double>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	double	optimal_len;
	{
		int		len = -1;
		dgesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<double> work((int)optimal_len);

	dgesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE_ARR(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	if (output_lamda)
	{
		output_lamda->release();
		output_lamda->length = lamda.length;
		output_lamda->v = new double[output_lamda->length];
		memcpy(output_lamda->v, lamda.v, sizeof(double)*output_lamda->length);
	}

	int num_eigen = lamda.length;
	{
		double power = 0, p = 0;
		for (int i = 0; i < lamda.length; i++)
		{
			power += lamda.v[i]*lamda.v[i];
		}

		for (int i = 0; i < lamda.length; i++)
		{
			p += lamda.v[i]*lamda.v[i];
			if (p > percent*power)
			{
				num_eigen = i+1;
				break;
			}
		}
	}
	num_eigen = min(num_eigen, lamda.length);
	printf_s("SVD : %d / %d\n", num_eigen, lamda.length);

	////DEBUG
	//{
	//	for (int i = 0; i < num_eigen; i++)
	//		printf_s("%d : %g\n", i, ddot(&Vt.column, &Vt.m[i], &Vt.row, Vt.m, &Vt.row));
	//}
	//{
	//	FILE *fp;
	//	FOPEN(fp, "d:/temp/Microfacet/SVDVt.txt", "wt");
	//	for (int y = 0; y < Vt.row; y++)
	//	{
	//		for (int x = 0; x < Vt.column; x++)
	//			fprintf_s(fp, "%g ", Vt.m[y+Vt.row*x]);
	//		fprintf_s(fp, "\n");
	//	}
	//	fclose(fp);
	//}
	
	if (multiply_lambda_Vt)
	{
		int inc = Vt.row;
		for (int i = 0; i < num_eigen; i++)
			dscal(&Vt.column, &lamda.v[i], &Vt.m[i], &inc);
	}

	result_U.row	= C.row;
	result_U.column	= num_eigen;
	result_U.m = new double[result_U.row*result_U.column];
	memcpy(result_U.m, U.m, sizeof(double)*result_U.row*result_U.column);

	result_Vt.row	= num_eigen;
	result_Vt.column= C.column;
	result_Vt.m = new double[result_Vt.row*result_Vt.column];
	for (int i = 0; i < result_Vt.column; i++)
		memcpy(&result_Vt.m[i*result_Vt.row], &Vt.m[i*Vt.row], sizeof(double)*num_eigen);
}

void SVD_eigen_num(la_matrix<double> &result_U, la_matrix<double> &result_Vt,
					 la_matrix<double> &C, const int num_eigen,
					 la_vector<double> *output_lamda,
					 const bool multiply_lambda_Vt)
{
	la_matrix<double>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<double>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	double	optimal_len;
	{
		int		len = -1;
		dgesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<double> work((int)optimal_len);

	dgesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE_ARR(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	if (output_lamda)
	{
		output_lamda->release();
		output_lamda->length = lamda.length;
		output_lamda->v = new double[output_lamda->length];
		memcpy(output_lamda->v, lamda.v, sizeof(double)*output_lamda->length);
	}

	{
		double power = 0, p = 0;
		for (int i = 0; i < lamda.length; i++)
		{
			power += lamda.v[i]*lamda.v[i];
		}

		for (int i = 0; i < num_eigen; i++)
		{
			p += lamda.v[i]*lamda.v[i];
		}
		printf_s("SVD : %g%%\n", p*100/power);
	}

	if (multiply_lambda_Vt)
	{
		int inc = Vt.row;
		for (int i = 0; i < num_eigen; i++)
			dscal(&Vt.column, &lamda.v[i], &Vt.m[i], &inc);
	}

	result_U.row	= C.row;
	result_U.column	= num_eigen;
	result_U.m = new double[result_U.row*result_U.column];
	memcpy(result_U.m, U.m, sizeof(double)*result_U.row*result_U.column);

	result_Vt.row	= num_eigen;
	result_Vt.column= C.column;
	result_Vt.m = new double[result_Vt.row*result_Vt.column];
	for (int i = 0; i < result_Vt.column; i++)
		memcpy(&result_Vt.m[i*result_Vt.row], &Vt.m[i*Vt.row], sizeof(double)*num_eigen);
}

void gen_random_matrix(la_matrix<double> &M, const int r, const int c, const double &scalar)
{
	M.row	= r;
	M.column= c;
	M.m = new double[r*c];

	random_number_generator<double> rng;
	for (int i = 0; i < r*c; i++)
	{
		double x, y;
		normal_rng_pair(x, y, rng);
		M.m[i] = x*scalar;
	}
}

void gen_random_element(double &result,
									random_number_generator<double> &rng)
{
	double y;
	normal_rng_pair(result, y, rng);
}
