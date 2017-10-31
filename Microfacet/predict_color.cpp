#include "render_dataset.h"
#include <fstream>

void generate_grid_plane_color_reflectance(MicrofacetEditor& m_editor, int albedo_sample, const string path, const float roughness, const float scale, const float x, const float y)
{
	
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/violentdays", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0, "T:/Microfacet/data/cube_texture/cubelight.txt");
	
	std::clock_t start;
	double duration;
	m_editor.set_view_direction(Vector3(0, 1, 6), Vector3(0), Vector3(0, 1, 0));
	start = std::clock();
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(x, y, 0, scale, 0, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);

	float albedo_step = float(1) / albedo_sample;
	float rough = (float)floor(roughness * 10 + 0.5f) / 10;
	if (rough < 0.2) rough = 0.2f;
	if (rough > 0.9) rough = 0.9f;
	string material = "ward_" + precision(rough);

	for (int i = 1; i <= albedo_sample; i++)
	{
		float albedo = albedo_step*i;
		string filename = path + precision(albedo) + ".png";
		m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
		create_reflectance_table(m_editor, filename);
	}

	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "time: " << duration << '\n';
}

void generate_grid_plane_color_dataset(MicrofacetEditor& m_editor, int albedo_sample)
{
	string grayfile = "T:/Microfacet/output/color_variation/predict_gray_grid_plane.txt";
	ifstream fp(grayfile);
	int count;
	fp >> count;
	string path = "T:/Microfacet/output/color_variation/";

	for (int i = 0; i < count; i++)
	{
		
		string filename, folder;
		float roughness, scale, x, y, albedo;
		fp >> filename >> roughness >> albedo>> scale >> x >> y;

		folder = path + filename;
		std::wstring folder_wstring(folder.length(), L' ');
		std::copy(folder.begin(), folder.end(), folder_wstring.begin());
		CreateDirectory(folder_wstring.c_str(), NULL);

		folder = folder + "/";
		generate_grid_plane_color_reflectance(m_editor, albedo_sample, folder, roughness, scale, x, y);
		cout << folder << " finished!" << endl;

	}
	fp.close();
}


void generate_groove_color_reflectance(MicrofacetEditor& m_editor, int albedo_sample, const string path, const float roughness, const float p, const float h)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/violentdays", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0, "T:/Microfacet/data/cube_texture/cubelight.txt");

	std::clock_t start;
	double duration;
	m_editor.set_view_direction(Vector3(0, 1, 6), Vector3(0), Vector3(0, 1, 0));
	start = std::clock();
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_groove(p, h, binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);

	float albedo_step = float(1) / albedo_sample;
	float rough = (float)floor(roughness * 10 + 0.5f) / 10;
	if (rough < 0.2) rough = 0.2f;
	if (rough > 0.9) rough = 0.9f;
	string material = "ward_" + precision(rough);

	for (int i = 1; i <= albedo_sample; i++)
	{
		float albedo0 = albedo_step*i;
		for (int j = 1; j <= albedo_sample; j++)
		{
			float albedo1 = albedo_step*j;
			string filename = path + precision(albedo0) + "_" + precision(albedo1) + ".png";
			m_editor.load_material(Vector3(albedo0), Vector3(albedo1), material, "matr_binder_0", "matr_binder_1", "matr_distr_0");
			create_reflectance_table(m_editor, filename);
		}
	}

	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "time: " << duration << '\n';
}

void generate_groove_color_dataset(MicrofacetEditor& m_editor, int albedo_sample)
{
	string grayfile = "T:/Microfacet/output/color_variation/predict_gray_groove.txt";
	ifstream fp(grayfile);
	int count;
	fp >> count;
	string path = "T:/Microfacet/output/color_variation/";

	for (int i = 0; i < count; i++)
	{

		string filename, folder;
		float roughness, p, h, albedo0, albedo1;
		fp >> filename >> roughness >> albedo0 >> albedo1 >> p >> h;

		folder = path + filename;
		std::wstring folder_wstring(folder.length(), L' ');
		std::copy(folder.begin(), folder.end(), folder_wstring.begin());
		CreateDirectory(folder_wstring.c_str(), NULL);

		folder = folder + "/";
		generate_groove_color_reflectance(m_editor, albedo_sample, folder, roughness, p, h);
		cout << folder << " finished!" << endl;
	}
	fp.close();
}

