#include "MicrofacetEditor.h"
#include "distr_grid.h"
#include "binder_plane.h"

void MicrofacetEditor::generate_mesh(const string filename, const Matrix4 mat)
{
	p_base = new base_obj();
	p_base->mesh.load_obj(filename.c_str()); // already calculated normals
	p_base->apply_transform(mat);
}

void MicrofacetEditor::generate_background_mesh(const string filename, const Matrix4 mat)
{
	for (int i = 0; i < 1; i++)
	{
		base_obj* object = new base_obj();
		object->mesh.load_obj(filename.c_str()); // already calculated normals
		object->apply_transform(Scale(i,0,0));
		p_background.push_back(object);
	}
}

void MicrofacetEditor::generate_skybox(const string texture_file, const int mip_level,
	const Vector3 scale, const float dist, const Matrix4 mat)
{
	shared_ptr<cube_texture> cube = make_shared<cube_texture>();
	cube->load_texture(texture_file, mip_level);
	p_skybox = new r_skybox(scale, dist, cube, mat);
}

distr_grid_param MicrofacetEditor::generate_distr_grid(const float x, const float y,
	const float z, const float scale, const float height)
{
	distr_grid_param param_grid;
	param_grid.x_space = x;
	param_grid.y_space = y;
	param_grid.z_space = z;
	param_grid.scale = scale;
	param_grid.height = height;
	return param_grid;
}

distr_low_discrepancy_param MicrofacetEditor::generate_distr_low_discrp(const float density,
	const float scale, const float height)
{
	distr_low_discrepancy_param param;
	param.density = density;
	param.scale = scale;
	param.height = height;
	return param;
}

distr_low_discrepancy_3d_param MicrofacetEditor::generate_distr_low_discrp_3d(const float density,
	const float scale, const float height,
	const float relative_height_density)
{
	distr_low_discrepancy_3d_param param;
	param.density = density;
	param.scale = scale;
	param.height = height;
	param.relative_height_density = relative_height_density;
	return param;
}

distr_rod_param MicrofacetEditor::generate_distr_rod(const float density, const float scale, const float random)
{
	distr_rod_param param;
	param.density = density;
	param.scale = scale;
	param.randomness = random;
	param.pd_phi = .0f;
	param.pd_theta = .0f;
	return param;
}

binder_plane_param MicrofacetEditor::generate_binder_plane()
{
	binder_plane_param param;
	param.x_res = param.y_res = 1;
	return param;
}

binder_groove_param MicrofacetEditor::generate_binder_groove(const float precent, const float height)
{
	binder_groove_param param;
	param.plane_percent = precent;
	param.height = height;
	return param;
}

binder_brick_param MicrofacetEditor::generate_binder_brick(const int x, const int y,
	const float bottom_x, const float bottom_y,
	const float top_x, const float top_y,
	const float height)
{
	binder_brick_param param;
	param.x_num = x;
	param.y_num = y;
	param.bottom_percent_x = bottom_x;
	param.bottom_percent_y = bottom_y;
	param.top_percent_x = top_x;
	param.top_percent_y = top_y;
	param.height = height;
	return param;
}

binder_regpoly_param MicrofacetEditor::generate_binder_regpoly(const int edge, const float height,
	const float radius_top, const float radius_bottom)
{
	binder_regpoly_param param;
	param.num_edges = edge;
	param.height = height;
	param.radius_top = radius_top;
	param.radius_bottom = radius_bottom;
	return param;
}

binder_dispmap_param MicrofacetEditor::generate_binder_dispmap(const int x, const int y, const float amp_)
{
	binder_dispmap_param param;
	param.res_x = x;
	param.res_y = y;
	param.amp = amp_;
	return param;
}

binder_dispmap_param MicrofacetEditor::generate_binder_random_surface(const int x, const int y, const float amp_)
{
	binder_dispmap_param param;
	param.res_x = x;
	param.res_y = y;
	param.amp = amp_;
	return param;
}

binder_woven_param MicrofacetEditor::generate_binder_woven(const int x, const int y,
	const int verts, const int path,
	const float w_x, const float h_x,
	const float hegith_x, const float w_y,
	const float h_y, const float height_y)
{
	binder_woven_param param;
	param.x_num = x;
	param.y_num = y;
	param.bisect_verts = verts;
	param.total_path_verts = path;
	param.bisect_w_x = w_x;
	param.bisect_h_x = h_x;
	param.height_x = hegith_x;
	param.bisect_w_y = w_y;
	param.bisect_h_y = h_y;
	param.height_y = height_y;
	return param;
}

binder_woven_param MicrofacetEditor::generate_binder_woven2(const int x, const int y,
	const int verts, const int path,
	const float w_x, const float h_x,
	const float hegith_x, const float w_y,
	const float h_y, const float height_y)
{
	binder_woven_param param;
	param.x_num = x;
	param.y_num = y;
	param.bisect_verts = verts;
	param.total_path_verts = path;
	param.bisect_w_x = w_x;
	param.bisect_h_x = h_x;
	param.height_x = hegith_x;
	param.bisect_w_y = w_y;
	param.bisect_h_y = h_y;
	param.height_y = height_y;
	return param;
}

binder_woven_threads_param MicrofacetEditor::generate_binder_woven_threads(const int x, const int y,
	const int verts, const int path,
	const float w_x, const float h_x,
	const float w_y, const float h_y,
	const float height, const float r_x,
	const float r_y)
{
	binder_woven_threads_param param;
	param.x_num = x;
	param.y_num = y;
	param.bisect_verts = verts;
	param.total_path_verts = path;
	param.bisect_w_x = w_x;
	param.bisect_h_x = h_x;
	param.bisect_w_y = w_y;
	param.bisect_h_y = h_y;
	param.height = height;
	param.thread_r_x = r_x;
	param.thread_r_y = r_y;
	return param;
}

void MicrofacetEditor::generate_microfacet_details(microfacet_binder* binder, microfacet_distr* distr,
	int w, int h, float d,
	float dens, int num_area)
{
	details->init(w, h, d, dens, num_area, binder, distr);
}

void MicrofacetEditor::generate_init_matr(const Vector3 albedo, const string lib_matr_id, const string lib_matr)
{
	matr *p;
	p = new matr;
	p->basis_id = mff_singleton::get()->get_matr_id(lib_matr_id);
	p->albedo = albedo;
	mff_singleton::get()->add_matr(mff_singleton::get()->get_matr_id(lib_matr), p);
}
