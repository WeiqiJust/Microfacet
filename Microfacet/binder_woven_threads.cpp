#include "binder_woven_threads.h"
#include "r_geom_hierarchy.h"
#include "microfacet_factory.h"

#define DISTANCE_GUTTER		0.0001f

void binder_woven_threads::set_param(const binder_woven_threads_param &p)
{
	param = p;
}

void* binder_woven_threads::get_param()
{
	return &param;
}

float binder_woven_threads::find_equal_dist_point(const float theta,
												  const float a,
												  const float b,
												  const float dist)
{
	float theta_l, theta_r;
	theta_l = theta;

	float min_r = min(a, b);
	theta_r = theta + 2*acosf((dist/2)/min_r);

	float x1, y1, x2, y2;
	x1 = a*cosf(theta);
	y1 = b*sinf(theta);

	while (theta_r-theta_l > 0.003f)
	{
		float m = (theta_l+theta_r)/2;
		x2 = a*cosf(m);
		y2 = b*sinf(m);
		float	dx = x1-x2,
				dy = y1-y2;
		if (dx*dx+dy*dy < dist*dist)
			theta_l = m;
		else
			theta_r = m;
	}

	return (theta_l+theta_r)/2;
}

void binder_woven_threads::generate_geom(const float density, const int num_area_hits_scalar)
{
	//processing geom
	r_geometry* g;

	std::vector<Vector3> bisect;
	std::vector<Vector3> pts;
	float angle;
	bisect.resize(param.bisect_verts);
	pts.resize(5);

	//y_woven_threads
	for (int i = 0; i < param.bisect_verts; i++)
	{
		float t = (float)i/param.bisect_verts*2*PI;
		bisect[i].x = param.thread_r_y*cosf(t);
		bisect[i].y = 0;
		bisect[i].z = param.thread_r_y*sinf(t);
	}

	disp_y.clear();
	angle = 0;
	while (angle < PI)
	{
		disp_y.push_back(Vector3(param.bisect_w_y*cosf(angle), 0, param.bisect_h_y*sinf(angle)));
		angle = find_equal_dist_point(angle, param.bisect_w_y, param.bisect_h_y, 2*param.thread_r_y+DISTANCE_GUTTER);
	}
	//for (int i = 0; i < param.bisect_verts; i++)
	//{
	//	float t = (float)i/param.bisect_verts*2*PI;
	//	bisect[i].x = param.bisect_w_y*cosf(t);
	//	bisect[i].y = 0;
	//	bisect[i].z = param.bisect_h_y*sinf(t);
	//}

	for (int i = -1; i <= 3; i++)
		pts[i+1] = Vector3(0, (float)i/2/param.y_num, (i % 2 == 0) ? param.bisect_h_y : param.bisect_h_y+param.height);

	int path_verts_y = max(param.total_path_verts / param.y_num, 3);
	tri_mesh y_woven_threads;
	y_woven_threads.vertices.resize(param.bisect_verts*path_verts_y);
	for (int i = 0; i < path_verts_y; i++)
	{
		float	global_t = (float)i/(path_verts_y-1)*2;
		int		i_t = min((int)global_t, 1);
		float	t = global_t - i_t;

		Vector3 p;
		CatmullRom(p, t, pts[i_t], pts[i_t+1], pts[i_t+2], pts[i_t+3]);
		int idx = i*param.bisect_verts;
		for (int j = 0; j < param.bisect_verts; j++)
			y_woven_threads.vertices[idx + j] = (bisect[j] + Vector3(p.x, p.y, p.z));
	}

	for (int i = 0; i < path_verts_y-1; i++)
	{
		int idx = i*param.bisect_verts;
		for (int j = 0; j < param.bisect_verts; j++)
		{
			y_woven_threads.faces.push_back(triangle_face(idx + param.bisect_verts + j, idx + ((j + 1) % param.bisect_verts), idx + j));
			y_woven_threads.faces.push_back(triangle_face(idx + ((j + 1) % param.bisect_verts), idx + param.bisect_verts + j, idx + param.bisect_verts + ((j + 1) % param.bisect_verts)));
		}
	}
	y_woven_threads.calculate_face_normal();
	y_woven_threads.calculate_normal();

	for (int i = 0; i < param.bisect_verts; i++)
	{
		int idx = (path_verts_y-1)*param.bisect_verts;
		Vector3 n = y_woven_threads.normals[i]+y_woven_threads.normals[idx+i];
		n = Normalize(n);

		y_woven_threads.normals[i] = y_woven_threads.normals[idx+i] = n;
	}

	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&y_woven_threads);
	mff_singleton::get()->assign_geom("bd_thread_y", shared_ptr<r_geometry>(g));
	y_woven_threads.save_obj("T:/Microfacet/output/y_woven.obj");
	//x_woven_threads
	//for (int i = 0; i < param.bisect_verts; i++)
	//{
	//	float t = (float)i/param.bisect_verts*2*PI;
	//	bisect[i].x = 0;
	//	bisect[i].y = param.bisect_w_x*cosf(t);
	//	bisect[i].z = param.bisect_h_x*sinf(t);
	//}
	for (int i = 0; i < param.bisect_verts; i++)
	{
		float t = (float)i/param.bisect_verts*2*PI;
		bisect[i].x = 0;
		bisect[i].y = param.thread_r_x*cosf(t);
		bisect[i].z = param.thread_r_x*sinf(t);
	}

	disp_x.clear();
	angle = 0;
	while (angle < PI)
	{
		disp_x.push_back(Vector3(0, param.bisect_w_x*cosf(angle), param.bisect_h_x*sinf(angle)));
		angle = find_equal_dist_point(angle, param.bisect_w_x, param.bisect_h_x, 2*param.thread_r_x+DISTANCE_GUTTER);
	}

	for (int i = -1; i <= 3; i++)
		pts[i+1] = Vector3((float)i/2/param.x_num, 0, (i % 2 != 0) ? param.bisect_h_x : param.bisect_h_x+param.height);

	int path_verts_x = max(param.total_path_verts / param.x_num, 3);
	tri_mesh x_woven_threads;
	x_woven_threads.vertices.resize(param.bisect_verts*path_verts_x);
	for (int i = 0; i < path_verts_x; i++)
	{
		float	global_t = (float)i/(path_verts_x-1)*2;
		int		i_t = min((int)global_t, 1);
		float	t = global_t - i_t;

		Vector3 p;
		CatmullRom(p, t, pts[i_t], pts[i_t+1], pts[i_t+2], pts[i_t+3]);
		int idx = i*param.bisect_verts;
		for (int j = 0; j < param.bisect_verts; j++)
			x_woven_threads.vertices[idx + j] = (bisect[j] + Vector3(p.x, p.y, p.z));
	}

	for (int i = 0; i < path_verts_x-1; i++)
	{
		int idx = i*param.bisect_verts;
		for (int j = 0; j < param.bisect_verts; j++)
		{
			x_woven_threads.faces.push_back(triangle_face(idx + j, idx + ((j + 1) % param.bisect_verts), idx + param.bisect_verts + j));
			x_woven_threads.faces.push_back(triangle_face(idx + param.bisect_verts + ((j + 1) % param.bisect_verts), idx + param.bisect_verts + j, idx + ((j + 1) % param.bisect_verts)));
		}
	}
	x_woven_threads.calculate_face_normal();
	x_woven_threads.calculate_normal();

	for (int i = 0; i < param.bisect_verts; i++)
	{
		int idx = (path_verts_x-1)*param.bisect_verts;
		Vector3 n = x_woven_threads.normals[i]+x_woven_threads.normals[idx+i];
		n = Normalize(n);

		x_woven_threads.normals[i] = x_woven_threads.normals[idx+i] = n;
	}
	x_woven_threads.save_obj("T:/Microfacet/output/x_woven.obj");
	g = new r_geometry(mff_singleton::get()->get_handle());
	g->load(&x_woven_threads);
	mff_singleton::get()->assign_geom("bd_thread_x", shared_ptr<r_geometry>(g));

	//prcessing geom_hierarchy
	char name[2][32] = {"bd_thread_y", "bd_thread_x"};

	for (int i = 0; i < 2; i++)
	{
		shared_ptr<r_geom_hierarchy> p_gh;
		shared_ptr<r_geom_hierarchy> sp_gh = mff_singleton::get()->get_geom_hierarchy(name[i]);

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
			pp.s = 1.0f; pp.num = 16;
			scales.push_back(pp);
			p_gh->set_scales(scales);
			mff_singleton::get()->assign_geom_hierarchy(name[i], shared_ptr<r_geom_hierarchy>(p_gh));
		}
		p_gh->sample_points(density, num_area_hits_scalar);
	}
}

