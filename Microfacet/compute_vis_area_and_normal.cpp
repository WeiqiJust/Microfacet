#include "utils.h"
#include "mkl.h"
#include "la_math.h"
#include "microfacet.h"

//#define DEBUG_RENDERING
//#define OUTPUT_DIRECTIONAL_VISIBLE_POINTS
//#define OUTPUT_VISIBLE_POINTS
//#define OUTPUT_ALL_POINTS

void construct_matrix(la_matrix<float> &mat_vis_n,
					  std::vector<BYTE> &mat_vis,
					  std::vector<bool> &b_pt_vis,
					  const int pt_size,
					  const int total_pts,
					  std::vector<float> &single_vis_float, 
					  std::vector<float> &double_vis_float,
					  const int num_vis_dir,
					  const int sample_idx, //=samp_idx[i_samples]
					  sample_group &sample, //samples[i_samples]
					  matr *p_matr,
					  preconv_matr *basis_matr)
{
	int inc_one = 1, inc_three = 3, inc_zero = 0;
	float one = 1.0f;

	float color_dot = 0.0f, color[3];
	for (int j = 0; j < 3;j ++)
	{
		color[j] = p_matr->albedo[j];
		color_dot += color[j];
	}

	bit_vector<BYTE> bv;
	for (int j = 0; j < pt_size; j++)
		if (b_pt_vis[sample_idx+j])
		{
			bv.init(&mat_vis[sample_idx+j], num_vis_dir, total_pts);
			bv.self_multiplied_matrix(&double_vis_float[0], sample.n[j].weight, &single_vis_float[0]);

			int n_idx[4], num;
			float n_weight[4];
			basis_matr->fr_n.lerp(num, n_idx, n_weight, sample.n[j].normal, false);

			//DEBUG
			//num = 1;
			//n_idx[0]	= basis_matr->fr_n.map(sample.n[j].normal);
			//n_weight[0]	= 1.0f;

			int len = num_vis_dir*(num_vis_dir+1)/2;
			for (int k = 0; k < num; k++)
			{
				int nidx = n_idx[k];
				for (int ch = 0; ch < 3; ch++)
				{
					float alpha = n_weight[k]*color[ch];
					saxpy(&len, &alpha, &double_vis_float[0], &inc_one, &mat_vis_n.m[nidx*len*3+ch], &inc_three);
				}
			}

#ifdef DEBUG_RENDERING
			//The code is for render before SVD, etc...
			vis_normal.pos.push_back(sample.p[j]);// samples[i_samples].p[j]);
			vis_normal.pts.push_back(sample.n[j]);// samples[i_samples].n[j]);
			int offset = vis_normal.double_vis_matr.size();
			vis_normal.double_vis_matr.resize(offset+len*3);

			for (int k = 0; k < num_vis_dir; k++)
			{
				float alpha = (sample_Avis[k] > 0) ? sample_Avis[k] : 0.0f;
				sscal(&num_vis_dir, &alpha, &double_vis_float[k*num_vis_dir], &inc_one);
			}

			for (int ch = 0; ch < 3; ch++)
			{
				float alpha = p_matr->albedo[ch];
				saxpy(&len, &alpha, &double_vis_float[0], &inc_one, &vis_normal.double_vis_matr[offset+len*ch], &inc_one);
			}
#endif
		}
}

