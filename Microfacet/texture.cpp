#include "texture.h"

void single_texture_sampler::translate_texcoord(Vector2 &output, const Vector2 &input) const
{
	switch (address_mode) {
	case TXA_WRAP:
		if (input.x < 0 || input.x > 1)
		{
			output.x = fmod(input.x, 1.0f);
			if (output.x < 0)
				output.x += 1.0;
		}
		else {
			output.x = input.x;
		}

		if (input.y < 0 || input.y > 1)
		{
			output.y = fmod(input.y, 1.0f);
			if (output.y < 0)
				output.y += 1.0;
		}
		else {
			output.y = input.y;
		}
		break;
	case TXA_CLAMP:
		output.x = max(min(input.x, 1), 0);
		output.y = max(min(input.y, 1), 0);
		break;
	case TXA_BLACK:
		if (input.x < 0 || input.x > 1 || input.y < 0 || input.y > 1)
		{
			output.x = output.y = -1;
		}
		else {
			output = input;
		}
		break;
	}
}

raw_texture_2d::raw_texture_2d()
	: width(0), height(0), mip_level(0), num_components(0), mipmap(NULL)
{

}

raw_texture_2d::~raw_texture_2d()
{
	if (mipmap)
		for (int i = 0; i<mip_level; i++)
			SAFE_DELETE_ARR(mipmap[i]);
}

void raw_texture_2d::set_width(int w)
{
	width = w;
}

void raw_texture_2d::set_height(int h)
{
	height = h;
}

void raw_texture_2d::set_mip_level(int l)
{
	mip_level = l;
}

void raw_texture_2d::set_num_components(int n)
{
	num_components = n;
}

void raw_texture_2d::create_mipmap_level_0()
{
	mipmap = new float*[mip_level];
	memset(mipmap, 0, sizeof(float*)*mip_level);
	mipmap[0] = new float[width * height * num_components];
}

void raw_texture_2d::get_mipmap_level_0(float* &p)
{
	p = mipmap[0];
}


int raw_texture_2d::get_width() const
{
	return width;
}

int raw_texture_2d::get_height() const
{
	return height;
}

int raw_texture_2d::get_mip_level() const
{
	return mip_level;
}

void raw_texture_2d::get_texel(Vector3 &r,//codex::math::vector::vector<float> &r, 
	const int xx, const int yy, const int ll) const
{
	int		x, y, l;

	l = min(max(ll, 0), mip_level - 1);
	x = min(max(xx, 0), mip_width[l] - 1);
	y = min(max(yy, 0), mip_height[l] - 1);

	//r = codex::math::vector::vector<float>(num_components);//temp_vec[0];

	//Only first 3 components will be retrieved.
	for (int n = 0; n<min(3, num_components); n++)
		r[n] = mipmap[l][(x + y*mip_width[l])*num_components + n];
}

void raw_texture_2d::set_texel(Vector3 &r, const int xx, const int yy, const int ll)
{
	int		x, y, l;

	l = min(max(ll, 0), mip_level - 1);
	x = min(max(xx, 0), mip_width[l] - 1);
	y = min(max(yy, 0), mip_height[l] - 1);

	//Only first 3 components will be retrieved.
	for (int n = 0; n<min(3, num_components); n++)
		mipmap[l][(x + y*mip_width[l])*num_components + n] = r[n];
}

const int raw_texture_2d::compute_mip_level_from_dim() const
{
	int dim = min(width, height),
		r_mip_level = 0;

	while (dim > 0)
	{
		dim >>= 1;
		r_mip_level++;
	}

	return r_mip_level;
}

