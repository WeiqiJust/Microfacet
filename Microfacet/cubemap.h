#pragma once
#include "utils.h"
#include "tri_mesh.h"
//Conventions:
const int CUBEMAP_RIGHT	= 0;
const int CUBEMAP_LEFT	= 1;
const int CUBEMAP_TOP	= 2;
const int CUBEMAP_BOTTOM= 3;
const int CUBEMAP_BACK	= 4;
const int CUBEMAP_FRONT	= 5;

const Vector3
	box_frame_o[6] 
= {	Vector3(1, 0, 0), Vector3(-1, 0, 0), 		
	Vector3(0, 1, 0), Vector3( 0,-1, 0),
	Vector3( 0, 0,-1), Vector3(0, 0, 1),}, 
	box_frame_x[6]
= {	Vector3(0, 0, 1), Vector3(0, 0, -1), 
	Vector3(-1, 0, 0), Vector3(1, 0, 0),
	Vector3(1, 0, 0), Vector3(-1, 0, 0),}, 
	box_frame_y[6]
= {	Vector3(0, 1, 0), Vector3(0, 1, 0), 
	Vector3(0, 0, 1), Vector3(0, 0,-1),
	Vector3(0, 1, 0), Vector3(0, 1, 0),};

class cubemap
{
private:
	int dim;
	std::vector<float> data;

public:
	void init(const int dim);
	float* get_face(const int idx);
	const float* get_face(const int idx) const;
	int get_dim() const;
	int	get_addr(const Vector3 &n) const;

	//DEBUG
	void save_obj(const char *filename, const tri_mesh &mesh) const;
};

class hemi_cubemap
{
private:
	int dim;
	std::vector<float> data;

public:
	void init(const int dim);
	float* get_face();
	const float* get_face() const;
	int get_dim() const;
	int	get_addr(const Vector3 &n) const;

	//DEBUG
	void save_obj(const char *filename, const tri_mesh &mesh) const;
};
