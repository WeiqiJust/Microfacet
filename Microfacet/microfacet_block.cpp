#include "microfacet_block.h"
#include "microfacet_factory.h"
#define MIN_AREA		(2e-3f)//(5e-4f)

void inst_group::clear()
{
	insts.clear();
	mat_ids.clear();
}

int inst_group::idx(const int id)
{
	for (int i = 0; i < mat_ids.size(); i++)
		if (mat_ids[i] == id)
		{
			return i;
		}

	mat_ids.push_back(id);
	insts.resize(mat_ids.size());
	return (int)mat_ids.size() - 1;
}

int inst_group::get_id(const int i) const
{
	return mat_ids[i];
}

void vis_area::set_area(const std::vector<float> &v)
{
	area = v;
}

float vis_area::get_area(const int num,
	const int *idx,
	const float *weight) const
{
	float a = 0.0f;
	for (int i = 0; i < num; i++)
	{
		a += area[idx[i]] * weight[i];
	}
	return max(a, MIN_AREA);
}


void microfacet_block::get_reflectance_BRDF(Vector3 &result, const Vector3 &wi, const Vector3 &wo)
{
	result.x = result.y = result.z = 0;

	int num_idx_brdf;
	int idx[16];
	float weight[16];

	for (int i = 0; i < vis_normal.size(); i++)
	{
		vis_normal_distr &vis_n = vis_normal[i];
		preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(vis_n.basis_id);

		Vector3 refl;
		basis_matr->lerp_brdf(num_idx_brdf, idx, weight, wi, wo);
		for (int i = 0; i < num_idx_brdf; i++)
		{
			for (int ch = 0; ch < 3; ch++)
				refl[ch] += weight[i] * vis_n.BRDF.v[idx[i] * 3 + ch];
		}

		result += refl;
	}
}

void microfacet_block::debug_BRDF_no_lerp(Vector3 &result, Vector3 &result_wi, Vector3 &result_wo,
	const Vector3 &wi, const Vector3 &wo)
{
	preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(vis_normal[0].basis_id);

	int num_idx_brdf;
	int idx[16];
	float weight[16];
	basis_matr->lerp_brdf(num_idx_brdf, idx, weight, wi, wo);

	int idx_max;
	float max_weight = -1;
	for (int i = 0; i < num_idx_brdf; i++)
		if (weight[i] > max_weight)
		{
			weight[i] = max_weight;
			idx_max = idx[i];
		}

	for (int ch = 0; ch < 3; ch++)
		result[ch] += vis_normal[0].BRDF.v[idx_max * 3 + ch];
	result /= basis_matr->inv_avg;

	int sz = basis_matr->fr_brdf.get_n().size();
	for (int i = 0; i < sz; i++)
		if (idx_max >= i*(i + 1) / 2 && idx_max < (i + 1)*(i + 2) / 2)
		{
			int i_wi = idx_max - i*(i + 1) / 2,
				i_wo = i;

			printf_s("\nidx  = %d (%d %d)\nnidx = %d\n",
				idx_max, i_wi, i_wo,
				basis_matr->fr_n.map(Vector3(0, 0, 1)));

			result_wi = basis_matr->fr_brdf.get_n()[i_wi][0];
			result_wo = basis_matr->fr_brdf.get_n()[i_wo][0];
			return;
		}
}

void microfacet_block::release()
{
	for (int i = 0; i < insts.size(); i++)
		SAFE_DELETE(insts[i]);
	insts.clear();
	per_mat_groups.clear();
}

void microfacet_block::draw_with_shader(r_shader *sh, void* mat)
{
	for (int i = 0; i < insts.size(); i++)
	{
		sh->set_shader_input(insts[i]->get_shader_input(), (void*)mat);
		insts[i]->draw_with_shader(sh);
	}
}

