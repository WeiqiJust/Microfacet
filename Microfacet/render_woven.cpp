#include "render_dataset.h"
#include <sys/stat.h>
#include <fstream>

struct woven
{
	float roughness, w, d, h; // width, depth, height
	Vector3 albedo0, albedo1;
};

void generate_woven(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_woven/", mtr_name, file_name;
	float width, depth, height;
	string binder_name, distr_name;

	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	std::cout << "Start Rendering Woven Training Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt++) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		std::cout << "Rendering material: " << material << endl;
		for (int albedo_cnt_0 = 0; albedo_cnt_0 < 5; albedo_cnt_0++) // ++
		{
			float albedo_0 = 0.2*albedo_cnt_0 + 0.1;

			for (int albedo_cnt_1 = 0; albedo_cnt_1 < 5; albedo_cnt_1++) // ++
			{
				float albedo_1 = 0.2*albedo_cnt_1 + 0.1;
				{
					std::cout << "	diffuse: " << material << " albedo_0: " << albedo_0 << " albedo_1: " << albedo_1 << "....";

					mtr_name = file + material + "_" + precision(albedo_0) + "_" + precision(albedo_1);
					std::wstring mtr_name_wstring(mtr_name.length(), L' ');
					std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
					CreateDirectory(mtr_name_wstring.c_str(), NULL);

					m_editor.load_material(Vector3(albedo_0), Vector3(albedo_1), material, "matr_binder_0", "matr_binder_1", "matr_distr_0");
					for (int width_cnt = 1; width_cnt <= 5; width_cnt++)
					{
						width = width_cnt*0.03;
						for (int depth_cnt = 1; depth_cnt <= 5; depth_cnt++)
						{
							depth = depth_cnt*0.04;
							for (int height_cnt = 1; height_cnt <= 5; height_cnt++)
							{
								height = height_cnt*0.04;

								file_name = mtr_name + "/" + precision(width) + "_" + precision(depth) + "_" + precision(height) + ".jpg";

								struct stat buffer;
								if (stat(file_name.c_str(), &buffer) == 0)
									continue;
								microfacet_binder* binder = m_editor.generate_binder_woven(2, 2, 16, 32, width, depth, height, width, depth, height, binder_name);
								m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
								create_reflectance_table(m_editor, file_name);
							}
						}
					}
					std::cout << "done" << endl;
				}
			}
		}
	}
}

void generate_woven_test(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_woven_test/", mtr_name, file_name;
	float width, depth, height;
	string binder_name, distr_name;

	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	std::cout << "Start Rendering Woven Test Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt += 2) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		std::cout << "Rendering material: " << material << endl;
		for (int albedo_cnt_0 = 0; albedo_cnt_0 < 4; albedo_cnt_0++)
		{
			float albedo_0 = 0.25*albedo_cnt_0 + 0.25;

			for (int albedo_cnt_1 = 0; albedo_cnt_1 < 4; albedo_cnt_1++)
			{
				float albedo_1 = 0.25*albedo_cnt_1 + 0.25;
				{
					std::cout << "	diffuse: " << material << " albedo_0: " << albedo_0 << " albedo_1: " << albedo_1 << "....";

					mtr_name = file + material + "_" + precision(albedo_0) + "_" + precision(albedo_1);
					std::wstring mtr_name_wstring(mtr_name.length(), L' ');
					std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
					CreateDirectory(mtr_name_wstring.c_str(), NULL);

					m_editor.load_material(Vector3(albedo_0), Vector3(albedo_1), material, "matr_binder_0", "matr_binder_1", "matr_distr_0");
					for (int width_cnt = 1; width_cnt <= 5; width_cnt++)
					{
						width = width_cnt*0.03;
						for (int depth_cnt = 1; depth_cnt <= 3; depth_cnt++)
						{
							depth = depth_cnt*0.05;
							for (int height_cnt = 1; height_cnt <= 3; height_cnt++)
							{
								height = height_cnt*0.05;

								file_name = mtr_name + "/" + precision(width) + "_" + precision(depth) + "_" + precision(height) + ".jpg";

								struct stat buffer;
								if (stat(file_name.c_str(), &buffer) == 0)
									continue;
								microfacet_binder* binder = m_editor.generate_binder_woven(2, 2, 16, 32, width, depth, height, width, depth, height, binder_name);
								m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
								create_reflectance_table(m_editor, file_name);
							}
						}
					}
					std::cout << "done" << endl;
				}
			}
		}
	}
}

