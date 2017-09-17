#pragma once
#include "utils.h"

typedef enum
{
	TXF_NONE = 0,
	TXF_POINT = 1,
	TXF_LINEAR = 2,
} texture_filter_type;

typedef enum
{
	TXA_WRAP = 1,
	TXA_CLAMP = 2,
	TXA_BLACK = 3,
} texture_address_type;

class single_texture_sampler
{
public:
	texture_filter_type		filter_mode;
	texture_address_type	address_mode;

	void translate_texcoord(Vector2 &output, const Vector2 &input) const;
};

//*****************************************************************************
// texture_2d abstract class
//*****************************************************************************
class texture_2d// : public codex::math::smart_pointer
{
public:
	texture_2d() {};
	virtual ~texture_2d() {};

	virtual int get_width() const = 0;
	virtual int get_height() const = 0;
	virtual int get_mip_level() const = 0;
	virtual void get_texel(Vector3 &r, const int x, const int y, const int level) const = 0;
	virtual void set_texel(Vector3 &r, const int x, const int y, const int level) = 0;

	virtual void get_sample(//codex::math::vector::vector<float> &r, 
		Vector3 &r,
		const Vector2 &uv,
		const Vector2 &du_d, const Vector2 &dv_d,
		const single_texture_sampler &sampler) const = 0;
};

class raw_texture_2d : public texture_2d
{
private:
	int		width, height, mip_level, num_components;
	float	**mipmap;
	int		mip_width[MAX_MIP_LEVEL], mip_height[MAX_MIP_LEVEL];

	void get_single_texel_from_level(Vector3 &r,//codex::math::vector::vector<float> &r, 
		float s, float t, const int level,
		const single_texture_sampler &sampler) const;
protected:
	void set_width(int w);
	void set_height(int h);
	void set_mip_level(int l);
	void set_num_components(int n);
	void create_mipmap_level_0();
	void get_mipmap_level_0(float* &p);

	const int compute_mip_level_from_dim() const;
	void compute_mipmap();

public:
	raw_texture_2d();
	virtual ~raw_texture_2d();

	virtual int get_width() const;
	virtual int get_height() const;
	virtual int get_mip_level() const;
	virtual void get_texel(Vector3 &r,
		const int x, const int y, const int level) const;
	virtual void set_texel(Vector3 &r, const int x, const int y, const int level);

	//void load_raw_RGB_texture(const char *filename, const int user_mip_level = 0);
	void load_texture_from_data(const int data_width, const int data_height, const int data_num_comp,
		const float *data,
		const int user_mip_level = 0);

	virtual void get_sample(Vector3 &r,//codex::math::vector::vector<float> &r, 
		const Vector2 &uv,
		const Vector2 &du_d, const Vector2 &dv_d,
		const single_texture_sampler &sampler) const;
};