//DEBUG
void microfacet_block::get_n_distr_ours(la_vector<float> &result,
	const int ch,
	const parab_frame &fr_vis,
	const int num_vis_wi,
	const int *idx_vis_wi,
	const float *weight_vis_wi,
	const int num_vis_wo,
	const int *idx_vis_wo,
	const float *weight_vis_wo,
	vis_normal_distr &vis_n)
{
	int inc_one = 1;

	la_vector<float> vis_n_coef(vis_n.Un.column);
	vis_n_coef.clear();
	for (int oo = 0; oo < num_vis_wo; oo++)
		for (int ii = 0; ii < num_vis_wi; ii++)
		{
			float alpha = weight_vis_wi[ii] * weight_vis_wo[oo];
			saxpy(&vis_n_coef.length, &alpha,
				&vis_n.Un.m[(idx_vis_wi[ii] + idx_vis_wo[oo] * fr_vis.get_n().size()) * 3 + ch], &vis_n.Un.row,
				vis_n_coef.v, &inc_one);
		}

	memcpy(result.v, vis_n.avgn.v, sizeof(float)*vis_n.avgn.length);
	for (int i = 0; i < vis_n_coef.length; i++)
		saxpy(&vis_n.avgn.length, &vis_n_coef.v[i], &vis_n.Vnt.m[i], &vis_n.Vnt.row,
		result.v, &inc_one);
}

//DEBUG
void microfacet_block::get_n_distr_points(la_vector<float> &result,
	const int ch,
	const double_parab_frame &fr_n,
	const parab_frame &fr_vis,
	const int num_vis_wi,
	const int *idx_vis_wi,
	const float *weight_vis_wi,
	const int num_vis_wo,
	const int *idx_vis_wo,
	const float *weight_vis_wo,
	vis_normal_distr &vis_n)
{
	const std::vector<float> &weight = vis_n.double_vis_matr;
	const std::vector<normal_weight> &n = vis_n.pts;
	int len = fr_vis.get_n().size()*fr_vis.get_n().size();

	result.clear();
	for (int i = 0; i < n.size(); i++)
	{
		int n_num, n_idx[4];
		float n_weight[4];
		fr_n.lerp(n_num, n_idx, n_weight, n[i].normal, false);

		for (int j = 0; j < n_num; j++)
		{
			float temp = 0;
			for (int oo = 0; oo < num_vis_wo; oo++)
				for (int ii = 0; ii < num_vis_wi; ii++)
				{
					float	alpha = weight_vis_wi[ii] * weight_vis_wo[oo];
					int		idx = idx_vis_wi[ii] + idx_vis_wo[oo] * fr_vis.get_n().size();

					temp += alpha*weight[i*len * 3 + ch*len + idx];
				}

			result.v[n_idx[j]] += temp*n_weight[j];
		}
	}
}

//DEBUG
float microfacet_block::n_distr_to_reflectance_ours(const Vector3 &wi, const Vector3 &wo,
	const la_vector<float> &n_distr,
	preconv_matr *p_basis_matr)
{
	int num_brdf_wi, num_brdf_wo;
	int idx_brdf_wi[4], idx_brdf_wo[4];
	float weight_brdf_wi[4], weight_brdf_wo[4];
	p_basis_matr->fr_brdf.lerp(num_brdf_wi, idx_brdf_wi, weight_brdf_wi, wi, true, false);
	p_basis_matr->fr_brdf.lerp(num_brdf_wo, idx_brdf_wo, weight_brdf_wo, wo, true, false);

	int inc_one = 1;
	float result = 0;
	la_vector<float> n_b_coef(p_basis_matr->Vm.column);
	for (int i = 0; i < n_distr.length; i++)
	{
		for (int j = 0; j < n_b_coef.length; j++)
			n_b_coef.v[j] = p_basis_matr->Vm.m[i + j*p_basis_matr->Vm.row];

		float r = 0;
		for (int i_wo = 0; i_wo < num_brdf_wo; i_wo++)
			for (int i_wi = 0; i_wi < num_brdf_wi; i_wi++)
			{
				float temp = 0;
				temp = sdot(&n_b_coef.length, n_b_coef.v, &inc_one,
					&p_basis_matr->Umt.m[brdf_idx(idx_brdf_wi[i_wi], idx_brdf_wo[i_wo])*p_basis_matr->Umt.row], &inc_one);
				temp += p_basis_matr->avgm.v[brdf_idx(idx_brdf_wi[i_wi], idx_brdf_wo[i_wo])] * p_basis_matr->inv_n_area.v[i];

				r += temp*weight_brdf_wi[i_wi] * weight_brdf_wo[i_wo];
			}
		result += r*n_distr.v[i];
	}

	return result;
}

