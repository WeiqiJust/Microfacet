#pragma once

extern void sse_saxpy(float *y, const int n, const float alpha, const float *x);
extern void sse_sscal(float *y, const int n, const float alpha);
extern void sse_sadd(float *z, const int n, const float *x, const float *y);
extern void sse_ssub(float *z, const int n, const float *x, const float *y);
extern void sse_sdiv(float *z, const int n, const float *x, const float *y);
extern void sse_sdiv_fast(float *z, const int n, const float *x, const float *y);
extern float sse_sdot(const int n, const float *x, const float *y);

extern void sse_selfsqrt(float *z, const int n);
extern void sse_selfsub(float *z, const int n, const float *x);
extern void sse_selfadd(float *z, const int n, const float *x);
extern void sse_selfmul(float *z, const int n, const float *x);