void compute_Avis(microfacet_block &block,
				  const parab_frame &fr_Avis,
				  const parab_frame &fr_vis,
				  std::vector<BYTE> &mat_vis,
				  std::vector<bool> &b_pt_vis,
				  const int num_vis_dir,
				  const int total_pts,
				  std::vector<float> &sample_Avis,
				  std::vector<float> &SVD_scalar,
				  std::vector<float> &inv_SVD_scalar,
				  std::vector<float> &area,
				  std::vector<sample_group> &samples,
				  int x, int y,
				  batch_area_sampler &avis_sampler)
{
	//******* GPU code for computing A_vis. Should do it in two parts. One is to render, the second is to gather the result. ******* 
	//std::vector<Vector3> dirs;
	//for (int i = 0; i < fr_vis.get_n().size(); i++)
	//	dirs.push_back(fr_vis.get_n()[i][0]);
	//while ((dirs.size() % NUM_AREA_DIR_PER_BATCH) != 0)
	//	dirs.push_back(Vector3(0, 0, 0));
	//std::vector<float> area_vis;
	//area_vis.resize(dirs.size());

	//codex::utils::timer t;
	//Vector3 lookat(x+0.5f, y+0.5f, 0);
	//mff_singleton::get()->get_layer("pos")->set();
	//for (int i = 0; i < dirs.size()/NUM_AREA_DIR_PER_BATCH; i++)
	//{
	//	avis_sampler.begin_sample(lookat, &dirs[i*NUM_AREA_DIR_PER_BATCH]);
	//	avis_sampler.get_shader()->setup_cb_attr(1);
	//	block.draw_with_shader(avis_sampler.get_shader(), NULL);
	//	avis_sampler.get_shader()->setup_cb_attr(0);
	//	for (int j = 1; j < block.neighbors_and_self.size(); j++)
	//		block.neighbors_and_self[j]->draw_with_shader(avis_sampler.get_shader(), NULL);
	//	avis_sampler.end_sample();

	//	avis_sampler.get_area(&area_vis[i*NUM_AREA_DIR_PER_BATCH]);
	//}

	//int		num_v, idx_v[4];
	//float	weight_v[4];
	//UINT	offset[4], mask[4];

	//for (int j = 0; j < area.size(); j++)
	//{
	//	const Vector3 &v = fr_Avis.get_n()[j][0];

	//	fr_vis.lerp(num_v, idx_v, weight_v, v, false, false);

	//	area[j] = 0.0f;
	//	for (int k = 0; k < num_v; k++)
	//		area[j] += area_vis[idx_v[k]]*weight_v[k];
	//}

	int		num_v, idx_v[4];
	float	weight_v[4];
	UINT	offset[4], mask[4];

	for (int j = 0; j < area.size(); j++)
	{
		const Vector3 &v = fr_Avis.get_n()[j][0];

		fr_vis.lerp(num_v, idx_v, weight_v, v, false, false);

		for (int k = 0; k < num_v; k++)
		{
			offset[k]	= idx_v[k] / NUM_VIS_PER_BATCH,
			mask[k]		= 1 << (idx_v[k] % NUM_VIS_PER_BATCH);
		}

		//debug
		//int vis_pts = 0;

		int pt_offset = 0;
		area[j] = 0.0f;
		for (int p = 0; p < samples.size(); p++)
		{
			int sz = samples[p].p.size();
			for (int k = 0; k < sz; k++)
				if (b_pt_vis[pt_offset+k])
				{
					float val = max(0.0f, Dot(v, samples[p].n[k].normal));
					if (val > 0)
					{
						float sum_weight = 0.0f;
						for (int l = 0; l < num_v; l++)
							if (mat_vis[offset[l]*total_pts+pt_offset+k] & mask[l])
								sum_weight += weight_v[l];

						area[j] += sum_weight*val*samples[p].n[k].weight;						
					}
					//vis_pts++;
				}
			pt_offset += sz;
		}
		//printf_s("%d\n", vis_pts);
	}
	block.Avis.set_area(area);

	//compute sample_Avis
	int a_idx[4], a_num;
	float a_weight[4];
	sample_Avis.resize(num_vis_dir);
	for (int k = 0; k < num_vis_dir; k++)
	{
		fr_Avis.lerp(a_num, a_idx, a_weight, fr_vis.get_n()[k][0], true, false);
		sample_Avis[k] = block.Avis.get_area(a_num, a_idx, a_weight);
	}

	//compute SVD_scalar
	SVD_scalar.resize(num_vis_dir*(num_vis_dir+1)/2);
	inv_SVD_scalar.resize(num_vis_dir*(num_vis_dir+1)/2);
	for (int i_wo = 0; i_wo < num_vis_dir; i_wo++)
		for (int i_wi = 0; i_wi <= i_wo; i_wi++)
		{
			int idx = brdf_idx(i_wi, i_wo);
			
			float a1, a2;
			a1 = (sample_Avis[i_wi] > 0) ? 1.0f/sample_Avis[i_wi] : 0;
			a2 = (sample_Avis[i_wo] > 0) ? 1.0f/sample_Avis[i_wo] : 0;

			inv_SVD_scalar[idx] = a1*a1+a2*a2;
			if (i_wi == i_wo)
				inv_SVD_scalar[idx] /= 2;
			inv_SVD_scalar[idx] = sqrt(inv_SVD_scalar[idx]);
			SVD_scalar[idx] = (inv_SVD_scalar[idx] > 0) ? 1.0 / inv_SVD_scalar[idx] : 0;
		}
}

