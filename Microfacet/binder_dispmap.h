#include "binder.h"

class binder_dispmap_param
{
public:
	int		res_x, res_y;
	float	amp;
};

class binder_dispmap : public microfacet_binder
{
protected:
	binder_dispmap_param disp_param;

	int width, height;
	std::vector<float> dispmap;
	base_tri_mesh obj;

	void init_obj();
	void generate_geom_from_dispmap(const float density, const int num_area_hits_scalar);

public:
	void set_dispmap(const std::vector<float> &data);

	void set_param(const binder_dispmap_param &p);
	virtual void* get_param();
	virtual void generate(std::vector<instance_property*> &results,
		const Vector3 &v_min, const Vector3 &v_max,
		const std::vector<const std::vector<instance_property*>*> &neighbors);
	virtual void generate_geom(const float density, const int num_area_hits_scalar);
};