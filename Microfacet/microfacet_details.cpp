#include "microfacet_details.h"
#include "microfacet_factory.h"
microfacet_details::microfacet_details()
	:pdist(NULL)
{

}

void microfacet_details::init(int w, int h, float d, float dens, int num_area,
	microfacet_binder *pb, microfacet_distr *pd)
{
	width = w;
	height = h;
	depth = d;
	density = dens;
	num_area_hits_scalar = num_area;

	pdist = pd;
	pbinder = pb;

	inst_prop.resize((width + EXTRA_BLOCK * 2)*(height + EXTRA_BLOCK * 2));

	p_buffer.resize(VIS_POINT_BUFFER_DIM*VIS_POINT_BUFFER_DIM);
	n_buffer.resize(VIS_POINT_BUFFER_DIM*VIS_POINT_BUFFER_DIM);
}

int microfacet_details::get_width() const
{
	return width;
}

int microfacet_details::get_height() const
{
	return height;
}

float microfacet_details::get_depth() const
{
	return depth;
}

int microfacet_details::compute_idx(const int x, const int y) const
{
	return x + EXTRA_BLOCK + (y + EXTRA_BLOCK)*(width + EXTRA_BLOCK * 2);
}

void microfacet_details::compute_back_idx(int &x, int &y, const int idx) const
{
	x = (idx % (width + EXTRA_BLOCK * 2)) - EXTRA_BLOCK;
	y = (idx / (width + EXTRA_BLOCK * 2)) - EXTRA_BLOCK;
}

void microfacet_details::delete_one_block(std::vector<instance_property*> &block)
{
	for (int i = 0; i < block.size(); i++)
		SAFE_DELETE(block[i]);
	block.clear();
}

void microfacet_details::update_with_distr(const std::vector<int> &idx)
{
	pbinder->generate_geom(density, num_area_hits_scalar);

	// all neighbors combination, no 0,0 since it is the center point
	int delta[] = { -1, 0, 1, 0, 0, 1, 0, -1,
		-1, -1, 1, -1, -1, 1, 1, 1 };

	int w = width + EXTRA_BLOCK * 2,
		h = height + EXTRA_BLOCK * 2;
	for (int i = 0; i < idx.size(); i++)
	{
		int x, y;
		compute_back_idx(x, y, idx[i]);
		Vector3 v_min(x, y, 0),
			v_max(x + 1, y + 1, depth);

		std::vector<const std::vector<instance_property*>*> neighbors;
		for (int d = 0; d < sizeof(delta) / sizeof(int) / 2; d++)
		{
			int dx, dy, xx, yy;
			dx = delta[d * 2];
			dy = delta[d * 2 + 1];
			xx = x + dx;
			yy = y + dy;
			// add all neighbors into the list
			// inst_prop boundary items that are not initialized will not be triggered
			if (xx >= 0 && xx < w && yy >= 0 && yy < h)
				neighbors.push_back(&inst_prop[xx + yy*w]);
		}

		delete_one_block(inst_prop[idx[i]]);
		pbinder->generate(inst_prop[idx[i]], v_min, v_max, neighbors);
		pdist->generate(inst_prop[idx[i]], v_min, v_max, neighbors);
	}
}

void microfacet_details::init_blocks(std::vector<microfacet_block> &blocks)
{
	int delta[] = { -1, 0, 1, 0, 0, 1, 0, -1,
		-1, -1, 1, -1, -1, 1, 1, 1 };

	int w = width + EXTRA_BLOCK * 2,
		h = height + EXTRA_BLOCK * 2;

	blocks.resize(w*h);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int idx = compute_idx(x, y);
			blocks[idx].neighbors_and_self.clear();
			blocks[idx].neighbors_and_self.push_back(&blocks[idx]); // add self block

			for (int d = 0; d < sizeof(delta) / sizeof(int) / 2; d++)
			{
				int dx, dy, xx, yy;
				dx = delta[d * 2];
				dy = delta[d * 2 + 1];
				xx = x + dx;
				yy = y + dy;
				if (xx >= -EXTRA_BLOCK && xx < w + EXTRA_BLOCK &&
					yy >= -EXTRA_BLOCK && yy < h + EXTRA_BLOCK)
					blocks[idx].neighbors_and_self.push_back(&blocks[compute_idx(xx, yy)]); // add neighbor block
			}
		}
}