void compute_vis_area_and_normal(microfacet_details &details,
											 const std::vector<int> &idx_blocks,
											 std::vector<microfacet_block> &blocks,
											 const parab_frame &fr_Avis,
											 const parab_frame &fr_vis,
											 batch_area_sampler &avis_sampler,
											 vis_point_sampler &vismask_sampler,
											 rp_solver &rp)
{
	//codex::utils::timer tm;

	//codex::utils::timer t;
	//Common vars
	int		inc_one = 1, inc_zero = 0, inc_three = 3;
	float	one = 1.0f, zero = 0.0f;
	int		num_vis_dir = fr_vis.get_n().size();
	
	std::vector<float>	single_vis_float, double_vis_float;
	std::vector<float> area;
	single_vis_float.resize(num_vis_dir);
	double_vis_float.resize(num_vis_dir*(num_vis_dir+1)/2);
	area.resize(fr_Avis.get_n().size());

	for (int xx = 0; xx < idx_blocks.size(); xx++)
		if (details.in_boundary(idx_blocks[xx]))
		{
			std::vector<sample_group>	samples;
			std::vector<int>			samp_idx;
			int							total_pts, x, y;
			microfacet_block			&block = blocks[idx_blocks[xx]];
			details.compute_back_idx(x, y, idx_blocks[xx]);

			//*****************************************************
			// Generate depth maps
			//*****************************************************
			//t.update();
			r_shader_basic *p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
			mff_singleton::get()->get_layer("basic")->set();
			
			vismask_sampler.set_scene_center(Vector3(x+0.5f, y+0.5f, 0));
			for (int l = 0; l < vismask_sampler.num_views(); l++)
			{
				Matrix4 matLight;
				vismask_sampler.begin_gen_shadow(l, matLight);
				for (int i = 0; i < block.neighbors_and_self.size(); i++)
					block.neighbors_and_self[i]->draw_with_shader(p_sh_shadow, &matLight);
				vismask_sampler.end_gen_shadow(l);
			}
			//t.update();
			//printf_s("depth %gsecs\n", t.elapsed_time());
			printf_s("depth map generation finished\n");
			//*****************************************************

			//*****************************************************
			// Sample object-space points
			//*****************************************************
			printf_s("getting samples...");
			details.get_samples(samples, samp_idx, block);
			total_pts = samp_idx[samp_idx.size()-1];
			printf_s("done.\n");
			//t.update();
			//printf_s("pts %gsecs\n", t.elapsed_time());
			//*****************************************************

			//*****************************************************
			// Sample visibility
			//*****************************************************
			std::vector<BYTE> mat_vis;
			std::vector<bool> b_pt_vis;
			int num_vis_batch = (num_vis_dir+NUM_VIS_PER_BATCH-1)/NUM_VIS_PER_BATCH;
			mat_vis.resize(num_vis_batch*total_pts);
			b_pt_vis.resize(total_pts, false);

			for (int i = 0; i < num_vis_batch; i++)
			{
				vismask_sampler.compute_vis_masks(samples, i*NUM_VIS_PER_BATCH, NUM_VIS_PER_BATCH);
				vismask_sampler.get_vis_masks(&mat_vis[i*total_pts], total_pts);
				for (int j = 0; j < total_pts; j++)
					if (b_pt_vis[j] == false && mat_vis[i*total_pts+j] != 0)
						b_pt_vis[j] = true;
			}
			//t.update();
			//printf_s("vis %gsecs\n", t.elapsed_time());
			int count = 0;
			for (int i = 0; i < mat_vis.size(); i++)
			{
				if (mat_vis[i] != 0)
					count++;
			}
			printf_s("vis generation mat_vis non-zero size = %d\n", count);
			//*****************************************************

			//*****************************************************
			// Sample A_vis
			//*****************************************************
			std::vector<float> sample_Avis, SVD_scalar;
			std::vector<float> &inv_SVD_scalar = block.inv_SVD_scalar;
			compute_Avis(block,	fr_Avis, fr_vis, mat_vis, b_pt_vis,
				num_vis_dir, total_pts, sample_Avis, 
				SVD_scalar, inv_SVD_scalar, area, samples, x, y, avis_sampler);
			//t.update();
			//printf_s("Avis %gsecs\n", t.elapsed_time());
			printf_s("Avis generation\n");

			//Group samples that have the same basis materials
			int last_basis_id = -1;
			int num_vis_normal = 0;
			for (int i = 0; i < samples.size(); i++)
			{
				matr* p_matr = mff_singleton::get()->get_matr(samples[i].mat_id);
				if (p_matr->basis_id != last_basis_id)
				{
					last_basis_id = p_matr->basis_id;
					num_vis_normal++;
				}
			}
			if (block.vis_normal.size() != num_vis_normal)
			{
				for (int i = 0; i < block.vis_normal.size(); i++)
					block.vis_normal[i].release();
				block.vis_normal.resize(num_vis_normal);
			}

			int i_vis_normal, i_samples;
			i_vis_normal = i_samples = 0;
			
			while (i_vis_normal < num_vis_normal)
			{
				//t.update();
				last_basis_id = mff_singleton::get()->get_matr(samples[i_samples].mat_id)->basis_id;
				preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(last_basis_id);
				vis_normal_distr &vis_normal = block.vis_normal[i_vis_normal];
				i_vis_normal++;
				int n_size = basis_matr->fr_n.get_back_idx().size();
				vis_normal.basis_id = last_basis_id;

#ifdef DEBUG_RENDERING
				vis_normal.pos.clear();
				vis_normal.pts.clear();
				vis_normal.double_vis_matr.clear();
#endif

				////save to image
				//{
				//	std::vector<float> img;
				//	img.resize(basis_matr->fr_n.get_idx().size()*3, 0.5);
				//	for (int i = 0; i < basis_matr->fr_n.get_idx().size(); i++)
				//	{
				//		int ii = basis_matr->fr_n.get_idx()[i];
				//		if (ii >= 0)
				//		{
				//			float v = basis_matr->SVD_scalar.v[ii];
				//			img[i*3] = img[i*3+1] = img[i*3+2] = v;
				//		}
				//	}
				//	save_image_color("d:/temp/Microfacet/SVD_scalar.png", img, basis_matr->fr_n.get_actual_dim(), basis_matr->fr_n.get_actual_dim()*2);

				//	//basis_matr->fr_n.compute_spherical_area(4000000);
				//	//float max_area = 0;
				//	//for (int i = 0; i < basis_matr->fr_n.get_spherical_area().size(); i++)
				//	//{
				//	//	max_area = max(basis_matr->fr_n.get_spherical_area()[i], max_area);
				//	//}

				//	//img.resize(basis_matr->fr_n.get_idx().size()*3, 0.5);
				//	//for (int i = 0; i < basis_matr->fr_n.get_idx().size(); i++)
				//	//{
				//	//	int ii = basis_matr->fr_n.get_idx()[i];
				//	//	if (ii >= 0)
				//	//	{
				//	//		float v = basis_matr->fr_n.get_spherical_area()[ii] / max_area;
				//	//		img[i*3] = img[i*3+1] = img[i*3+2] = v;
				//	//	}
				//	//}
				//	//save_image_color("d:/temp/Microfacet/scalar.png", img, basis_matr->fr_n.get_actual_dim(), basis_matr->fr_n.get_actual_dim()*2);
				//}		

				la_matrix<float> mat_vis_n(num_vis_dir*(num_vis_dir+1)/2*3, n_size);
				la_vector<float> avg_n_distr(n_size);
				mat_vis_n.clear();
				avg_n_distr.clear();

#ifdef USE_SVD_SCALAR
				vis_normal.inv_sUn.release();
				vis_normal.inv_sUn.length = basis_matr->inv_sUn.length;
				vis_normal.inv_sUn.v = new float[vis_normal.inv_sUn.length];
				//FIX ME: faster?
				for (int k = 0; k < num_vis_dir; k++)
					for (int l = 0; l < num_vis_dir; l++)
						vis_normal.inv_sUn.v[k*num_vis_dir+l] = sample_Avis[k]*basis_matr->inv_sUn.v[k*num_vis_dir+l];
#endif

				int batch_pt_size = 0;

				while (i_samples < samples.size())
				{
					int pt_size = samples[i_samples].p.size();
					batch_pt_size += pt_size;
					matr *p_matr = mff_singleton::get()->get_matr(samples[i_samples].mat_id);
					if (last_basis_id != p_matr->basis_id)
						break;
					
					construct_matrix(mat_vis_n, mat_vis,
						b_pt_vis, pt_size, total_pts,
						single_vis_float, double_vis_float,
						num_vis_dir, samp_idx[i_samples], samples[i_samples],
						p_matr, basis_matr);

					////DEBUG
					////wavelet tests
					//float total_s, total_e;
					//int total_number;
					//total_s = total_e = 0;
					//total_number = 0;
					//for (int ii = 0; ii < NUM_VIS_DIR*NUM_VIS_DIR; ii++)
					//{
					//	std::vector<float> vec;
					//	vec.resize(dim*dim);
					//	//int ii = NUM_VIS_DIR*NUM_VIS_DIR/2;
					//	for (int j = 0; j < dim*dim; j++)
					//		vec[j] = mat_vis_n.m[ii+j*NUM_VIS_DIR*NUM_VIS_DIR];

					//	wavelet_image img;
					//	img.wavelet_transform(dim, &vec[0], 0.0025);

					//	float rms_s, rms_e;
					//	img.compute_snr(rms_s, rms_e, dim, &vec[0]);
					//	total_s += rms_s;
					//	total_e += rms_e;
					//	total_number += img.num_coeffs();
					//	//printf_s("%d %gdb\n", img.num_coeffs(), float2db(rms_s/rms_e));
					//}
					//printf_s("%d %gdb\n", total_number / (NUM_VIS_DIR*NUM_VIS_DIR), float2db(total_s/total_e));

					i_samples++;
				}
				//string file = string("T:/Microfacet/output/mat_vis_n") + std::to_string(i_vis_normal) + ".txt";
				//mat_vis_n.save_txt(file.c_str());
				int count_num = 0;
				for (int z = 0; z < mat_vis_n.row*mat_vis_n.column; z++)
				{
					if (mat_vis_n.m[z] != 0.0f)
						count_num++;
				}
				cout << "mat_vis_n non-zero number = " << count_num << endl;
				//t.update();
				//printf_s("mat_vis %d pts %gsecs\n", batch_pt_size, t.elapsed_time());
				printf_s("mat_vis %d pts\n", batch_pt_size);

				////MnVm
				//matrix_mul(vis_normal.MnVm, mat_vis_n, basis_matr->Vm);
				//vis_normal.MnVm_SSE.from_unaligned(vis_normal.MnVm.row, vis_normal.MnVm.column, vis_normal.MnVm.m,
				//	0, true);
				////Mnavgm_scalar
				//vis_normal.Mnavgm_scalar.init(mat_vis_n.row);
				//for (int i = 0; i < vis_normal.Mnavgm_scalar.length; i++)
				//	vis_normal.Mnavgm_scalar.v[i] = sdot(&basis_matr->inv_n_area.length, 
				//		basis_matr->inv_n_area.v, &inc_one, &mat_vis_n.m[i], &mat_vis_n.row);
				//vis_normal.Mnavgm_scalar_SSE.from_unaligned(vis_normal.Mnavgm_scalar.length, vis_normal.Mnavgm_scalar.v);
				//t.update();
				//printf_s("compute %gsecs\n", t.elapsed_time());

				rp.process(vis_normal.Un, vis_normal.Vnt, avg_n_distr, mat_vis_n, 
					&SVD_scalar[0], &inv_SVD_scalar[0], 0.9f
					//0.45f
					);
				//t.update();
				//printf_s("SVD (%d, %d) %gsecs\n", mat_vis_n.row, mat_vis_n.column, t.elapsed_time());
				printf_s("SVD (%d, %d) \n", mat_vis_n.row, mat_vis_n.column);

				vis_normal.avgn.release();
				vis_normal.avgn = avg_n_distr;
				avg_n_distr.v = NULL;
				vis_normal.Un_SSE.from_unaligned(vis_normal.Un.row, vis_normal.Un.column, vis_normal.Un.m, 0.0f, true);

				//VntVmUmt
				la_matrix<float> VntVm;

	
				matrix_mul(VntVm, vis_normal.Vnt, basis_matr->Vm);
				//basis_matr->Vm.save_txt("T:/Microfacet/output/Vm.txt");
				//vis_normal.Vnt.save_txt("T:/Microfacet/output/Vnt.txt");
				//VntVm.save_txt("T:/Microfacet/output/VntVm.txt");
				vis_normal.VntVmUmt.release();
				matrix_mul(vis_normal.VntVmUmt, VntVm, basis_matr->Umt);
				vis_normal.VntVmUmt_SSE.from_unaligned(vis_normal.VntVmUmt.row, vis_normal.VntVmUmt.column, vis_normal.VntVmUmt.m);
				//t.update();
				//printf_s("VntVmUmt %gsecs\n", t.elapsed_time());

				//avgnVmUmt
				la_vector<float> avgnVm;
				vector_matrix_mul(avgnVm, vis_normal.avgn, basis_matr->Vm);
				vis_normal.avgnVmUmt.release();
				vector_matrix_mul(vis_normal.avgnVmUmt, avgnVm, basis_matr->Umt);
				vis_normal.avgnVmUmt_SSE.from_unaligned(vis_normal.avgnVmUmt.length, vis_normal.avgnVmUmt.v);
				//t.update();
				//printf_s("avgnVm %gsecs\n", t.elapsed_time());

				//Vntavgm_scalar
				vis_normal.Vntavgm_scalar.release();
				vis_normal.Vntavgm_scalar.length = vis_normal.Vnt.row;
				vis_normal.Vntavgm_scalar.v = new float[vis_normal.Vntavgm_scalar.length];
				for (int j = 0; j < vis_normal.Vntavgm_scalar.length; j++)
					vis_normal.Vntavgm_scalar.v[j] = 
						sdot(&vis_normal.Vnt.column, &vis_normal.Vnt.m[j], &vis_normal.Vnt.row,
							basis_matr->inv_n_area.v, &inc_one);
				vis_normal.Vntavgm_scalar_SSE.from_unaligned(vis_normal.Vntavgm_scalar.length, vis_normal.Vntavgm_scalar.v);

				vis_normal.sum_avgn = sdot(&vis_normal.avgn.length, vis_normal.avgn.v, &inc_one,
					basis_matr->inv_n_area.v, &inc_one);
				//t.update();
				//printf_s("Vntavgm %gsecs\n", t.elapsed_time());

#ifndef DEBUG_RENDERING
				vis_normal.Un.release();
				vis_normal.Vnt.release();
				vis_normal.VntVmUmt.release();
				vis_normal.avgnVmUmt.release();
				vis_normal.Vntavgm_scalar.release();
				vis_normal.avgn.release();
#endif
				//t.update();
				//printf_s("after SVD %gsecs\n", t.elapsed_time());
				printf_s("after SVD \n");
				//bake BRDF
				vis_normal.compute_BRDF(fr_vis, &inv_SVD_scalar[0]);
				//vis_normal.compute_BRDF_direct(fr_vis);
				//t.update();
				//printf_s("compute BRDF %gsecs\n", t.elapsed_time());
				printf_s("compute BRDF \n");
			}

#ifdef OUTPUT_VISIBLE_POINTS
			//Output visible points
			{
				int num = 0, offset = 0;
				for (int j = 0; j < samples.size(); j++)
				{
					for (int i = 0; i < samples[j].p.size(); i++)
						if (b_pt_vis[i+offset])
							num++;
					offset += samples[j].p.size();
				}

				FILE *fp;
				FOPEN(fp, "T:/Microfacet/output/vis_pts.txt", "wt");
				fprintf_s(fp, "%d\n", num);

				offset = 0;
				for (int j = 0; j < samples.size(); j++)
				{
					for (int i = 0; i < samples[j].p.size(); i++)
						if (b_pt_vis[i+offset])
							fprintf_s(fp, "%g %g %g\n", samples[j].p[i].x, samples[j].p[i].y, samples[j].p[i].z);
					offset += samples[j].p.size();
				}
				fclose(fp);
			}
#endif

#ifdef OUTPUT_DIRECTIONAL_VISIBLE_POINTS
			//DEBUG Output directional visible points
			{
				//printf_s("idx = %d\n", fr_vis.map(Vector3(0, 0, 1)));
				int num = 0, offset = 0;
				for (int j = 0; j < samples.size(); j++)
				{
					for (int i = 0; i < samples[j].p.size(); i++)
						if (mat_vis[i+offset+6*total_pts] & 0x01)
							num++;
					offset += samples[j].p.size();
				}

				FILE *fp;
				FOPEN(fp, "T:/Microfacet/output/pts_directional.txt", "wt");
				fprintf_s(fp, "%d\n", num);

				offset = 0;
				for (int j = 0; j < samples.size(); j++)
				{
					for (int i = 0; i < samples[j].p.size(); i++)
						if (mat_vis[i+offset+6*total_pts] & 0x01)
							fprintf_s(fp, "%g %g %g\n", samples[j].p[i].x, samples[j].p[i].y, samples[j].p[i].z);
					offset += samples[j].p.size();
				}
				fclose(fp);
			}
#endif

			//t.update();
			//printf_s("samples %gsecs\n", t.elapsed_time());
#ifdef OUTPUT_ALL_POINTS
			{
				int num = 0;
				for (int j = 0; j < samples.size(); j++)
					num += samples[j].p.size();

				FILE *fp;
				FOPEN(fp, "T:/Microfacet/output/pts.txt", "wt");
				fprintf_s(fp, "%d\n", num);
				for (int j = 0; j < samples.size(); j++)
					for (int i = 0; i < samples[j].p.size(); i++)
						fprintf_s(fp, "%g %g %g\n", samples[j].p[i].x, samples[j].p[i].y, samples[j].p[i].z);
				fclose(fp);
			}
#endif
			
		}

	//tm.update();
	//printf_s("total time = %gsecs\n", tm.elapsed_time());
	printf_s("compute vis area normal finished\n");
}

