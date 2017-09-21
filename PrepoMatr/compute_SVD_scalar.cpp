#include "main.h"


void compute_SVD_scalar(const char *filename,
									const char *input_filename,
									const int dim_vis)
{
	parab_frame fr_vis;
	fr_vis.init(dim_vis, 256, DEFAULT_AREA_SAMPLES);
	fr_vis.normalize_n();
	fr_vis.compute_spherical_area(SPHERICAL_AREA_SAMPLES);

	la_matrix<float> fUt, fV;
	la_vector<float> favg, temp, vec_n_area;
	
	int dim_fr_n, samples_per_texel_fr_n, 
		dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf;

	FILE *fp;
	FOPEN(fp, input_filename, "rb");
	fread(&dim_fr_n, sizeof(int), 1, fp);
	fread(&samples_per_texel_fr_n, sizeof(int), 1, fp);
	fread(&dim_fr_brdf, sizeof(int), 1, fp);
	fread(&samples_per_texel_fr_brdf, sizeof(int), 1, fp);
	fread(&area_samples_fr_brdf, sizeof(int), 1, fp);
	favg.load(fp);
	fUt.load(fp);
	fV.load(fp);
	vec_n_area.load(fp);
	temp.load(fp);

	vector3f Mc;
	fread(Mc.v, sizeof(float)*3, 1, fp);
	fclose(fp);

	double_parab_frame fr_n;
	parab_frame fr_brdf;
	fr_n.init(dim_fr_n, 256);
	fr_n.compute_spherical_area(SPHERICAL_AREA_SAMPLES);
	fr_n.reduce_n(samples_per_texel_fr_n);
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);

	int num_vis = fr_vis.get_n().size(),
		num_n = fr_n.get_n().size(),
		num_brdf = fr_brdf.get_n().size();
	la_matrix<double> M(num_vis*num_vis, num_n);
	M.clear();
	
	int inc_one = 1;
	for (int i_n = 0; i_n < num_n; i_n++)
		for (int i_wo = 0; i_wo < num_brdf; i_wo++)
		{
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf_s("(%d/%d) ", i_n*num_brdf+i_wo, (num_brdf*num_n));

			for (int i_wi = 0; i_wi < num_brdf; i_wi++)
			{
				//compute the brdf
				int idx_brdf = brdf_idx(i_wi, i_wo);
				float refl = sdot(&fV.column, &fV.m[i_n], &fV.row, &fUt.m[idx_brdf*fUt.row], &inc_one)
					+ favg.v[idx_brdf]/vec_n_area.v[i_n];

				int num_vis_wi, num_vis_wo;
				int idx_vis_wi[4], idx_vis_wo[4];
				float weight_vis_wi[4], weight_vis_wo[4];
				fr_vis.lerp(num_vis_wi, idx_vis_wi, weight_vis_wi, fr_brdf.get_n()[i_wi][0], false, false);
				fr_vis.lerp(num_vis_wo, idx_vis_wo, weight_vis_wo, fr_brdf.get_n()[i_wo][0], false, false);

				for (int i_vis_wi = 0; i_vis_wi < num_vis_wi; i_vis_wi++)
					for (int i_vis_wo = 0; i_vis_wo < num_vis_wo; i_vis_wo++)
					{
						M.m[idx_vis_wi[i_vis_wi]*num_vis+idx_vis_wo[i_vis_wo]+i_n*num_vis*num_vis] += refl*weight_vis_wi[i_vis_wi]*weight_vis_wo[i_vis_wo];
					}
			}
		}

	const std::vector<float> &area = fr_vis.get_area();
	for (int i_n = 0; i_n < num_n; i_n++)
		for (int i_wo = 0; i_wo < num_vis; i_wo++)
			for (int i_wi = 0; i_wi < num_vis; i_wi++)
				M.m[i_wi*num_vis+i_wo+i_n*num_vis*num_vis] *= vec_n_area.v[i_n] * area[i_wi] * area[i_wo];

	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf("done.                \n");

	la_matrix<double> U, Vt;
	SVD_eigen_num(U, Vt, M, 1);

	la_vector<float> sU(U.row*U.column), sVt(Vt.row*Vt.column);
	for (int i = 0; i < sU.length; i++)
		sU.v[i] = (float)U.m[i];
	for (int i = 0; i < sVt.length; i++)
		sVt.v[i] = (float)Vt.m[i];

	FOPEN(fp, filename, "wb");
	sU.save(fp);
	sVt.save(fp);
	fclose(fp);
}
