#include <Magick++.h> 
#include "render_dataset.h"
#include <ctime>

void create_reflectance_table(MicrofacetEditor& m_editor)
{
	m_editor.compute_microfacet_change();
	m_editor.create_reflectance_table();
	m_editor.update_render();
}

void create_reflectance_table(MicrofacetEditor& m_editor, const string filename)
{
	m_editor.compute_microfacet_change();
	m_editor.create_reflectance_table();
	m_editor.update_render(filename);
}

void render(MicrofacetEditor& m_editor)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render();
	m_editor.update_render();
	//m_editor.render_visualization();
	//m_editor.update_render();
}

void render(MicrofacetEditor& m_editor, const string filename)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render();
	m_editor.update_render(filename);
	//m_editor.render_visualization();
	//m_editor.update_render();
}

void render_ground_truth(MicrofacetEditor& m_editor)
{
	m_editor.compute_ground_truth_BRDF();
	m_editor.render_buffer();
	m_editor.render_ground_truth();
	m_editor.update_render();
}

void render_animation(MicrofacetEditor& m_editor)
{
	m_editor.compute_microfacet_change();
	m_editor.gen_anim_ours();
	m_editor.update_render();
}

void render_animation_truth(MicrofacetEditor& m_editor)
{
	m_editor.compute_ground_truth_BRDF();
	m_editor.gen_anim_truth();
	m_editor.update_render();
}

void render_ref_BRDF(MicrofacetEditor& m_editor, string roughness, Vector3 albedo, const string filename)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render_ref_BRDF(roughness, albedo);
	m_editor.update_render(filename);
}

void render_measured_BRDF(MicrofacetEditor& m_editor, string measuredBrdf, const string filename)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render_measured_BRDF(measuredBrdf);
	m_editor.update_render(filename);
}

void render_ref_BRDF(MicrofacetEditor& m_editor, string roughness, Vector3 albedo)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render_ref_BRDF(roughness, albedo);
	m_editor.update_render();
}


void generate_image(MicrofacetEditor& m_editor)
{
	m_editor.load_material(Vector3(0.8), Vector3(0.2), "ward_0.2", "matr_binder_0", "matr_binder_1", "matr_distr_0");
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/violentdays", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0, "T:/Microfacet/data/cube_texture/cubelight.txt");
	std::clock_t start;
	double duration;
	m_editor.set_view_direction(Vector3(0, 1, 6), Vector3(0), Vector3(0, 1, 0));
	start = std::clock();
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_woven(2, 2, 16, 32, 1.5, 0.05, 0.1, 1.5, 0.05, 0.1, binder_name);// = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	//m_editor.set_view_direction(v);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, true);

	string path = "T:/MeasuredBRDF/brdfs/gold-metallic-paint.binary";
	//render_measured_BRDF(m_editor, path, "T:/Microfacet/output/img_ref_brdf.png");
	//render(m_editor);
	create_reflectance_table(m_editor, "T:/Microfacet/output/test1.jpg");
	//render_ref_BRDF(m_editor, "0.5", Vector3(0.8, 0.6, 0.8));
	
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

	std::cout << "time: " << duration << '\n';
	
}

int main()
{
	
	MicrofacetEditor m_editor;
	m_editor.load_scene(DATA_PATH"teapot.obj");

	//generate_grid_plane(m_editor);
	//generate_grid_plane_prediction(m_editor);
	
	//generate_grid_plane_color_dataset(m_editor, 20);
	
	//generate_image(m_editor);
	
	//render_grid_plane_prediction(m_editor);

	generate_groove_prediction(m_editor);

	return 0;
}