//Code Backup
//void my_SVD_rp(la_matrix<number> &U, la_matrix<number> &Vt, 
//			   la_matrix<number> &Rinv, la_matrix<number> &RC, const number percent)
//{
//	codex::utils::timer t;
//	SVD_Vt_L1(Vt, RC, percent);
//	t.update();
//	printf_s("SVD_L1 %gsecs\n", t.elapsed_time());
//
//	U.release();
//	U.row = Rinv.row;
//	U.column = Vt.row;
//	U.m = new number[U.row*U.column];
//
//	float alpha = 1.0f, beta = 0.0f;
//
//	la_matrix<float> RCV(RC.row, Vt.row);
//	sgemm("N", "T", &RCV.row, &RCV.column, &RC.column, 
//		&alpha, RC.m, &RC.row, Vt.m, &Vt.row, 
//		&beta, RCV.m, &RCV.row);
//
//	sgemm("N", "N", &U.row, &U.column, &Rinv.column, 
//		&alpha, Rinv.m, &Rinv.row, RCV.m, &RCV.row, 
//		&beta, U.m, &U.row);
//}
//
//void SVD_rp_vis(la_matrix<float> &mat_vis_n, 
//				vis_normal_distr &vis_normal,
//				preconv_matr *basis_matr,
//				std::vector<float> &inv_SVD_scalar,
//				const int num_vis_dir)
//{
//	int dim_reduced = 50;
//	la_matrix<float> Rt;
//	gen_random_matrix(Rt, dim_reduced, mat_vis_n.row, sqrt(1.0f/(float)dim_reduced));
//	la_matrix<float> Rinv;
//	inverse_matrix(Rinv, Rt, 0.0001f);
//
//	float one = 1.0f, zero = 0.0f;
//	la_matrix<float> RC(Rt.row, mat_vis_n.column);
//	sgemm("N", "N", &Rt.row, &mat_vis_n.column, &mat_vis_n.row,
//		&one, Rt.m, &Rt.row, mat_vis_n.m, &mat_vis_n.row, 
//		&zero, RC.m, &RC.row);
//
//	my_SVD_rp(vis_normal.Un, vis_normal.Vnt, Rinv, RC, 0.9f);
//
//	//FIX ME: the following code is very slow
//	for (int j = 0; j < vis_normal.Un.row; j++)
//		sscal(&vis_normal.Un.column, &inv_SVD_scalar[j/3], &vis_normal.Un.m[j], &vis_normal.Un.row);
//}

