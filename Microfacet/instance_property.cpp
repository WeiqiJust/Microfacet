#include "instance_property.h"
#include "r_geom_hierarchy.h"
#include "r_instance.h"
#include "microfacet_factory.h"
void base_obj::apply_transform(const Matrix4 &mat)
{
	Matrix4 matNormal = mat;
	matNormal.m[0][3] = matNormal.m[1][3] = matNormal.m[2][3] = 0;

	for (int i = 0; i < mesh.get_vertex_number(); i++)
		mesh.vertices[i] = mat*mesh.vertices[i];

	// Potential bugs here!!
	for (int i = 0; i < mesh.get_vertex_number(); i++)
		mesh.normals[i] = Normalize(matNormal*mesh.normals[i]);
	/*
	if (mesh.get_vertex_attr() & VERT_ATTR_TANGENT)
	{
		for (int i = 0; i < mesh.num_vertices(); i++)
		{
			Vector3 t = matNormal*mesh.tangent(i);
			mesh.tangent(i) = (t - (mesh.normal(i)*t)*mesh.normal(i)).normalized_Vector3();
		}
	}
	*/
}

void base_obj::convert_to_instance(r_instance* &result, const int id, D3D_dev_handle *pdev)
{
	r_geometry *p_geom = new r_geometry(pdev);
	p_geom->load(&mesh);
	shared_ptr<r_geom_hierarchy> p_gh = make_shared<r_geom_hierarchy>();
	p_gh->set_geom(shared_ptr<r_geometry>(p_geom));

	instance_property prop;
	prop.id = id;
	prop.setup_matrix(Vector3(0, 0, 0),
		Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
	prop.p_geom = p_gh;
	int mat_id;
	mff_singleton::get()->produce(result, prop, mat_id);
}

void instance_property::setup_matrix(const Vector3 &pos,
	const Vector3 &x, const Vector3 &y, const Vector3 &z,
	const float s)
{
	Matrix4 mat_scale;
	mat_scale = mat_scale * Scale(s, s, s);

	Matrix4 mat_rot_trans(x.x, y.x, z.x, pos.x,
		x.y, y.y, z.y, pos.y,
		x.z, y.z, z.z, pos.z,
		0, 0, 0, 1);

	mat = mat_rot_trans*mat_scale;

	scale = s;
}
