#include "r_geometry.h"
#include "utils.h"
#include "kd_tree_3d_vec.h"
#include "hammersley.h"

r_input_layer::r_input_layer(D3D_dev_handle* pdev)
	:p_vertexlayout(NULL), pdev(pdev)
{

}

r_input_layer::~r_input_layer()
{
	SAFE_RELEASE(p_vertexlayout);
}

void r_input_layer::init(const tri_mesh_d3dx11 *pmesh,
	void *shader_sig, SIZE_T bytecode_length)
{
	//create input vertex layout
	D3D11_INPUT_ELEMENT_DESC decls[64];
	int	decls_count, sz;
	pmesh->get_layout(decls_count, sz, decls);

	pdev->get_device()->CreateInputLayout(decls, decls_count,
		shader_sig, bytecode_length, &p_vertexlayout);
}

void r_input_layer::set()
{
	pdev->get_context()->IASetInputLayout(p_vertexlayout);
}

r_geometry::r_geometry(D3D_dev_handle* pdev, bool b_face_normal)
	: pdev(pdev), p_mesh(NULL), p_ib(NULL), p_vb(NULL),
	b_face_normal(b_face_normal)
{
}

r_geometry::~r_geometry()
{
	SAFE_RELEASE(p_ib);
	SAFE_RELEASE(p_vb);
	SAFE_DELETE(p_mesh);
}

void r_geometry::convert_from_tri_mesh()
{
	num_verts = p_mesh->get_vertex_number();
	num_faces = p_mesh->get_face_number();

	//compute size_per_vertex
	D3D11_INPUT_ELEMENT_DESC decls[64];
	int	decls_count, sz;
	p_mesh->get_layout(decls_count, sz, decls);
	size_per_vertex = UINT(sz);


	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA initdata;
	void *vb_data, *ib_data;
	p_mesh->create_buffer_data(vb_data, ib_data, false);

	//create vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = num_verts*size_per_vertex;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&initdata, sizeof(initdata));
	initdata.pSysMem = vb_data;
	HRESULT hr = pdev->get_device()->CreateBuffer(&bd, &initdata, &p_vb);
	if (FAILED(hr))
	{
		cout << "Error: Create vertex buffer failed!" << endl;
	}

	//create index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = num_faces * 3 * sizeof(UINT);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initdata.pSysMem = ib_data;
	hr = pdev->get_device()->CreateBuffer(&bd, &initdata, &p_ib);
	if (FAILED(hr))
	{
		cout << "Error: Create index buffer failed!" << endl;
	}
	if (p_mesh->face_areas.size() == 0)
		p_mesh->calculate_face_area();
	for (int i = 0; i < p_mesh->get_face_number(); i++)
	{
		total_area += p_mesh->face_areas[i];
	}
	face_sampler = step_1D_prob(p_mesh->face_areas);
}

void r_geometry::load(const char *filename, Matrix4 *mat)
{
	p_mesh = new tri_mesh_d3dx11(pdev->get_device());

	p_mesh->load_obj(filename);
	//p_mesh->invert_all_faces(); //from original code, do not understand why
	if (mat)
	{
		for (int i = 0; i < p_mesh->get_vertex_number(); i++)
			p_mesh->vertices[i] = (*mat) * p_mesh->vertices[i];
		p_mesh->calculate_normal();
	}
	p_mesh->mark_attribute();
	convert_from_tri_mesh();
}

void r_geometry::load(const tri_mesh *init_mesh, bool b_no_compute_normal)
{
	p_mesh = new tri_mesh_d3dx11(pdev->get_device(), init_mesh->vertices, init_mesh->normals, init_mesh->uvs, init_mesh->tangents, init_mesh->faces);
	//cout << "load geometry " << p_mesh->get_attr() << endl;
	convert_from_tri_mesh();
}

tri_mesh_d3dx11* r_geometry::get_mesh()
{
	return p_mesh;
}

