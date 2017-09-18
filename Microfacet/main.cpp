#include "MicrofacetEditor.h"

int main()
{
	MicrofacetEditor m_editor;
	cout << "Main::after create editer" << endl;

	m_editor.compute_microfacet_change();
	//m_editor.compute_ground_truth_BRDF();
	cout << "Main::after compute mifrofacet" << endl;
	

	m_editor.render_buffer();
	cout << "Main::after render buffer" << endl;
	m_editor.render();
	//m_editor.update_render();
	//m_editor.render_visualization();
	//m_editor.render_raytrace();
	//m_editor.render_ground_truth();
	cout << "Main::after render" << endl;
	
	//m_editor.gen_anim_ours();
	m_editor.update_render();
	cout << "Main::finish." << endl;
	return 0;
}