void raw_texture_2d::compute_mipmap()
{
	mip_width[0] = width;
	mip_height[0] = height;

	for (int l = 1; l<mip_level; l++)
	{
		int last_l = l - 1;

		mip_width[l] = mip_width[last_l] / 2;
		mip_height[l] = mip_height[last_l] / 2;
		mipmap[l] = new float[mip_width[l] * mip_height[l] * num_components];
		memset(mipmap[l], 0, sizeof(float) * mip_width[l] * mip_height[l] * num_components);

		for (int y = 0; y<mip_height[l]; y++)
			for (int x = 0; x<mip_width[l]; x++)
			{
				for (int k = 0; k<4; k++)
					for (int n = 0; n<num_components; n++)
						mipmap[l][(x + y*mip_width[l])*num_components + n] +=
						mipmap[last_l][(x * 2 + (k & 1) + (y * 2 + (k / 2))*mip_width[last_l])*num_components + n] / 4;
			}
	}
}

/*
void raw_texture_2d::load_raw_RGB_texture(const char *filename, const int user_mip_level)
{
	FILE *fp;

	fopen_s(&fp, filename, "rb");
	if (!fp)
		throw_error(ERROR_POS, codex::math::EXP_IO, "(raw_texture_2d::load_raw_RGB_texture) failed to open data file '%s'",
		filename);

	codex::graphics::raw_file_header header;

	fread(&header, sizeof(header), 1, fp);

	set_width(header.width);
	set_height(header.height);

	if (header.num_channels != 3)
		throw_error(ERROR_POS, codex::math::EXP_IO, "(raw_texture_2d::load_raw_RGB_texture) # channels <> 3");

	set_num_components(NUM_SPECTRUM_SAMPLES);

	if (user_mip_level == 0)
		set_mip_level(compute_mip_level_from_dim());
	else
		set_mip_level(min(user_mip_level, compute_mip_level_from_dim()));

	create_mipmap_level_0();

	for (int i = 0; i<get_width() * get_height(); i++)
	{
		Vector3 c;

		if ((header.flag & RAW_HEADER_FLOAT_STORAGE) == 0)
		{
			BYTE br, bg, bb;

			fread(&br, sizeof(br), 1, fp);
			fread(&bg, sizeof(bg), 1, fp);
			fread(&bb, sizeof(bb), 1, fp);

			c.x = br / 255.0;
			c.y = bg / 255.0;
			c.z = bb / 255.0;
		}
		else {
			float fr, fg, fb;

			fread(&fr, sizeof(fr), 1, fp);
			fread(&fg, sizeof(fg), 1, fp);
			fread(&fb, sizeof(fb), 1, fp);

			c.x = fr;
			c.y = fg;
			c.z = fb;
		}

		Vector3	s;
		Vector3	xyz;

		RGB_to_XYZ(xyz, c);
		s.from_XYZ(xyz);

		for (int j = 0; j<NUM_SPECTRUM_SAMPLES; j++)
			mipmap[0][i*NUM_SPECTRUM_SAMPLES + j] = s[j];
	}

	fclose(fp);

	compute_mipmap();
}
*/

void raw_texture_2d::load_texture_from_data(const int data_width, const int data_height, const int data_num_comp,
	const float *data,
	const int user_mip_level)
{
	width = data_width;
	height = data_height;
	num_components
		= data_num_comp;
	if (user_mip_level == 0)
		mip_level = compute_mip_level_from_dim();
	else
		mip_level = min(user_mip_level, compute_mip_level_from_dim());

	mipmap = new float*[mip_level];
	memset(mipmap, 0, sizeof(float*)*mip_level);
	mipmap[0] = new float[width * height * num_components];

	memcpy(mipmap[0], data, sizeof(float) * width * height * num_components);

	compute_mipmap();

	//codex::math::vector::vector<float> temp(num_components);

	//for (int i = 0; i<9; i++)
	//{
	//	temp_vec[i].reset();
	//	temp_vec[i] = temp;
	//}
}