//************previous code for sample A_vis***************

//*****************************************************
// DEBUG A_vis ground_truth
//*****************************************************
//{
//	std::vector<float> area_dbg;

//	std::vector<Vector3> dirs;
//	for (int i = 0; i < fr_Avis.get_n().size(); i++)
//		dirs.push_back(fr_Avis.get_n()[i][0]);
//	while ((dirs.size() % NUM_AREA_DIR_PER_BATCH) != 0)
//		dirs.push_back(Vector3(0, 0, 0));
//	area_dbg.resize(dirs.size());

//	codex::utils::timer t;
//	Vector3 lookat(x+0.5f, y+0.5f, 0);
//	mff_singleton::get()->get_layer("pos")->set();
//	for (int i = 0; i < dirs.size()/NUM_AREA_DIR_PER_BATCH; i++)
//	{
//		avis_sampler.begin_sample(lookat, &dirs[i*NUM_AREA_DIR_PER_BATCH]);
//		avis_sampler.get_shader()->setup_cb_attr(1);
//		block.draw_with_shader(avis_sampler.get_shader(), NULL);
//		avis_sampler.get_shader()->setup_cb_attr(0);
//		for (int j = 0; j < block.neighbors.size(); j++)
//			block.neighbors[j]->draw_with_shader(avis_sampler.get_shader(), NULL);
//		avis_sampler.end_sample();

