#include "main.h"

void compute_color_matrix(Vector3 &Mc,
									  //Vector3 &avgc,
									  const int dim_fr_n,
									  const int samples_per_texel_fr_n,
									  const int dim_fr_brdf,
									  const int samples_per_texel_fr_brdf,
									  const BRDF_interface *brdf)
{
	double_parab_frame fr_n;
	parab_frame fr_brdf;
	const std::vector<float> &area = fr_brdf.get_spherical_area();
	fr_n.init(dim_fr_n, samples_per_texel_fr_n);
	fr_n.normalize_n();
	fr_brdf.init(dim_fr_brdf, samples_per_texel_fr_brdf, DEFAULT_AREA_SAMPLES);
	fr_brdf.normalize_n();
	fr_brdf.compute_spherical_area(SPHERICAL_AREA_SAMPLES);

	//vector3 avg;
	la_matrix<double> M(3, fr_n.get_n().size()*fr_brdf.get_n().size()*fr_brdf.get_n().size());
	
	int idx = 0;
	for (int i0 = 0; i0 < fr_n.get_n().size(); i0++)
	{
		const Vector3 &n = fr_n.get_n()[i0][0];

		for (int j0 = 0; j0 < fr_brdf.get_n().size(); j0++)
		{
			const Vector3 &wo = fr_brdf.get_n()[j0][0];
			for (int k0 = 0; k0 <= j0; k0++)
			{
				Vector3 result;
				brdf->sample(result, fr_brdf.get_n()[k0][0], wo, n);
				result *= area[k0]*area[j0];
				M.m[idx*3+0] = result.x;
				M.m[idx*3+1] = result.y;
				M.m[idx*3+2] = result.z;
				
				//avg.x += result.x;
				//avg.y += result.y;
				//avg.z += result.z;
				idx++;
			}
		}
	}
	//avg /= M.column;

	int		inc_one = 1, inc_zero = 0, inc_three = 3;
	//double	alpha = -1;
	//for (int i = 0; i < 3; i++)
	//	daxpy(&M.column, &alpha, &avg.v[i], &inc_zero, &M.m[i], &inc_three);

	la_matrix<double> U, Vt, C(M.row, M.column);
	la_vector<double> lamda;

	memcpy(C.m, M.m, sizeof(double)*M.row*M.column);
	SVD(U, Vt, M, 1.0, &lamda, false);

	//printf_s("percent = %.2f%%\n", 100*lamda.v[0]*lamda.v[0]/(lamda.v[0]*lamda.v[0]+lamda.v[1]*lamda.v[1]+lamda.v[2]*lamda.v[2]));
	printf_s("%.2f%%\n", 100*lamda.v[0]*lamda.v[0]/(lamda.v[0]*lamda.v[0]+lamda.v[1]*lamda.v[1]+lamda.v[2]*lamda.v[2]));

	//avgc.x = avg.x;
	//avgc.y = avg.y;
	//avgc.z = avg.z;
	//printf_s("avg %g %g %g\n", avg.x, avg.y, avg.z);

	Mc.x = U.m[0];
	Mc.y = U.m[1];
	Mc.z = U.m[2];

	//printf_s("M   %g %g %g\n", Mc.x, Mc.y, Mc.z);
}