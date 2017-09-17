#pragma once
#include "mkl.h"
#include "utils.h"
typedef float number;

const number EPSILON_EIGEN	= (number)1e-3;

template <class number>
class la_matrix
{
public:
	int			row, column;
	number		*m;

	la_matrix() : row(0), column(0), m(NULL) {}
	la_matrix(int r, int c) : row(0), column(0), m(NULL)
	{ 
		init(r, c);
	}
	~la_matrix() { release(); }
	
	void init(int r, int c)
	{
		if (row != r || column != c)
		{
			release();
			row = r;
			column = c;
			m = new number[row*column];
		}
	}
	
	void release()
	{
		row = 0;
		column = 0;
		SAFE_DELETE(m);
	}

	void clear()
	{
		memset(m, 0, sizeof(number)*row*column);
	}

	void save(FILE *fp)
	{
		fwrite(&row, sizeof(int), 1, fp);
		fwrite(&column, sizeof(int), 1, fp);
		fwrite(m, sizeof(number)*row*column, 1, fp);
	}

	void load(FILE *fp)
	{
		SAFE_DELETE_ARR(m);
		//SAFE_DELETE(m);
		fread(&row, sizeof(int), 1, fp);
		fread(&column, sizeof(int), 1, fp);
		m = new number[row*column];
		fread(m, sizeof(number)*row*column, 1, fp);
	}

	//DEBUG
	void save_txt(const char *filename)
	{
		FILE *fp;
		FOPEN(fp, filename, "wt");
		for (int y = 0; y < row; y++)
		{
			for (int x = 0; x < column; x++)
				fprintf_s(fp, "%g ", m[y+row*x]);
			fprintf_s(fp, "\n");
		}
		fclose(fp);
	}
};

template <class number>
class la_vector
{
public:
	int			length;
	number		*v;

	la_vector() : length(0), v(NULL) {}
	la_vector(int l) : length(0), v(NULL)
	{ 
		init(l);
	}
	~la_vector() { release(); }
	void release()
	{
		length = 0;
		SAFE_DELETE_ARR(v);
		//SAFE_DELETE(v);
	}

	void init(int l)
	{
		if (length != l)
		{
			release();
			length = l;
			v = new number[length];
		}
	}

	void clear()
	{
		memset(v, 0, sizeof(number)*length);
	}

	void save(FILE *fp)
	{
		fwrite(&length, sizeof(int), 1, fp);
		fwrite(v, sizeof(number)*length, 1, fp);
	}

	void load(FILE *fp)
	{
		SAFE_DELETE_ARR(v);
		//SAFE_DELETE(v);
		fread(&length, sizeof(int), 1, fp);
		v = new number[length];
		fread(v, sizeof(number)*length, 1, fp);
	}
};

//inline functions
inline void my_gesdd(char *jobz,int *m,int *n,
					 number *a,int *lda,number *s,number *u,int *ldu,number *vt,int *ldvt,
					 number *work,int *lwork,int *iwork,int *info)
{
	sgesdd(jobz, m, n, a, lda, s, u, ldu, vt, ldvt,
		work, lwork, iwork, info);
}

inline void my_scal(int *n,number *a,number *x,int *incx)
{
	sscal(n, a, x, incx);
}

inline number my_nrm2(int *n,number *x,int *incx)
{
	return snrm2(n, x, incx);
}

inline void my_axpy(int *n,number *alpha,number *x,int *incx,number *y,int *incy)
{
	saxpy(n, alpha, x, incx, y, incy);
}

inline void my_gemm(char *transa,char *transb,int *m,int *n,int *k,
					number *alpha,number *a,int *lda,number *b,int *ldb,number *beta,number *c,int *ldc)
{
	sgemm(transa, transb, m, n, k,
		alpha, a, lda, b, ldb, beta, c, ldc);
}

inline void matrix_mul(la_matrix<float> &C, 
					   const la_matrix<float> &A, 
					   const la_matrix<float> &B)
{
	C.row = A.row;
	C.column = B.column;
	C.m = new float[C.row*C.column];

	float alpha = 1.0f, beta = 0.0f;
	sgemm("N", "N", &A.row, &B.column, &A.column, 
		&alpha, A.m, &A.row, B.m, &B.row, &beta, C.m, &C.row);
}

inline void matrix_mul(la_matrix<double> &C, 
					   const la_matrix<double> &A, 
					   const la_matrix<double> &B)
{
	C.row = A.row;
	C.column = B.column;
	C.m = new double[C.row*C.column];

	double alpha = 1.0, beta = 0.0;
	dgemm("N", "N", &A.row, &B.column, &A.column, 
		&alpha, A.m, &A.row, B.m, &B.row, &beta, C.m, &C.row);
}

inline void matrix_vector_mul(la_vector<float> &C, 
							  const la_matrix<float> &A, 
							  const la_vector<float> &B)
{
	C.length = A.row;
	C.v = new float[C.length];

	float alpha = 1.0f, beta = 0.0f;
	int one = 1;
	sgemm("N", "N", &A.row, &one, &A.column, 
		&alpha, A.m, &A.row, B.v, &B.length, &beta, C.v, &C.length);
}

inline void vector_matrix_mul(la_vector<float> &C, 
							  const la_vector<float> &A,
							  const la_matrix<float> &B)
{
	C.length = B.column;
	C.v = new float[C.length];

	float alpha = 1.0f, beta = 0.0f;
	int one = 1;
	sgemm("N", "N", &one, &B.column, &B.row, 
		&alpha, A.v, &one, B.m, &B.row, &beta, C.v, &one);
}

inline float vector_dot(const la_vector<float> &a, const la_vector<float> &b)
{
	int inc_one = 1;
	return sdot(&a.length, a.v, &inc_one, b.v, &inc_one);
}

inline void vector_add(la_vector<float> &y, const la_vector<float> &x, const float alpha)
{
	int		inc_one = 1;
	saxpy(&y.length, &alpha, x.v, &inc_one, y.v, &inc_one);
}