#include "render_dataset.h"


void generate_grid_plane(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_grid_plane/", mtr_name, file_name;
	float scale, x, y;
	std::cout << "Start Rendering Grid_Plane Training Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 4; rough_cnt++) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 9; albedo_cnt >=0; albedo_cnt--) //0<5
		{
			float albedo;
			if (albedo_cnt == 0)
				albedo = 0.05;
			else
				albedo = 0.1*albedo_cnt;
			std::cout << "Rendering material: " << material << " albedo: " << albedo << endl;

			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
			string binder_name, distr_name;
			microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
			for (int scale_cnt = 1; scale_cnt <= 5; scale_cnt++) //1<=5
			{
				scale = scale_cnt*0.05;
				std::cout << "    Microstructure scale = " << scale << "....";
				for (int x_cnt = 1; x_cnt < 10; x_cnt++) //1<10
				{
					x = x_cnt*0.1;
					for (int y_cnt = 1; y_cnt < 10; y_cnt++)//1<10
					{
						y = y_cnt*0.1;
						
						microfacet_distr* distr = m_editor.generate_distr_grid(x, y, 0, scale, 0, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
						file_name = mtr_name + "/" + precision(scale) + "_" + precision(x) + "_" + precision(y) + ".jpg";
						render(m_editor, file_name);
					}
				}
				std::cout << "done" << endl;
			}
			std::cout << "    Microstructure scale = " << 0 << "....";
			microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 1, 0, 0, distr_name);
			m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
			file_name = mtr_name + "/0.0_0.0_0.0.jpg";
			render(m_editor, file_name);
			std::cout << "done" << endl;
		}
	}
}

void generate_grid_plane_test(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_grid_plane_test/", mtr_name, file_name;
	float scale, x, y;
	std::cout << "Start Rendering Grid_Plane Test Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt++) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 0; albedo_cnt < 5; albedo_cnt++) //0<5
		{
			float albedo = 0.2*albedo_cnt + 0.2;
			std::cout << "Rendering material: " << material << " albedo: " << albedo << endl;

			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
			string binder_name, distr_name;
			microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
			for (int scale_cnt = 1; scale_cnt <= 5; scale_cnt++) //1<=5
			{
				scale = scale_cnt*0.05;
				std::cout << "    Microstructure scale = " << scale << "....";
				for (int x_cnt = 0; x_cnt < 5; x_cnt++) //0<5
				{
					x = x_cnt*0.2 + 0.05;
					for (int y_cnt = 0; y_cnt < 5; y_cnt+=2)//0<5
					{
						y = y_cnt*0.2 + 0.05;

						microfacet_distr* distr = m_editor.generate_distr_grid(x, y, 0, scale, 0, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
						file_name = mtr_name + "/" + precision(scale) + "_" + precision(x) + "_" + precision(y) + ".jpg";
						render(m_editor, file_name);
					}
				}
				std::cout << "done" << endl;
			}
			std::cout << "    Microstructure scale = " << 0 << "....";
			microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 1, 0, 0, distr_name);
			m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
			file_name = mtr_name + "/0.0_0.0_0.0.jpg";
			render(m_editor, file_name);
			std::cout << "done" << endl;
		}
	}
}

