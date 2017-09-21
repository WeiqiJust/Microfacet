#include "main.h"


#define BUFFER_SIZE	(2000*1048576) // 2.0G

float compute_avg(const BRDF_interface *brdf,
							 const parab_frame &fr_brdf,
							 const vector3f &Mc)
{
	const std::vector<float> &area = fr_brdf.get_spherical_area();

	vector3f n(0, 0, 1);
	float avg = 0;
	for (int i_wo = 0; i_wo < fr_brdf.get_back_idx().size(); i_wo++)
		for (int i_wi = 0; i_wi < fr_brdf.get_back_idx().size(); i_wi++)
		{
			const vector3f &wi = fr_brdf.get_n()[i_wi][0];
			const vector3f &wo = fr_brdf.get_n()[i_wo][0];

			vector3f result;
			brdf->sample(result, wi, wo, n);
			avg += (result*Mc)*area[i_wi]*area[i_wo];
		}
	avg /= 4*PI*PI;
	avg *= Mc.length();

	return abs(avg);
}

void preconv_matr_rp_brdf(const char *filename_prefix,
									  const int dim_fr_n,
									  const int samples_per_texel_fr_n,
									  const int dim_fr_brdf,
									  const int samples_per_texel_fr_brdf,
									  const int area_samples_fr_brdf,
									  const BRDF_interface *brdf,
									  const int reduced_dim,
									  const double eigen_percent,
									  const vector3f &Mc)
{
	//DEBUG
	//return;

	char filename[MAX_PATH];
	sprintf_s(filename, "%s_%d_%d_%d.preconv_material", filename_prefix, dim_fr_n, dim_fr_brdf, int(eigen_percent*100));

	//codex::utils::timer t;
	double_parab_frame fr_n;
	parab_frame fr_brdf;
	fr_n.init(dim_fr_n, 256);
	fr_n.compute_spherical_area(SPHERICAL_AREA_SAMPLES);
	const std::vector<float> &n_area = fr_n.get_spherical_area();
	const std::vector<float> &area = fr_brdf.get_spherical_area();
	for (int i = 0; i < n_area.size(); i++)
		if (n_area[i] == 0)
		{
			//throw_error(ERROR_POS, EXP_GENERIC, "Error in compute_spherical_area. Please increase the number of samples!!!");
			cout << "1. Error in compute_spherical_area. Please increase the number of samples!!! " << i << endl;
		}
	fr_n.reduce_n(samples_per_texel_fr_n);
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);
	for (int i = 0; i < area.size(); i++)
		if (area[i] == 0)
		{
			//throw_error(ERROR_POS, EXP_GENERIC, "Error in compute_spherical_area. Please increase the number of samples!!!");
			cout << "2. Error in compute_spherical_area. Please increase the number of samples!!!" << endl;
		}

	int inc_one = 1,
		inc_zero = 0;
	int num_n = fr_n.get_back_idx().size(),
		num_wi, num_wo, num_brdf, num_brdf_reduced;
	num_wi = num_wo = fr_brdf.get_back_idx().size();
	num_brdf = (num_wo+1)*num_wo/2;
	num_brdf_reduced = reduced_dim;

	__int64 num_total_elements, num_non_zero_elements;
	num_total_elements = num_non_zero_elements = 0;

	printf_s("n     : %d\n", num_n);
	printf_s("wi/wo : %d\n", num_wi);
	printf_s("brdf  : %d\n", num_brdf);
	printf_s("brdf* : %d\n", num_brdf_reduced);

	double scalar = 1 / sqrt((double)num_brdf_reduced);

	int batch_n = min(BUFFER_SIZE / sizeof(double) / (num_brdf), num_n);
	la_matrix<double>	B(num_brdf_reduced, num_n),
						A_columns(num_brdf, batch_n);
	la_vector<double>	Rt_row(num_brdf),						
						avg_brdf(num_brdf),
						avg_B(num_brdf_reduced);
	la_vector<double>	SVD_scalar(num_n);
	avg_brdf.clear();
	SVD_scalar.clear();

	printf_s("building the matrix...\n");
	int max_ii = (int)ceil((float)num_n / batch_n);
	for (int ii = 0; ii <  max_ii; ii++)
	{
		int i_start		= ii*batch_n;
		int batch_size	= min(num_n - i_start, batch_n);

		A_columns.clear();
		for (int i0 = i_start; i0 < i_start+batch_size; i0++)
		//i_start = 510;
		//int i0 = 510;
		{
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf_s("sample (%d/%d) ", i0, num_n);
			int		len = num_brdf;
			double	weight = 1.0 / fr_n.get_n()[i0].size();

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

						//DEBUG
						if (i0 == 510 && brdf_idx(k0, j0) == 12873)
						{
							vector3f temp = result;
							printf_s("------------------- %d %d\n", j0, k0);
							printf_s("n  : %g %g %g\n", n.x, n.y, n.z);							
							printf_s("inc: %g, %d:%g, %d:%g, %d:%g, total:%g\n", result*Mc, j0, area[j0], k0, area[k0], i0, n_area[i0], A_columns.m[(i0-i_start)*A_columns.row+brdf_idx(k0, j0)]);
							//printf_s("wi : %g %g %g\n", fr_brdf.get_n()[k0][0].x, fr_brdf.get_n()[k0][0].y, fr_brdf.get_n()[k0][0].z);
							//printf_s("wo : %g %g %g\n", wo.x, wo.y, wo.z);

							temp.x = exp(temp.x)-1.0f;
							temp.y = exp(temp.y)-1.0f;
							temp.z = exp(temp.z)-1.0f;
							printf_s("r  : %g %g %g\n", temp.x, temp.y, temp.z);
						}

						if (result.x != 0 || result.y != 0 || result.z != 0)
							num_non_zero_elements++;
						num_total_elements++;
						//result -= avgc;
						double dot = result*Mc;
						A_columns.m[(i0-i_start)*A_columns.row+brdf_idx(k0, j0)] += (result*Mc)*area[j0]*area[k0]*n_area[i0];
					}
				}
			}
			dscal(&len, &weight, &A_columns.m[(i0-i_start)*A_columns.row], &inc_one);

			//DEBUG
			if (i0 == 510)
			{
				printf_s("-------------------\n");
				double a = A_columns.m[(510-i_start)*A_columns.row+12873];
				printf_s("a: %g : %g\n", a, area[153]*area[159]*n_area[510]);
				a /= area[153]*area[159]*n_area[510];
				printf_s("a: %g\n", a);
				vector3f result;
				for (int ch = 0; ch < 3; ch++)
					result.v[ch] = a*Mc.v[ch];
				result.x = exp(result.x)-1.0f;
				result.y = exp(result.y)-1.0f;
				result.z = exp(result.z)-1.0f;
				printf_s("avg: %g %g %g\n", result.x, result.y, result.z);
			}

			//compute average
			double alpha = 1.0 / num_n;
			daxpy(&len, &alpha, &A_columns.m[(i0-i_start)*A_columns.row], &inc_one, avg_brdf.v, &inc_one);

			SVD_scalar.v[i0] = sqrt(
				ddot(&len, &A_columns.m[(i0-i_start)*A_columns.row], &inc_one, &A_columns.m[(i0-i_start)*A_columns.row], &inc_one)
				);
		}

		random_number_generator<double> rng;
		for (int j = 0; j < num_brdf_reduced; j++)
		{
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf_s("(%d/%d) ", i_start*num_brdf_reduced+j+1, num_n*num_brdf_reduced);
			for (int k = 0; k < Rt_row.length; k++)
			{
				gen_random_element(Rt_row.v[k], rng);
				Rt_row.v[k] *= scalar;
			}

			int one = 1;
			double	alpha = 1.0, beta = 0.0;
			dgemm("N", "N", &one, &batch_size, &Rt_row.length, 
				&alpha, Rt_row.v, &inc_one, A_columns.m, &A_columns.row,
				&beta, &B.m[i_start*B.row+j], &B.row);
		}
	}
	A_columns.release();
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	{
		random_number_generator<double> rng;
		for (int j = 0; j < num_brdf_reduced; j++)
		{
			for (int k = 0; k < Rt_row.length; k++)
			{
				gen_random_element(Rt_row.v[k], rng);
				Rt_row.v[k] *= scalar;
			}
			avg_B.v[j] = ddot(&Rt_row.length, Rt_row.v, &inc_one, avg_brdf.v, &inc_one);
		}
		for (int i = 0; i < B.column; i++)
		{
			double alpha = -1;
			daxpy(&avg_B.length, &alpha, avg_B.v, &inc_one, &B.m[i*B.row], &inc_one);
		}
	}
	printf("done.                \n");

	printf_s("non-zero %% = %I64d/%I64d = %.2f%%\n", 
		num_non_zero_elements, num_total_elements,
		num_non_zero_elements*100.0/num_total_elements);

	////DEBUG
	//B.save_txt("d:/temp/Microfacet/B.txt");
	//{
	//	FILE *fp;
	//	FOPEN(fp, "d:/temp/Microfacet/B.save", "wb");
	//	B.save(fp);
	//	fclose(fp);

	//	la_matrix<double> BU, Vt;
	//	SVD(BU, Vt, B, eigen_percent, true);

	//	FOPEN(fp, "d:/temp/Microfacet/B.save", "rb");
	//	B.load(fp);
	//	fclose(fp);

	//	la_matrix<double> C;
	//	matrix_mul(C, BU, Vt);

	//	double power_e, power_s;
	//	power_e = power_s = 0;
	//	for (int i = 0; i < C.row*C.column; i++)
	//	{
	//		power_s += B.m[i]*B.m[i];
	//		power_e += (B.m[i]-C.m[i])*(B.m[i]-C.m[i]);
	//	}
	//	printf_s("SNR = %gdb\n", double2db(power_s/power_e));

	//	double norm_B, norm_C;
	//	int len = B.row*B.column;
	//	norm_B = ddot(&len, B.m, &inc_one, B.m, &inc_one);
	//	norm_C = ddot(&len, C.m, &inc_one, C.m, &inc_one);
	//	printf_s("%g %g %g\n", norm_B, norm_C, norm_C / norm_B);

	//	return;
	//}

	la_matrix<double> BU, Vt;
	SVD(BU, Vt, B, eigen_percent, NULL, false);
	BU.release();

	//compute U
	printf_s("computing U...\n");
	la_matrix<double>	U(num_brdf, Vt.row);
	la_vector<double>	A_row(num_n);
	for (int i_wo = 0; i_wo < num_wo; i_wo++)
		for (int i_wi = 0; i_wi <= i_wo; i_wi++)
		{
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf_s("(%d/%d) ", brdf_idx(i_wi, i_wo), num_brdf);

			const vector3f &wi = fr_brdf.get_n()[i_wi][0];
			const vector3f &wo = fr_brdf.get_n()[i_wo][0];
			A_row.clear();
			for (int i0 = 0; i0 < num_n; i0++)
			{
				double weight = 1.0 / fr_n.get_n()[i0].size();
				for (int i1 = 0; i1 < fr_n.get_n()[i0].size(); i1++)
				{
					vector3f result;
					brdf->sample(result, wi, wo, fr_n.get_n()[i0][i1]);
					//result -= avgc;
					A_row.v[i0] += (result*Mc)*weight*area[i_wi]*area[i_wo]*n_area[i0];
				}
			}

			int		one = 1;
			double	alpha = -1.0, beta = 0.0;
			daxpy(&A_row.length, &alpha, &avg_brdf.v[brdf_idx(i_wi, i_wo)], &inc_zero, A_row.v, &inc_one);

			alpha = 1.0;
			dgemm("N", "T", &one, &Vt.row, &Vt.column, 
				&alpha, A_row.v, &one, Vt.m, &Vt.row, 
				&beta, &U.m[brdf_idx(i_wi, i_wo)], &U.row);
		}
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf("done.                \n");

	////DEBUG
	//{
	//	FILE *fp;
	//	FOPEN(fp, "d:/temp/Microfacet/Vt.txt", "wt");
	//	for (int y = 0; y < Vt.row; y++)
	//	{
	//		for (int x = 0; x < Vt.column; x++)
	//			fprintf_s(fp, "%g ", Vt.m[y+Vt.row*x]);
	//		fprintf_s(fp, "\n");
	//	}
	//	fclose(fp);
	//}

	//save the result
	la_matrix<float> fUt, fV;
	la_vector<float> favg;

	fUt.row = U.column;
	fUt.column = U.row;
	fUt.m = new float[fUt.row*fUt.column];
	for (int i_wo = 0; i_wo < num_wo; i_wo++)
		for (int i_wi = 0; i_wi <= i_wo; i_wi++)
		{
			int x = brdf_idx(i_wi, i_wo);
			for (int y = 0; y < fUt.row; y++)
				fUt.m[x*fUt.row+y] = (float)(U.m[y*U.row+x]/(area[i_wi]*area[i_wo]));
		}

	fV.row = Vt.column;
	fV.column = Vt.row;
	fV.m = new float[fV.row*fV.column];
	for (int x = 0; x < fV.column; x++)
		for (int y = 0; y < fV.row; y++)
			fV.m[x*fV.row+y] = (float)(Vt.m[y*Vt.row+x]/n_area[y]);

	favg.length = avg_brdf.length;
	favg.v = new float[favg.length];
	for (int i_wo = 0; i_wo < num_wo; i_wo++)
		for (int i_wi = 0; i_wi <= i_wo; i_wi++)
		{
			favg.v[brdf_idx(i_wi, i_wo)] = (float)(avg_brdf.v[brdf_idx(i_wi, i_wo)]/(area[i_wi]*area[i_wo]));
		}

	//normalize SVD_scalar
	double max_SVD_scalar = 0;
	for (int i = 0; i < SVD_scalar.length; i++)
		max_SVD_scalar = max(max_SVD_scalar, SVD_scalar.v[i]);
	for (int i = 0; i < SVD_scalar.length; i++)
		SVD_scalar.v[i] /= max_SVD_scalar;
	la_vector<float> fSVD_scalar(SVD_scalar.length);
	for (int i = 0; i < fSVD_scalar.length; i++)
		fSVD_scalar.v[i] = (float)SVD_scalar.v[i];
	
	la_vector<float> vec_n_area(n_area.size());
	for (int i = 0; i < n_area.size(); i++)
		vec_n_area.v[i] = n_area[i];

	FILE *fp;
	FOPEN(fp, filename, "wb");
	fwrite(&dim_fr_n, sizeof(int), 1, fp);
	fwrite(&samples_per_texel_fr_n, sizeof(int), 1, fp);
	fwrite(&dim_fr_brdf, sizeof(int), 1, fp);
	fwrite(&samples_per_texel_fr_brdf, sizeof(int), 1, fp);
	fwrite(&area_samples_fr_brdf, sizeof(int), 1, fp);
	favg.save(fp);
	fUt.save(fp);
	fV.save(fp);
	vec_n_area.save(fp);
	fSVD_scalar.save(fp);
	fwrite(Mc.v, sizeof(float)*3, 1, fp);
	float avg = compute_avg(brdf, fr_brdf, Mc);
	fwrite(&avg, sizeof(float), 1, fp);
	//fwrite(avgc.v, sizeof(float)*3, 1, fp);
	fclose(fp);

	//printf_s("Ut %d %d\n", fUt.row, fUt.column);
	//printf_s("V  %d %d\n", fV.row, fV.column);

	//t.update();
	//printf_s("Time = %gsecs\n", t.elapsed_time());

	//compute SNR
	printf_s("computing SNR...\n");
	double power_e, power_s;
	power_e = power_s = 0;

	la_vector<double> recon_col(num_brdf), A_column(num_brdf);
	for (int i0 = 0; i0 < num_n; i0++)
	{
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		printf_s("(%d/%d) ", i0, num_n);
		int		len = num_brdf;
		double	weight = 1.0 / fr_n.get_n()[i0].size();

		A_column.clear();
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

					//DEBUG
					if (i0 == 510 && brdf_idx(k0, j0) == 12873)
					{
						printf_s("-------------------\n");
						printf_s("n  : %g %g %g\n", n.x, n.y, n.z);
						printf_s("inc: %g, %d:%g, %d:%g, %d:%g, total:%g\n", result*Mc, j0, area[j0], k0, area[k0], i0, n_area[i0], A_column.v[brdf_idx(k0,j0)]);
						//printf_s("wi : %g %g %g\n", fr_brdf.get_n()[k0][0].x, fr_brdf.get_n()[k0][0].y, fr_brdf.get_n()[k0][0].z);
						//printf_s("wo : %g %g %g\n", wo.x, wo.y, wo.z);

						vector3f temp = result;
						temp.x = exp(temp.x)-1.0f;
						temp.y = exp(temp.y)-1.0f;
						temp.z = exp(temp.z)-1.0f;
						printf_s("r  : %g %g %g\n", temp.x, temp.y, temp.z);
					}

					//result -= avgc;
					A_column.v[brdf_idx(k0,j0)] += (result*Mc)*area[j0]*area[k0]*n_area[i0];
				}
			}
		}
		dscal(&len, &weight, A_column.v, &inc_one);

		int		one = 1;
		double	alpha = 1.0, beta = 0.0;

		dgemm("N", "N", &U.row, &one, &U.column,
			&alpha, U.m, &U.row, &Vt.m[i0*Vt.row], &Vt.row, 
			&beta, recon_col.v, &recon_col.length);
		daxpy(&recon_col.length, &alpha, avg_brdf.v, &inc_one, recon_col.v, &inc_one);

		for (int i = 0; i < recon_col.length; i++)
		{
			power_s += A_column.v[i]*A_column.v[i];
			power_e += (A_column.v[i]-recon_col.v[i])*(A_column.v[i]-recon_col.v[i]);
		}

		//DEBUG
		if (i0 == 510)
		{
			vector3f result;

			double a = A_column.v[12873];
			printf_s("-------------------\n");
			printf_s("a: %g : %g\n", a, area[153]*area[159]*n_area[510]);
			a /= area[153]*area[159]*n_area[510];
			printf_s("a: %g\n", a);
			for (int ch = 0; ch < 3; ch++)
				result.v[ch] = a*Mc.v[ch];
			result.x = exp(result.x)-1.0f;
			result.y = exp(result.y)-1.0f;
			result.z = exp(result.z)-1.0f;

			printf_s("avg  : %g %g %g\n", result.x, result.y, result.z);

			a = recon_col.v[12873];
			a /= area[153]*area[159]*n_area[510];
			for (int ch = 0; ch < 3; ch++)
				result.v[ch] = a*Mc.v[ch];
			result.x = exp(result.x)-1.0f;
			result.y = exp(result.y)-1.0f;
			result.z = exp(result.z)-1.0f;
			printf_s("recon: %g %g %g\n", result.x, result.y, result.z);
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf("done.                \n");
	//printf_s("SNR = %gdb\n", double2db(power_s/power_e));
	printf_s("========================\n");
}

