#pragma once
#include "vis_point_sampler.h"
#include "microfacet_block.h"
#include "binder.h"
#define EXTRA_BLOCK		1

class microfacet_details
{
public:
	microfacet_details();
	void init(int w, int h, float d,
		float density, int num_area_hits_scalar,
		microfacet_binder *pb, microfacet_distr *pd);

	int get_width() const;
	int get_height() const;
	float get_depth() const;
	int compute_idx(const int x, const int y) const;
	void compute_back_idx(int &x, int &y, const int idx) const;

	// change the facest with idxs by modifying the neighboors and update in binder, geometry
	void update_with_distr(const std::vector<int> &idx);
	void init_blocks(std::vector<microfacet_block> &blocks);

	// generate blocks from inst_prop, material
	void generate_blocks(const std::vector<int> &idx, std::vector<microfacet_block> &results);

	void sample_points(const std::vector<int> &idx, std::vector<microfacet_block> &results);
	void get_visible_samples(std::vector<sample_group> &samples, const int idx,
		microfacet_block &block, vis_point_sampler &sampler);

	void get_samples(std::vector<sample_group> &samples, std::vector<int> &samp_idx,
		microfacet_block &block);

	void idx_all(std::vector<int> &idx) const;
	bool in_boundary(const int idx) const;

	microfacet_binder* get_binder();

private:
	int		width, height;
	float	depth;

	float	density;
	int		num_area_hits_scalar;

	microfacet_distr
		*pdist;
	microfacet_binder
		*pbinder;
	std::vector<std::vector<instance_property*>>
		inst_prop;

	std::vector<Vector3>		p_buffer;
	std::vector<normal_weight>	n_buffer;

	void delete_one_block(std::vector<instance_property*> &block);
	//void delete_one_instance_block(std::vector<r_instance*> &block);

};