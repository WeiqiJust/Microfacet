#pragma once
#include "utils.h"

class parab_frame
{
private:
	int		dim;
	bool	b_upper;
	std::vector<int>  idx, back_idx;
	std::vector<float> area, spherical_area;

	// each item is the vector of the normals that are sampled from a grid 
	std::vector<std::vector<Vector3>> n;

public:
	void init(const int d, const int samples_per_texel, const int area_samples_per_texel,
		const bool b_upper = true);
	void normalize_n();
	void compute_spherical_area(const int num_samples);

	void load(FILE *&fp);
	void save(FILE *&fp) const;

	int map(const Vector3 &n) const;
	void lerp(int &num, int *idx, float *weight, const Vector3 &n,
		const bool area_weighted, const bool b_normalize) const;

	const std::vector<int>& get_idx() const
	{
		return idx;
	}

	const std::vector<int>& get_back_idx() const
	{
		return back_idx;
	}

	const std::vector<float>& get_area() const
	{
		return area;
	}

	const std::vector<float>& get_spherical_area() const
	{
		return spherical_area;
	}

	const std::vector<std::vector<Vector3>>& get_n() const
	{
		return n;
	}

	int get_dim() const
	{
		return dim;
	}
};

class double_parab_frame
{
private:
	int		dim, actual_dim, size_single_hemi;
	std::vector<int> idx, back_idx;
	std::vector<std::vector<Vector3>> n;
	std::vector<float> spherical_area;

	void cluster_vec(const int num_clusters, 
		const int max_iter,
		const std::vector<Vector3> &input, 
		std::vector<Vector3> &output);

public:
	void init(const int d, const int samples_per_texel);
	void normalize_n();
	void reduce_n(const int samples_per_texel);

	// use the number of sample points of each "pixel" to calculate their areas
	void compute_spherical_area(const int num_samples);

	void load(FILE *&fp);
	void save(FILE *&fp) const;

	int map(const Vector3 &n) const;

	// num is the total number of visible facets of the neightbours
	// idx is the vector of index mapping from 2d to 1d
	void lerp(int &num, int *idx, float *weight, const Vector3 &n,
		const bool b_normalize) const;

	const std::vector<int>& get_idx() const
	{
		return idx;
	}

	const std::vector<int>& get_back_idx() const
	{
		return back_idx;
	}

	const std::vector<float>& get_spherical_area() const
	{
		return spherical_area;
	}

	const std::vector<std::vector<Vector3>>& get_n() const
	{
		return n;
	}

	int get_dim() const
	{
		return dim;
	}

	int get_actual_dim() const
	{
		return actual_dim;
	}
};

// map 3d to 2d first and then construct index of idx (1d)
int	parab_map(const Vector3 &v, const int dim, const int actual_dim, const bool b_upper);

// map 3d to 2d
void parab_map(Vector2 &result, const Vector3 &v, const int dim, const int actual_dim, const bool b_upper);

// inverse mapping every pixel used for lookup
void back_parab_map(Vector3 &v, const Vector2 &coord, const int dim, const int actual_dim, const bool b_upper);
