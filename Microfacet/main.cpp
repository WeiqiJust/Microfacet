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
	
	//render(m_editor);

	render_ground_truth(m_editor);

	//render_animation(m_editor);

	//render_animation_truth(m_editor);

	//render_ref_BRDF(m_editor);

	return 0;
}