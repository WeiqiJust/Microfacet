#include "binder_regpoly.h"
#include "r_geom_hierarchy.h"
#include "microfacet_factory.h"

void binder_regpoly::set_param(const binder_regpoly_param &p)
{
	param = p;
}

void* binder_regpoly::get_param()
{
	return &param;
}

void binder_regpoly::generate_geom(const float density, const int num_area_hits_scalar)
{
	//processing geom
	r_geometry* g;

	//plane
	tri_mesh plane;
	for (int i = 0; i < 4; i++)
	{
		plane.normals.push_back(Vector3(0, 0, 1));
	}
	plane.vertices.push_back(Vector3(0, 0, 0));
	plane.vertices.push_back(Vector3(1, 0, 0));
	plane.vertices.push_back(Vector3(0, 1, 0));
	plane.vertices.push_back(Vector3(1, 1, 0));

	plane.faces.push_back(triangle_face(1, 2, 0));
	plane.faces.push_back(triangle_face(3, 2, 1));

	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&plane);
	mff_singleton::get()->assign_geom("bd_plane", shared_ptr<r_geometry>(g));

	//regpoly
	tri_mesh regpoly;

	regpoly.vertices.push_back(Vector3(0.5, 0.5, param.height));

	std::vector<Vector3> verts;
	verts.resize(param.num_edges);
	for (int i = 0; i < param.num_edges; i++)
	{
		float	theta = 2*PI*i/param.num_edges, 
				x = cos(theta), 
				y = sin(theta);
		verts[i] = Vector3(x, y, 0);
	}

	for (int i = 0; i < param.num_edges; i++)
	{
		int v0, v1, v2, v3;
		regpoly.vertices.push_back(verts[i]*param.radius_bottom+Vector3(0.5, 0.5, 0));
		v0 = regpoly.get_vertex_number() - 1;
		regpoly.vertices.push_back(verts[(i + 1) % param.num_edges] * param.radius_bottom + Vector3(0.5, 0.5, 0));
		v1 = regpoly.get_vertex_number() - 1;
		regpoly.vertices.push_back(verts[i] * param.radius_top + Vector3(0.5, 0.5, param.height));
		v2 = regpoly.get_vertex_number() - 1;
		regpoly.vertices.push_back(verts[(i + 1) % param.num_edges] * param.radius_top + Vector3(0.5, 0.5, param.height));
		v3 = regpoly.get_vertex_number() - 1;

		regpoly.faces.push_back(triangle_face(v1, v2, v0));
		regpoly.faces.push_back(triangle_face(v3, v2, v1));
	}

	for (int i = 0; i < param.num_edges; i++)
	{
		int v0, v1;
		regpoly.vertices.push_back(verts[i] * param.radius_top + Vector3(0.5, 0.5, param.height));
		v0 = regpoly.get_vertex_number() - 1;
		regpoly.vertices.push_back(verts[(i + 1) % param.num_edges] * param.radius_top + Vector3(0.5, 0.5, param.height));
		v1 = regpoly.get_vertex_number() - 1;

		regpoly.faces.push_back(triangle_face(v1, 0, v0));
	}
	regpoly.calculate_face_normal();
	regpoly.calculate_normal();
	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&regpoly);
	mff_singleton::get()->assign_geom("bd_regpoly", shared_ptr<r_geometry>(g));

	//prcessing geom_hierarchy
	char name[][32] = {"bd_plane", "bd_regpoly"};

	for (int i = 0; i < 2; i++)
	{
		shared_ptr<r_geom_hierarchy> p_gh;
		shared_ptr<r_geom_hierarchy> sp_gh = mff_singleton::get()->get_geom_hierarchy(name[i]);

		if (sp_gh != NULL)
		{
			p_gh = sp_gh;
			p_gh->clear_samples();
			p_gh->set_geom(mff_singleton::get()->get_geom(name[i]));
		} else {
			std::vector<point_sample_param> scales;
			point_sample_param pp;

			p_gh = make_shared<r_geom_hierarchy>();
			p_gh->set_geom(mff_singleton::get()->get_geom(name[i]));
			pp.s = 1.0f; pp.num = 3;
			scales.push_back(pp);
			p_gh->set_scales(scales);
			mff_singleton::get()->assign_geom_hierarchy(name[i], shared_ptr<r_geom_hierarchy>(p_gh));
		}
		p_gh->sample_points(density, num_area_hits_scalar);
	}
}

void binder_regpoly::generate(std::vector<instance_property*> &results,
							const Vector3 &v_min, const Vector3 &v_max, 
							const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	instance_property *p;
	
	p = new instance_property;
	p->id = M_ID_BINDER_PLANE;
	p->setup_matrix(v_min, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
	results.push_back(p);

	p = new instance_property;
	p->id = M_ID_BINDER_REGPOLY;
	p->setup_matrix(v_min, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
	results.push_back(p);
}