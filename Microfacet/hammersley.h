#pragma once
# include <cmath>
# include <cstdlib>
# include <cstring>
# include <ctime>
# include <iomanip>
# include <iostream>
#include <random>
#include <string>

using namespace std;
class hammersley
{
public:
	hammersley(int base_value, int dimention_value);
	float* get_sample();
	int hammersley_inverse(float r[], int m, int n);
	float *hammersley_sequence(int i1, int i2, int m, int n);

private:
	int i4vec_sum(int n, int a[]);
	int prime(int n);
	void r8mat_print(int m, int n, float a[], string title);
	void r8mat_print_some(int m, int n, float a[], int ilo, int jlo, int ihi,
		int jhi, string title);
	void timestamp();

	int iter;
	int base;
	int dimention;

};