//		avis_sampler.get_area(&area_dbg[i*NUM_AREA_DIR_PER_BATCH]);
//	}

//	area = area_dbg;
//	block.Avis.set_area(area);

//	t.update();
//	printf_s("Avis %gsecs\n", t.elapsed_time());

//	//save to image
//	{
//		std::vector<float> img;
//		img.resize(fr_Avis.get_idx().size()*3, 0.5);
//		for (int i = 0; i < fr_Avis.get_idx().size(); i++)
//		{
//			int ii = fr_Avis.get_idx()[i];
//			if (ii >= 0)
//			{
//				float v = area_dbg[ii];
//				img[i*3] = img[i*3+1] = img[i*3+2] = v;
//			}
//		}
//		save_image_color("d:/temp/Microfacet/area_truth.png", img, fr_Avis.get_dim(), fr_Avis.get_dim());
//	}
//}
//*****************************************************

////DEBUG Avis
////save to image
//{
//	int num;
//	int idx[4];
//	float weight[4];

//	parab_frame fr;
//	fr.init(128, 128, 128);
//	fr.normalize_n();

//	std::vector<float> img;
//	img.resize(fr.get_idx().size()*3, 0.5);
//	for (int i = 0; i < fr.get_idx().size(); i++)
//	{
//		int ii = fr.get_idx()[i];
//		if (ii >= 0)
//		{
//			fr_Avis.lerp(num, idx, weight, fr.get_n()[ii][0], false, false);
//			float v = Avis.get_area(num, idx, weight);	
//			img[i*3] = img[i*3+1] = img[i*3+2] = v;
//		}
//	}
//	save_image_color("d:/temp/Microfacet/area_zoom.png", img, fr.get_dim(), fr.get_dim());
//}

////Test
//{
//	const Vector3 &v = fr_vis.get_n()[47][0];

//	fr_vis.lerp(num_v, idx_v, weight_v, v, true, false);

//	for (int k = 0; k < num_v; k++)
//	{
//		offset[k]	= idx_v[k] / NUM_VIS_PER_BATCH,
//		mask[k]		= 1 << (idx_v[k] % NUM_VIS_PER_BATCH);
//		printf_s("%d %g\n", k, weight_v[k]);
//	}

