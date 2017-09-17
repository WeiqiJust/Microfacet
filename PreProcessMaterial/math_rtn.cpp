#include "math_rtn.h"
#include "mkl.h"
#include "la_math.h"


void SVD(la_matrix<number> &result_U, la_matrix<number> &result_Vt,
					 la_matrix<number> &C, const number percent)
{
	la_matrix<number>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<number>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	number	optimal_len;
	{
		int		len = -1;
		my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<number> work((int)optimal_len);

	my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	int num_eigen = lamda.length;
	{
		number power = 0, p = 0;
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

	for (int i = 0; i < num_eigen; i++)
		my_scal(&Vt.column, &lamda.v[i], &Vt.m[i], &Vt.row);

	result_U.release();
	result_U.row	= C.row;
	result_U.column	= num_eigen;
	result_U.m = new number[result_U.row*result_U.column];
	memcpy(result_U.m, U.m, sizeof(number)*result_U.row*result_U.column);

	result_Vt.release();
	result_Vt.row	= num_eigen;
	result_Vt.column= C.column;
	result_Vt.m = new number[result_Vt.row*result_Vt.column];
	for (int i = 0; i < result_Vt.column; i++)
		memcpy(&result_Vt.m[i*result_Vt.row], &Vt.m[i*Vt.row], sizeof(number)*num_eigen);
}

void inverse_matrix(la_matrix<number> &A,
								const la_matrix<number> &org, const number percent)
{
	la_matrix<number>	C(org.row, org.column);
	memcpy(C.m, org.m, sizeof(number)*org.row*org.column);

	la_matrix<number>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<number>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	number	optimal_len;
	{
		int		len = -1;
		my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<number> work((int)optimal_len);

	my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	int num_eigen = 0;
	for (int i = 0; i < lamda.length; i++)
	{
		if (lamda.v[i] >= lamda.v[0]*percent)
			lamda.v[i] = 1 / lamda.v[i];
		else 
			break;
		my_scal(&Vt.column, &lamda.v[i], &Vt.m[i], &Vt.row);
		num_eigen++;
	}

	if (A.row != org.column || A.column != org.row)
	{
		A.release();
		A.row	= org.column;
		A.column= org.row;
		A.m = new number[A.row*A.column];
	}

	float alpha = 1.0f, beta = 0.0f;
	sgemm("T", "T", &A.row, &A.column, &num_eigen, 
		&alpha, Vt.m, &Vt.row, U.m, &U.row,
		&beta, A.m, &A.row);
}

void SVD_U_L1(la_matrix<number> &result_U,
					 la_matrix<number> &C, const number percent)
{
	la_matrix<number>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<number>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	number	optimal_len;
	{
		int		len = -1;
		my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<number> work((int)optimal_len);

	my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	int num_eigen = lamda.length;
	{
		number power = 0, p = 0;
		for (int i = 0; i < lamda.length; i++)
		{
			power += lamda.v[i];//*lamda.v[i];
		}

		for (int i = 0; i < lamda.length; i++)
		{
			p += lamda.v[i];//*lamda.v[i];
			if (p > percent*power)
			{
				num_eigen = i+1;
				break;
			}
		}
	}
	num_eigen = min(num_eigen, lamda.length);
	printf_s("SVD : %d / %d\n", num_eigen, lamda.length);

	result_U.release();
	result_U.row	= C.row;
	result_U.column	= num_eigen;
	result_U.m = new number[result_U.row*result_U.column];
	memcpy(result_U.m, U.m, sizeof(number)*result_U.row*result_U.column);
}

void SVD_Vt_L1(la_matrix<number> &result_Vt,
						la_matrix<number> &C, const number percent)
{
	la_matrix<number>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<number>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	number	optimal_len;
	{
		int		len = -1;
		my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<number> work((int)optimal_len);

	my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	int num_eigen = lamda.length;
	{
		number power = 0, p = 0;
		for (int i = 0; i < lamda.length; i++)
		{
			power += lamda.v[i];//*lamda.v[i];
		}

		for (int i = 0; i < lamda.length; i++)
		{
			p += lamda.v[i];//*lamda.v[i];
			if (p > percent*power)
			{
				num_eigen = i+1;
				break;
			}
		}
	}
	num_eigen = min(num_eigen, lamda.length);
	//num_eigen = 4;
	printf_s("SVD : %d / %d\n", num_eigen, lamda.length);

	//////////////////
	//for (int i = 0; i < lamda.length; i++)
	//	printf_s("%g\n", lamda.v[i]);
	//////////////////

	result_Vt.release();
	result_Vt.row	= num_eigen;
	result_Vt.column= C.column;
	result_Vt.m = new number[result_Vt.row*result_Vt.column];
	for (int i = 0; i < result_Vt.column; i++)
		memcpy(&result_Vt.m[i*result_Vt.row], &Vt.m[i*Vt.row], sizeof(number)*num_eigen);
}

void SVD(la_matrix<number> &result_U,
					 la_matrix<number> &C, const number percent)
{
	la_matrix<number>	U(C.row, min(C.row, C.column)), Vt(min(C.row, C.column), C.column);
	la_vector<number>	lamda(min(C.row, C.column));
	int				info;
	int				*iwork = new int[8*min(C.row, C.column)];

	number	optimal_len;
	{
		int		len = -1;
		my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
			U.m, &U.row, Vt.m, &Vt.row, &optimal_len, &len, iwork, &info);
	}
	la_vector<number> work((int)optimal_len);

	my_gesdd("S", &C.row, &C.column, C.m, &C.row, lamda.v, 
		U.m, &U.row, Vt.m, &Vt.row, work.v, &work.length, iwork, &info);
	SAFE_DELETE(iwork);
	if (info != 0)
	{
		printf_s("\nwarning : gesdd failed\n");
		return;
	}

	int num_eigen = lamda.length;
	{
		number power = 0, p = 0;
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

	result_U.release();
	result_U.row	= C.row;
	result_U.column	= num_eigen;
	result_U.m = new number[result_U.row*result_U.column];
	memcpy(result_U.m, U.m, sizeof(number)*result_U.row*result_U.column);
}

void gen_random_element(number &result,
									random &rng)
{
	
	result = rng.get_random_float();
}

void gen_random_matrix(la_matrix<number> &M, const int r, const int c, const number &scalar)
{
	M.release();
	M.row	= r;
	M.column= c;
	M.m = new number[r*c];

	random rng;
	for (int i = 0; i < r*c; i++)
	{
		float x = rng.get_random_float();
		M.m[i] = x*scalar;
	}
}