int compare_mat_id(const void *arg1, const void *arg2)
{
	return *((int*)arg1) - *((int*)arg2);
}

void microfacet_details::generate_blocks(const std::vector<int> &idx, std::vector<microfacet_block> &result)
{
	for (int i = 0; i < idx.size(); i++)
	{
		int ii = idx[i];
		result[ii].release();

		std::vector<int> mat_ids;
		std::vector<r_instance*> p_insts;
		//cout << "generate_blocks" << endl;
		for (int j = 0; j < inst_prop[ii].size(); j++)
		{
			r_instance *p;
			int mat_id;
			mff_singleton::get()->produce(p, *inst_prop[ii][j], mat_id);
			mat_ids.push_back(mat_id);
			mat_ids.push_back(j);
			p_insts.push_back(p);

			if (inst_prop[ii][j]->p_geom != NULL)
				p->set_geom(inst_prop[ii][j]->p_geom, inst_prop[ii][j]->scale);
		}

		qsort(&mat_ids[0], mat_ids.size() / 2, sizeof(int) * 2, compare_mat_id);
		for (int j = 0; j < p_insts.size(); j++)
		{
			r_instance *p = p_insts[mat_ids[j * 2 + 1]];
			result[ii].insts.push_back(p);
			result[ii].per_mat_groups.insts[result[ii].per_mat_groups.idx(mat_ids[j * 2])].push_back(p);// save the p with same mat group
		}
	}
	//cout << "after generate_blocks" << endl;
}

void microfacet_details::idx_all(std::vector<int> &idx) const
{
	idx.clear();
	idx.resize((width + EXTRA_BLOCK * 2)*(height + EXTRA_BLOCK * 2));
	for (int i = 0; i < idx.size(); i++)
		idx[i] = i;
}

bool microfacet_details::in_boundary(const int idx) const
{
	int x, y;
	compute_back_idx(x, y, idx);
	return (x >= 0 && x < width && y >= 0 && y < height);
}

microfacet_binder* microfacet_details::get_binder()
{
	return pbinder;
}

void microfacet_details::sample_points(const std::vector<int> &idx,
	std::vector<microfacet_block> &result)
{
	for (int i = 0; i < idx.size(); i++)
	{
		int ii = idx[i], x, y;
		compute_back_idx(x, y, ii);

		if (x >= 0 && x < width && y >= 0 && y < height)
		{
			for (int j = 0; j < result[ii].insts.size(); j++)
				result[ii].insts[j]->sample_points(density, num_area_hits_scalar);// this function will trigger r_geom_inst.sample_point
		}
	}
}

