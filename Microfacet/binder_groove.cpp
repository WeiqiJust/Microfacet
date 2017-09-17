#include "binder_groove.h"
#include "r_geom_hierarchy.h"
#include "microfacet_factory.h"

void binder_groove::set_param(const binder_groove_param &p)
{
	param = p;
}

void* binder_groove::get_param()
{
	return &param;
}

void binder_groove::generate_geom(const float density, const int num_area_hits_scalar)
{
	//processing geom
	r_geometry* g;
	
	tri_mesh plane, hill;
	for (int y = 0; y <= 1; y++)
		for (int x = 0; x <= 1; x++)
		{
			Vector3 v;
			v.x = (float)x*param.plane_percent;
			v.y = (float)y;
			v.z = 0.0f;
			plane.vertices.push_back(v);
			plane.normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		}

	plane.faces.push_back(triangle_face(1, 2, 0));
	plane.faces.push_back(triangle_face(3, 2, 1));

	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&plane);
	mff_singleton::get()->assign_geom("bd_groove_p", shared_ptr<r_geometry>(g));

	for (int y = 0; y <= 1; y++)
	{
		Vector3 v;
		v.x = 1.0f*param.plane_percent;
		v.y = (float)y;
		v.z = 0.0f;
		hill.vertices.push_back(v);
	}
	
	for (int x = 0; x <= 1; x++)
		for (int y = 0; y <= 1; y++)
		{
			Vector3 v;
			v.x = 1.0f*param.plane_percent + 0.5f*(1.0f-param.plane_percent);
			v.y = (float)y;
			v.z = param.height;
			hill.vertices.push_back(v);
		}

	for (int y = 0; y <= 1; y++)
	{
		Vector3 v;
		v.x = 1.0f;
		v.y = (float)y;
		v.z = 0;
		hill.vertices.push_back(v);
	}

	for (int i = 0; i <= 1; i++)
	{
		int offset = i*4;
		hill.faces.push_back(triangle_face(2 + offset, 1 + offset, 0 + offset));
		hill.faces.push_back(triangle_face(3 + offset, 1 + offset, 1 + offset));
	}

	/* Original normals calculation
	hill.compute_face_normal();
	for (int i = 0; i < 4; i++)
	{
		hill.normal(i)	= hill.face_normal(0);
		hill.normal(i+4)= hill.face_normal(2);
	}*/

	hill.calculate_face_normal();
	hill.calculate_normal();

	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&hill);
	mff_singleton::get()->assign_geom("bd_groove_h", shared_ptr<r_geometry>(g));


	//prcessing geom_hierarchy
	char name[2][32] = {"bd_groove_p", "bd_groove_h"};

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
			pp.s = 1.0f; pp.num = 5;
			scales.push_back(pp);
			p_gh->set_scales(scales);
			mff_singleton::get()->assign_geom_hierarchy(name[i], shared_ptr<r_geom_hierarchy>(p_gh));
		}
		p_gh->sample_points(density, num_area_hits_scalar);
	}
}

void binder_groove::generate(std::vector<instance_property*> &results,
							const Vector3 &v_min, const Vector3 &v_max, 
							const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	for (int i = 0; i < 1; i++)
	{
		instance_property *p;
		
		p = new instance_property;
		p->id = M_ID_BINDER_GROOVE_PLANE;
		p->setup_matrix(v_min+Vector3((float)i, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
		results.push_back(p);

		p = new instance_property;
		p->id = M_ID_BINDER_GROOVE_HILL;
		p->setup_matrix(v_min+Vector3((float)i, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
		results.push_back(p);
	}
}