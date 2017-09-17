#pragma once
#include "utils.h"

struct index_dist_3d
{
	int index;
	float dist_squared;
};

class kd_compare_node_3d_vec {
public:
	int	axis;
	const std::vector<Vector3>	*v_arr;

	kd_compare_node_3d_vec(const std::vector<Vector3> *v_arr, int axis) : v_arr(v_arr), axis(axis){}
	bool operator()(const int d1, const int d2) const
	{
		return (*v_arr)[d1][axis] == (*v_arr)[d2][axis] ? (d1 < d2) :
			(*v_arr)[d1][axis] < (*v_arr)[d2][axis];
	}
};

struct kd_compare_dist_vec_3d : public std::binary_function <index_dist_3d, index_dist_3d, bool>
{
	bool operator()(
		const index_dist_3d& _Left,
		const index_dist_3d& _Right
		) const
	{
		return _Left.dist_squared < _Right.dist_squared;
	}
};

class kd_search_param_3d
{
public:
	float		p[3];
	float		radius_squared;
	int			num_lookup;
	std::priority_queue<index_dist_3d, std::vector<index_dist_3d>, kd_compare_dist_vec_3d> q;
};


class kd_tree_3d_node
{
public:
	unsigned int axis, index;
	kd_tree_3d_node *left, *right;

	kd_tree_3d_node() : left(NULL), right(NULL) {}
	~kd_tree_3d_node()
	{
		SAFE_DELETE(left);
		SAFE_DELETE(right);
	}
};

class kd_tree_3d_vec
{
private:
	const std::vector<Vector3>	*v_arr;

	void build(kd_tree_3d_node* &p, int start, int end, std::vector<int> &temp_vec);

	void fixed_radius_search(kd_tree_3d_node *p, kd_search_param_3d &param) const;
	void process_current_node(kd_tree_3d_node *p, kd_search_param_3d &param) const;

protected:
	int		num_pts;
	kd_tree_3d_node
		*root;

public:
	kd_tree_3d_vec(const std::vector<Vector3> *v_arr, int num_pts);
	kd_tree_3d_vec();

	void init(const std::vector<Vector3>	*v_arr, int num_pts);

	virtual ~kd_tree_3d_vec();

	int fixed_radius_search_with_dist(const Vector3 &p,
		const float radius_squared, int num_lookup,
		int *index, float *dist) const; // type Number->float
	int fixed_radius_search(const Vector3 &p,
		const float radius_squared, int num_lookup,
		int *index) const;
};