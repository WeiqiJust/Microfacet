#pragma once

#include <d3d11.h>
#include "tri_mesh.h"
#include "utils.h"

class tri_mesh_d3dx11 : public tri_mesh
{
private:
	ID3D11Device*	pd3dDevice;
	BYTE*			pvb;
	UINT*			pib;


public:
	tri_mesh_d3dx11(ID3D11Device* pd3dDevice);
	tri_mesh_d3dx11(ID3D11Device* pd3dDevice, const std::vector<Vector3> vertex, const std::vector<triangle_face> face);
	tri_mesh_d3dx11(ID3D11Device* pd3dDevice, const std::vector<Vector3> vertex, 
		const std::vector<Vector3> normal, const std::vector<Vector2> uv,
		const std::vector<Vector3> tangent, const std::vector<triangle_face> face);
	virtual ~tri_mesh_d3dx11();

	ID3D11Device* get_device();
	void get_layout(int &count, int &size_per_vert,
		D3D11_INPUT_ELEMENT_DESC *decls) const;
	void create_buffer_data(void* &vb_data, void* &ib_data, bool b_left_handed);
	void destroy_buffer_data();
	void set_vertex_attr(int attribute);
};
