#include "kd_tree_3d_vec.h"


kd_tree_3d_vec::kd_tree_3d_vec(const std::vector<Vector3> *v_arr, int num_pts)
	: v_arr(v_arr), num_pts(num_pts), root(NULL)
{
	std::vector<int> temp_vec;

	for (int i = 0; i<num_pts; i++)
		temp_vec.push_back(i);

	build(root, 0, num_pts - 1, temp_vec);
}

kd_tree_3d_vec::kd_tree_3d_vec()
	: v_arr(NULL), num_pts(0), root(NULL)
{

}

void kd_tree_3d_vec::init(const std::vector<Vector3> *arr, int n_pts)
{
	std::vector<int> temp_vec;

	v_arr = arr;
	num_pts = n_pts;
	for (int i = 0; i<num_pts; i++)
		temp_vec.push_back(i);

	build(root, 0, num_pts - 1, temp_vec);
}


kd_tree_3d_vec::~kd_tree_3d_vec()
{
	SAFE_DELETE(root);
}

void kd_tree_3d_vec::build(kd_tree_3d_node* &p, int start, int end, std::vector<int> &temp_vec)
{
	if (start >= end)
	{
		if (start == end)
		{
			p = new kd_tree_3d_node();
			p->index = temp_vec[start];
		}
		else {
			p = NULL;
		}
		return;
	}

	// build bounding box
	float min_p[3], max_p[3];

	for (int i = 0; i<3; i++)
		max_p[i] = min_p[i] = (*v_arr)[temp_vec[start]][i];

	for (int i = start + 1; i <= end; i++)
		for (int j = 0; j<3; j++)
		{
			min_p[j] = (std::min)(min_p[j], (*v_arr)[temp_vec[i]][j]);
			max_p[j] = (std::max)(max_p[j], (*v_arr)[temp_vec[i]][j]);
		}

	// find the longest axis
	int longest_axis;

	for (int i = 0; i < 3; i++)
		if (max_p[i] - min_p[i] >= max_p[(i + 1) % 3] - min_p[(i + 1) % 3] &&
			max_p[i] - min_p[i] >= max_p[(i + 2) % 3] - min_p[(i + 2) % 3])
		{
			longest_axis = i;
			break;
		}

	int split_pos = (start + end) / 2;

	std::nth_element(&temp_vec[start], &temp_vec[split_pos], 1 + &temp_vec[end], kd_compare_node_3d_vec(v_arr, longest_axis));

	p = new kd_tree_3d_node();
	p->axis = (unsigned int)longest_axis;
	p->index = temp_vec[split_pos];

	build(p->left, start, split_pos - 1, temp_vec);
	build(p->right, split_pos + 1, end, temp_vec);
}

int kd_tree_3d_vec::fixed_radius_search(const Vector3 &p, const float radius_squared, int num_lookup, int *index) const
{
	kd_search_param_3d	param;

	param.p[0] = (float)p.x;
	param.p[1] = (float)p.y;
	param.p[2] = (float)p.z;
	param.num_lookup = num_lookup;
	param.radius_squared
		= (float)radius_squared;

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

int kd_tree_3d_vec::fixed_radius_search_with_dist(const Vector3 &p, const float radius_squared, int num_lookup, int *index, float *dist) const
{
	kd_search_param_3d	param;

	param.p[0] = (float)p.x;
	param.p[1] = (float)p.y;
	param.p[2] = (float)p.z;
	param.num_lookup = num_lookup;
	param.radius_squared
		= (float)radius_squared;

	fixed_radius_search(root, param);

	int	queue_size =(std::min)((int)param.q.size(), num_lookup);

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

void kd_tree_3d_vec::process_current_node(kd_tree_3d_node *p, kd_search_param_3d &param) const
{
	index_dist_3d d;

	d.index = p->index;
	d.dist_squared =
		((*v_arr)[p->index][0] - param.p[0])*((*v_arr)[p->index][0] - param.p[0]) +
		((*v_arr)[p->index][1] - param.p[1])*((*v_arr)[p->index][1] - param.p[1]) +
		((*v_arr)[p->index][2] - param.p[2])*((*v_arr)[p->index][2] - param.p[2]);

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

void kd_tree_3d_vec::fixed_radius_search(kd_tree_3d_node *p, kd_search_param_3d &param) const
{
	if (!p) return;

	float relative_coord = param.p[p->axis] - (*v_arr)[p->index][p->axis];

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