void raw_texture_2d::get_single_texel_from_level(Vector3 &r,//codex::math::vector::vector<float> &r, 
	float s, float t, const int level,
	const single_texture_sampler &sampler) const
{
	Vector2 uv;
	sampler.translate_texcoord(uv, Vector2(s, t));
	if (uv.x < 0 || uv.y < 0)
	{
		for (int i = 0; i<num_components; i++)
			r[i] = 0;
		return;
	}

	//switch (address_mode) {
	//case TXA_WRAP:
	//	s = fmod(s, 1.0);
	//	if (s < 0) 
	//		s += 1.0;

	//	t = fmod(t, 1.0);
	//	if (t < 0) 
	//		t += 1.0;
	//	break;
	//case TXA_CLAMP:
	//	s = max(min(s, 1), 0);
	//	t = max(min(t, 1), 0);
	//	break;
	//case TXA_BLACK:
	//	if (s < 0 || s > 1 || t < 0 || t > 1)
	//	{
	//		for (int i = 0; i<num_components; i++)
	//			r(i+1) = 0;
	//		return;
	//	}
	//	break;
	//}

	int x, y;

	x = int(uv.x * mip_width[level]);
	x = min(x, mip_width[level] - 1);
	y = int(uv.y * mip_height[level]);
	y = min(y, mip_height[level] - 1);

	for (int i = 0; i<min(3, num_components); i++)
		r[i] = mipmap[level][(x + y*mip_width[level])*num_components + i];
}


