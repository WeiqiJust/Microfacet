#pragma once
#include "GPGPU_env_D3D11.h"
#include "tri_mesh_d3dx11.h"

class r_input_layer
{
public:
	r_input_layer(D3D_dev_handle *pdev);
	~r_input_layer();

	void init(const tri_mesh_d3dx11 *pmesh,
		void *shader_sig, SIZE_T bytecode_length);
	void set();
private:
	D3D_dev_handle		*pdev;
	ID3D11InputLayout	*p_vertexlayout;
};

class r_geometry
{
public:
	r_geometry(D3D_dev_handle* pdev, bool b_face_normal = false);
	~r_geometry();

	void load(const char *filename, Matrix4 *mat = NULL);
	void load(const tri_mesh *init_mesh, bool b_no_compute_normal = false);

	tri_mesh_d3dx11* get_mesh();

	float get_total_area()	{ return total_area; }

	void set();
	void draw();

	void set_face_scalar(const std::vector<float> &fs); // this function is not used
	// processing geometry and sample small scale details 3.2
	void sample_points(
		std::vector<Vector3> &points,
		std::vector<normal_weight> &normals,
		const float density,
		const int num_area_hits_scalar,
		random &rng,
		const Vector2 &random_offset);

private:
	D3D_dev_handle*		pdev;
	ID3D11Buffer		*p_vb, *p_ib;
	UINT				size_per_vertex, num_verts, num_faces;

	bool				b_face_normal;
	tri_mesh_d3dx11		*p_mesh;

	std::vector<float>	face_scalar;
	float				total_area;
	step_1D_prob		face_sampler;

	void convert_from_tri_mesh();
};