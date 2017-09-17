#include "microfacet.h"
#include "worker.h"

void compute_normal_mapped_BRDF(microfacet_details &details,
											const std::vector<int> &idx_blocks,
											std::vector<microfacet_block> &blocks)
{
	//codex::utils::timer tm, t;

	//Common vars
	int		inc_one = 1, inc_zero = 0, inc_three = 3;
	float	one = 1.0f, zero = 0.0f;

	for (int xx = 0; xx < idx_blocks.size(); xx++)
		if (details.in_boundary(idx_blocks[xx]))
		{
			std::vector<sample_group>	samples;
			std::vector<int>			samp_idx;
			int							total_pts, x, y;
			microfacet_block			&block = blocks[idx_blocks[xx]];
			details.compute_back_idx(x, y, idx_blocks[xx]);

			//*****************************************************
			// Determining the sampling rate for the BRDF
			//*****************************************************
			preconv_matr *spec_basis_matr = NULL;
			for (int i = 0; i < block.per_mat_groups.insts.size(); i++)
			{
				int id = block.per_mat_groups.get_id(i);
				matr* p_matr = mff_singleton::get()->get_matr(id);
				preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

				if (spec_basis_matr == NULL ||
					basis_matr->fr_brdf.get_n().size() > spec_basis_matr->fr_brdf.get_n().size())
					spec_basis_matr = basis_matr;
			}
			const parab_frame &fr = spec_basis_matr->fr_brdf;
			int brdf_dim = fr.get_n().size();
			block.BRDF_truth.clear();
			block.BRDF_truth.resize(3*brdf_dim*brdf_dim, 0);

			//*****************************************************
			// Sample object-space points
			//*****************************************************
			printf_s("getting samples...");
			details.get_samples(samples, samp_idx, block);
			total_pts = samp_idx[samp_idx.size()-1];
			printf_s("done.\n");

			//*****************************************************
			// Compute total area
			//*****************************************************
			float total_area = 0;
			for (int i = 0; i < samples.size(); i++)
				for (int j = 0; j < samples[i].n.size(); j++)
					total_area += samples[i].n[j].weight;

			//*****************************************************
			// Generate BRDF
			//*****************************************************
			for (int i = 0; i < samples.size(); i++)
			{
				matr* p_matr = mff_singleton::get()->get_matr(samples[i].mat_id);
				preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

				for (int j = 0; j < samples[i].n.size(); j++)
				{
					printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
					printf_s("(%d/%d) ", j+1, samples[i].n.size());

					const Vector3 &n = samples[i].n[j].normal;

					for (int i_wo = 0; i_wo < brdf_dim; i_wo++)
						for (int i_wi = 0; i_wi < brdf_dim; i_wi++)
						{
							const Vector3 &wo = fr.get_n()[i_wo][0];
							const Vector3 &wi = fr.get_n()[i_wi][0];

							Vector3 refl;
							basis_matr->pBRDF->sample(refl, wi, wo, n);
#ifdef LOG_MATERIAL
							refl.x = exp(refl.x)-1.0f;
							refl.y = exp(refl.y)-1.0f;
							refl.z = exp(refl.z)-1.0f;
#endif // LOG_MATERIAL
							refl /= Dot(n, wo);
							Vector3 c = p_matr->albedo;
							c.x *= refl.x;
							c.y *= refl.y;
							c.z *= refl.z;
							c *= samples[i].n[j].weight / total_area;

							block.BRDF_truth[3*(brdf_dim*i_wo+i_wi)+0] += c.x;
							block.BRDF_truth[3*(brdf_dim*i_wo+i_wi)+1] += c.y;
							block.BRDF_truth[3*(brdf_dim*i_wo+i_wi)+2] += c.z;
						}
				}
			}
		}

	printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf_s("done.          ");

	//tm.update();
	//printf_s("total time = %gsecs\n", tm.elapsed_time());
}

void worker_microfacet::compute_BRDF_normal(task_microfacet_changed &tmc)
{
	compute_normal_mapped_BRDF(*tmc.details, *tmc.idx, *tmc.blocks);
}