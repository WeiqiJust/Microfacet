#include <Magick++.h> 
#include "render_dataset.h"
#include <ctime>

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

void render_ref_BRDF(MicrofacetEditor& m_editor)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render_ref_BRDF();
	m_editor.update_render();
}


void generate_image(MicrofacetEditor& m_editor)
{
	m_editor.load_material(Vector3(0.8, 0.8, 0.8), "ward_0.3", "matr_binder_0", "matr_distr_0");
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/violentdays", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0, "T:/Microfacet/data/cube_texture/cubelight.txt");
	std::clock_t start;
	double duration;
	start = std::clock();
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(0.4, 0.4, 0, 0, 0, distr_name);
	//m_editor.set_view_direction(v);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
	
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	render(m_editor);
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

	std::cout << "time: " << duration << '\n';
	//render_ground_truth(m_editor);

	//render_animation(m_editor);

	//render_animation_truth(m_editor);

	//render_ref_BRDF(m_editor);
}

int main()
{
	
	MicrofacetEditor m_editor;
	m_editor.load_scene(DATA_PATH"cylinder.obj");

	generate_grid_plane(m_editor);
	//generate_image(m_editor);
	
	//generate_database(m_editor);

	//generate_database_test(m_editor);

	return 0;
}