void binder_woven_threads::generate(std::vector<instance_property*> &results,
							const Vector3 &v_min, const Vector3 &v_max, 
							const std::vector<const std::vector<instance_property*>*> &neighbors)
{
	instance_property *p;

	for (int x = 0; x < param.x_num*2; x++)
		for (int y = 0; y < param.y_num; y++)
		{
			for (int i = 0; i < disp_y.size(); i++)
			{
				p = new instance_property;
				p->id = M_ID_BINDER_WOVEN_THREAD_Y;
				p->setup_matrix(v_min+disp_y[i]+Vector3((float)(x)/param.x_num/2, (float)(y+((x % 2 == 0) ? 0.5 : 0))/param.y_num, 0), 
					Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
				results.push_back(p);
			}
		}

	for (int y = 0; y < param.y_num*2; y++)
		for (int x = 0; x < param.x_num; x++)
		{
			for (int i = 0; i < disp_x.size(); i++)
			{
				p = new instance_property;
				p->id = M_ID_BINDER_WOVEN_THREAD_X;
				p->setup_matrix(v_min+disp_x[i]+Vector3((float)(x+((y % 2 == 0) ? 0.5 : 0))/param.x_num, 
					(float)(y)/param.y_num/2, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), 1);
				results.push_back(p);
			}
		}
}