#pragma once
#include "utils.h"

struct triangle_face
{
	triangle_face() {};
	triangle_face(int i1, int i2, int i3) : v1(i1), v2(i2), v3(i3) {};
	int v1, v2, v3;
};

class tri_mesh
{
public:
	tri_mesh() {};

	tri_mesh(const char * path);

	tri_mesh(const std::vector<Vector3> vertex, const std::vector<triangle_face> face);

	tri_mesh(const std::vector<Vector3> vertex, const std::vector<Vector3> normal, const std::vector<triangle_face> face);

	tri_mesh(const std::vector<Vector3> vertex,
		const std::vector<Vector3> normal, const std::vector<Vector2> uv,
		const std::vector<Vector3> tangent, const std::vector<triangle_face> face);

	void merge_mesh(const std::vector<Vector3> vertex, const std::vector<triangle_face> face);

	void clear();

	bool load_obj(const char * path);

	void calculate_face_area();

	void calculate_normal();

	void calculate_tangent();

	void calculate_face_normal();

	void mark_attribute();

	void set_attribute(int attribute) { attr = attribute; }

	size_t get_vertex_number() const { return vertices.size(); }

	size_t get_normal_number() const { return normals.size(); }

	size_t get_uv_number() const { return uvs.size(); }

	size_t get_tangent_number() const { return tangents.size(); }

	size_t get_face_number() const { return faces.size(); }

	void invert_face(const int fidx);

	void invert_all_faces();

	int get_attr() const { return attr; }

	void save_obj(string filename);

	// consider calculate tangent and bitangent in the future
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;
	std::vector<Vector3> tangents;
	std::vector<triangle_face> faces;
	std::vector<Vector3> face_normals;
	std::vector<float> face_areas;

protected:
	int				attr;
};