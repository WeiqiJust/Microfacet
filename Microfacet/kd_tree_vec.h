#pragma once
#include "utils.h"

struct index_dist
{
	int index;
	float dist_squared;
};

class kd_compare_node_vec {
public:
	const int	axis, dim;
	const float* &v_arr;

	kd_compare_node_vec(const float* &v_arr, const int dim, const int axis) : v_arr(v_arr), axis(axis), dim(dim){}
	bool operator()(const int d1, const int d2) const
	{
		return v_arr[d1*dim + axis] == v_arr[d2*dim + axis] ? (d1 < d2) : v_arr[d1*dim + axis] < v_arr[d2*dim + axis];
	}
};

struct kd_compare_dist_vec : public std::binary_function <index_dist, index_dist, bool>
{
	bool operator()(
		const index_dist& _Left,
		const index_dist& _Right
		) const
	{
		return _Left.dist_squared < _Right.dist_squared;
	}
};

class kd_search_param_vec
{
public:
	const float	*p;
	float		radius_squared;
	int			num_lookup;
	std::priority_queue<index_dist, std::vector<index_dist>, kd_compare_dist_vec> q;
};

class kd_tree_vec_node
{
public:
	unsigned int axis, index;
	kd_tree_vec_node *left, *right;

	kd_tree_vec_node() : left(NULL), right(NULL) {}
	~kd_tree_vec_node()
	{
		SAFE_DELETE(left);
		SAFE_DELETE(right);
	}
};

class kd_tree_vec
{
private:
	const float* v_arr;

	void build(kd_tree_vec_node* &p, int start, int end, std::vector<int> &temp_vec);

	void fixed_radius_search(kd_tree_vec_node *p, kd_search_param_vec &param) const;
	void process_current_node(kd_tree_vec_node *p, kd_search_param_vec &param) const;

protected:
	int		num_pts, dim;
	kd_tree_vec_node
		*root;

public:
	kd_tree_vec(const float* v_arr, const int dim, const int num_pts);
	virtual ~kd_tree_vec();

	int fixed_radius_search_with_dist(const float *p,
		float radius_squared, int num_lookup,
		int *index, float *dist) const;
	int fixed_radius_search(const float *p,
		float radius_squared, int num_lookup,
		int *index) const;
};