void microfacet_details::get_samples(std::vector<sample_group> &samples,
	std::vector<int> &samp_idx,
	microfacet_block &block)
{
	//const std::vector<Vector3>			*pp; 
	//const std::vector<normal_weight>	*pn;
	//int	num_samples = 0;

	//for (int i = 0; i < block.insts.size(); i++)
	//{
	//	float scale;
	//	block.insts[i]->get_samples(pp, pn, scale);
	//	num_samples += pp->size();
	//}

	//p.clear();
	//p.reserve(num_samples);
	//n.clear();
	//n.reserve(num_samples);

	//for (int i = 0; i < block.insts.size(); i++)
	//{
	//	float scale;
	//	block.insts[i]->get_samples(pp, pn, scale);

	//	r_shader_input *p_shinput = (r_shader_input*)block.insts[i]->get_shader_input();
	//	Matrix4 mat_world;
	//	Matrix4_d_to_f(mat_world, p_shinput->matWorld);
	//	Matrix4 mat_normal(mat_world);

	//	mat_normal._14 = mat_normal._24 = mat_normal._34 = 0;

	//	float norm;
	//	norm = sqrt(mat_normal._11*mat_normal._11 + mat_normal._21*mat_normal._21 + mat_normal._31*mat_normal._31);
	//	mat_normal._11 /= norm;
	//	mat_normal._21 /= norm;
	//	mat_normal._31 /= norm;
	//	norm = sqrt(mat_normal._12*mat_normal._12 + mat_normal._22*mat_normal._22 + mat_normal._32*mat_normal._32);
	//	mat_normal._12 /= norm;
	//	mat_normal._22 /= norm;
	//	mat_normal._32 /= norm;
	//	norm = sqrt(mat_normal._13*mat_normal._13 + mat_normal._23*mat_normal._23 + mat_normal._33*mat_normal._33);
	//	mat_normal._13 /= norm;
	//	mat_normal._23 /= norm;
	//	mat_normal._33 /= norm;

	//	for (int j = 0; j < pp->size(); j++)
	//	{
	//		p.push_back(mat_world*(*pp)[j]);
	//		normal_weight w;
	//		w.normal = mat_normal*(*pn)[j].normal;
	//		w.weight = (*pn)[j].weight*scale*scale;
	//		n.push_back(w);
	//	}
	//}

	samples.resize(block.per_mat_groups.insts.size());
	samp_idx.resize(samples.size() + 1);
	samp_idx[0] = 0;
	for (int i = 0; i < block.per_mat_groups.insts.size(); i++)
	{
		std::vector<r_instance*> &insts = block.per_mat_groups.insts[i];
		samples[i].mat_id = block.per_mat_groups.get_id(i);
		samples[i].p.clear();
		samples[i].n.clear();

		for (int j = 0; j < insts.size(); j++)
		{
			const std::vector<Vector3>			*pp;
			const std::vector<normal_weight>	*pn;
			float scale;
			insts[j]->get_samples(pp, pn, scale);

			if (pp->size() > 0)
			{
				r_shader_input *p_shinput = (r_shader_input*)insts[j]->get_shader_input();
				Matrix4 mat_world = p_shinput->matWorld;
				Matrix4 mat_normal(mat_world);

				mat_normal.m[0][3] = mat_normal.m[1][3] = mat_normal.m[2][3] = 0;

				float norm;
				norm = sqrt(mat_normal.m[0][0] * mat_normal.m[0][0] + mat_normal.m[1][0] * mat_normal.m[1][0] + mat_normal.m[2][0] * mat_normal.m[2][0]);
				mat_normal.m[0][0] /= norm;
				mat_normal.m[1][0] /= norm;
				mat_normal.m[2][0] /= norm;
				norm = sqrt(mat_normal.m[0][1] * mat_normal.m[0][1] + mat_normal.m[1][1] * mat_normal.m[1][1] + mat_normal.m[2][1] * mat_normal.m[2][1]);
				mat_normal.m[0][1] /= norm;
				mat_normal.m[1][1] /= norm;
				mat_normal.m[2][1] /= norm;
				norm = sqrt(mat_normal.m[0][2] * mat_normal.m[0][2] + mat_normal.m[1][2] * mat_normal.m[1][2] + mat_normal.m[2][2] * mat_normal.m[2][2]);
				mat_normal.m[0][2] /= norm;
				mat_normal.m[1][2] /= norm;
				mat_normal.m[2][2] /= norm;

				for (int k = 0; k < pp->size(); k++)
				{
					normal_weight w;
					w.normal = mat_normal*(*pn)[k].normal;
					//if (w.normal.z > 0)
					{
						samples[i].p.push_back(mat_world*(*pp)[k]);
						w.weight = (*pn)[k].weight*scale*scale;
						samples[i].n.push_back(w);
					}
				}
			}
		}

		samp_idx[i + 1] = samp_idx[i] + samples[i].p.size();
	}
}

