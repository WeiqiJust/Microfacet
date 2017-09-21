#include "main.h"


void resample_brdf(const char* filename,
							   const BRDF_interface *brdf,
							   const int dim_fr_brdf,
							   const int samples_per_texel_fr_brdf,
							   const int area_samples_fr_brdf)
{
	parab_frame fr_brdf;
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, area_samples_fr_brdf);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);

	vector3f n(0, 0, 1);
	int brdf_dim = fr_brdf.get_n().size();

	std::vector<float> BRDF;
	BRDF.resize(3*brdf_dim*brdf_dim, 0);

	for (int i_wo = 0; i_wo < brdf_dim; i_wo++)
		for (int i_wi = 0; i_wi < brdf_dim; i_wi++)
		{
			const vector3f &wo = fr_brdf.get_n()[i_wo][0];
			const vector3f &wi = fr_brdf.get_n()[i_wi][0];

			vector3f refl;
			brdf->sample(refl, wi, wo, n);
#ifdef LOG_MATERIAL
			refl.x = exp(refl.x)-1.0f;
			refl.y = exp(refl.y)-1.0f;
			refl.z = exp(refl.z)-1.0f;
#endif // LOG_MATERIAL
			refl /= n*wo;

			BRDF[3*(brdf_dim*i_wo+i_wi)+0] += refl.x;
			BRDF[3*(brdf_dim*i_wo+i_wi)+1] += refl.y;
			BRDF[3*(brdf_dim*i_wo+i_wi)+2] += refl.z;
		}

	FILE *fp;
	FOPEN(fp, filename, "wb");
	int len = BRDF.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&BRDF[0], BRDF.size()*sizeof(float), 1, fp);
	fclose(fp);
}