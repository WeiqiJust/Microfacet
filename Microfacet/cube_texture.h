#pragma once
#include "utils.h"
#include "texture.h"

class cube_texture
{
public:
	cube_texture()
	{
		for (int i = 0; i < 6; i++)
			faces[i] = new raw_texture_2d();
		dim = -1;
	};

	~cube_texture()
	{
		for (int i = 0; i < 6; i++)
			SAFE_DELETE(faces[i]);
	}

	void load_texture(const string filename, const int user_mip_level); //filename should be full path without subscript

	vector<Vector3> get_face(int face);

	Vector3 get_texel(int face, int x, int y);

	int get_dim() { return dim; }

	void cube_texture::get_sample(Vector3 &r,//codex::math::vector::vector<Number> &r, 
		const int face_index,
		const Vector2 &uv,
		const Vector2 &du_d, const Vector2 &dv_d,
		const single_texture_sampler &sampler) const;
private:
	int dim;
	vector<vector<Vector3> > data; // each face -> each pixel (1d) -> color channel
	raw_texture_2d *faces[6]; // Save RGB texutre instead of XYZ in zrt!!!
};