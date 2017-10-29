#include "MicrofacetEditor.h"

void create_reflectance_table(MicrofacetEditor& m_editor, const string filename);

void render(MicrofacetEditor& m_editor, const string filename);

void render_ref_BRDF(MicrofacetEditor& m_editor, string roughness, Vector3 albedo, const string filename);

void render_measured_BRDF(MicrofacetEditor& m_editor, string measuredBrdf, const string filename);

/* generate dataset for neural net */
void generate_grid_plane(MicrofacetEditor& m_editor);

void generate_grid_plane_test(MicrofacetEditor& m_editor);

void generate_grid_plane_prediction(MicrofacetEditor& m_editor);

void generate_groove_prediction(MicrofacetEditor& m_editor);

/* generate color dataset with various albedo */
void generate_grid_plane_color_dataset(MicrofacetEditor& m_editor, int albedo_sample);

/* render final result and ground truth*/

void render_grid_plane_prediction(MicrofacetEditor& m_editor);

inline vector<string> split_filename(const string name)
{
	std::string delimiter = "_";
	string filename = name;
	size_t pos = 0;
	std::string token;
	vector<string> param;
	while ((pos = filename.find(delimiter)) != std::string::npos) {
		token = filename.substr(0, pos);
		param.push_back(token);
		filename.erase(0, pos + delimiter.length());
	}
	param.push_back(filename);
	return param;
}

/* legacy code */
void generate_database(MicrofacetEditor& m_editor);

void generate_database_test(MicrofacetEditor& m_editor);