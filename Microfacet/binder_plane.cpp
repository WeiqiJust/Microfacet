#include "binder_plane.h"
#include "r_geom_hierarchy.h"
#include "microfacet_factory.h"

void binder_plane::set_param(const binder_plane_param &p)
{
	param = p;
}

void* binder_plane::get_param()
{
	return &param;
}

void binder_plane::generate_geom(const float density, const int num_area_hits_scalar)
{
	//processing geom
	r_geometry* g = new r_geometry(mff_singleton::get()->get_handle());
	
	tri_mesh plane;
	for (int y = 0; y <= param.y_res; y++)
		for (int x = 0; x <= param.x_res; x++)
		{
			Vector3 v;
			v.x = (float)x/param.x_res;
			v.y = (float)y / param.y_res;
			v.z = .0f;
			plane.vertices.push_back(v);
			plane.normals.push_back(Vector3(.0f, .0f, 1.0f));
		}

	for (int y = 0; y < param.y_res; y++)
		for (int x = 0; x < param.x_res; x++)
		{
			int v0, v1, v2, v3;
			v0 = x+y*(param.x_res+1);
			v1 = v0+1;
			v2 = x+(y+1)*(param.x_res+1);
			v3 = v2+1;

			plane.faces.push_back(triangle_face(v1, v2, v0));
			plane.faces.push_back(triangle_face(v3, v2, v1));
		}
	g->load(&plane);
	mff_singleton::get()->assign_geom("bd_plane", shared_ptr<r_geometry>(g));

	//prcessing geom_hierarchy
	shared_ptr<r_geom_hierarchy> p_gh;
	shared_ptr<r_geom_hierarchy> sp_gh = mff_singleton::get()->get_geom_hierarchy("bd_plane");

	if (sp_gh != NULL)
	{
		p_gh = sp_gh;
		p_gh->clear_samples();
		p_gh->set_geom(mff_singleton::get()->get_geom("bd_plane"));
	} else {
		std::vector<point_sample_param> scales;
		point_sample_param pp;

		p_gh = make_shared<r_geom_hierarchy>();
		p_gh->set_geom(mff_singleton::get()->get_geom("bd_plane"));
		pp.s = 1;// 1.0f; 
		pp.num = 1;
		scales.push_back(pp);
		p_gh->set_scales(scales);
		mff_singleton::get()->assign_geom_hierarchy("bd_plane", std::shared_ptr<r_geom_hierarchy>(p_gh));
	}
	p_gh->sample_points(density, num_area_hits_scalar);
}

void binder_plane::generate(std::vector<instance_property*> &results,
							const Vector3 &v_min, const Vector3 &v_max, 
							const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	instance_property *p = new instance_property;
	p->id = M_ID_BINDER_PLANE;
	p->setup_matrix(v_min, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
	results.push_back(p);
}