//DEBUG
void microfacet_block::get_reflectance_assembler(Vector3 &result,
	const Vector3 &wi, const Vector3 &wo,
	const parab_frame &fr_vis, const bool b_debug)
{
	int num_vis_wi, num_vis_wo;
	int idx_vis_wi[4], idx_vis_wo[4];
	float weight_vis_wi[4], weight_vis_wo[4];
	fr_vis.lerp(num_vis_wi, idx_vis_wi, weight_vis_wi, wi, false, false);
	fr_vis.lerp(num_vis_wo, idx_vis_wo, weight_vis_wo, wo, false, false);

	result.x = result.y = result.z = 0;

	for (int i = 0; i < vis_normal.size(); i++)
	{
		vis_normal_distr &vis_n = vis_normal[i];
		preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(vis_n.basis_id);

		la_vector<float> n_distr(basis_matr->fr_n.get_n().size());
		for (int ch = 0; ch < 3; ch++)
		{
			//get_n_distr_ours(n_distr, ch, fr_vis, 
			//	num_vis_wi, idx_vis_wi, weight_vis_wi,
			//	num_vis_wo, idx_vis_wo, weight_vis_wo, vis_n);
			get_n_distr_points(n_distr, ch, basis_matr->fr_n, fr_vis,
				num_vis_wi, idx_vis_wi, weight_vis_wi,
				num_vis_wo, idx_vis_wo, weight_vis_wo, vis_n);
			result[ch] += n_distr_to_reflectance_ours(wi, wo, n_distr, basis_matr)*basis_matr->Mc[ch];
		}
	}

	if (b_debug)
		printf_s("result asm : %g %g %g\n", result.x, result.y, result.z);
}

#include <xmmintrin.h>	// Need this for SSE compiler intrinsics
#include <intrin.h>
#include <cmath>

//FIX ME: the following code does not handle exceptional case when num_idx_brdf % 4 != 0
float SSE_compute_reflectance(const int num_idx_brdf,
	const float *weight,
	const float temp_SSE,
	const SSE_vector &vis_n_coef_SSE,
	const SSE_matrix &VntVmUmt_SSE,
	const SSE_vector &avgm_SSE,
	const SSE_vector &avgnVmUmt_SSE
	)
{
	const int sse_length = num_idx_brdf / 4;
	const float *px = avgm_SSE.v,
		*py = avgnVmUmt_SSE.v,
		*pw = weight;
	__m128	result = _mm_set_ps1(0),
		temp_sse = _mm_set_ps1(temp_SSE);

	for (int i = 0; i < sse_length; i++)
	{
		__m128	x_sse = _mm_load_ps(px),
			y_sse = _mm_load_ps(py),
			w_sse = _mm_load_ps(pw);
		__m128 temp1 = _mm_add_ps(_mm_mul_ps(x_sse, temp_sse), y_sse);

		__m128 temp2 = _mm_set_ps1(0);
		for (int j = 0; j < VntVmUmt_SSE.column; j++)
		{
			__m128 m = _mm_load_ps(&VntVmUmt_SSE.m[j*VntVmUmt_SSE.row_aligned + i * 4]);
			__m128 dot = _mm_mul_ps(m, _mm_set_ps1(vis_n_coef_SSE.v[j]));
			//dot = _mm_hadd_ps(dot, dot);
			temp2 = _mm_add_ps(temp2, dot);
		}
		//temp2 = _mm_hadd_ps(temp2, temp2);

		__m128 temp = _mm_mul_ps(_mm_add_ps(temp1, temp2), w_sse);
		temp = _mm_hadd_ps(temp, temp);
		result = _mm_add_ps(result, temp);

		px += 4;
		py += 4;
		pw += 4;
	}

	result = _mm_hadd_ps(result, result);

	float final_result;
	_mm_store_ss(&final_result, result);
	return final_result;

	//for (int j = 0; j < num_idx_brdf; j++)
	//{
	//	refl.v[ch] += (temp_SSE*basis_matr->avgm.v[idx[j]] + 
	//		//sse_sdot(vis_n_coef_SSE.length, vis_n_coef_SSE.v, &vis_n.VntVmUmt_SSE.m[idx[j]*vis_n.VntVmUmt_SSE.row_aligned]) +
	//		vis_n.avgnVmUmt_SSE.v[idx[j]]) 
	//		* weight[j];
	//}
}