//	int pt_offset = 0;
//	float area, area1, area2;
//	area = area1 = area2 = 0.0f;
//	for (int p = 0; p < samples.size(); p++)
//	{
//		int sz = samples[p].p.size();
//		for (int k = 0; k < sz; k++)
//			//if (b_pt_vis[pt_offset+k])
//			{
//				for (int l = 0; l < num_v; l++)
//				{
//					if (mat_vis[offset[l]*total_pts+pt_offset+k] & mask[l])
//					{
//						area += weight_v[l]*samples[p].n[k].weight*max(0, v*samples[p].n[k].normal);
//						if (samples[p].p[k].z > 0)
//							area1 += weight_v[l]*samples[p].n[k].weight*max(0, v*samples[p].n[k].normal);
//						//else
//						//	area2 += weight_v[l]*samples[p].n[k].weight*max(0, v*samples[p].n[k].normal);
//					} else {
//						if (samples[p].p[k].z <= 0)
//							area2 += weight_v[l]*samples[p].n[k].weight*abs(v*samples[p].n[k].normal);
//					}
//					//if (samples[p].p[k].z > 0)
//					//	area1 += weight_v[l]*samples[p].n[k].weight*abs(v*samples[p].n[k].normal)/2;
//					//else
//				}
//			}
//			pt_offset += sz;
//	}
//	printf_s("area %g ref %g\n", area, v.z);
//	printf_s("area1 %g ref %g\n", area1, 16*0.01*codex::math::PI);
//	printf_s("area2 %g ref %g\n", area2, v.z - 16*0.01*codex::math::PI);
//}

//block.Avis.resize(NUM_AREA_DIR);
//Vector3 lookat(x+0.5f, y+0.5f, 0);
//mff_singleton::get()->get_layer("pos")->set();
//for (int i = 0; i < area_dir.size()/NUM_AREA_DIR_PER_BATCH; i++)
//{
//	avis_sampler.begin_sample(lookat, &vis_dir[i*NUM_AREA_DIR_PER_BATCH]);
//	avis_sampler.get_shader()->setup_cb_attr(1);
//	block.draw_with_shader(avis_sampler.get_shader(), NULL);
//	avis_sampler.get_shader()->setup_cb_attr(0);
//	for (int j = 0; j < block.neighbors.size(); j++)
//		block.neighbors[j]->draw_with_shader(avis_sampler.get_shader(), NULL);
//	avis_sampler.end_sample();

//	avis_sampler.get_area(&block.Avis[i*NUM_AREA_DIR_PER_BATCH]);
//}

//We should generate all the depth maps first.
////************sample visible points***************
//printf_s("getting visible samples...");
////t.update();
//details.get_visible_samples(samples, idx[i], block, vispt_sampler);
////t.update();
////printf_s("%gsecs\n", t.elapsed_time());
//printf_s("done.\n");

////DEBUG Test alternative A_vis
//{
//	parab_frame fr;
//	fr.init(32, 64);
//	fr.normalize_n();

//	std::vector<float> area;
//	area.resize(fr.get_n().size(), 0);
//	codex::utils::timer t;
//	for (int j = 0; j < area.size(); j++)
//	{
//		const Vector3 &w = fr.get_n()[j][0];
//		
//		int idx_w;
//		tree.fixed_radius_search(w, 4.0f, 1, &idx_w);

//		int offset = idx_w / 8,
//			mask = 1 << (idx_w % 8);

//		for (int k = 0; k < pt_size; k++)
//			if (mat_vis[offset*pt_size+k] & mask)
//			{
//				area[j] += samples[i].n[k].weight*max(0, w*samples[i].n[k].normal);
//			}
//	}
//	t.update();
//	printf_s("%gsecs\n", t.elapsed_time());

//	float power_e, power_s;
//	power_e = power_s = 0;
//	for (int i = 0; i < area.size(); i++)
//	{
//		power_s += area_dbg[i]*area_dbg[i];
//		power_e += (area[i]-area_dbg[i])*(area[i]-area_dbg[i]);
//	}
//	printf_s("%gdb\n", double2db(power_s/power_e));

//	//save to image
//	{
//		std::vector<float> img;
//		img.resize(fr.get_idx().size()*3, 0.5);
//		for (int i = 0; i < fr.get_idx().size(); i++)
//		{
//			int ii = fr.get_idx()[i];
//			if (ii >= 0)
//			{
//				float v = area[ii];
//				img[i*3] = img[i*3+1] = img[i*3+2] = v;
//			}
//		}
//		save_image_color("d:/temp/Microfacet/area_sum.png", img, fr.get_dim(), fr.get_dim());
//	}

//	return;
//}

//DEBUG Get reflectance
//int num = basis_matr->fr_brdf.get_back_idx().size();
//la_vector<float> brdf(num*num), temp2(basis_matr->Vm.column), temp3(num*num);

//int idx_brdf = 203577;//136891;

//brdf.clear();
//for (int j = 0; j < n_size; j++)
//{
//	for (int i = 0; i < basis_matr->Vm.column; i++)
//		temp2.v[i] = basis_matr->Vm.m[j+basis_matr->Vm.row*i];
//	vector_matrix_mul(temp3, temp2, basis_matr->Umt);
//	vector_add(temp3, basis_matr->avgm, 1.0f);
//	vector_add(brdf, temp3, n_distr.v[j]);
//}

//float final = brdf.v[idx_brdf];
//printf_s("n_distr_debug : %g\n", final);

////DEBUG
//std::vector<Vector3> test_dir;
//{
//	int num_test = 16;
//	codex::math::utils::Hammersley_Zaremba_seq<float> seq(num_test);
//	random_number_generator<float> rng;
//	vector2f offset(rng.rand_real(), rng.rand_real());
//	for (int i = 0; i < num_test; i++)
//	{
//		float		pdf;
//		Vector3	rv;
//		seq.get_sample(rv);
//		rv.x = add_random_offset(rv.x, offset.x);
//		rv.y = add_random_offset(rv.y, offset.y);
//		codex::math::prob::uniform_hemi_direction_3d<float> prob;
//		Vector3	v;
//		prob.sample(v, pdf, rv);
//		test_dir.push_back(v);
//	}
//}

