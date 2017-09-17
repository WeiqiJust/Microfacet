#include "cube_texture.h"
#include <Magick++.h> 
#include <string>
using namespace Magick;
void cube_texture::load_texture(const string filename, const int user_mip_level)
{
	for (int i = 0; i < 6; i++)
	{
		string file = filename + "_" + std::to_string(i) + ".jpg";
		Magick::Image image(file);
		//Magick::Image image("T:/test.png");
		if (image.columns() != image.rows())
			cout << "Error: cube texture width and height are not same! : " << file << endl;
		if (i == 0)
			dim = min(image.columns(), image.rows());
		else
		{
			if (dim != min(image.columns(), image.rows()))
				cout << "Error: cube texture images are not in same size!" << endl;
			dim = min(min(image.columns(), image.rows()), dim);
		}
		Magick::Quantum *pixels = image.getPixels(0, 0, image.columns(), image.rows());
		vector<Vector3> face;
		float* data_zrt = new float[image.columns()*image.rows()*image.channels()];
		//vector<float> test_image;
		for (int x = 0; x < image.columns(); x++)
			for (int y = 0; y < image.rows(); y++)
			{
				int offset = image.channels() *(image.rows()*x + y);
				Vector3 color = Vector3((float)pixels[offset] / 65535, (float)pixels[offset + 1] / 65535, (float)pixels[offset + 2]/65535);
				face.push_back(color);

				for (int k = 0; k < image.channels(); k++)
				{ 
					data_zrt[offset + k] = (float)pixels[offset + k]/65535;
					//cout << data_zrt[offset + k] << " ";
					//test_image.push_back((float)pixels[offset + k] / 65535);
				}

				
			}
		data.push_back(face);
		faces[i]->load_texture_from_data(image.columns(), image.rows(), image.channels(), data_zrt, user_mip_level);
		SAFE_DELETE_ARR(data_zrt);
		//string filetest = filename + "_test" + std::to_string(i) + ".jpg";
		//save_image_color(filetest.c_str(), test_image, image.columns(), image.rows());
	}
	cout << "Finish loading cube texutre." << endl;
}

vector<Vector3> cube_texture::get_face(int face)
{
	return data[face];
}

Vector3 cube_texture::get_texel(int face, int x, int y)
{
	Vector3 texel;
	faces[face]->get_texel(texel, x, y, 0);
	return texel;
	//return data[face][dim*x + y];
}

void cube_texture::get_sample(Vector3 &r,//codex::math::vector::vector<Number> &r, 
	const int face_index,
	const Vector2 &uv,
	const Vector2 &du_d, const Vector2 &dv_d,
	const single_texture_sampler &sampler) const
{
	if (face_index >= 0 && face_index < 6)
	{
		faces[face_index]->get_sample(r, uv, du_d, dv_d, sampler);
	}
}