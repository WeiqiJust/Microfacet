#include "binder_random_surface.h"
#include "r_geom_hierarchy.h"
#include "microfacet_factory.h"

void binder_dispmap::init_obj()
{
	obj.clear();
	for (int y = 0; y <= height; y++)
		for (int x = 0; x <= width; x++)
		{
			obj.vertices.push_back(Vector3((float)x / width, (float)y / height, .0f));
		}

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			obj.faces.push_back(triangle_face(x + y*(width + 1), x + 1 + y*(width + 1), x + (y + 1)*(width + 1)));
			obj.faces.push_back(triangle_face(x + 1 + y*(width + 1), x + 1 + (y + 1)*(width + 1), x + (y + 1)*(width + 1)));
		}

}

void binder_dispmap::set_param(const binder_dispmap_param &p)
{
	disp_param = p;
}

void* binder_dispmap::get_param()
{
	return &disp_param;
}

void binder_dispmap::set_dispmap(const std::vector<float> &data)
{
	dispmap = data;
}

void binder_dispmap::generate_geom_from_dispmap(const float density, const int num_area_hits_scalar)
{
	if (width != disp_param.res_x || height != disp_param.res_y)
	{
		width = disp_param.res_x;
		height = disp_param.res_y;
		dispmap.resize(width*height, 0);
		init_obj();
	}

	for (int y = 0; y <= height; y++)
		for (int x = 0; x <= width; x++)
			obj.vertices[x + y*(width + 1)].z = dispmap[((x) % width) + ((height - y) % height)*width] * disp_param.amp;
	obj.calculate_face_normal();
	obj.calculate_normal();

	for (int y = 1; y < height; y++)
	{
		Vector3 n = obj.normals[0 + y*(width + 1)] + obj.normals[width + y*(width + 1)];
		n = Normalize(n);
		obj.normals[0 + y*(width + 1)] = obj.normals[width + y*(width + 1)] = n;
	}

	for (int x = 1; x < width; x++)
	{
		Vector3 n = obj.normals[x + 0 * (width + 1)] + obj.normals[x + height*(width + 1)];
		n = Normalize(n);
		obj.normals[x + 0 * (width + 1)] = obj.normals[x + height*(width + 1)] = n;
	}

	r_geometry* g;
	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&obj);
	mff_singleton::get()->assign_geom("bd_dispmap", shared_ptr<r_geometry>(g));

	shared_ptr<r_geom_hierarchy> p_gh;
	shared_ptr<r_geom_hierarchy> sp_gh = mff_singleton::get()->get_geom_hierarchy("bd_dispmap");

	if (sp_gh != NULL)
	{
		p_gh = sp_gh;
		p_gh->clear_samples();
		p_gh->set_geom(mff_singleton::get()->get_geom("bd_dispmap"));
	}
	else {
		std::vector<point_sample_param> scales;
		point_sample_param pp;

		p_gh = make_shared<r_geom_hierarchy>();;
		p_gh->set_geom(mff_singleton::get()->get_geom("bd_dispmap"));
		pp.s = 1.0f; pp.num = 1;
		scales.push_back(pp);
		p_gh->set_scales(scales);
		mff_singleton::get()->assign_geom_hierarchy("bd_dispmap", std::shared_ptr<r_geom_hierarchy>(p_gh));
	}
	p_gh->sample_points(density, num_area_hits_scalar);
}

void binder_dispmap::generate_geom(const float density, const int num_area_hits_scalar)
{
	generate_geom_from_dispmap(density, num_area_hits_scalar);
}

void binder_dispmap::generate(std::vector<instance_property*> &results,
	const Vector3 &v_min, const Vector3 &v_max,
	const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	instance_property *p;

	p = new instance_property;
	p->id = M_ID_BINDER_DISPMAP;
	p->setup_matrix(v_min, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
	results.push_back(p);
}

void binder_random_surface::generate_geom(const float density, const int num_area_hits_scalar)
{
	if (width !=  disp_param.res_x || height != disp_param.res_y)
	{
		width	= disp_param.res_x;
		height	= disp_param.res_y;
		dispmap.resize(width*height);
		init_obj();
	}

	for (int i = 0; i < dispmap.size(); i++)
		dispmap[i] = rng.get_random_float();

	generate_geom_from_dispmap(density, num_area_hits_scalar);
}
