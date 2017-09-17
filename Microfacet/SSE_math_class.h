#pragma once

#include "SSE_math.h"

#define SAFE_ALIGNED_FREE(a) { if (a) { _aligned_free(a); a = NULL; }}

//Column-major
class SSE_matrix
{
public:
	int			row, row_aligned, column;
	float		*m;

	SSE_matrix() : row(0), column(0), row_aligned(0), m(NULL) {}
	SSE_matrix(int r, int c)
	: row(0), column(0), row_aligned(0), m(NULL)
	{ 
		init(r, c);
	}
	~SSE_matrix() { release(); }

	void init(int r, int c)
	{
		if (r != row || c != column)
		{
			SAFE_ALIGNED_FREE(m);

			row				= r;
			column			= c;
			row_aligned		= ((row+3)/4)*4;

			m = (float*)_aligned_malloc(sizeof(float)*row_aligned*column, 16);
		}
	}

	void release()
	{
		row = column = 0;
		row_aligned = 0;
		SAFE_ALIGNED_FREE(m);
	}

	void clear()
	{
		memset(m, 0, sizeof(float)*row_aligned*column);
	}

	void from_unaligned(int r, int c, const float *x, 
						const float padded_value = 0.0f,
						const bool b_transpose = false)
	{
		if (!b_transpose)
		{
			init(r, c);
			for (int i = 0; i < column; i++)
			{
				memcpy(&m[i*row_aligned], &x[i*row], sizeof(float)*row);
				for (int j = row; j < row_aligned; j++)
					m[i*row_aligned+j] = padded_value;
			}
		} else {
			init(c, r);
			for (int i = 0; i < column; i++)
			{
				for (int j = 0; j < row; j++)
					m[i*row_aligned+j] = x[j*column+i];
				for (int j = row; j < row_aligned; j++)
					m[i*row_aligned+j] = padded_value;
			}
		}
	}
};

class SSE_vector
{
public:
	int			length, length_aligned;
	float		*v;

	SSE_vector() : length(0), length_aligned(0), v(NULL) {}
	SSE_vector(int l)
	: length(0), length_aligned(0), v(NULL)
	{ 
		init(l);
	}
	~SSE_vector() { release(); }

	void init(int l)
	{
		if (l != length)
		{
			SAFE_ALIGNED_FREE(v);

			length			= l;
			length_aligned	= ((length+3)/4)*4;

			v = (float*)_aligned_malloc(sizeof(float)*length_aligned, 16); 
		}
	}

	void release()
	{
		length = length_aligned = 0;
		SAFE_ALIGNED_FREE(v);
	}

	void clear()
	{
		memset(v, 0, sizeof(float)*length_aligned);
	}

	void from_unaligned(int l, const float *x, const float padded_value = 0.0f)
	{
		init(l);
		memcpy(v, x, sizeof(float)*length);
		for (int i = length; i < length_aligned; i++)
			v[i] = padded_value;
	}
};

extern void SSE_vector_matrix_mul(SSE_vector &C, const SSE_vector &A, const SSE_matrix &B);
extern void SSE_matrix_mul(SSE_matrix &C, const SSE_matrix &A, const SSE_matrix &B);