#define USE_SSE_IN_REFL

double t_init, t_init_loop, t_basis_coef1, t_basis_coef2, t_basis_lerp, t_refl,
t_total;
//codex::utils::timer tmr;


void microfacet_block::get_reflectance(Vector3 &result, const Vector3 &wi, const Vector3 &wo,
	const parab_frame &fr_vis)
{
	//tmr.update();
	int num_vis_wi, num_vis_wo;
	int idx_vis_wi[4], idx_vis_wo[4];
	float weight_vis_wi[4], weight_vis_wo[4];
	fr_vis.lerp(num_vis_wi, idx_vis_wi, weight_vis_wi, wi, false, false);
	fr_vis.lerp(num_vis_wo, idx_vis_wo, weight_vis_wo, wo, false, false);
	int num_n = fr_vis.get_n().size();

	int num_idx_brdf;
	int idx[16];
	__declspec(align(16)) float weight[16];

	int	inc_one = 1;
	result.x = result.y = result.z = 0;
	//tmr.update();
	//t_init += tmr.elapsed_time();

	for (int i = 0; i < vis_normal.size(); i++)
	{
		//tmr.update();
		vis_normal_distr &vis_n = vis_normal[i];
		preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(vis_n.basis_id);

		int aligned_length = vis_n.Un_SSE.row_aligned;
		SSE_vector vis_n_coef_SSE_ch(aligned_length * 3);

		Vector3 refl;
		//tmr.update();
		//t_init_loop += tmr.elapsed_time();

		basis_matr->lerp_brdf(num_idx_brdf, idx, weight, wi, wo);
		//tmr.update();
		//t_basis_lerp += tmr.elapsed_time();

		float avg_n = 0;
		vis_n_coef_SSE_ch.clear();
		for (int oo = 0; oo < num_vis_wo; oo++)
			for (int ii = 0; ii < num_vis_wi; ii++)
			{
				int idx_double_vis = brdf_idx(idx_vis_wo[oo], idx_vis_wi[ii]);
				float weight = weight_vis_wi[ii] * weight_vis_wo[oo];
				sse_saxpy(vis_n_coef_SSE_ch.v, aligned_length * 3,
					weight,
					&vis_n.Un_SSE.m[(idx_double_vis * 3)*aligned_length]);
#ifdef USE_SVD_SCALAR
				avg_n += weight*vis_n.inv_sUn.v[idx_double_vis];
#else
				avg_n += weight*inv_SVD_scalar[idx_double_vis];
#endif
			}
		//tmr.update();
		//t_basis_coef1 += tmr.elapsed_time();

		//SSE_matrix VntVmUmt_SSE(num_idx_brdf, vis_n.VntVmUmt.row);
		//SSE_vector avgm_SSE(num_idx_brdf), avgnVmUmt_SSE(num_idx_brdf);
		//for (int j = 0; j < num_idx_brdf; j++)
		//{
		//	for (int k = 0; k < VntVmUmt_SSE.column; k++)
		//		VntVmUmt_SSE.m[k*VntVmUmt_SSE.row_aligned+j] = vis_n.VntVmUmt.m[idx[j]*vis_n.VntVmUmt.row+k];
		//	avgm_SSE.v[j]		= basis_matr->avgm.v[idx[j]];
		//	avgnVmUmt_SSE.v[j]	= vis_n.avgnVmUmt_SSE.v[idx[j]];
		//}
		//tmr.update();
		//t_basis_coef2 += tmr.elapsed_time();

		SSE_vector vis_n_coef_SSE;
		vis_n_coef_SSE.length = vis_n.Un_SSE.row;
		vis_n_coef_SSE.length_aligned = vis_n.Un_SSE.row_aligned;

		for (int ch = 0; ch < 3; ch++)
		{
			vis_n_coef_SSE.v = &vis_n_coef_SSE_ch.v[ch*vis_n.Un_SSE.row_aligned];

			float temp_SSE = sse_sdot(vis_n_coef_SSE.length, vis_n_coef_SSE.v, vis_n.Vntavgm_scalar_SSE.v) + avg_n*vis_n.sum_avgn;
			for (int j = 0; j < num_idx_brdf; j++)
			{
				refl[ch] += (temp_SSE*basis_matr->avgm.v[idx[j]] +
					sse_sdot(vis_n_coef_SSE.length, vis_n_coef_SSE.v, &vis_n.VntVmUmt_SSE.m[idx[j] * vis_n.VntVmUmt_SSE.row_aligned]) +
					avg_n*vis_n.avgnVmUmt_SSE.v[idx[j]])
					* weight[j];
			}
			//refl.v[ch] = SSE_compute_reflectance(num_idx_brdf, weight, temp_SSE,
			//	vis_n_coef_SSE, VntVmUmt_SSE, avgm_SSE, avgnVmUmt_SSE);
			result[ch] += refl[ch] * basis_matr->Mc[ch];
		}
		vis_n_coef_SSE.v = NULL;

		//tmr.update();
		//t_refl += tmr.elapsed_time();
	}
}

