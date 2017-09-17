#include "preconv_matr.h"
#include "math_rtn.h"
#include "BRDF.h"
#include "parab_map.h"
#include "microfacet_factory.h"

#define MAX_PATH 260

void BRDF_sampler::init(const BRDF_interface* pBRDF)
{
	const parab_frame &fr = mff_singleton::fr();

	int num_wo = fr.get_dim() / 2;
	cdf.clear();
	cdf.resize(num_wo);

	Vector3 n(0, 0, 1);
	for (int i = 0; i < num_wo; i++)
	{
		Vector3 wo = fr.get_n()[fr.get_idx()[num_wo*fr.get_dim() + fr.get_dim() - i - 1]][0];
		float wo_dot_n = Dot(wo, n);

		//debug
		//float total = 0;

		std::vector<float> brdf_val;
		brdf_val.resize(fr.get_n().size());
		for (int j = 0; j < fr.get_n().size(); j++)
		{
			Vector3 wi = fr.get_n()[j][0], result;
			pBRDF->sample(result, wi, wo, n);
			brdf_val[j] = (result.Length() / wo_dot_n*fr.get_spherical_area()[j]);
			//total += (result.x+result.y+result.z)/3/wo_dot_n/(wi*n)*fr.get_spherical_area()[j];
		}

		//printf_s("[%g] ", total);

		cdf[i] = step_1D_prob(brdf_val);
	}
}

void BRDF_sampler::sample_dir(Vector3 &wi, float &pdf,
	const Vector3 &wo, random &rng)
{
	float	sin_phi = sqrt(max(1 - wo.z*wo.z, 0));
	float	cos_theta = wo.x / sin_phi,
		sin_theta = wo.y / sin_phi;

	Vector3 wdir;
	wdir.x = sin_phi;
	wdir.y = 0;
	wdir.z = min(wo.z, 1);

	const parab_frame &fr = mff_singleton::fr();
	int iwo = fr.get_dim() - 1 - fr.get_back_idx()[fr.map(wdir)] % fr.get_dim();
	int iwi;

	cdf[iwo].sample(iwi, rng.get_random_float());
	pdf /= fr.get_spherical_area()[iwi];

	int ii = fr.get_back_idx()[iwi];
	if (ii != -1)
	{
		int x = ii % fr.get_dim(),
			y = ii / fr.get_dim();

		Vector3 dir;
		do {
			back_parab_map(dir, Vector2(x + rng.get_random_float(), y + rng.get_random_float()), fr.get_dim(), fr.get_dim(), true);
		} while (dir.z <= 0);
		wi.z = dir.z;
		wi.x = dir.x*cos_theta - dir.y*sin_theta;
		wi.y = dir.x*sin_theta + dir.y*cos_theta;
	}
}

preconv_matr::~preconv_matr()
{
	SAFE_DELETE(pBRDF);
	SAFE_DELETE(p_sampler);
}


#define SPHERICAL_AREA_SAMPLES	16000000

float compute_avg(const BRDF_interface *brdf,
				  const int dim_fr_brdf, 
				  const int samples_per_texel_fr_brdf, 
				  const int area_samples_fr_brdf,
				  const Vector3 &Mc)
{
	parab_frame fr_brdf;
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);

	const std::vector<float> &area = fr_brdf.get_spherical_area();

	Vector3 n(0, 0, 1);
	float avg = 0;
	for (int i_wo = 0; i_wo < fr_brdf.get_back_idx().size(); i_wo++)
		for (int i_wi = 0; i_wi < fr_brdf.get_back_idx().size(); i_wi++)
		{
			const Vector3 &wi = fr_brdf.get_n()[i_wi][0];
			const Vector3 &wo = fr_brdf.get_n()[i_wo][0];

			Vector3 result;
			brdf->sample(result, wi, wo, n);
			avg += (Dot(result, Mc))*area[i_wi]*area[i_wo];
		}
	avg /= 4*PI*PI;
	avg *= Mc.Length();

	return abs(avg);
}