void generate_woven_prediction(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_woven_prediction/", mtr_name, file_name;
	float width, depth, height;
	string binder_name, distr_name;

	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	std::cout << "Start Rendering Woven Prediction Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt++ ) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		std::cout << "Rendering material: " << material << endl;
		for (int albedo_cnt_0 = 1; albedo_cnt_0 < 4; albedo_cnt_0++)
		{
			float albedo_0 = 0.2*albedo_cnt_0;

			for (int albedo_cnt_1 = 1; albedo_cnt_1 < 4; albedo_cnt_1++)
			{
				float albedo_1 = 0.2*albedo_cnt_1;
				{
					std::cout << "	diffuse: " << material << " albedo_0: " << albedo_0 << " albedo_1: " << albedo_1 << "....";

					mtr_name = file + material + "_" + precision(albedo_0) + "_" + precision(albedo_1);
					std::wstring mtr_name_wstring(mtr_name.length(), L' ');
					std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
					CreateDirectory(mtr_name_wstring.c_str(), NULL);

					m_editor.load_material(Vector3(albedo_0), Vector3(albedo_1), material, "matr_binder_0", "matr_binder_1", "matr_distr_0");
					for (int width_cnt = 1; width_cnt <= 3; width_cnt++)
					{
						width = width_cnt*0.04;
						for (int depth_cnt = 1; depth_cnt <= 2; depth_cnt++)
						{
							depth = depth_cnt*0.07;
							for (int height_cnt = 1; height_cnt <= 2; height_cnt++)
							{
								height = height_cnt*0.07;

								file_name = mtr_name + "/" + precision(width) + "_" + precision(depth) + "_" + precision(height) + ".jpg";

								struct stat buffer;
								if (stat(file_name.c_str(), &buffer) == 0)
									continue;
								microfacet_binder* binder = m_editor.generate_binder_woven(2, 2, 16, 32, width, depth, height, width, depth, height, binder_name);
								m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
								create_reflectance_table(m_editor, file_name);
							}
						}
					}
					std::cout << "done" << endl;
				}
			}
		}
	}
}


void render_woven_prediction(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string grayfile = "T:/Microfacet/output/result/predict_gray_woven.txt";
	string colorfile = "T:/Microfacet/output/result/predict_color_woven.txt";
	string path = "T:/Microfacet/output/result/woven";
	ifstream fpg(grayfile), fpc(colorfile);
	int count_g, count_c;
	fpg >> count_g;
	fpc >> count_c;
	if (count_g != count_c)
	{
		cout << "Error: the number of gray scale and color images are not match!" << endl;
		exit(0);
	}

	m_editor.set_view_direction(Vector3(0, 1, 8), Vector3(0), Vector3(0, 1, 0));
	string binder_name, distr_name;
	microfacet_binder* binder;
	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	map<string, woven> predict;
	for (int i = 0; i < count_g; i++)
	{
		woven param;
		string filename;
		float albedo0, albedo1;
		fpg >> filename >> param.roughness >> albedo0 >> albedo1 >> param.w >> param.d >> param.h;
		predict[filename] = param;
	}
	fpg.close();

	for (int i = 0; i < count_c; i++)
	{
		Vector3 albedo0, albedo1;
		string filename;
		fpc >> filename >> albedo0.x >> albedo0.y >> albedo0.z >> albedo1.x >> albedo1.y >> albedo1.z;
		if (predict.find(filename) != predict.end())
		{
			predict[filename].albedo0 = albedo0;
			predict[filename].albedo1 = albedo1;
		}
		else
		{
			cout << "Error: Could not find file name " << filename << " in predict_color.txt!" << endl;
			exit(0);
		}
	}
	fpc.close();

	for (map<string, woven>::iterator it = predict.begin(); it != predict.end(); ++it)
	{
		string filename = it->first;
		woven param = it->second;

		float roughness = (float)floor(param.roughness * 10 + 0.5f) / 10;
		if (roughness < 0.2) roughness = 0.2f;
		if (roughness > 0.9) roughness = 0.9f;
		string material = "ward_" + precision(roughness);
		m_editor.load_material(param.albedo0, param.albedo1, material, "matr_binder_0", "matr_binder_1", "matr_distr_0");
		binder = m_editor.generate_binder_woven(2, 2, 16, 32, param.w, param.d, param.h, param.w, param.d, param.h, binder_name);
		m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
		string predictfilename = path + filename + "_predict.png";
		render(m_editor, predictfilename);

		
		vector<string> ground_truth_param = split_filename(filename);
		string gtfilename = path + filename + ".png";
		if (ground_truth_param.size() > 1)
		render_ref_BRDF(m_editor, ground_truth_param[0], Vector3(std::stod(ground_truth_param[1]), std::stod(ground_truth_param[2]), std::stod(ground_truth_param[3])), gtfilename);
		else
		{
			string path = "T:/MeasuredBRDF/brdfs/" + ground_truth_param[0] + ".binary";
			render_measured_BRDF(m_editor, path, gtfilename);
		}
		

	}
}