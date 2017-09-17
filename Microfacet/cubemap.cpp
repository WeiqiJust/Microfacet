#include "cubemap.h"
#include "hammersley.h"

//cubemap
void cubemap::init(const int d)
{
	dim = d;
	data.resize(6*dim*dim);
}

float* cubemap::get_face(const int idx)
{
	return &data[idx*dim*dim];
}

const float* cubemap::get_face(const int idx) const
{
	return &data[idx*dim*dim];
}

int main_axis(const Vector3 &p)
{
	float	absx, absy, absz;
	int		axis;

	absx = abs(p.x);
	absy = abs(p.y);
	absz = abs(p.z);

	if (absx >= absy && absx >= absz)
		axis = 0;
	else if (absy >= absx && absy >= absz)
		axis = 1;
	else
		axis = 2;

	return axis;
}

int cubemap::get_dim() const
{
	return dim;
}

int	cubemap::get_addr(const Vector3 &n) const
{
	int axis = main_axis(n), face;

	switch (axis) {
	case 0:
		face = (n[axis] > 0) ? CUBEMAP_RIGHT : CUBEMAP_LEFT;
		break;
	case 1:
		face = (n[axis] > 0) ? CUBEMAP_TOP : CUBEMAP_BOTTOM;
		break;
	case 2:
		face = (n[axis] > 0) ? CUBEMAP_FRONT : CUBEMAP_BACK;
		break;
	}

	Vector3 v_proj = n / abs(n[axis]);
	Vector2 uv;

	uv.x = (Dot(v_proj, box_frame_x[face]) + 1)*get_dim()/2.0f - 0.5f;
	uv.y = (Dot(-v_proj, box_frame_y[face]) + 1)*get_dim() / 2.0f - 0.5f;

	int x, y;
	x = min(max(int(uv.x+0.5f), 0), get_dim()-1);
	y = min(max(int(uv.y+0.5f), 0), get_dim()-1);

	return face*get_dim()*get_dim()+y*get_dim()+x;
}

#define CUBEMAP_SUBSAMPLE	32

void cubemap::save_obj(const char *filename, const tri_mesh &mesh) const
{
	tri_mesh m = mesh;

	for (int i = 0; i < m.get_vertex_number(); i++)
	{
		hammersley seq(CUBEMAP_SUBSAMPLE, 2);
		Vector3 dir = m.normals[i];
		
		Vector3 t, b;
		build_frame(t, b, dir);

		float v = 0;
		for (int j = 0; j < CUBEMAP_SUBSAMPLE; j++)
		{
			Vector2 temp, rv;
			float *re = seq.get_sample();
			rv = Vector2(re[0], re[1]);
			uniform_disk_sampling(temp, rv);

			Vector3 vec = dir + t*temp.x + b*temp.y;
			vec = Normalize(vec);
			v += get_face(0)[get_addr(Vector3(vec.x, vec.y, vec.z))] / CUBEMAP_SUBSAMPLE;
		}

		m.vertices[i] = v*dir;
	}

	m.save_obj(filename);
}

//hemi_cubemap
void hemi_cubemap::init(const int d)
{
	dim = d;
	data.resize(3*dim*dim);
}

float* hemi_cubemap::get_face()
{
	return &data[0];
}

const float* hemi_cubemap::get_face() const
{
	return &data[0];
}

int hemi_cubemap::get_dim() const
{
	return dim;
}

int	hemi_cubemap::get_addr(const Vector3 &n) const
{
	if (n.z < 0) 
		return -1;

	int axis = main_axis(n), face;

	switch (axis) {
	case 0:
		face = (n[axis] > 0) ? CUBEMAP_RIGHT : CUBEMAP_LEFT;
		break;
	case 1:
		face = (n[axis] > 0) ? CUBEMAP_TOP : CUBEMAP_BOTTOM;
		break;
	case 2:
		face = (n[axis] > 0) ? CUBEMAP_FRONT : CUBEMAP_BACK;
		break;
	}

	Vector3 v_proj = n / abs(n[axis]);
	Vector2 uv;

	uv.x = ( Dot(v_proj, box_frame_x[face]) + 1)*get_dim()/2.0f - 0.5f;
	uv.y = (Dot(-v_proj, box_frame_y[face]) + 1)*get_dim()/2.0f - 0.5f;

	int x, y, addr;

	switch (face) {
	case CUBEMAP_FRONT:
		x = min(max(int(uv.x+0.5f), 0), get_dim()-1);
		y = min(max(int(uv.y+0.5f), 0), get_dim()-1);
		addr = y*get_dim()+x;
		break;
	case CUBEMAP_BOTTOM:
		uv.y -= get_dim()/2.0f;
		x = min(max(int(uv.x+0.5f), 0), get_dim()-1);
		y = min(max(int(uv.y+0.5f), 0), get_dim()/2-1);
		addr = get_dim()*get_dim() + x+y*get_dim();
		break;
	case CUBEMAP_TOP:
		x = min(max(int(uv.x+0.5f), 0), get_dim()-1);
		y = min(max(int(uv.y+0.5f), 0), get_dim()/2-1);
		addr = get_dim()*get_dim()*3/2 + x+y*get_dim();
		break;
	case CUBEMAP_LEFT:
		x = min(max(int(uv.x+0.5f), 0), get_dim()/2-1);
		y = min(max(int(uv.y+0.5f), 0), get_dim()-1);
		addr = get_dim()*get_dim()*2 + x+y*get_dim()/2;
		break;
	case CUBEMAP_RIGHT:
		uv.x -= get_dim()/2.0f;
		x = min(max(int(uv.x+0.5f), 0), get_dim()/2-1);
		y = min(max(int(uv.y+0.5f), 0), get_dim()-1);
		addr = get_dim()*get_dim()*5/2 + x+y*get_dim()/2;
		break;
	}

	return addr;
}

void hemi_cubemap::save_obj(const char *filename, const tri_mesh &mesh) const
{
	tri_mesh m = mesh;

	for (int i = 0; i < m.get_vertex_number(); i++)
	{
		hammersley seq(CUBEMAP_SUBSAMPLE, 2);
		Vector3 dir = m.normals[i];

		Vector3 t, b;
		build_frame(t, b, dir);

		float v = 0;
		for (int j = 0; j < CUBEMAP_SUBSAMPLE; j++)
		{
			Vector2 temp, rv;
			float *re = seq.get_sample();
			rv = Vector2(re[0], re[1]);
			uniform_disk_sampling(temp, rv);

			Vector3 vec = dir + t*temp.x + b*temp.y;
			vec = Normalize(vec);

			int addr = get_addr(Vector3(vec.x, vec.y, vec.z));
			if (addr >= 0)
				v += get_face()[addr] / CUBEMAP_SUBSAMPLE;
		}

		m.vertices[i] = v*dir;
	}

	m.save_obj(filename);
}