void microfacet_details::get_visible_samples(std::vector<sample_group> &samples,
	const int idx,
	microfacet_block &block,
	vis_point_sampler &sampler)
{
	//codex::utils::timer t;
	//double t_draw = 0;
	//gen shadow maps
	r_shader_basic *p_sh_shadow = (r_shader_basic*)mff_singleton::get()->get_shader("shadow");
	mff_singleton::get()->get_layer("basic")->set();

	int x, y;
	compute_back_idx(x, y, idx);
	sampler.set_scene_center(Vector3(x + 0.5, y + 0.5, depth / 2));
	for (int l = 0; l < sampler.num_views(); l++)
	{
		Matrix4 matLight;
		sampler.begin_gen_shadow(l, matLight);

		//t.update();
		for (int i = 0; i < block.neighbors_and_self.size(); i++)
			block.neighbors_and_self[i]->draw_with_shader(p_sh_shadow, &matLight);
		//t.update();
		//t_draw += t.elapsed_time();

		sampler.end_gen_shadow(l);
	}
	//t.update();
	//printf_s("%g %gsecs\n", t_draw, t.elapsed_time());

	samples.resize(block.per_mat_groups.insts.size());
	for (int i = 0; i < block.per_mat_groups.insts.size(); i++)
	{
		samples[i].mat_id = block.per_mat_groups.get_id(i);
		std::vector<r_instance*> &insts = block.per_mat_groups.insts[i];
		int ii = 0;
		for (int j = 0; j < insts.size(); j++)
		{
			const std::vector<Vector3>			*pp;
			const std::vector<normal_weight>	*pn;
			float scale;
			insts[j]->get_samples(pp, pn, scale);

			if (pp->size() > 0)
			{
				r_shader_input *p_shinput = (r_shader_input*)insts[j]->get_shader_input();
				Matrix4 mat_world = p_shinput->matWorld;
				Matrix4 mat_normal(mat_world);

				mat_normal.m[0][3] = mat_normal.m[1][3] = mat_normal.m[2][3] = 0;

				float norm;
				norm = sqrt(mat_normal.m[0][0] * mat_normal.m[0][0] + mat_normal.m[1][0] * mat_normal.m[1][0] + mat_normal.m[2][0] * mat_normal.m[2][0]);
				mat_normal.m[0][0] /= norm;
				mat_normal.m[1][0] /= norm;
				mat_normal.m[2][0] /= norm;
				norm = sqrt(mat_normal.m[0][1] * mat_normal.m[0][1] + mat_normal.m[1][1] * mat_normal.m[1][1] + mat_normal.m[2][1] * mat_normal.m[2][1]);
				mat_normal.m[0][1] /= norm;
				mat_normal.m[1][1] /= norm;
				mat_normal.m[2][1] /= norm;
				norm = sqrt(mat_normal.m[0][2] * mat_normal.m[0][2] + mat_normal.m[1][2] * mat_normal.m[1][2] + mat_normal.m[2][2] * mat_normal.m[2][2]);
				mat_normal.m[0][2] /= norm;
				mat_normal.m[1][2] /= norm;
				mat_normal.m[2][2] /= norm;

				for (int k = 0; k < pp->size(); k++)
				{
					n_buffer[ii].normal = mat_normal*(*pn)[k].normal;
					if (n_buffer[ii].normal.z > 0)
					{
						p_buffer[ii] = mat_world*(*pp)[k];
						n_buffer[ii].weight = (*pn)[k].weight*scale*scale;
						ii++;
					}
				}
			}
		}
		sampler.sample_vis_points(samples[i].p, samples[i].n, ii, p_buffer, n_buffer);
		
	}
}
