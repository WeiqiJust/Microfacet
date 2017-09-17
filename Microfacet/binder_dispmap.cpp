#include "MicrofacetLib.h"

using namespace microfacet;

void binder_dispmap::init_obj()
{
	obj.clear();
	obj.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL);
	for (int y = 0; y <= height; y++)
		for (int x = 0; x <= width; x++)
		{
			obj.add_vertex();
			obj.position(x+y*(width+1)) = Vector3((Number)x/width, (Number)y/height, 0);
		}

	triangle_index f;
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			f.i0 = x+y*(width+1);
			f.i1 = x+1+y*(width+1);
			f.i2 = x+(y+1)*(width+1);
			obj.add_face(f);
			f.i0 = x+1+y*(width+1);
			f.i1 = x+1+(y+1)*(width+1);
			f.i2 = x+(y+1)*(width+1);
			obj.add_face(f);
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
	if (width !=  disp_param.res_x || height != disp_param.res_y)
	{
		width	= disp_param.res_x;
		height	= disp_param.res_y;
		dispmap.resize(width*height, 0);
		init_obj();
	}

	for (int y = 0; y <= height; y++)
		for (int x = 0; x <= width; x++)
			obj.position(x+y*(width+1)).z = dispmap[((x) % width)+((height-y) % height)*width]*disp_param.amp;
	Vector3 n(0, 0, 0);
	for (int i = 0; i < obj.num_vertices(); i++)
		obj.normal(i) = n;
	obj.compute_normal();

	for (int y = 1; y < height; y++)
	{
		Vector3 n = obj.normal(0+y*(width+1))+obj.normal(width+y*(width+1));
		n.normalize();
		obj.normal(0+y*(width+1)) = obj.normal(width+y*(width+1)) = n;
	}

	for (int x = 1; x < width; x++)
	{
		Vector3 n = obj.normal(x+0*(width+1))+obj.normal(x+height*(width+1));
		n.normalize();
		obj.normal(x+0*(width+1)) = obj.normal(x+height*(width+1)) = n;
	}

	r_geometry* g;
	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&obj);
	mff_singleton::get()->assign_geom("bd_dispmap", smart_pointer<r_geometry*>(g));

	r_geom_hierarchy *p_gh;
	smart_pointer<r_geom_hierarchy*> sp_gh = mff_singleton::get()->get_geom_hierarchy("bd_dispmap");

	if (sp_gh != NULL)
	{
		p_gh = (r_geom_hierarchy*)sp_gh;
		p_gh->clear_samples();
		p_gh->set_geom(mff_singleton::get()->get_geom("bd_dispmap"));
	} else {
		std::vector<point_sample_param> scales;
		point_sample_param pp;

		p_gh = new r_geom_hierarchy();
		p_gh->set_geom(mff_singleton::get()->get_geom("bd_dispmap"));
		pp.s = 1.0f; pp.num = 1;
		scales.push_back(pp);
		p_gh->set_scales(scales);
		mff_singleton::get()->assign_geom_hierarchy("bd_dispmap", smart_pointer<r_geom_hierarchy*>(p_gh));
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