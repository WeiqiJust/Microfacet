#include "main.h"


double test_n_sampling_rate(const int dim_fr_n,
										const int samples_per_texel_fr_n,
										const int dim_fr_brdf,
										const int samples_per_texel_fr_brdf,
										const BRDF_interface *brdf,
										const vector3f &Mc)
{
	double_parab_frame fr_n;
	parab_frame fr_brdf;
	const std::vector<float> &area = fr_brdf.get_spherical_area();
	fr_n.init(dim_fr_n, samples_per_texel_fr_n);
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, DEFAULT_AREA_SAMPLES);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);

	int inc_one = 1;
	int num_n = (int)fr_n.get_back_idx().size(),
		num_wi, num_wo;
	num_wi = num_wo = (int)fr_brdf.get_back_idx().size();
	int num_brdf = (num_wo+1)*num_wo/2;

	la_vector<double> temp(num_brdf*samples_per_texel_fr_n), C_col(num_brdf);
	double power_e, power_s;
	power_e = power_s = 0;

	//printf_s("building the matrix...\n");
	for (int i0 = 0; i0 < num_n; i0++)
	{
		//printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		//printf_s("(%d/%d) ", i0+1, num_n);

		double weight = 1.0 / fr_n.get_n()[i0].size();
		int len = num_brdf;
		C_col.clear();
		for (int i1 = 0; i1 < fr_n.get_n()[i0].size(); i1++)
		{
			const vector3f &n = fr_n.get_n()[i0][i1];

			for (int j0 = 0; j0 < num_wo; j0++)
			{
				const vector3f &wo = fr_brdf.get_n()[j0][0];
				for (int k0 = 0; k0 <= j0; k0++)
				{
					vector3f result;
					brdf->sample(result, fr_brdf.get_n()[k0][0], wo, n);
					//result -= avgc;
					temp.v[num_brdf*i1+brdf_idx(k0, j0)] = (result*Mc)*area[k0]*area[j0];
				}
			}
			daxpy(&len, &weight, &temp.v[num_brdf*i1], &inc_one, C_col.v, &inc_one);
		}
		power_s += ddot(&len, C_col.v, &inc_one, C_col.v, &inc_one);

		for (int i1 = 0; i1 < fr_n.get_n()[i0].size(); i1++)
		{
			double weight = -1;
			daxpy(&len, &weight, C_col.v, &inc_one, &temp.v[num_brdf*i1], &inc_one);
			power_e += ddot(&len, &temp.v[num_brdf*i1], &inc_one, &temp.v[num_brdf*i1], &inc_one) / fr_n.get_n()[i0].size();
		}
	}
	//printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	//printf("done.                \n");

	double snr = double2db(power_s/power_e);
	//printf_s("normal : SNR = %gdb\n", snr);
	return snr;
}

double test_brdf_sampling_rate(const int dim_fr_n,
										   const int samples_per_texel_fr_n,
										   const int dim_fr_brdf,
										   const int samples_per_texel_fr_brdf,
										   const BRDF_interface *brdf,
										   const vector3f &Mc)
{
	double_parab_frame fr_n;
	parab_frame fr_brdf;
	const std::vector<float> &area = fr_n.get_spherical_area();
	fr_n.init(dim_fr_n, samples_per_texel_fr_n);//, DEFAULT_AREA_SAMPLES);
	fr_n.normalize_n();
	fr_n.compute_spherical_area(SPHERICAL_AREA_SAMPLES);
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, DEFAULT_AREA_SAMPLES);

	int inc_one = 1, inc_zero = 0;
	int num_n = (int)fr_n.get_back_idx().size(),
		num_wi, num_wo;
	num_wi = num_wo = (int)fr_brdf.get_back_idx().size();

	double power_e, power_s;
	power_e = power_s = 0;

	//printf_s("building the matrix...\n");
	for (int i = 0; i < num_n; i++)
	{
		//printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		//printf_s("(%d/%d) ", i+1, num_n);

		const vector3f &n = fr_n.get_n()[i][0];
		for (int j0 = 0; j0 < num_wo; j0++)
			for (int k0 = 0; k0 <= j0; k0++)
			{
				int sz_wo = fr_brdf.get_n()[j0].size(),
					sz_wi = fr_brdf.get_n()[k0].size();
				
				la_vector<double> temp(sz_wo*sz_wi);
				double avg = 0;

				for (int j1 = 0; j1 < sz_wo; j1++)
				{
					const vector3f &wo = fr_brdf.get_n()[j0][j1];
					for (int k1 = 0; k1 < sz_wi; k1++)
					{
						vector3f result;
						brdf->sample(result, fr_brdf.get_n()[k0][k1], wo, n);
						//result -= avgc;

						float d = (result*Mc)*area[i];
						temp.v[k1+j1*sz_wi] = d;
						avg += d;
					}
				}

				avg /= sz_wi*sz_wo;

				int len = sz_wi*sz_wo;
				double alpha = -1.0;
				daxpy(&len, &alpha, &avg, &inc_zero, temp.v, &inc_one);

				power_s += avg*avg*len;
				power_e += ddot(&len, temp.v, &inc_one, temp.v, &inc_one);
			}
	}
	//printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	//printf("done.                \n");

	double snr = double2db(power_s/power_e);
	//printf_s("BRDF : SNR = %gdb\n", snr);
	return snr;
}

void compute_sampling_rate(int &dim_fr_n,
									   int &dim_fr_brdf,
									   const int dim_n_lo,
									   const int dim_n_up,
									   const double snr_n,
									   const int dim_brdf_lo,
									   const int dim_brdf_up,
									   const double snr_brdf,
									   const BRDF_interface *brdf,
									   const vector3f &Mc)
{
	printf_s("n\n");
	int l, r, m;

	l = dim_n_lo;
	r = dim_n_up;
	while (l < r)
	{
		printf_s("[%d %d] ", l, r);
		m = (l+r) / 2;
		double snr = test_n_sampling_rate(
			m, 16, 4, 8, brdf, Mc);

		if (snr >= snr_n)
		{
			r = m-1;
		} else if (snr < snr_n)
		{
			l = m+1;
		}
	}
	dim_fr_n = m;
	printf_s("%d \n", dim_fr_n);

	printf_s("brdf\n");
	l = dim_brdf_lo;
	r = dim_brdf_up;
	while (l < r)
	{
		printf_s("[%d %d] ", l, r);
		m = (l+r) / 2;
		double snr = test_brdf_sampling_rate(
			4, 16, m, 4, brdf, Mc);

		if (snr >= snr_brdf)
		{
			r = m-1;
		} else if (snr < snr_brdf)
		{
			l = m+1;
		}
	}
	dim_fr_brdf = m;
	printf_s("%d \n", dim_fr_brdf);
}