void generate_database(MicrofacetEditor& m_editor)
{
	random rng;
	Vector3 up;
	Vector2 direction;
	m_editor.load_cube_map("T:/cubemap/", 10, 2, Vector3(1.0f), 2, Identity()); //10
	string file = "C:/Users/justin/Documents/Microfacet_data_test/", mtr_name, micro_name, file_name;
	float scale, x, y;
	for (int rough_cnt = 4; rough_cnt < 5; rough_cnt++) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 4; albedo_cnt >= 0; albedo_cnt--) //0<5
		{
			float albedo;
			if (albedo_cnt == 0)
				albedo = 0.05;
			else
				albedo = albedo_cnt*0.2;
			std::cout << "Rendering material: " << material << " albedo: " << albedo << endl;
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
					if (x_cnt == 0)
						x = 0.1;
					else
						x = x_cnt*0.2;
					for (int y_cnt = 0; y_cnt < 5; y_cnt++)//0<5
					{
						if (y_cnt == 0)
							y = 0.1;
						else
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
							string samplefile = "T:/cubemap/" + to_string(cubemap) + ".txt";
							m_editor.load_sky_box(cubemap, samplefile);
							for (int view = 0; view < 10; view++) //0<10
							{
								//uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
								//v = v * 3;

								up = Vector3(cos(view*PI / 5), sin(view*PI / 5), 0);
								//up = up.normalize();
								m_editor.set_view_direction(Vector3(0, 0, 2), Vector3(0), up);
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

			micro_name = mtr_name + "/0.0_0.0_0.0";
			std::wstring micro_name_wstring(micro_name.length(), L' ');
			std::copy(micro_name.begin(), micro_name.end(), micro_name_wstring.begin());
			CreateDirectory(micro_name_wstring.c_str(), NULL);

			for (int cubemap = 0; cubemap < 10; cubemap++) //0<10
			{
				string samplefile = "T:/cubemap/" + to_string(cubemap) + ".txt";
				m_editor.load_sky_box(cubemap, samplefile);
				for (int view = 0; view < 10; view++)//0<10
				{
					//uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
					//v = v * 3;
					up = Vector3(rng.get_random_float(), rng.get_random_float(), 0);
					up = up.normalize();
					m_editor.set_view_direction(Vector3(0, 0, 2), Vector3(0), up);
					file_name = micro_name + "/" + to_string(cubemap) + "_" + to_string(view) + ".jpg";
					render(m_editor, file_name);
				}
			}
			std::cout << "done" << endl;
		}
	}
}

void generate_database_test(MicrofacetEditor& m_editor)
{
	random rng;
	Vector3 up;
	m_editor.load_cube_map("T:/cubemap/", 10, 2, Vector3(1.0f), 2, Identity()); //10
	string file = "C:/Users/justin/Documents/Microfacet_data_test_test/", mtr_name, micro_name, file_name;
	float scale, x, y;
	for (int rough_cnt = 4; rough_cnt < 5; rough_cnt++) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 3; albedo_cnt >= 0; albedo_cnt--) //0<5
		{
			float albedo;
			albedo = albedo_cnt*0.2 + 0.1;
			std::cout << "Rendering material: " << material << " albedo: " << albedo << endl;
			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());

			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
			string binder_name, distr_name;
			microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
			// for scale = 0.05, 0.1
			for (int scale_cnt = 0; scale_cnt < 3; scale_cnt++) //0<3
			{
				scale = (2 * scale_cnt + 1)*0.025;
				for (int x_cnt = 0; x_cnt < 5; x_cnt++) //0<5
				{
					if (x_cnt == 0)
						x = 0.1;
					else
						x = x_cnt*0.2 + 0.1;
					for (int y_cnt = 0; y_cnt < 5; y_cnt++)//0<5
					{
						if (y_cnt == 0)
							y = 0.1;
						else
							y = y_cnt*0.2 + 0.1;
						std::cout << "    Microstructure scale = " << scale << " x = " << x << " y = " << y << "....";
						microfacet_distr* distr = m_editor.generate_distr_grid(x, y, 0, scale, 0, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 500, 16, binder_name, distr_name, false);

						micro_name = mtr_name + "/" + precision(scale) + "_" + precision(x) + "_" + precision(y);
						std::wstring micro_name_wstring(micro_name.length(), L' ');
						std::copy(micro_name.begin(), micro_name.end(), micro_name_wstring.begin());
						CreateDirectory(micro_name_wstring.c_str(), NULL);

						for (int cubemap = 0; cubemap < 10; cubemap++) //0<10
						{
							string samplefile = "T:/cubemap/" + to_string(cubemap) + ".txt";
							m_editor.load_sky_box(cubemap, samplefile);

							//uniform_sphere_sample(v, rng.get_random_float(), rng.get_random_float());
							//v = v * 3;
							m_editor.set_view_direction(Vector3(0, 0, 2), Vector3(0), up);
							file_name = micro_name + "/" + to_string(cubemap) + "_0" + ".jpg";
							render(m_editor, file_name);
						}
						std::cout << "done" << endl;
					}
				}
			}
		}
	}

}