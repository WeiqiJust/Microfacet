#pragma once
#include "utils.h"
#include "r_geom_inst.h"
#include "r_shader.h"
class r_instance
{
private:
	bool			b_vertex_mapping;
	r_geom_inst		ginst;
	r_shader		*p_sh;
	r_shader_input	*p_shinput;

public:
	r_instance();
	virtual ~r_instance();

	void init(std::shared_ptr<r_geom_hierarchy> &p, const float scale, r_shader *psh, bool b_v_map);
	void draw();
	void draw_with_shader(r_shader *psh);

	void set_geom(shared_ptr<r_geom_hierarchy> &p, const float scale);
	shared_ptr<r_geometry> get_geom();
	r_shader* get_shader();
	void set_shader_input(r_shader_input *p);
	r_shader_input *get_shader_input();

	void get_samples(const std::vector<Vector3> *&p,
		const std::vector<normal_weight> *&n,
		float &scale);
	void sample_points(const float density,
		const int num_area_hits_scalar);

	//for uv-mapping purposes
	bool is_vertex_mapping() const;
	void duplicate(r_instance* &p);
};
