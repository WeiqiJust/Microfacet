#include "r_instance.h"

r_instance::r_instance()
	: p_sh(NULL), p_shinput(NULL), b_vertex_mapping(false)
{

}

r_instance::~r_instance()
{
	SAFE_DELETE(p_shinput);
}

void r_instance::init(shared_ptr<r_geom_hierarchy> &p, const float scale, r_shader *psh, bool b_v_map)
{
	//cout << "in produce init " << endl;
	//cout << "total area = " << p->get_geom()->get_total_area() << endl;
	set_geom(p, scale);
	p_sh = psh;
	p_sh->create_shader_input(p_shinput);
	b_vertex_mapping = b_v_map;
}

void r_instance::draw()
{
	draw_with_shader(p_sh);
}

void r_instance::draw_with_shader(r_shader *psh)
{
	ginst.get_geom()->set();
	psh->setup_render();
	ginst.get_geom()->draw();
}

void r_instance::set_geom(shared_ptr<r_geom_hierarchy> &p, const float scale)
{
	ginst.init(p);
	ginst.set_scale(scale);
}

shared_ptr<r_geometry> r_instance::get_geom()
{
	return ginst.get_geom();
}

r_shader* r_instance::get_shader()
{
	return p_sh;
}

void r_instance::set_shader_input(r_shader_input *p)
{
	p_shinput = p;
}

r_shader_input *r_instance::get_shader_input()
{
	return p_shinput;
}

void r_instance::get_samples(const std::vector<Vector3> *&p,
	const std::vector<normal_weight> *&n,
	float &scale)
{
	ginst.get_samples(p, n, scale);
}

void r_instance::sample_points(const float density,
	const int num_area_hits_scalar)
{
	ginst.sample_points(density, num_area_hits_scalar);
}

bool r_instance::is_vertex_mapping() const
{
	return b_vertex_mapping;
}

void r_instance::duplicate(r_instance* &p)
{
	p = new r_instance;
	*p = *this;

	r_shader_input *pshinput;
	p_sh->create_shader_input(pshinput);
	p->set_shader_input(pshinput);
}