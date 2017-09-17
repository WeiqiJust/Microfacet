#include "kd_tree_vec.h"


kd_tree_vec::kd_tree_vec(const float* v_arr, const int dim, const int num_pts)
	: v_arr(v_arr), dim(dim), num_pts(num_pts), root(NULL)
{
	std::vector<int> temp_vec;

	for (int i = 0; i<num_pts; i++)
		temp_vec.push_back(i);

	build(root, 0, num_pts - 1, temp_vec);
}

kd_tree_vec::~kd_tree_vec()
{
	SAFE_DELETE(root);
}

void kd_tree_vec::build(kd_tree_vec_node* &p, int start, int end, std::vector<int> &temp_vec)
{
	if (start >= end)
	{
		if (start == end)
		{
			p = new kd_tree_vec_node();
			p->index = temp_vec[start];
			p->axis = -1;
		}
		else {
			p = NULL;
		}
		return;
	}

	// build bounding box
	std::vector<float> min_p, max_p;

	min_p.resize(dim);
	max_p.resize(dim);

	for (int i = 0; i < dim; i++)
		max_p[i] = min_p[i] = v_arr[temp_vec[start] * dim + i];

	for (int i = start + 1; i <= end; i++)
		for (int j = 0; j<dim; j++)
		{
			min_p[j] = (std::min)(min_p[j], v_arr[temp_vec[i] * dim + j]);
			max_p[j] = (std::max)(max_p[j], v_arr[temp_vec[i] * dim + j]);
		}

	// find the longest axis
	int longest_axis;
	float longest_dist = -1;

	for (int i = 0; i < dim; i++)
		if (max_p[i] - min_p[i] >= longest_dist)
		{
			longest_dist = max_p[i] - min_p[i];
			longest_axis = i;
			break;
		}

	int split_pos = (start + end) / 2;

	std::nth_element(&temp_vec[start], &temp_vec[split_pos], 1 + &temp_vec[end], kd_compare_node_vec(v_arr, dim, longest_axis));

	p = new kd_tree_vec_node();
	p->axis = (unsigned int)longest_axis;
	p->index = temp_vec[split_pos];

	build(p->left, start, split_pos - 1, temp_vec);
	build(p->right, split_pos + 1, end, temp_vec);
}

int kd_tree_vec::fixed_radius_search(const float *p,
	float radius_squared, int num_lookup,
	int *index) const
{
	kd_search_param_vec	param;

	param.p = p;
	param.num_lookup = num_lookup;
	param.radius_squared = radius_squared;

	fixed_radius_search(root, param);

	int	queue_size = (std::min)((int)param.q.size(), num_lookup);

	for (int i = 0; i < queue_size; i++)
	{
		index[queue_size - 1 - i] = param.q.top().index;
		param.q.pop();
	}

	if (queue_size < num_lookup)
		index[queue_size] = -1;

	return queue_size;
}

int kd_tree_vec::fixed_radius_search_with_dist(const float *p,
	float radius_squared, int num_lookup,
	int *index, float *dist) const
{
	kd_search_param_vec	param;

	param.p = p;
	param.num_lookup = num_lookup;
	param.radius_squared = radius_squared;

	fixed_radius_search(root, param);

	int	queue_size = (std::min)((int)param.q.size(), num_lookup);

	for (int i = 0; i < queue_size; i++)
	{
		index[queue_size - 1 - i] = param.q.top().index;
		dist[queue_size - 1 - i] = sqrt(param.q.top().dist_squared);
		param.q.pop();
	}

	if (queue_size < num_lookup)
		index[queue_size] = -1;

	return queue_size;
}

void kd_tree_vec::process_current_node(kd_tree_vec_node *p, kd_search_param_vec &param) const
{
	index_dist d;

	d.index = p->index;

	//FIX ME: accelerate using MKL
	d.dist_squared = 0;
	for (int i = 0; i < dim; i++)
	{
		float delta = (v_arr[p->index*dim + i] - param.p[i]);
		d.dist_squared += delta*delta;
	}

	if (d.dist_squared <= param.radius_squared)
	{
		while ((int)param.q.size() >= param.num_lookup)
		{
			param.q.pop();
		}

		param.q.push(d);

		if (param.q.size() == param.num_lookup)
			param.radius_squared = param.q.top().dist_squared;
	}
}

void kd_tree_vec::fixed_radius_search(kd_tree_vec_node *p, kd_search_param_vec &param) const
{
	if (!p) return;

	if (p->left == NULL && p->right == NULL)
	{
		process_current_node(p, param);
	}
	else {
		float relative_coord = param.p[p->axis] - v_arr[p->index*dim + p->axis];

		if (relative_coord < 0)
		{
			if (p->left)
				fixed_radius_search(p->left, param);

			process_current_node(p, param);

			if (p->right && relative_coord*relative_coord <= param.radius_squared)
				fixed_radius_search(p->right, param);
		}
		else {
			if (p->right)
				fixed_radius_search(p->right, param);

			process_current_node(p, param);

			if (p->left && relative_coord*relative_coord <= param.radius_squared)
				fixed_radius_search(p->left, param);
		}
	}
}