void generate_woven_color_reflectance(MicrofacetEditor& m_editor, int albedo_sample, const string path, const float roughness, const float w, const float d, const float h)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/violentdays", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0, "T:/Microfacet/data/cube_texture/cubelight.txt");

	std::clock_t start;
	double duration;
	m_editor.set_view_direction(Vector3(0, 1, 6), Vector3(0), Vector3(0, 1, 0));
	start = std::clock();
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_woven(2, 2, 16, 32, w, d, h, w, d, h, binder_name);
	microfacet_distr* distr = m_editor.generate_distr_grid(1, 1, 0, 0, 0, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);

	float albedo_step = float(1) / albedo_sample;
	float rough = (float)floor(roughness * 10 + 0.5f) / 10;
	if (rough < 0.2) rough = 0.2f;
	if (rough > 0.9) rough = 0.9f;
	string material = "ward_" + precision(rough);

	for (int i = 1; i <= albedo_sample; i++)
	{
		float albedo0 = albedo_step*i;
		for (int j = 1; j <= albedo_sample; j++)
		{
			float albedo1 = albedo_step*j;
			string filename = path + precision(albedo0) + "_" + precision(albedo1) + ".png";
			m_editor.load_material(Vector3(albedo0), Vector3(albedo1), material, "matr_binder_0", "matr_binder_1", "matr_distr_0");
			create_reflectance_table(m_editor, filename);
		}
	}

	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "time: " << duration << '\n';
}

void generate_woven_color_dataset(MicrofacetEditor& m_editor, int albedo_sample)
{
	string grayfile = "T:/Microfacet/output/color_variation/predict_gray_woven.txt";
	ifstream fp(grayfile);
	int count;
	fp >> count;
	string path = "T:/Microfacet/output/color_variation/";

	for (int i = 0; i < count; i++)
	{

		string filename, folder;
		float roughness, w, d, h, albedo0, albedo1;
		fp >> filename >> roughness >> albedo0 >> albedo1 >> w >> d >> h;

		folder = path + filename;
		std::wstring folder_wstring(folder.length(), L' ');
		std::copy(folder.begin(), folder.end(), folder_wstring.begin());
		CreateDirectory(folder_wstring.c_str(), NULL);

		folder = folder + "/";
		generate_woven_color_reflectance(m_editor, albedo_sample, folder, roughness, w, d, h);
		cout << folder << " finished!" << endl;
	}
	fp.close();
}

void generate_rod_color_reflectance(MicrofacetEditor& m_editor, int albedo_sample, const string path, const float roughness, const float density, const float theta, const float phi)
{

	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/violentdays", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.load_sky_box(0, "T:/Microfacet/data/cube_texture/cubelight.txt");

	std::clock_t start;
	double duration;
	m_editor.set_view_direction(Vector3(0, 1, 6), Vector3(0), Vector3(0, 1, 0));
	start = std::clock();
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	microfacet_distr* distr = m_editor.generate_distr_rod(density, theta, phi, distr_name);
	m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);

	float albedo_step = float(1) / albedo_sample;
	float rough = (float)floor(roughness * 10 + 0.5f) / 10;
	if (rough < 0.2) rough = 0.2f;
	if (rough > 0.9) rough = 0.9f;
	string material = "ward_" + precision(rough);

	for (int i = 1; i <= albedo_sample; i++)
	{
		float albedo = albedo_step*i;
		string filename = path + precision(albedo) + ".png";
		m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");
		create_reflectance_table(m_editor, filename);
	}

	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "time: " << duration << '\n';
}

void generate_rod_color_dataset(MicrofacetEditor& m_editor, int albedo_sample)
{
	string grayfile = "T:/Microfacet/output/color_variation/predict_gray_rod.txt";
	ifstream fp(grayfile);
	int count;
	fp >> count;
	string path = "T:/Microfacet/output/color_variation/";

	for (int i = 0; i < count; i++)
	{

		string filename, folder;
		float roughness, density, theta, phi, albedo;
		fp >> filename >> roughness >> albedo >> density >> theta >> phi;

		folder = path + filename;
		std::wstring folder_wstring(folder.length(), L' ');
		std::copy(folder.begin(), folder.end(), folder_wstring.begin());
		CreateDirectory(folder_wstring.c_str(), NULL);

		folder = folder + "/";
		generate_rod_color_reflectance(m_editor, albedo_sample, folder, roughness, density, theta, phi);
		cout << folder << " finished!" << endl;

	}
	fp.close();
}


