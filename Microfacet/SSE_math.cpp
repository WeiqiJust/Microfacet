#include <xmmintrin.h>	// Need this for SSE compiler intrinsics
#include <intrin.h>
#include <cmath>

void sse_selfmul(float *z, const int n, const float *x)
{
	const int sse_length = n/4;
	const float	*px = x;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		*z_sse = _mm_mul_ps(*z_sse, x_sse);

		px += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] *= x[i];
}

void sse_selfadd(float *z, const int n, const float *x)
{
	const int sse_length = n/4;
	const float	*px = x;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		*z_sse = _mm_add_ps(*z_sse, x_sse);

		px += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] += x[i];
}

void sse_selfsub(float *z, const int n, const float *x)
{
	const int sse_length = n/4;
	const float	*px = x;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		*z_sse = _mm_sub_ps(*z_sse, x_sse);

		px += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] -= x[i];
}

void sse_selfsqrt(float *z, const int n)
{
	const int sse_length = n/4;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		*z_sse = _mm_sqrt_ps(*z_sse);
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] = sqrt(z[i]);
}

void sse_sscal(float *y, const int n, const float alpha)
{
	const int sse_length = n/4;

	__m128 *y_sse = (__m128*)y;
	__m128 scalar = _mm_set_ps1(alpha);

	for (int i = 0; i < sse_length; i++)
	{
		*y_sse = _mm_mul_ps(scalar, *y_sse);
		y_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		y[i] *= alpha;
}

void sse_saxpy(float *y, const int n, const float alpha, const float *x)
{
	const int sse_length = n/4;
	const float	*px = x;

	__m128 *y_sse = (__m128*)y;
	__m128 scalar = _mm_set_ps1(alpha);

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		__m128 ax = _mm_mul_ps(scalar, x_sse);
		*y_sse = _mm_add_ps(ax, *y_sse);

		px += 4;
		y_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		y[i] += alpha*x[i];
}

void sse_sdiv(float *z, const int n, const float *x, const float *y)
{
	const int sse_length = n/4;
	const float	*px = x, 
		*py = y;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		__m128 y_sse = _mm_load_ps(py);
		*z_sse = _mm_div_ps(x_sse, y_sse);

		px += 4;
		py += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] = x[i] / y[i];
}

void sse_sadd(float *z, const int n, const float *x, const float *y)
{
	const int sse_length = n/4;
	const float	*px = x, 
		*py = y;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		__m128 y_sse = _mm_load_ps(py);
		*z_sse = _mm_add_ps(x_sse, y_sse);

		px += 4;
		py += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] = x[i]+y[i];
}

void sse_ssub(float *z, const int n, const float *x, const float *y)
{
	const int sse_length = n/4;
	const float	*px = x, 
		*py = y;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		__m128 y_sse = _mm_load_ps(py);
		*z_sse = _mm_sub_ps(x_sse, y_sse);

		px += 4;
		py += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] = x[i]-y[i];
}

void sse_sdiv_fast(float *z, const int n, const float *x, const float *y)
{
	const int sse_length = n/4;
	const float	*px = x, 
		*py = y;

	__m128 *z_sse = (__m128*)z;

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(x);
		__m128 y_sse = _mm_load_ps(y);

		__m128 y_recip = _mm_rcp_ps(y_sse);

		*z_sse = _mm_mul_ps(x_sse, y_recip);

		px += 4;
		py += 4;
		z_sse++;
	}

	for (int i = sse_length*4; i < n; i++)
		z[i] = x[i] / y[i];
}

float sse_sdot(const int n, const float *x, const float *y)
{
	const int sse_length = n/4;
	const float	*px = x, 
				*py = y;
	float z;

	__m128	total_z = _mm_set_ps1(0), 
			z_sse;	

	for (int i = 0; i < sse_length; i++)
	{
		__m128 x_sse = _mm_load_ps(px);
		__m128 y_sse = _mm_load_ps(py);
		z_sse = _mm_mul_ps(x_sse, y_sse);
		z_sse = _mm_hadd_ps(z_sse, z_sse);
		total_z = _mm_add_ps(total_z, z_sse);

		px += 4;
		py += 4;
	}

	total_z = _mm_hadd_ps(total_z, total_z);
	_mm_store_ss(&z, total_z);

	for (int i = sse_length*4; i < n; i++)
		z += x[i]*y[i];

	return z;
}