void r_geometry::set()
{
	UINT offset = 0;

	pdev->get_context()->IASetVertexBuffers(0, 1, &p_vb, &size_per_vertex, &offset);
	pdev->get_context()->IASetIndexBuffer(p_ib, DXGI_FORMAT_R32_UINT, 0);

	pdev->get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void r_geometry::draw()
{
	pdev->get_context()->DrawIndexed(num_faces * 3, 0, 0);
}

void r_geometry::set_face_scalar(const std::vector<float> &fs)
{
	float actual_area = 0;
	total_area = 0;
	face_scalar = fs;

	for (int i = 0; i < p_mesh->get_face_number(); i++)
	{
		float area = face_scalar[i] * p_mesh->face_areas[i];
		total_area += area;
		actual_area += p_mesh->face_areas[i];
	}

	face_sampler = step_1D_prob(p_mesh->face_areas);
	for (int i = 0; i < face_scalar.size(); i++)
		face_scalar[i] *= actual_area / total_area;
	total_area = actual_area;
}

void r_geometry::sample_points(std::vector<Vector3> &points,
	std::vector<normal_weight> &normals,
	const float density,
	const int num_area_hits_scalar,
	random &rng,
	const Vector2 &random_offset)
{
	//cout << "in sample points total_area = " << total_area << endl;
	int num_samples = int(total_area*density);
	if (rng.get_random_float_open() < total_area*density - num_samples)
		num_samples++;

	points.resize(num_samples);
	normals.resize(num_samples);

	if (num_samples == 0)
		return;

	if (face_scalar.size() == 0)
		face_scalar.assign(p_mesh->get_face_number(), 1.0f);

	hammersley seq_face(num_samples, 1);
	//codex::math::prob::uniform_triangle_barycentric<float> prob_tri;
	int n_sample = 0, write_pos = 0, next_face, face;

	float rv;
	rv = seq_face.get_sample()[0];
	rv = add_random_offset(rv, random_offset.x);
	//rv = rng.rand_real_open();
	face_sampler.sample(face, rv);

	//sample points face-by-face
	while (n_sample < num_samples)
	{
		int count = 0;
		// randomly select a face. One face may have multiple center and normals
		do {
			if (num_samples > 4)
			{
				rv = seq_face.get_sample()[0];
				rv = add_random_offset(rv, random_offset.x);
			}
			else
				rv = rng.get_random_float();
			face_sampler.sample(next_face, rv);
			count++;
			n_sample++;
		} while (next_face == face && n_sample < num_samples);

		hammersley seq_uv(count, 1);
		for (int i = 0; i < count; i++)
		{
			Vector2 uv, rv2;
			if (count <= 4)
			{
				rv2.x = rng.get_random_float();
				rv2.y = rng.get_random_float();
			}
			else {
				rv2 = seq_uv.get_sample()[0];
				rv2.x = add_random_offset(rv2.x, random_offset.x);
				rv2.y = add_random_offset(rv2.y, random_offset.y);
			}
			// sample a point in side the triangle by propotion
			//prob_tri.sample(uv, pdf, rv2);

			{
				float rt_u = sqrt(rv2.x);

				uv.x = 1 - rt_u;
				uv.y = rv2.y * rt_u;
			}

			float tu, tv, tw;
			tu = uv.x;
			tv = uv.y;
			tw = 1 - tu - tv;

			const triangle_face f = p_mesh->faces[face];

			// get the sampled point
			Vector3 p = p_mesh->vertices[f.v3] * tu +
				p_mesh->vertices[f.v2] * tv +
				p_mesh->vertices[f.v1] * tw;
			Vector3	n;

			//get the normal of the center point
			if (b_face_normal)
				n = p_mesh->face_normals[face];
			else
				n = p_mesh->normals[f.v3] * tu +
				p_mesh->normals[f.v2] * tv +
				p_mesh->normals[f.v1] * tw;

			points[write_pos] = Vector3(p.x, p.y, p.z);
			normals[write_pos].normal = Normalize(Vector3(n.x, n.y, n.z));
			normals[write_pos].weight = 0;
			write_pos++;
		}

		face = next_face;
	}

	kd_tree_3d_vec tree(&points, points.size());

	//std::vector<int> hit;
	//hit.resize(num_samples, 0);


	//sample new points to calculate each facets area proportioanl to the number of points that are closest to its center
	float avg_r_2 = 9.0f / (PI*density);

	int num_total = num_samples*num_area_hits_scalar;
	for (int i = 0; i < num_total; i++)
	{
		int face;
		face_sampler.sample(face, rng.get_random_float());

		Vector2 uv = Vector2(rng.get_random_float(), rng.get_random_float());
		//prob_tri.sample(uv, pdf, Vector2(rng.rand_real(), rng.rand_real()));

		float tu, tv, tw;
		tu = 1 - sqrt(uv.x);;
		tv = sqrt(uv.x) * (1 - uv.y);
		tw = 1 - tu - tv;

		const triangle_face f = p_mesh->faces[face];
		Vector3 p = p_mesh->vertices[f.v3] * tu +
			p_mesh->vertices[f.v2] * tv +
			p_mesh->vertices[f.v1] * tw;

		int index;
		float r2 = avg_r_2;
		do {
			tree.fixed_radius_search(Vector3(p.x, p.y, p.z), r2, 1, &index);
			r2 *= 2;
		} while (index == -1);
		//hit[index]++;
		normals[index].weight += total_area / (face_scalar[face] * num_total);
	}

	////DEBUG:Test
	//float normal_weight = 0;
	//for (int i = 0; i < normals.size(); i++)
	//	normal_weight += normals[i].weight;
	//printf_s("\nweight %g %g\n", total_area, normal_weight);

	//Number avg_area_per_hit = total_area / (num_samples*num_area_hits_scalar);
	//for (int i = 0; i < num_samples; i++)
	//	normals[i].weight = avg_area_per_hit*hit[i];
}
