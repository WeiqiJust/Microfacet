#include "vis_normal_distr.h"
#include "preconv_matr.h"
#include "microfacet_factory.h"

void vis_normal_distr::compute_BRDF(const parab_frame &fr_vis,
	const float* inv_SVD_scalar)
{
	preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(basis_id);
	BRDF.init(3 * basis_matr->avgm.length);

	int num_vis_wi, num_vis_wo;
	int idx_vis_wi[4], idx_vis_wo[4];
	float weight_vis_wi[4], weight_vis_wo[4];
	int num_n = fr_vis.get_n().size(),
		num_brdf = basis_matr->fr_brdf.get_n().size();
	int	inc_one = 1;

	int aligned_length = Un_SSE.row_aligned;
	SSE_vector vis_n_coef_SSE_ch(aligned_length * 3);
	SSE_vector vis_n_coef_SSE;
	vis_n_coef_SSE.length = Un_SSE.row;
	vis_n_coef_SSE.length_aligned = Un_SSE.row_aligned;

	for (int brdf_o = 0; brdf_o < num_brdf; brdf_o++)
		for (int brdf_i = 0; brdf_i <= brdf_o; brdf_i++)
		{
			int idx_brdf = brdf_idx(brdf_i, brdf_o);

			const Vector3 &wi = basis_matr->fr_brdf.get_n()[brdf_i][0];
			const Vector3 &wo = basis_matr->fr_brdf.get_n()[brdf_o][0];

			fr_vis.lerp(num_vis_wi, idx_vis_wi, weight_vis_wi, wi, false, false);
			fr_vis.lerp(num_vis_wo, idx_vis_wo, weight_vis_wo, wo, false, false);

			//compute the vis normal distr
			float avg_n = 0;
			vis_n_coef_SSE_ch.clear();
			for (int oo = 0; oo < num_vis_wo; oo++)
				for (int ii = 0; ii < num_vis_wi; ii++)
				{
					int idx_double_vis = brdf_idx(idx_vis_wo[oo], idx_vis_wi[ii]);
					float weight = weight_vis_wi[ii] * weight_vis_wo[oo];
					sse_saxpy(vis_n_coef_SSE_ch.v, aligned_length * 3,
						weight, &Un_SSE.m[(idx_double_vis * 3)*aligned_length]);
					avg_n += weight*inv_SVD_scalar[idx_double_vis];
				}

			for (int ch = 0; ch < 3; ch++)
			{
				vis_n_coef_SSE.v = &vis_n_coef_SSE_ch.v[ch*Un_SSE.row_aligned];

				float temp_SSE = sse_sdot(vis_n_coef_SSE.length, vis_n_coef_SSE.v, Vntavgm_scalar_SSE.v) + avg_n*sum_avgn;

				BRDF.v[idx_brdf * 3 + ch] = (temp_SSE*basis_matr->avgm.v[idx_brdf] +
					sse_sdot(vis_n_coef_SSE.length, vis_n_coef_SSE.v, &VntVmUmt_SSE.m[idx_brdf*VntVmUmt_SSE.row_aligned]) +
					avg_n*avgnVmUmt_SSE.v[idx_brdf])*basis_matr->Mc[ch];

#ifdef LOG_MATERIAL
				BRDF.v[idx_brdf * 3 + ch] = (exp(BRDF.v[idx_brdf * 3 + ch]) - 1)*basis_matr->inv_avg;
#endif
			}
		}
	vis_n_coef_SSE.v = NULL;
}

void vis_normal_distr::release()
{
	Un_SSE.release();
	VntVmUmt_SSE.release();
	avgnVmUmt_SSE.release();
	Vntavgm_scalar_SSE.release();

	Un.release();
	Vnt.release();
	avgn.release();
	VntVmUmt.release();
	avgnVmUmt.release();
	Vntavgm_scalar.release();

	//MnVm.release();
	//Mnavgm_scalar.release();
	//MnVm_SSE.release();
	//Mnavgm_scalar_SSE.release();
	BRDF.release();
}