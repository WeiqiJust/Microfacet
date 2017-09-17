#include <Windows.h>
#include <cmath>
#include "SSE_math_class.h"

void SSE_vector_matrix_mul(SSE_vector &C, const SSE_vector &A, const SSE_matrix &B)
{
	C.init(B.column);
	for (int i = 0; i < C.length; i++)
		C.v[i] = sse_sdot(A.length, A.v, &B.m[i*B.row_aligned]);
}

void SSE_matrix_mul(SSE_matrix &C, const SSE_matrix &A, const SSE_matrix &B)
{
	C.init(A.column, B.column);
	for (int x = 0; x < C.column; x++)
		for (int y = 0; y < C.row; y++)
			C.m[y+x*C.row_aligned] = sse_sdot(A.row, &A.m[y*A.row_aligned], &B.m[x*B.row_aligned]);
}