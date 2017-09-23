#include "MicrofacetEditor.h"
#include "distr_grid.h"
#include "distr_low_discrepancy.h"
#include "distr_rod.h"
#include "binder.h"
#include "binder_plane.h"
#include "binder_brick.h"
#include "binder_groove.h"
#include "binder_random_surface.h"
#include "binder_regpoly.h"
#include "binder_woven.h"
#include "binder_woven_threads.h"
#include "binder_woven2.h"

#include <sstream>
#include <iomanip>

std::string precision(const float a_value, const int n = 3)
{
	std::ostringstream out;
	out << std::setprecision(n) << a_value;
	return out.str();
}

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

microfacet_distr* MicrofacetEditor::generate_distr_grid(const float x, const float y,
	const float z, const float scale, const float height, string& distr_name)
{
	distr_grid *dist = new distr_grid();
	distr_grid_param param_grid;
	param_grid.x_space = x;
	param_grid.y_space = y;
	param_grid.z_space = z;
	param_grid.scale = scale;
	param_grid.height = height;
	distr_name = "grid_" + precision(x) + "_" + precision(y) + "_" + precision(z) + "_" + precision(scale) + "_" + precision(height);
	dist->set_param(param_grid);
	return dist;
}

microfacet_distr* MicrofacetEditor::generate_distr_low_discrp(const float density,
	const float scale, const float height, string& distr_name)
{
	distr_low_discrepancy * distr = new distr_low_discrepancy();
	distr_low_discrepancy_param param;
	param.density = density;
	param.scale = scale;
	param.height = height;
	distr_name = "ld_" + precision(density) + "_" + precision(scale) + "_" + precision(height);
	distr->set_param(param);
	return distr;
}

microfacet_distr* MicrofacetEditor::generate_distr_low_discrp_3d(const float density,
	const float scale, const float height,
	const float relative_height_density, string& distr_name)
{
	distr_low_discrepancy_3d* distr = new distr_low_discrepancy_3d();
	distr_low_discrepancy_3d_param param;
	param.density = density;
	param.scale = scale;
	param.height = height;
	param.relative_height_density = relative_height_density;
	distr_name = "ld3d_" + precision(density) + "_" + precision(scale) + "_" + precision(height) + "_" + precision(relative_height_density);
	distr->set_param(param);
	return distr;
}

microfacet_distr* MicrofacetEditor::generate_distr_rod(const float density, const float scale, const float random, string& distr_name)
{
	distr_rod* distr = new distr_rod();
	distr_rod_param param;
	param.density = density;
	param.scale = scale;
	param.randomness = random;
	param.pd_phi = .0f;
	param.pd_theta = .0f;
	distr_name = "rod_" + precision(density) + "_" + precision(scale) + "_" + precision(random);
	distr->set_param(param);
	return distr;
}