void preconv_matr::load(const char *file_prefix)
{
	char filename[260];
	FILE *fp;

	sprintf_s(filename, "%s", file_prefix);
	printf_s("preconv_matr::loading %s...", filename);
	fp = fopen(filename, "rb");

	//codex::utils::timer t;
	
	int dim_fr_n, samples_per_texel_fr_n, 
		dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf;
	float avg = 0;

	fread(&dim_fr_n, sizeof(int), 1, fp);
	fread(&samples_per_texel_fr_n, sizeof(int), 1, fp);
	fread(&dim_fr_brdf, sizeof(int), 1, fp);
	fread(&samples_per_texel_fr_brdf, sizeof(int), 1, fp);
	fread(&area_samples_fr_brdf, sizeof(int), 1, fp);

	avgm.load(fp);
	Umt.load(fp);
	Vm.load(fp);
	inv_n_area.load(fp);
	for (int i = 0; i < inv_n_area.length; i++)
		inv_n_area.v[i] = 1.0f / inv_n_area.v[i];
	la_vector<float> temp;
	temp.load(fp);
	float mc[3];
	fread(&mc, sizeof(float)*3, 1, fp);
	Mc = Vector3(mc[0], mc[1], mc[2]);
	if (!feof(fp))
		fread(&avg, sizeof(float), 1, fp);
	fclose(fp);

	//SSE
	//Umt_SSE.from_unaligned(Umt.row, Umt.column, Umt.m);

	//compute avg
	if (avg == 0)
	{
		printf_s("computing avg...");
		avg = compute_avg(pBRDF, dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf, Mc);
		printf_s("%g", avg);
		fp = fopen(filename, "ab");
		fwrite(&avg, sizeof(float), 1, fp);
		fclose(fp);
		printf_s("done\n");
	}
	//t.update();
	//printf_s(" [%g] ", t.elapsed_time());

	inv_avg = 1.0f;
	printf_s("avg=%g ", avg);
	////scale
	//inv_avg = 0.4f / avg; 
	////0.15f / avg;

	//DEBUG
	printf_s("[[%d %d]]", dim_fr_n, samples_per_texel_fr_n);
	printf_s("(%d %d %d %g)", dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf, inv_avg);

#ifndef LOG_MATERIAL
	int inc_one = 1;
	int sz = Umt.row*Umt.column;
	sscal(&sz, &inv_avg, Umt.m, &inc_one);
	sscal(&avgm.length, &inv_avg, avgm.v, &inc_one);
#endif

	char path[MAX_PATH], name[MAX_PATH], ext[MAX_PATH], frame_name[MAX_PATH];
	decompose_file_name(filename, path, name, ext);
	sprintf_s(frame_name, "%s%s.frame", path, name);

	fopen_s(&fp, frame_name, "rb");
	if (fp == NULL)
	{
		fr_n.init(dim_fr_n, 1000);
		fr_n.reduce_n(samples_per_texel_fr_n);
		fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf);
		fr_brdf.normalize_n();

		fp = fopen(frame_name, "wb");
		fr_n.save(fp);
		fr_brdf.save(fp);
		fclose(fp);
	} else {
		fr_n.load(fp);
		fr_brdf.load(fp);
	}

	//t.update();
	//printf_s(" [%g] ", t.elapsed_time());

#ifdef USE_SVD_SCALAR
	sprintf_s(filename, "%s.scalar", file_prefix);
	FOPEN(fp, filename, "rb");
	sUn.load(fp);
	sVnt.load(fp);
	fclose(fp);

	inv_sUn.length = sUn.length;
	inv_sUn.v = new float[inv_sUn.length];
	for (int i = 0; i < inv_sUn.length; i++)
		inv_sUn.v[i] = (sUn.v[i] != 0) ? 1.0f / sUn.v[i] : 0.0f;

	inv_sVnt.length = sVnt.length;
	inv_sVnt.v = new float[inv_sVnt.length];
	for (int i = 0; i < inv_sVnt.length; i++)
		inv_sVnt.v[i] = (sVnt.v[i] != 0) ? 1.0f / sVnt.v[i] : 0.0f;
#endif 

	printf_s("done\n");
}

int preconv_matr::compute_idx_brdf(const Vector3 &wi, const Vector3 &wo) const
{
	return fr_brdf.map(wi)+fr_brdf.map(wo)*(int)fr_brdf.get_back_idx().size();
}

void preconv_matr::lerp_brdf(int &num, 
							 int *idx, float *weight,
							 const Vector3 &wi, const Vector3 &wo) const
{
	int num_i, num_o;
	int idx_i[4], idx_o[4];
	float weight_i[4], weight_o[4];

	fr_brdf.lerp(num_i, idx_i, weight_i, wi, true, false);
	fr_brdf.lerp(num_o, idx_o, weight_o, wo, true, false);

	num = 0;
	for (int i = 0; i < num_o; i++)
		for (int j = 0; j < num_i; j++)
		{
			idx[num] = brdf_idx(idx_i[j], idx_o[i]);
			weight[num] = weight_i[j]*weight_o[i];
			num++;
		}
}

void preconv_matr::init_R(const int dim_reduced)
{
	gen_random_matrix(R, fr_n.get_n().size(), dim_reduced, sqrt(1.0f/(float)dim_reduced));
}

void preconv_matr::SVD_rp(la_matrix<number> &U, la_matrix<number> &Vt, 
						  la_matrix<number> &C, la_matrix<number> &CR, const number percent)
{
	//codex::utils::timer t;
	SVD_U_L1(U, CR, percent);
	//t.update();
	//printf_s("SVD_L1 %gsecs\n", t.elapsed_time());
	printf_s("SVD_L1 finished\n");
	Vt.release();
	Vt.row = U.column;
	Vt.column = C.column;
	Vt.m = new number[Vt.row*Vt.column];

	float alpha = 1.0f, beta = 0.0f;
	sgemm("T", "N", &U.column, &C.column, &U.row, 
		&alpha, U.m, &U.row, C.m, &C.row,
		&beta, Vt.m, &Vt.row);
}