void microfacet_block::get_reflectance_debug(Vector3 &result,
	const Vector3 &wi, const Vector3 &wo,
	const parab_frame &fr_vis, const bool b_debug)
{
	int num_vis_wi, num_vis_wo;
	int idx_vis_wi[4], idx_vis_wo[4];
	float weight_vis_wi[4], weight_vis_wo[4];
	fr_vis.lerp(num_vis_wi, idx_vis_wi, weight_vis_wi, wi, false, false);
	fr_vis.lerp(num_vis_wo, idx_vis_wo, weight_vis_wo, wo, false, false);

	const std::vector<normal_weight> &n = vis_normal[0].pts;
	const std::vector<Vector3> &p = vis_normal[0].pos;
	const std::vector<float> &weight = vis_normal[0].double_vis_matr;

	preconv_matr *p_basis_matr = mff_singleton::get()->get_basis_matr(BASIS_MATR_LAMBERTIAN);
	std::vector<float> n_distr;
	if (b_debug)
	{
		n_distr.resize(p_basis_matr->fr_n.get_back_idx().size() * 3, 0.0f);
	}

	int num_brdf_wi, num_brdf_wo;
	int idx_brdf_wi[4], idx_brdf_wo[4];
	float weight_brdf_wi[4], weight_brdf_wo[4];
	p_basis_matr->fr_brdf.lerp(num_brdf_wi, idx_brdf_wi, weight_brdf_wi, wi, true, false);
	p_basis_matr->fr_brdf.lerp(num_brdf_wo, idx_brdf_wo, weight_brdf_wo, wo, true, false);

	//if (b_debug)
	//{
	//	printf_s("wi ");
	//	for (int j = 0; j < num_vis_wi; j++)
	//		printf_s("%d:%4.2f ", idx_vis_wi[j], weight_vis_wi[j]);
	//	printf_s("\n");
	//	printf_s("wo ");
	//	for (int j = 0; j < num_vis_wo; j++)
	//		printf_s("%d:%4.2f ", idx_vis_wo[j], weight_vis_wo[j]);
	//	printf_s("\n");
	//	printf_s("brdf_i ");
	//	for (int j = 0; j < num_brdf_wi; j++)
	//		printf_s("%d:%4.2f ", idx_brdf_wi[j], weight_brdf_wi[j]);
	//	printf_s("\n");
	//	printf_s("brdf_o ");
	//	for (int j = 0; j < num_brdf_wo; j++)
	//		printf_s("%d:%4.2f ", idx_brdf_wo[j], weight_brdf_wo[j]);
	//	printf_s("\n");
	//}

	//DEBUG
	Vector3 result_dbg;
	std::vector<Vector3> pos;

	int len = fr_vis.get_n().size()*fr_vis.get_n().size();
	for (int i = 0; i < n.size(); i++)
	{
		Vector3 r;
		p_basis_matr->pBRDF->sample(r, wi, wo, n[i].normal);

		Vector3 refl;
		for (int oo = 0; oo < num_vis_wo; oo++)
			for (int ii = 0; ii < num_vis_wi; ii++)
			{
				float	alpha = weight_vis_wi[ii] * weight_vis_wo[oo];
				int		idx = idx_vis_wi[ii] + idx_vis_wo[oo] * fr_vis.get_n().size();

				for (int ch = 0; ch < 3; ch++)
					refl[ch] += alpha*weight[i*len * 3 + ch*len + idx] * r[ch] * p_basis_matr->inv_avg;
			}

		if (b_debug)
		{
			int n_num, n_idx[4];
			float n_weight[4];
			p_basis_matr->fr_n.lerp(n_num, n_idx, n_weight, n[i].normal, false);

			for (int j = 0; j < n_num; j++)
			{
				n_distr[n_idx[j] * 3 + 0] += n_weight[j] * refl.x;
				n_distr[n_idx[j] * 3 + 1] += n_weight[j] * refl.y;
				n_distr[n_idx[j] * 3 + 2] += n_weight[j] * refl.z;
			}

			Vector3 refl_dbg;
			la_vector<float> n_b_coef(p_basis_matr->Vm.column);
			n_b_coef.clear();
			int inc_one = 1;
			for (int j = 0; j < n_num; j++)
				saxpy(&n_b_coef.length, &n_weight[j], &p_basis_matr->Vm.m[n_idx[j]], &p_basis_matr->Vm.row,
				n_b_coef.v, &inc_one);

			float wt = 0;
			for (int j = 0; j < n_num; j++)
				wt += n_weight[j] * p_basis_matr->inv_n_area.v[n_idx[j]];

			float rr = 0;
			for (int i_wo = 0; i_wo < num_brdf_wo; i_wo++)
				for (int i_wi = 0; i_wi < num_brdf_wi; i_wi++)
				{
					float temp = 0;
					temp = sdot(&n_b_coef.length, n_b_coef.v, &inc_one,
						&p_basis_matr->Umt.m[brdf_idx(idx_brdf_wi[i_wi], idx_brdf_wo[i_wo])*p_basis_matr->Umt.row], &inc_one);
					temp += p_basis_matr->avgm.v[brdf_idx(idx_brdf_wi[i_wi], idx_brdf_wo[i_wo])] * wt;

					rr += temp*weight_brdf_wi[i_wi] * weight_brdf_wo[i_wo];
				}

			for (int oo = 0; oo < num_vis_wo; oo++)
				for (int ii = 0; ii < num_vis_wi; ii++)
				{
					float	alpha = weight_vis_wi[ii] * weight_vis_wo[oo];
					int		idx = idx_vis_wi[ii] + idx_vis_wo[oo] * fr_vis.get_n().size();

					for (int ch = 0; ch < 3; ch++)
						refl_dbg[ch] += alpha*weight[i*len * 3 + ch*len + idx] * rr*p_basis_matr->Mc[ch];
				}
			result_dbg += refl_dbg;

			if (refl.Length() > 0)
			{
				pos.push_back(p[i]);
			}
		}

		result += refl;
	}

	if (b_debug)
	{
		std::vector<float> img;
		img.resize(p_basis_matr->fr_n.get_idx().size() * 3, 0.5);
		float scalar = 128;
		for (int i = 0; i < p_basis_matr->fr_n.get_idx().size(); i++)
		{
			int ii = p_basis_matr->fr_n.get_idx()[i];
			if (ii >= 0)
			{
				img[i * 3 + 0] = n_distr[ii * 3 + 0] * scalar;
				img[i * 3 + 1] = n_distr[ii * 3 + 1] * scalar;
				img[i * 3 + 2] = n_distr[ii * 3 + 2] * scalar;
			}
		}
		save_image_color("T:/Microfacet/avg_n_ours.png", img,
			p_basis_matr->fr_n.get_actual_dim(), p_basis_matr->fr_n.get_actual_dim() * 2);

		printf_s("result     : %g %g %g\n", result.x, result.y, result.z);
		printf_s("result_dbg : %g %g %g\n", result_dbg.x, result_dbg.y, result_dbg.z);

		FILE *fp;
		fp = fopen("T:/Microfacet/pts.txt", "wt");
		fprintf_s(fp, "%d\n", pos.size());
		for (int i = 0; i < pos.size(); i++)
			fprintf_s(fp, "%g %g %g\n", pos[i].x, pos[i].y, pos[i].z);
		fclose(fp);
	}
}