microfacet_binder* MicrofacetEditor::generate_binder_plane(string& binder_name)
{
	binder_plane* binder = new binder_plane();
	binder_plane_param param;
	param.x_res = param.y_res = 1;
	binder_name = "plane";
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_groove(const float precent, const float height, string& binder_name)
{
	binder_groove* binder = new binder_groove();
	binder_groove_param param;
	param.plane_percent = precent;
	param.height = height;
	binder_name = "groove_" + precision(precent) + "_" + precision(height);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_brick(const int x, const int y,
	const float bottom_x, const float bottom_y,
	const float top_x, const float top_y,
	const float height, string& binder_name)
{
	binder_brick* binder = new binder_brick();
	binder_brick_param param;
	param.x_num = x;
	param.y_num = y;
	param.bottom_percent_x = bottom_x;
	param.bottom_percent_y = bottom_y;
	param.top_percent_x = top_x;
	param.top_percent_y = top_y;
	param.height = height;
	binder_name = "brick_" + to_string(x) + "_" + to_string(y) + "_" + precision(bottom_x) + 
		"_" + precision(bottom_y) + "_" + precision(top_x) + "_" + precision(top_y) + "_" + precision(height);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_regpoly(const int edge, const float height,
	const float radius_top, const float radius_bottom, string& binder_name)
{
	binder_regpoly* binder = new binder_regpoly();
	binder_regpoly_param param;
	param.num_edges = edge;
	param.height = height;
	param.radius_top = radius_top;
	param.radius_bottom = radius_bottom;
	binder_name = "regpoly_" + to_string(edge) + "_" + precision(height) + "_" + precision(radius_top) + "_" + precision(radius_bottom);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_dispmap(const int x, const int y, const float amp_, string& binder_name)
{
	binder_dispmap* binder = new binder_dispmap();
	binder_dispmap_param param;
	param.res_x = x;
	param.res_y = y;
	param.amp = amp_;
	binder_name = "dispmap_" + to_string(x) + "_" + to_string(y) + "_" + precision(amp_);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_random_surface(const int x, const int y, const float amp_, string& binder_name)
{
	binder_random_surface* binder = new binder_random_surface();
	binder_dispmap_param param;
	param.res_x = x;
	param.res_y = y;
	param.amp = amp_;
	binder_name = "randsurface_" + to_string(x) + "_" + to_string(y) + "_" + precision(amp_);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_woven(const int x, const int y,
	const int verts, const int path,
	const float w_x, const float h_x,
	const float hegith_x, const float w_y,
	const float h_y, const float height_y, string& binder_name)
{
	binder_woven* binder = new binder_woven();
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
	binder_name = "woven_" + to_string(x) + "_" + to_string(y) + "_" + to_string(verts) + "_" + precision(hegith_x) + "_" + precision(height_y);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_woven2(const int x, const int y,
	const int verts, const int path,
	const float w_x, const float h_x,
	const float hegith_x, const float w_y,
	const float h_y, const float height_y, string& binder_name)
{
	binder_woven2* binder = new binder_woven2();
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
	binder_name = "woven2_" + to_string(x) + "_" + to_string(y) + "_" + to_string(verts) + "_" + precision(hegith_x) + "_" + precision(height_y);
	binder->set_param(param);
	return binder;
}

microfacet_binder* MicrofacetEditor::generate_binder_woven_threads(const int x, const int y,
	const int verts, const int path,
	const float w_x, const float h_x,
	const float w_y, const float h_y,
	const float height, const float r_x,
	const float r_y, string& binder_name)
{
	binder_woven_threads* binder = new binder_woven_threads();
	binder_woven_threads_param param;
	param.x_num = x; //thickness of the ridge
	param.y_num = y; //thickness of the ridge
	param.bisect_verts = verts; // the shape of each structure
	param.total_path_verts = path;
	param.bisect_w_x = w_x;
	param.bisect_h_x = h_x;
	param.bisect_w_y = w_y;
	param.bisect_h_y = h_y;
	param.height = height;  // the height of the ridge
	param.thread_r_x = r_x; // the width of x_woven
	param.thread_r_y = r_y; // the width of y_woven
	binder_name = "threads_" + to_string(x) + "_" + to_string(y) + "_" + to_string(verts) + "_" + precision(height) + "_" + precision(r_x) + "_" + precision(r_y);
	binder->set_param(param);
	return binder;
}

void MicrofacetEditor::generate_microfacet_details(microfacet_binder* binder, microfacet_distr* distr,
	int w, int h, float d, float dens, int num_area,
	string binder_name, string distr_name, bool save_mesh)
{
	details = new microfacet_details;
	//details->init(1, 1, 10.0, 500, 16, binder, distr);
	details->init(w, h, d, dens, num_area, binder, distr);
	details->init_blocks(final_details);

	details->idx_all(selection);
	details->update_with_distr(selection);
	details->generate_blocks(selection, final_details);

	printf_s("sampling points...");
	details->sample_points(selection, final_details);
	printf_s("done.\n");

	if (save_mesh)
	{
		tri_mesh mesh;
		save_details_as_obj("T:/Microfacet/output/", mesh, distr_name, binder_name, final_details);
		printf_s("finish save obj.\n");
	}
}

void MicrofacetEditor::generate_init_matr(const Vector3 albedo, const string lib_matr_id, const string lib_matr)
{
	matr *p;
	p = new matr;
	p->basis_id = mff_singleton::get()->get_matr_id(lib_matr_id);
	p->albedo = albedo;
	mff_singleton::get()->add_matr(mff_singleton::get()->get_matr_id(lib_matr), p);
}
