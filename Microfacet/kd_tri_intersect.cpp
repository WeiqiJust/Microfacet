#include "kd_tri_intersect.h"

bool triangle_intersection(const Ray& ray, const Triangle* tri, Vector3& hit_point, float& distance)
{
	Vector3 e1 = tri->vertex[1] - tri->vertex[0];
	Vector3 e2 = tri->vertex[2] - tri->vertex[0];
	// Calculate planes normal vector
	Vector3 pvec = Cross(ray.direction, e2);
	float det = Dot(e1, pvec);

	// Ray is parallel to plane
	if (det < 1e-8 && det > -1e-8) {
		return false;
	}

	float inv_det = 1 / det;
	Vector3 tvec = ray.origin - tri->vertex[0];
	float u = Dot(tvec, pvec) * inv_det;
	if (u < 0 || u > 1) {
		return false;
	}

	Vector3 qvec = Cross(tvec, e1);
	float v = Dot(ray.direction, qvec) * inv_det;
	if (v < 0 || u + v > 1) {
		return false;
	}
	distance =  Dot(e2, qvec) * inv_det;
	hit_point = ray.origin + ray.direction*distance;
	return true;
}

kd_tri_intersect::kd_tri_intersect() : left(NULL), right(NULL)
{

}

kd_tri_intersect* kd_tri_intersect::build(vector<Triangle*>& tris, int depth) const
{
	kd_tri_intersect* node = new kd_tri_intersect();
	node->triangles = tris;
	node->left = NULL;
	node->right = NULL;
	bounding_box node_bbox;
	node->bbox = node_bbox;

	if (tris.size() == 0)
		return node;
	if (tris.size() == 1)
	{
		node->bbox.add_triangle(*tris[0]);
		node->left = new kd_tri_intersect();
		node->right = new kd_tri_intersect();
		node->left->triangles = vector<Triangle*>();
		node->right->triangles = vector<Triangle*>();
		return node;
	}
	for (int i = 0; i < tris.size(); i++)
		node->bbox.add_triangle(*tris[i]);
	Vector3 mid(0.0f);
	for (int i = 0; i < tris.size(); i++)
	{
		mid += tris[i]->get_mid_point()/tris.size();
	}

	vector<Triangle*> left_tris;
	vector<Triangle*> right_tris;
	int axis = node->bbox.longest_axis();
	for (int i = 0; i < tris.size(); i++)
	{
		switch (axis) 
		{
			case 0:
				mid.x >= tris[i]->get_mid_point().x ? right_tris.push_back(tris[i]) : left_tris.push_back(tris[i]);
				break;
			case 1:
				mid.y >= tris[i]->get_mid_point().y ? right_tris.push_back(tris[i]) : left_tris.push_back(tris[i]);
				break;
			case 2:
				mid.z >= tris[i]->get_mid_point().z ? right_tris.push_back(tris[i]) : left_tris.push_back(tris[i]);
				break;
		}
	}
	if (left_tris.size() == 0 && right_tris.size() > 0) left_tris = right_tris;
	if (right_tris.size() == 0 && left_tris.size() > 0) right_tris = left_tris;

	int match = 0;
	for (int i = 0; i < left_tris.size(); i++)
		for (int j = 0; j < right_tris.size(); j++)
		{
			if (left_tris[i] == right_tris[j])
				match++;
		}
	if ((float)match / left_tris.size() < 0.5 && (float)match / right_tris.size() < 0.5)
	{
		node->left = build(left_tris,  depth + 1);
		node->right = build(right_tris, depth + 1);
	}
	else
	{
		node->left = new kd_tri_intersect();
		node->right = new kd_tri_intersect();
		node->left->triangles = vector<Triangle*>();
		node->right->triangles = vector<Triangle*>();
	}
	return node;
}

bool kd_tri_intersect::hit(const kd_tri_intersect* node, const Ray& ray, intersection_result& result) const
{
	if (node->bbox.hit(ray, 0, 10000.0f))
	{
		if (node->left->triangles.size() > 0 || node->right->triangles.size() > 0)
		{
			bool hit_left = hit(node->left, ray, result);
			bool hit_right = hit(node->right, ray, result);
			return hit_left || hit_right;
		}
		else
		{
			for (int i = 0; i < node->triangles.size(); i++)
			{
				Vector3 hit_point;
				float distance;
				if (triangle_intersection(ray, node->triangles[i], hit_point, distance))
				{
					result.hit_point = hit_point;
					result.triangle = node->triangles[i];
					result.distance = distance;
					return true;
				}
			}
		}
	}
	return false;
}