void microfacet_block::save_BRDF_for_fit(const char *filename, const parab_frame &fr_Avis)
{
	preconv_matr *spec_basis_matr = NULL;
	for (int i = 0; i < per_mat_groups.insts.size(); i++)
	{
		int id = per_mat_groups.get_id(i);
		matr* p_matr = mff_singleton::get()->get_matr(id);
		preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

		if (spec_basis_matr == NULL ||
			basis_matr->fr_brdf.get_n().size() > spec_basis_matr->fr_brdf.get_n().size())
			spec_basis_matr = basis_matr;
	}
	int brdf_dim = spec_basis_matr->fr_brdf.get_n().size();
	const parab_frame &fr = spec_basis_matr->fr_brdf;

	int num_area;
	int idx_area[4];
	float weight_area[4];

	FILE *fp;
	fp = fopen(filename, "wb");
	fwrite(&brdf_dim, sizeof(int), 1, fp);

	for (int i_wo = 0; i_wo < brdf_dim; i_wo++)
	{
		fr_Avis.lerp(num_area, idx_area, weight_area, fr.get_n()[i_wo][0], true, false);
		float area = Avis.get_area(num_area, idx_area, weight_area);

		for (int i_wi = 0; i_wi <= i_wo; i_wi++)
		{
			Vector3 ours, wi, wo;
			wi = fr.get_n()[i_wi][0];
			wo = fr.get_n()[i_wo][0];
			get_reflectance_BRDF(ours, wi, wo);
			ours /= area*wi.z;

			fwrite(&ours.x, sizeof(float), 1, fp);
			fwrite(&ours.y, sizeof(float), 1, fp);
			fwrite(&ours.z, sizeof(float), 1, fp);
			fwrite(&wi.x, sizeof(float), 1, fp);
			fwrite(&wi.y, sizeof(float), 1, fp);
			fwrite(&wi.z, sizeof(float), 1, fp);
			fwrite(&wo.x, sizeof(float), 1, fp);
			fwrite(&wo.y, sizeof(float), 1, fp);
			fwrite(&wo.z, sizeof(float), 1, fp);
			double weight = fr.get_area()[i_wi] * fr.get_area()[i_wo] * wi.z*wo.z;
			//double weight = wi.z*wo.z;
			fwrite(&weight, sizeof(double), 1, fp);
		}
	}

	fclose(fp);
}

void microfacet_block::save_BRDF_truth(const char *filename)
{
	FILE *fp;
	fp = fopen(filename, "wb");
	int len = BRDF_truth.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&BRDF_truth[0], BRDF_truth.size()*sizeof(float), 1, fp);
	fclose(fp);
}

void microfacet_block::load_BRDF_truth(const char *filename)
{
	FILE *fp;
	fp = fopen(filename, "rb");
	int len;
	fread(&len, sizeof(len), 1, fp);
	BRDF_truth.resize(len);
	fread(&BRDF_truth[0], BRDF_truth.size()*sizeof(float), 1, fp);
	fclose(fp);
}