void raw_texture_2d::get_sample(Vector3 &r, const Vector2 &uv,
	const Vector2 &du_d, const Vector2 &dv_d,
	const single_texture_sampler &sampler) const
{
	if (sampler.filter_mode == TXF_NONE)
	{
		get_single_texel_from_level(r, uv.x, uv.y, 0, sampler);
		return;
	}

	float	sampling_rate_u, sampling_rate_v, d;

	sampling_rate_u = 1.0 / width;
	sampling_rate_v = 1.0 / height;

	d = max(max(abs(du_d.x), abs(du_d.y)) / sampling_rate_u, max(abs(dv_d.x), abs(dv_d.y)) / sampling_rate_v);

	if (d <= 1.0 || d >= float(1 << (mip_level - 1))) // lowest or highest mip level
	{
		int level;

		if (d <= 1.0)
			level = 0;
		else
			level = mip_level - 1;

		switch (sampler.filter_mode) {
		case TXF_POINT:
			//r = codex::math::vector::vector<float>(num_components);//temp_vec[0]; //make sure r has the right dimension
			get_single_texel_from_level(r, uv.x, uv.y, level, sampler);
			break;
		case TXF_LINEAR:
		{
			//codex::math::vector::vector<float> tvec(num_components);
			Vector3 tvec[4];

			int c = 0;
			float x_alpha, y_alpha;

			x_alpha = uv.x * mip_width[level] - floor(uv.x * mip_width[level]);
			y_alpha = uv.y * mip_height[level] - floor(uv.y * mip_height[level]);

			get_single_texel_from_level(tvec[0], uv.x, uv.y, level, sampler);
			//r = tvec*(1-x_alpha)*(1-y_alpha); 
			get_single_texel_from_level(tvec[1], uv.x + 1.0 / mip_width[level], uv.y, level, sampler);
			//r += tvec*x_alpha*(1-y_alpha);
			get_single_texel_from_level(tvec[2], uv.x, uv.y + 1.0 / mip_height[level], level, sampler);
			//r += tvec*(1-x_alpha)*y_alpha;
			get_single_texel_from_level(tvec[3], uv.x + 1.0 / mip_width[level], uv.y + 1.0 / mip_height[level], level, sampler);
			//r += tvec*x_alpha*y_alpha;

			r = tvec[0] * (1 - x_alpha)*(1 - y_alpha) +
				tvec[1] * x_alpha*(1 - y_alpha) +
				tvec[2] * (1 - x_alpha)*y_alpha +
				tvec[3] * x_alpha*y_alpha;
			break;
		}
		}
	}
	else { // in-between
		float	log_d, z_alpha;
		int		level;

		log_d = log(d) / log(2.0);
		level = int(log_d);

		//z_alpha = log_d - level;
		z_alpha = float(d - (1 << level)) / float(1 << level);

		////DEBUG
		//printf_s("\n(%g %g %g %g)\n", du_d.x, du_d.y, dv_d.x, dv_d.y);
		//if (level == 0)
		//{
		//	r(1) = 1;
		//	r(2) = 1;
		//	r(3) = 0;
		//}
		//if (level == 1)
		//{
		//	r(1) = 1;
		//	r(2) = 0;
		//	r(3) = 0;
		//}
		//if (level == 2)
		//{
		//	r(1) = 0;
		//	r(2) = 1;
		//	r(3) = 0;
		//}
		//if (level == 3)
		//{
		//	r(1) = 0;
		//	r(2) = 0;
		//	r(3) = 1;
		//}
		//return;

		switch (sampler.filter_mode) {
		case TXF_POINT:
			//r = codex::math::vector::vector<float>(num_components); //temp_vec[0]; //make sure r has the right dimension
			get_single_texel_from_level(r, uv.x, uv.y, min(int(log_d + 0.5), mip_level - 1), sampler);
			break;
		case TXF_LINEAR:
		{
			//codex::math::vector::vector<float> tvec(num_components);
			Vector3 tvec[8];

			int c = 0;
			float x_alpha, y_alpha;

			x_alpha = uv.x * mip_width[level] - floor(uv.x * mip_width[level]);
			y_alpha = uv.y * mip_height[level] - floor(uv.y * mip_height[level]);

			get_single_texel_from_level(tvec[0], uv.x, uv.y, level, sampler);
			//r = tvec*(1-x_alpha)*(1-y_alpha) * (1-z_alpha);
			get_single_texel_from_level(tvec[1], uv.x + 1.0 / mip_width[level], uv.y, level, sampler);
			//r += tvec*x_alpha*(1-y_alpha) * (1-z_alpha);
			get_single_texel_from_level(tvec[2], uv.x, uv.y + 1.0 / mip_height[level], level, sampler);
			//r += tvec*(1-x_alpha)*y_alpha * (1-z_alpha);
			get_single_texel_from_level(tvec[3], uv.x + 1.0 / mip_width[level], uv.y + 1.0 / mip_height[level], level, sampler);
			//r += tvec*x_alpha*y_alpha * (1-z_alpha);
			get_single_texel_from_level(tvec[4], uv.x, uv.y, level + 1, sampler);
			//r += tvec*(1-x_alpha)*(1-y_alpha) * z_alpha;
			get_single_texel_from_level(tvec[5], uv.x + 1.0 / mip_width[level], uv.y, level + 1, sampler);
			//r += tvec*x_alpha*(1-y_alpha) * z_alpha;
			get_single_texel_from_level(tvec[6], uv.x, uv.y + 1.0 / mip_height[level], level + 1, sampler);
			//r += tvec*(1-x_alpha)*y_alpha * z_alpha;
			get_single_texel_from_level(tvec[7], uv.x + 1.0 / mip_width[level], uv.y + 1.0 / mip_height[level], level + 1, sampler);
			//r += tvec*x_alpha*y_alpha * z_alpha;

			r = (tvec[0] * (1 - x_alpha)*(1 - y_alpha) +
				tvec[1] * x_alpha*(1 - y_alpha) +
				tvec[2] * (1 - x_alpha)*y_alpha +
				tvec[3] * x_alpha*y_alpha) * (1 - z_alpha) +

				(tvec[4] * (1 - x_alpha)*(1 - y_alpha) +
				tvec[5] * x_alpha*(1 - y_alpha) +
				tvec[6] * (1 - x_alpha)*y_alpha +
				tvec[7] * x_alpha*y_alpha) * z_alpha;
			//tvec[4]*z_alpha;
			break;
		}
		}
	}
}