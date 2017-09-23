#include "MicrofacetEditor.h"

void render(MicrofacetEditor& m_editor)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render();
	m_editor.update_render();
	m_editor.render_visualization();
	m_editor.update_render();
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

int main()
{
	MicrofacetEditor m_editor;

	m_editor.load_sky_box("T:/Microfacet/data/cube_texture/white", 2, Vector3(1.0f), 2, Identity());
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(0.25, 0.25, 0.25, 0.1, 0.1, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);
	m_editor.load_material(Vector3(0.5), "Phong_2", "matr_binder_0", "matr_distr_0");

	render(m_editor);

	//render_ground_truth(m_editor);

	//render_animation(m_editor);

	//render_animation_truth(m_editor);

	//render_ref_BRDF(m_editor);

	return 0;
}