#include "main.h"


//void preconvolve_material(const char *filename,
//									  const int dim_fr_n,
//									  const int samples_per_texel_fr_n,
//									  const int dim_fr_brdf,
//									  const int samples_per_texel_fr_brdf,
//									  const BRDF_interface *brdf)
//{
//	parab_frame fr_n, fr_brdf;
//	fr_n.init(dim_fr_n, samples_per_texel_fr_n);
//	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf);
//	fr_brdf.normalize_n();
//
//	int inc_one = 1;
//	int num_n = (int)fr_n.get_back_idx().size(),
//		num_wi, num_wo;
//	num_wi = num_wo = (int)fr_brdf.get_back_idx().size();
//
//	la_matrix<double> C(num_wi*num_wo, num_n);
//	la_vector<double> temp(num_wi*num_wo*samples_per_texel_fr_n), avg(num_wi*num_wo);
//	double power_e, power_s;
//	power_e = power_s = 0;
//
//	C.clear();
//	avg.clear();
//	printf_s("building the matrix...\n");
//	for (int i0 = 0; i0 < num_n; i0++)
//	//int i0 = fr_n.map(vector3f(0,0,1));
//	{
//		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
//		printf_s("(%d/%d) ", i0+1, num_n);
//		double weight = 1.0 / fr_n.get_n()[i0].size();
//		int len = num_wi*num_wo;
//		for (int i1 = 0; i1 < fr_n.get_n()[i0].size(); i1++)
//		{
//			const vector3f &n = fr_n.get_n()[i0][i1];
//
//			for (int j0 = 0; j0 < num_wo; j0++)
//			{
//				const vector3f &wo = fr_brdf.get_n()[j0][0];
//				for (int k0 = 0; k0 < num_wi; k0++)
//				{
//					float result;
//					brdf->sample(result, fr_brdf.get_n()[k0][0], wo, n);
//					temp.v[num_wi*num_wo*i1+k0+j0*num_wi] = result;
//					////DEBUG
//					//if (i0 == 118 && k0+j0*num_wi == 377633)
//					//{
//					//	printf_s("%g (%g %g %g)\n", result, n.x, n.y, n.z);
//					//	vector3f wi = fr_brdf.get_n()[k0][0];
//					//	printf_s("(%g %g %g) (%g %g %g)\n", wi.x, wi.y, wi.z, wo.x, wo.y, wo.z);
//					//}
//				}
//			}
//			daxpy(&len, &weight, &temp.v[num_wi*num_wo*i1], &inc_one, &C.m[num_wi*num_wo*i0], &inc_one);
//		}
//		power_s += ddot(&len, &C.m[num_wi*num_wo*i0], &inc_one, &C.m[num_wi*num_wo*i0], &inc_one);
//
//		for (int i1 = 0; i1 < fr_n.get_n()[i0].size(); i1++)
//		{
//			double weight = -1;
//			daxpy(&len, &weight, &C.m[num_wi*num_wo*i0], &inc_one, &temp.v[num_wi*num_wo*i1], &inc_one);
//			power_e += ddot(&len, &temp.v[num_wi*num_wo*i1], &inc_one, &temp.v[num_wi*num_wo*i1], &inc_one) / fr_n.get_n()[i0].size();
//		}
//
//		double alpha = 1.0 / num_n;
//		daxpy(&len, &alpha, &C.m[num_wi*num_wo*i0], &inc_one, avg.v, &inc_one);
//	}
//	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
//	printf("done.                \n");
//	printf_s("SNR = %gdb\n", double2db(power_s/power_e));
//
//	//Save
//	{
//		FILE *fp;
//		FOPEN(fp, "d:/temp/Microfacet/C.save", "wb");
//		C.save(fp);
//		fclose(fp);
//	}
//
//	printf_s("SVD decomposition...");
//	//centering...
//	for (int i = 0; i < num_n; i++)
//	{
//		double alpha = -1.0;
//		int len = num_wi*num_wo;
//		daxpy(&len, &alpha, avg.v, &inc_one, &C.m[num_wi*num_wo*i], &inc_one);
//	}
//	
//	la_matrix<double> U, Vt;
//	la_matrix<float> fUt, fV;
//	la_vector<float> favg;
//	SVD(U, Vt, C, 0.9);
//	//SVD(U, Vt, C, 0.999);
//	printf_s("done.\n");
//
//	//Load
//	{
//		FILE *fp;
//		FOPEN(fp, "d:/temp/Microfacet/C.save", "rb");
//		C.load(fp);
//		fclose(fp);
//	}
//
//	//compute SVD reconstruction error
//	la_matrix<double> recon;
//	for (int i = 0; i < U.row*U.column; i++)
//	{
//		float temp = (float)U.m[i];
//		U.m[i] = temp;
//	}
//	for (int i = 0; i < Vt.row*Vt.column; i++)
//	{
//		float temp = (float)Vt.m[i];
//		Vt.m[i] = temp;
//	}
//	matrix_mul(recon, U, Vt);
//	for (int i = 0; i < num_n; i++)
//	{
//		double alpha = 1.0;
//		int len = num_wi*num_wo;
//		daxpy(&len, &alpha, avg.v, &inc_one, &recon.m[num_wi*num_wo*i], &inc_one);
//	}
//
//	power_e = power_s = 0;
//	for (int i = 0; i < recon.row*recon.column; i++)
//	{
//		power_s += C.m[i]*C.m[i];
//		power_e += (C.m[i]-recon.m[i])*(C.m[i]-recon.m[i]);
//	}
//	printf_s("SVD SNR = %gdb\n", double2db(power_s/power_e));
///*
//	fUt.row = U.column;
//	fUt.column = U.row;
//	fUt.m = new float[fUt.row*fUt.column];
//	for (int x = 0; x < fUt.column; x++)
//		for (int y = 0; y < fUt.row; y++)
//			fUt.m[x*fUt.row+y] = (float)U.m[y*U.row+x];
//
//	fV.row = Vt.column;
//	fV.column = Vt.row;
//	fV.m = new float[fV.row*fV.column];
//	for (int x = 0; x < fV.column; x++)
//		for (int y = 0; y < fV.row; y++)
//			fV.m[x*fV.row+y] = (float)Vt.m[y*Vt.row+x];
//
//	favg.length = avg.length;
//	favg.v = new float[favg.length];
//	for (int i = 0; i < avg.length; i++)
//		favg.v[i] = (float)avg.v[i];
//
//	FILE *fp;
//	FOPEN(fp, filename, "wb");
//	fwrite(&dim_fr_n, sizeof(int), 1, fp);
//	fwrite(&samples_per_texel_fr_n, sizeof(int), 1, fp);
//	fwrite(&dim_fr_brdf, sizeof(int), 1, fp);
//	fwrite(&samples_per_texel_fr_brdf, sizeof(int), 1, fp);
//	favg.save(fp);
//	fUt.save(fp);
//	fV.save(fp);
//	fclose(fp);*/
//}
