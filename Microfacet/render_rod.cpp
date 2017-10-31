#include "render_dataset.h"
#include <sys/stat.h>
#include <fstream>

struct rod
{
	float roughness, density, theta, phi;
	Vector3 albedo;
};

void generate_rod(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_rod/", mtr_name, file_name;
	float density, theta, phi;
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	std::cout << "Start Rendering Rod Training Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt++) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 0; albedo_cnt < 10; albedo_cnt += 2)
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

			for (int density_cnt = 1; density_cnt <= 10; density_cnt++)
			{
				density = density_cnt;
				std::cout << "    Microstructure scale = " << density << "....";
				for (int theta_cnt = 0; theta_cnt <= 6; theta_cnt++)
				{
					theta = 15 * theta_cnt;
					for (int phi_cnt = 0; phi_cnt < 10; phi_cnt++)
					{
						phi = phi_cnt * 36;
						file_name = mtr_name + "/" + precision(density) + "_" + precision(theta) + "_" + precision(phi) + ".jpg";
						struct stat buffer;
						if (stat(file_name.c_str(), &buffer) == 0)
							continue;
						microfacet_distr* distr = m_editor.generate_distr_rod(density, theta, phi, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
						create_reflectance_table(m_editor, file_name);
					}
				}
				std::cout << "done" << endl;
			}

		}
	}
}

void generate_rod_test(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_rod_test/", mtr_name, file_name;
	float density, theta, phi;
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	std::cout << "Start Rendering Rod Training Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt += 2) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 1; albedo_cnt < 10; albedo_cnt += 2)
		{
			float albedo = 0.1*albedo_cnt;
			std::cout << "Rendering material: " << material << " albedo: " << albedo << endl;

			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");

			for (int density_cnt = 1; density_cnt <= 10; density_cnt += 2)
			{
				density = density_cnt + 0.5;
				std::cout << "    Microstructure scale = " << density << "....";
				for (int theta_cnt = 1; theta_cnt <= 4; theta_cnt++)
				{
					theta = 20 * theta_cnt;
					for (int phi_cnt = 1; phi_cnt <= 5; phi_cnt++)
					{
						phi = phi_cnt * 60;
						file_name = mtr_name + "/" + precision(density) + "_" + precision(theta) + "_" + precision(phi) + ".jpg";
						struct stat buffer;
						if (stat(file_name.c_str(), &buffer) == 0)
							continue;
						microfacet_distr* distr = m_editor.generate_distr_rod(density, theta, phi, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
						create_reflectance_table(m_editor, file_name);
					}
				}
				std::cout << "done" << endl;
			}

		}
	}
}

// edit param
void generate_rod_prediction(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string file = "C:/Users/justin/Documents/Microfacet_rod_prediction/", mtr_name, file_name;
	float density, theta, phi;
	string binder_name, distr_name;
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);
	std::cout << "Start Rendering Rod Training Dataset " << endl;
	for (int rough_cnt = 2; rough_cnt < 10; rough_cnt += 2) //2<10
	{
		string material = "ward_0." + to_string(rough_cnt);
		for (int albedo_cnt = 1; albedo_cnt < 10; albedo_cnt += 2)
		{
			float albedo = 0.1*albedo_cnt;
			std::cout << "Rendering material: " << material << " albedo: " << albedo << endl;

			mtr_name = file + material + "_" + precision(albedo);
			std::wstring mtr_name_wstring(mtr_name.length(), L' ');
			std::copy(mtr_name.begin(), mtr_name.end(), mtr_name_wstring.begin());
			CreateDirectory(mtr_name_wstring.c_str(), NULL);

			m_editor.load_material(Vector3(albedo), material, "matr_binder_0", "matr_distr_0");

			for (int density_cnt = 1; density_cnt <= 10; density_cnt += 2)
			{
				density = density_cnt + 0.5;
				std::cout << "    Microstructure scale = " << density << "....";
				for (int theta_cnt = 1; theta_cnt <= 4; theta_cnt++)
				{
					theta = 20 * theta_cnt;
					for (int phi_cnt = 1; phi_cnt <= 5; phi_cnt++)
					{
						phi = phi_cnt * 60;
						file_name = mtr_name + "/" + precision(density) + "_" + precision(theta) + "_" + precision(phi) + ".jpg";
						struct stat buffer;
						if (stat(file_name.c_str(), &buffer) == 0)
							continue;
						microfacet_distr* distr = m_editor.generate_distr_rod(density, theta, phi, distr_name);
						m_editor.generate_microfacet_details(binder, distr, 1, 1, 10.0, 200, 8, binder_name, distr_name, false);
						create_reflectance_table(m_editor, file_name);
					}
				}
				std::cout << "done" << endl;
			}

		}
	}
}

void render_rod_prediction(MicrofacetEditor& m_editor)
{
	m_editor.load_cube_map("T:/Microfacet/data/cube_texture/cube", 0, 2, Vector3(1.0f), 2, Identity());
	m_editor.set_view_direction(Vector3(0, 0.5, 3), Vector3(0), Vector3(0, 1, 0));
	string samplefile = "T:/Microfacet/data/cube_texture/cubelight.txt";
	m_editor.load_sky_box(0, samplefile);
	string grayfile = "T:/Microfacet/output/result/predict_gray_rod.txt";
	string colorfile = "T:/Microfacet/output/result/predict_color_rod.txt";
	string path = "T:/Microfacet/output/result/rod";
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
	microfacet_binder* binder = m_editor.generate_binder_plane(binder_name);;
	microfacet_distr* distr;
	map<string, rod> predict;
	for (int i = 0; i < count_g; i++)
	{
		rod param;
		string filename;
		float albedo;
		fpg >> filename >> param.roughness >> albedo >> param.density >> param.theta >> param.phi;
		predict[filename] = param;
	}
	fpg.close();

	for (int i = 0; i < count_c; i++)
	{
		Vector3 albedo;
		string filename;
		fpc >> filename >> albedo.x >> albedo.y >> albedo.z;
		if (predict.find(filename) != predict.end())
		{
			predict[filename].albedo = albedo;
		}
		else
		{
			cout << "Error: Could not find file name " << filename << " in predict_color.txt!" << endl;
			exit(0);
		}
	}
	fpc.close();

	for (map<string, rod>::iterator it = predict.begin(); it != predict.end(); ++it)
	{
		string filename = it->first;
		rod param = it->second;

		float roughness = (float)floor(param.roughness * 10 + 0.5f) / 10;
		if (roughness < 0.2) roughness = 0.2f;
		if (roughness > 0.9) roughness = 0.9f;
		string material = "ward_" + precision(roughness);
		m_editor.load_material(param.albedo, material, "matr_binder_0", "matr_distr_0");
		distr = m_editor.generate_distr_rod(param.density, param.theta, param.phi, distr_name);
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
