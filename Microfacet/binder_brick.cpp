#include "binder_brick.h"
#include "r_geom_hierarchy.h"
#include "microfacet_factory.h"


void binder_brick::set_param(const binder_brick_param &p)
{
	param = p;
}

void* binder_brick::get_param()
{
	return &param;
}

void binder_brick::generate_geom(const float density, const int num_area_hits_scalar)
{
	//processing geom
	r_geometry* g;
	
	float	brick_w = (1-param.bottom_percent_x)/param.x_num,
			brick_h = (1-param.bottom_percent_y)/param.y_num,
			slope_w = (brick_w*(1-param.top_percent_x)/2)/param.x_num,
			slope_h = (brick_h*(1-param.top_percent_y)/2)/param.y_num;

	std::vector<Vector3> verts;
	verts.resize(11);
	verts[0] = Vector3(0, 0, 0);
	verts[1] = Vector3(brick_w, 0, 0);
	verts[2] = Vector3(0, brick_h, 0);
	verts[3] = Vector3(brick_w, brick_h, 0);
	verts[4] = Vector3(slope_w, slope_h, param.height);
	verts[5] = Vector3(brick_w-slope_w, slope_h, param.height);
	verts[6] = Vector3(slope_w, brick_h-slope_h, param.height);
	verts[7] = Vector3(brick_w-slope_w, brick_h-slope_h, param.height);
	verts[8] = Vector3(0, 1.0/param.y_num, 0);
	verts[9] = Vector3(1.0/param.x_num, 1.0/param.y_num, 0);
	verts[10] = Vector3(1.0/param.x_num, 0, 0);

	//bottom
	std::vector<Vector3> bottom_vertice, bottom_normals;
	bottom_vertice.push_back(verts[2]);
	bottom_vertice.push_back(verts[3]);
	bottom_vertice.push_back(verts[8]);
	bottom_vertice.push_back(verts[9]);
	bottom_vertice.push_back(verts[1]);
	bottom_vertice.push_back(verts[10]);

	for (int i = 0; i < 6; i++)
		bottom_normals.push_back(Vector3(0, 0, 1));

	std::vector<triangle_face> bottom_face;
	bottom_face.push_back(triangle_face(1, 2, 0));
	bottom_face.push_back(triangle_face(3, 2, 1));
	bottom_face.push_back(triangle_face(4, 5, 1));
	bottom_face.push_back(triangle_face(5, 3, 1));

	tri_mesh bottom(bottom_vertice, bottom_normals, bottom_face);

	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&bottom);
	mff_singleton::get()->assign_geom("bd_brick_b", std::shared_ptr<r_geometry>(g));
	g->get_mesh()->save_obj("T:/Microfacet/output/brick_bottom.obj");
	//slope
	std::vector<Vector3> slope_vertice;
	int slope_vert[] = {0, 4, 2, 6, 6, 7, 2, 3, 5, 1, 7, 3, 0, 1, 4, 5};
	for (int i = 0; i < 16; i++)
		slope_vertice.push_back(verts[slope_vert[i]]);
	
	std::vector<triangle_face> slope_face;
	for (int i = 0; i < 4; i++)
	{
		int idx = 4*i;
		slope_face.push_back(triangle_face(idx + 0, idx + 1, idx + 2));
		slope_face.push_back(triangle_face(idx + 1, idx + 3, idx + 2));
	}
	tri_mesh slope(slope_vertice, slope_face);
	slope.calculate_face_normal();
	slope.calculate_normal();
	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&slope);
	mff_singleton::get()->assign_geom("bd_brick_s", std::shared_ptr<r_geometry>(g));
	g->get_mesh()->save_obj("T:/Microfacet/output/brick_slope.obj");
	//top
	std::vector<Vector3> top_vertice, top_normals;
	for (int i = 0; i < 4; i++)
	{
		top_vertice.push_back(verts[i + 4]);
		top_normals.push_back(Vector3(0, 0, 1));
	}
	std::vector<triangle_face> top_face;
	top_face.push_back(triangle_face(0, 1, 2));
	top_face.push_back(triangle_face(3, 2, 1));

	tri_mesh top(top_vertice, top_normals, top_face);

	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&top);
	mff_singleton::get()->assign_geom("bd_brick_t", std::shared_ptr<r_geometry>(g));
	g->get_mesh()->save_obj("T:/Microfacet/output/brick_top.obj");
	//prcessing geom_hierarchy
	char name[3][32] = {"bd_brick_b", "bd_brick_s", "bd_brick_t"};

	for (int i = 0; i < 3; i++)
	{
		std::shared_ptr<r_geom_hierarchy> p_gh;
		std::shared_ptr<r_geom_hierarchy> sp_gh = mff_singleton::get()->get_geom_hierarchy(name[i]);

		if (sp_gh != NULL)
		{
			p_gh = sp_gh;
			p_gh->clear_samples();
			p_gh->set_geom(mff_singleton::get()->get_geom(name[i]));
		} 
		else 
		{
			std::vector<point_sample_param> scales;
			point_sample_param pp;

			p_gh = make_shared<r_geom_hierarchy>();
			p_gh->set_geom(mff_singleton::get()->get_geom(name[i]));
			pp.s = 1.0f; pp.num = 3;
			scales.push_back(pp);
			p_gh->set_scales(scales);
			mff_singleton::get()->assign_geom_hierarchy(name[i], std::shared_ptr<r_geom_hierarchy>(p_gh));
		}
		p_gh->sample_points(density, num_area_hits_scalar);
	}
}

void binder_brick::generate(std::vector<instance_property*> &results,
							const Vector3 &v_min, const Vector3 &v_max, 
							const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	for (int y = 0; y < param.y_num; y++)
		for (int x = 0; x < param.x_num; x++)
		{
			instance_property *p;
			
			p = new instance_property;
			p->id = M_ID_BINDER_BRICK_BOTTOM;
			p->setup_matrix(v_min + Vector3((float)x / param.x_num, (float)y / param.y_num, 0.0f), Vector3(1.0f, .0f, .0f), Vector3(.0f, 1.0f, .0f), Vector3(.0f, .0f, 1.0f), 1);
			results.push_back(p);

			p = new instance_property;
			p->id = M_ID_BINDER_BRICK_SLOPE;
			p->setup_matrix(v_min + Vector3((float)x / param.x_num, (float)y / param.y_num, 0.0f), Vector3(1.0f, .0f, .0f), Vector3(.0f, 1.0f, .0f), Vector3(.0f, .0f, 1.0f), 1);
			results.push_back(p);

			p = new instance_property;
			p->id = M_ID_BINDER_BRICK_TOP;
			p->setup_matrix(v_min + Vector3((float)x / param.x_num, (float)y / param.y_num, 0.0f), Vector3(1.0f, .0f, .0f), Vector3(.0f, 1.0f, .0f), Vector3(.0f, .0f, 1.0f), 1);
			results.push_back(p);
		}
}