//float power_e, power_s;
//power_e = power_s = 0;

//vis_point_sampler v_sampler;
//v_sampler.init(mff_singleton::get()->get_handle(), 2);

//Vector3 wi = Vector3(-0.374075, 0.415894, 0.828915);
//Vector3 wo = Vector3(0.692284, -0.330651, 0.641415);

////explicit test
//{
//	la_vector<float> n_distr(n_size), temp(vis_normal.Un.column);

//	int idx_wi, idx_wo;
//	kd_vis_dir.fixed_radius_search(wi, 4.0f, 1, &idx_wi);
//	kd_vis_dir.fixed_radius_search(wo, 4.0f, 1, &idx_wo);
//	int idx_double_vis = idx_wi+idx_wo*NUM_VIS_DIR;
//	int idx_brdf = basis_matr->fr_brdf.map(wi)+basis_matr->fr_brdf.map(wo)*basis_matr->fr_brdf.get_back_idx().size();

//	for (int i = 0; i < vis_normal.Un.column; i++)
//		temp.v[i] = vis_normal.Un.m[idx_double_vis+vis_normal.Un.row*i];
//	vector_matrix_mul(n_distr, temp, vis_normal.Vnt);
//	vector_add(n_distr, vis_normal.avgn, 1.0f);
//	//save to image
//	{
//		std::vector<float> img;
//		img.resize(basis_matr->fr_n.get_idx().size()*3, 0.5);
//		for (int i = 0; i < basis_matr->fr_n.get_idx().size(); i++)
//		{
//			int ii = basis_matr->fr_n.get_idx()[i];
//			if (ii >= 0)
//			{
//				float v = n_distr.v[ii]*200;
//				img[i*3] = img[i*3+1] = img[i*3+2] = v;
//			}
//		}
//		save_image_color("d:/temp/Microfacet/avg_n_recon.png", img, basis_matr->fr_n.get_dim(), basis_matr->fr_n.get_dim());
//	}

//	Vector3 result;
//	get_reflectance_n_distr_debug(result, wi, wo, n_distr, basis_matr->fr_n);
//	printf_s("n_distr %g\n", result.x);
//}

//	//DEBUG reflectance values
//	
//	//for (int iwi = 0; iwi < test_dir.size(); iwi++)
//		//for (int iwo = 0; iwo < test_dir.size(); iwo++)
//		{
//			//Vector3 wi = test_dir[iwi];
//			//Vector3 wo = test_dir[iwo];

//			std::vector<Vector3> &p = samples[0].p;
//			std::vector<normal_weight> &n = samples[0].n;

//			std::vector<Vector3> views;
//			views.push_back(wi);
//			views.push_back(wo);
//			v_sampler.set_views(views);

//			mff_singleton::get()->get_layer("basic")->set();
//			v_sampler.set_scene_center(Vector3(x+0.5f, y+0.5f, 0));
//			for (int l = 0; l < v_sampler.num_views(); l++)
//			{
//				Matrix4 matLight;
//				v_sampler.begin_gen_shadow(l, matLight);
//				block.draw_with_shader(p_sh_shadow, &matLight);
//				for (int i = 0; i < block.neighbors.size(); i++)
//					block.neighbors[i]->draw_with_shader(p_sh_shadow, &matLight);
//				v_sampler.end_gen_shadow(l);
//			}

//			std::vector<BYTE> point_vis;
//			point_vis.resize(p.size());
//			std::vector<sample_group> temp_group;
//			temp_group.resize(1);
//			temp_group[0].p = p;
//			temp_group[0].n = n;
//			v_sampler.compute_vis_masks(temp_group, 0, 2);
//			v_sampler.get_vis_masks(&point_vis[0], p.size());

//			std::vector<bool> b_vis;
//			b_vis.resize(point_vis.size());
//			for (int i = 0; i < point_vis.size(); i++)
//				b_vis[i] = (point_vis[i] == 3);

//			Vector3 refl_debug, refl;
//			block.get_reflectance_debug(refl_debug, wi, wo, n, b_vis);
//			block.get_reflectance(refl, wi, wo, kd_vis_dir);

//			power_s += refl_debug.x*refl_debug.x;
//			power_e += (refl_debug.x-refl.x)*(refl_debug.x-refl.x);
//			//printf_s("debug %g refl %g\n", refl_debug.x, refl.x);
//			//if (double2db(refl_debug.x*refl_debug.x/((refl_debug.x-refl.x)*(refl_debug.x-refl.x))) < 12.0)
//			{
//				printf_s("wi (%g %g %g) wo (%g %g %g)\n", wi.x, wi.y, wi.z, wo.x, wo.y, wo.z);
//				printf_s("debug %g refl %g\n", refl_debug.x, refl.x);
//			}

//			//return;
//		}
//	//printf_s("\n%gdb\n", double2db(power_s/power_e));
//}
//void get_reflectance_n_distr_debug(Vector3 &result, 
//								   const Vector3 &wi, const Vector3 &wo,
//								   const la_vector<float> &n_distr,
//								   const parab_frame &fr_n)
//{
//	//DEBUG
//	Lambert brdf;
//	parab_frame fr = fr_n;
//	fr.normalize_n();
//
//	float refl = 0;
//	for (int i = 0; i < n_distr.length; i++)
//		if (n_distr.v[i] > 0)
//		{
//			float r;
//			brdf.sample(r, wi, wo, fr.get_n()[i][0]);
//			refl += r*n_distr.v[i];
//		}
//		//DEBUG
//		result = Vector3(refl, refl, refl);
//}