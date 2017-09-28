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

void generate_database(MicrofacetEditor& m_editor)
{
	random rng;
	Vector3 v;
	m_editor.load_cube_map("T:/cubemap/", 10, 2, Vector3(1.0f), 2, Identity()); //10
	string file = "C:/Users/justin/Documents/Microfacet_data/", mtr_name, micro_name, file_name;
	float scale, x, y;
	for (int rough_cnt = 3; rough_cnt < 4; rough_cnt++) //2<10
	{
		string material = "ward_0" + to_string(rough_cnt);
		for (int albedo_cnt = 0; albedo_cnt < 5; albedo_cnt++) //0<5
		{
			float albedo;
			if (albedo_cnt == 0)
				albedo = 0.05;
			else
				albedo = albedo_cnt*0.2;
			std::cout << "Rendering material: " << material << " albedo: " << albedo<<endl;
			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
			
			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
			string binder_name, distr_name;
			microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
			// for scale = 0.05, 0.1
			for (int scale_cnt = 1; scale_cnt < 3; scale_cnt++) //1<3
			{
				scale = scale_cnt*0.05;
				for (int x_cnt = 0; x_cnt < 5; x_cnt++) //0<5
				{
					x = x_cnt*0.2;
					for (int y_cnt = 0; y_cnt < 5; y_cnt++)//0<5
					{
						y = y_cnt*0.2;
						std::cout << "    Microstructure scale = " << scale << " x = " << x << " y = " << y << "....";
						microfacet_distr* distr = m_editor.generate_distr_grid(x, y, 0, scale, 0, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);

						micro_name = mtr_name + "/" + precision(scale) + "_" + precision(x) + "_" + precision(y);
						std::wstring micro_name_wstring(micro_name.length(), L' ');
						std::copy(micro_name.begin(), micro_name.end(), micro_name_wstring.begin());
						CreateDirectory(micro_name_wstring.c_str(), NULL);
						
						for (int cubemap = 0; cubemap < 10; cubemap++) //0<10
						{
							m_editor.load_sky_box(cubemap);
							for (int view = 0; view < 10; view++) //0<10
							{
								uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
								v = v * 3;
								m_editor.set_view_direction(v);
								file_name = micro_name + "/" + to_string(cubemap) + "_" + to_string(view) + ".jpg";
								render(m_editor, file_name);
							}
						}
						std::cout << "done" << endl;
					}
				}
			}
			// for scale = 0
			std::cout << "    Microstructure scale = " << 0 << "....";
			microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 1, 0, 0, distr_name);
			m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);

			micro_name = mtr_name + "/" + to_string(0) + "_" + to_string(0) + "_" + to_string(0);
			std::wstring micro_name_wstring(micro_name.length(), L' ');
			std::copy(micro_name.begin(), micro_name.end(), micro_name_wstring.begin());
			CreateDirectory(micro_name_wstring.c_str(), NULL);
		
			for (int cubemap = 0; cubemap < 10; cubemap++) //0<10
			{
				m_editor.load_sky_box(cubemap);
				for (int view = 0; view < 10; view++)//0<10
				{
					uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
					v = v * 3;
					m_editor.set_view_direction(v);
					file_name = micro_name + "/" + to_string(cubemap) + "_" + to_string(view) + ".jpg";
					render(m_editor, file_name);
				}
			}
			std::cout << "done" << endl;
		}
	}

}

void generate_image(MicrofacetEditor& m_editor)
{
	m_editor.load_material(Vector3(0.2, 0.2, 0.2), "ward_03", "matr_binder_0", "matr_distr_0");
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(0.2, 0.2, 1, 0.1, 0, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);

	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0);

	//render(m_editor);
	//render_ground_truth(m_editor);

	render_animation(m_editor);

	//render_animation_truth(m_editor);

	//render_ref_BRDF(m_editor);
}

int main()
{
	
	MicrofacetEditor m_editor;

	generate_image(m_editor);
	
	//generate_database(m_editor);

	return 0;
}