#include "MicrofacetEditor.h"

void create_reflectance_table(MicrofacetEditor& m_editor, const string filename);

void render(MicrofacetEditor& m_editor, const string filename);

void render_ref_BRDF(MicrofacetEditor& m_editor, string roughness, Vector3 albedo, const string filename);

void render_measured_BRDF(MicrofacetEditor& m_editor, string measuredBrdf, const string filename);

void generate_grid_plane(MicrofacetEditor& m_editor);

void generate_grid_plane_test(MicrofacetEditor& m_editor);

void render_grid_plane_prediction(MicrofacetEditor& m_editor);

void generate_database(MicrofacetEditor& m_editor);

void generate_database_test(MicrofacetEditor& m_editor);