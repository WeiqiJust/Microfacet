#include "my_float_types.h"


class parab_frame
{
private:
	int		dim;
	bool	b_upper;
	std::vector<int> 
			idx, back_idx;
	std::vector<float>
			area, spherical_area;
	std::vector<std::vector<vector3f>>
			n;

public:
	void init(const int d, const int samples_per_texel, const int area_samples_per_texel,
		const bool b_upper = true);
	void normalize_n();
	void compute_spherical_area(const int num_samples);

	void load(FILE *&fp);
	void save(FILE *&fp) const;

	int map(const vector3f &n) const;
	void lerp(int &num, int *idx, float *weight, const vector3f &n,
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

	const std::vector<std::vector<vector3f>>& get_n() const
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
	std::vector<int> 
			idx, back_idx;
	std::vector<std::vector<vector3f>>
			n;
	std::vector<float>
			spherical_area;

	void cluster_vec(const int num_clusters, 
		const int max_iter,
		const std::vector<vector3f> &input, 
		std::vector<vector3f> &output);

public:
	void init(const int d, const int samples_per_texel);
	void normalize_n();
	void reduce_n(const int samples_per_texel);
	void compute_spherical_area(const int num_samples);

	void load(FILE *&fp);
	void save(FILE *&fp) const;

	int map(const vector3f &n) const;
	void lerp(int &num, int *idx, float *weight, const vector3f &n,
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

	const std::vector<std::vector<vector3f>>& get_n() const
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

int	parab_map(const vector3f &v, const int dim, const int actual_dim, const bool b_upper);
void parab_map(vector2f &result, const vector3f &v, const int dim, const int actual_dim, const bool b_upper);
void back_parab_map(vector3f &v, const vector2f &coord, const int dim, const int actual_dim, const bool b_upper);