void compute_nonzero_elements(const int dim_fr_n,
										  const int samples_per_texel_fr_n,
										  const int dim_fr_brdf,
										  const int samples_per_texel_fr_brdf,
										  const int area_samples_fr_brdf,
										  const BRDF_interface *brdf)
{
	double_parab_frame fr_n;
	parab_frame fr_brdf;
	fr_n.init(dim_fr_n, 256);
	fr_n.compute_spherical_area(SPHERICAL_AREA_SAMPLES);
	const std::vector<float> &n_area = fr_n.get_spherical_area();
	const std::vector<float> &area = fr_brdf.get_spherical_area();
	for (int i = 0; i < n_area.size(); i++)
		if (n_area[i] == 0)
		{
			//throw_error(ERROR_POS, EXP_GENERIC, "Error in compute_spherical_area. Please increase the number of samples!!!");
			cout << " Error in compute_nonzero_elements!!!" << endl;
		}
	fr_n.reduce_n(samples_per_texel_fr_n);
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);
	for (int i = 0; i < area.size(); i++)
		if (area[i] == 0)
		{
			//throw_error(ERROR_POS, EXP_GENERIC, "Error in compute_spherical_area. Please increase the number of samples!!!");
			cout << " Error in compute_nonzero_elements!!!" << endl;
		}

	int inc_one = 1,
		inc_zero = 0;
	int num_n = fr_n.get_back_idx().size(),
		num_wi, num_wo, num_brdf, num_brdf_reduced;
	num_wi = num_wo = fr_brdf.get_back_idx().size();
	num_brdf = (num_wo+1)*num_wo/2;

	__int64 num_total_elements, num_non_zero_elements;
	num_total_elements = num_non_zero_elements = 0;

	printf_s("n     : %d\n", num_n);
	printf_s("wi/wo : %d\n", num_wi);
	printf_s("brdf  : %d\n", num_brdf);
	printf_s("brdf* : %d\n", num_brdf_reduced);

	printf_s("building the matrix...\n");
	for (int i0 = 0; i0 < num_n; i0++)
	{
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		printf_s("sample (%d/%d) ", i0, num_n);
		int		len = num_brdf;
		double	weight = 1.0 / fr_n.get_n()[i0].size();

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
					if (result.x != 0 || result.y != 0 || result.z != 0)
						num_non_zero_elements++;
					num_total_elements++;
				}
			}
		}
	}

	printf_s("non-zero %% = %I64d/%I64d = %.2f%%\n", 
		num_non_zero_elements, num_total_elements,
		num_non_zero_elements*100.0/num_total_elements);
}