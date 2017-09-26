#include "MicrofacetEditor.h"
#include <Magick++.h> 

void render(MicrofacetEditor& m_editor)
{
	m_editor.compute_microfacet_change();
	m_editor.render_buffer();
	m_editor.render();
	m_editor.update_render();
	m_editor.render_visualization();
	m_editor.update_render();
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
using namespace Magick;
int main()
{
	
	MicrofacetEditor m_editor;
	random rng;
	Vector3 v;
	string skybox = "T:/cubemap/", skybox_name, file = "C:/Users/justin/Documents/Microfacet_data/", mtr_name, micro_name, file_name;
	float scale, x, y;
	for (int rough_cnt = 3; rough_cnt < 4; rough_cnt++) //2<10
	{
		string material = "Lambert";//"ward_0" + to_string(rough_cnt);
		for (int albedo_cnt = 4; albedo_cnt < 5; albedo_cnt++) //0<5
		{
			float albedo = albedo_cnt*0.2;

			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
			cout << mtr_name << endl;
			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
			string binder_name, distr_name;
			microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
			// for scale = 0.05, 0.1
			for (int scale_cnt = 2; scale_cnt < 3; scale_cnt++) //1<3
			{ 
				scale = scale_cnt*0.05;
				for (int x_cnt = 4; x_cnt < 5; x_cnt++) //0<5
				{ 
					x = x_cnt*0.2;
					for (int y_cnt = 4; y_cnt < 5; y_cnt++)//0<5
					{
						y = y_cnt*0.2;
						microfacet_distr* distr = m_editor.generate_distr_grid(x, y, 0, scale, 0, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);

						micro_name = mtr_name + "/" + precision(scale) + "_" + precision(x) + "_" + precision(y);
						std::wstring micro_name_wstring(micro_name.length(), L' ');
						std::copy(micro_name.begin(), micro_name.end(), micro_name_wstring.begin());
						CreateDirectory(micro_name_wstring.c_str(), NULL);
						cout << micro_name << endl;
						for (int cubemap = 0; cubemap <= 0; cubemap++) //0<10
						{
							skybox_name = skybox + to_string(cubemap) + "/cube";
							m_editor.load_sky_box(skybox_name, 2, Vector3(1.0f), 2, Identity());
							for (int view = 0; view < 1; view++) //0<10
							{
								uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
								v = v * 3;
								m_editor.set_view_direction(v);
								file_name = micro_name + "/" + to_string(cubemap) + "_" + to_string(view) + ".jpg";
								render(m_editor, file_name);
							}
						}
					}
				}
			}
			// for scale = 0
			microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 1, 0, 0, distr_name);
			m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);

			micro_name = mtr_name + "/" + to_string(0) + "_" + to_string(0) + "_" + to_string(0);
			std::wstring micro_name_wstring(micro_name.length(), L' ');
			std::copy(micro_name.begin(), micro_name.end(), micro_name_wstring.begin());
			CreateDirectory(micro_name_wstring.c_str(), NULL);
			for (int cubemap = 0; cubemap <= 0; cubemap++) //0<10
			{
				skybox_name = skybox + to_string(cubemap) + "/cube";
				m_editor.load_sky_box(skybox_name, 2, Vector3(1.0f), 2, Identity());
				for (int view = 0; view < 1; view++)//0<10
				{
					uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
					v = v * 3;
					m_editor.set_view_direction(v);
					file_name = micro_name + "/" + to_string(cubemap) + "_" + to_string(view) + ".jpg";
					render(m_editor, file_name);
				}
			}
		}
	}
	
	
	/*
	m_editor.load_material(Vector3(0.5), "Lambert", "matr_binder_0", "matr_distr_0");
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(0.8, 0.8, 1, 0.1, 0, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, true);
	
	m_editor.load_sky_box("T:/Microfacet/data/cube_texture/white", 2, Vector3(1.0f), 2, Identity());
	render(m_editor);
	*/
	//render_ground_truth(m_editor);

	//render_animation(m_editor);

	//render_animation_truth(m_editor);

	//render_ref_BRDF(m_editor);


	return 0;
}