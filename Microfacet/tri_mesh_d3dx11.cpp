//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   10-20-2010
//  File:         codex_graphics_d3dx_tri_mesh10.h
//  Content:      mesh-related classes / functions definitions
//                for DirectX 11
//
//////////////////////////////////////////////////////////////////////////////

#include "tri_mesh_d3dx11.h"

static const char semantic_str[5][32] = 
{	
	"POSITION",
	"NORMAL",
	"TANGENT",
	"BINORMAL",
	"TEXCOORD"
};

const int	SEM_POS = 0;
const int	SEM_NORMAL = 1;
const int	SEM_TANGENT = 2;
const int	SEM_BINORMAL = 3;
const int	SEM_TEX = 4;

tri_mesh_d3dx11::tri_mesh_d3dx11(ID3D11Device* pd3dDevice)
	: pd3dDevice(pd3dDevice), pib(NULL), pvb(NULL)
{

}

tri_mesh_d3dx11::tri_mesh_d3dx11(ID3D11Device* pd3dDevice, const std::vector<Vector3> vertex, const std::vector<triangle_face> face)
	: pd3dDevice(pd3dDevice), tri_mesh(vertex, face)
{
	mark_attribute();
}

tri_mesh_d3dx11::tri_mesh_d3dx11(ID3D11Device* pd3dDevice, const std::vector<Vector3> vertex,
	const std::vector<Vector3> normal, const std::vector<Vector2> uv,
	const std::vector<Vector3> tangent, const std::vector<triangle_face> face)
	: pd3dDevice(pd3dDevice), tri_mesh(vertex, normal, uv, tangent, face)
{
}

tri_mesh_d3dx11::~tri_mesh_d3dx11()
{
	destroy_buffer_data();
}

ID3D11Device* tri_mesh_d3dx11::get_device()
{
	return pd3dDevice;
}

void tri_mesh_d3dx11::get_layout(int &count, int &size_per_vert,
												  D3D11_INPUT_ELEMENT_DESC *decls) const
{
	D3D11_INPUT_ELEMENT_DESC entry = 
	{"", 0, DXGI_FORMAT_UNKNOWN, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	int	i = 0, offset = 0;

	if (attr & VERT_ATTR_POSITION)
	{
		decls[i]					= entry;
		decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_POS];
		decls[i].Format				= DXGI_FORMAT_R32G32B32_FLOAT;
		decls[i].AlignedByteOffset	= offset;

		i++;
		offset += 12;
	}

	if (attr & VERT_ATTR_NORMAL)
	{
		decls[i]					= entry;
		decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_NORMAL];
		decls[i].Format				= DXGI_FORMAT_R32G32B32_FLOAT;
		decls[i].AlignedByteOffset	= offset;

		i++;
		offset += 12;
	}

	
	if (attr & VERT_ATTR_TANGENT)
	{
		decls[i]					= entry;
		decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_TANGENT];
		decls[i].Format				= DXGI_FORMAT_R32G32B32_FLOAT;
		decls[i].AlignedByteOffset	= offset;

		i++;
		offset += 12;
	}
	/*
	if (attr &  VERT_ATTR_BINORMAL)
	{
		decls[i]					= entry;
		decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_BINORMAL];
		decls[i].Format				= DXGI_FORMAT_R32G32B32_FLOAT;
		decls[i].AlignedByteOffset	= offset;

		i++;
		offset += 12;
	}*/

	int uvidx = 0;
	// Weiqi:assume we only have 1 uv for each vertex for now
	if (attr & VERT_ATTR_UV1)
	{
		//for (int j = 0; j<num_uv(); j++) // each vertex may have multiple uvs, for now only one
		//{
			decls[i]					= entry;
			decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_TEX];
			decls[i].Format				= DXGI_FORMAT_R32G32_FLOAT;
			decls[i].AlignedByteOffset	= offset;
			decls[i].SemanticIndex		= uvidx;
			uvidx++;

			i++;
			offset += 8;
		//}
	}

	/*
	if (num_uvw())
	{
		for (int j = 0; j<num_uvw(); j++)
		{
			decls[i]					= entry;
			decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_TEX];
			decls[i].Format				= DXGI_FORMAT_R32G32B32_FLOAT;
			decls[i].AlignedByteOffset	= offset;
			decls[i].SemanticIndex		= uvidx;
			uvidx++;

			i++;
			offset += 12;
		}
	}

	if (num_uvwx())
	{
		for (int j = 0; j<num_uvwx(); j++)
		{
			decls[i]					= entry;
			decls[i].SemanticName		= (LPCSTR)semantic_str[SEM_TEX];
			decls[i].Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
			decls[i].AlignedByteOffset	= offset;
			decls[i].SemanticIndex		= uvidx;
			uvidx++;

			i++;
			offset += 16;
		}
	}*/

	count			= i;
	size_per_vert	= offset;
}

void tri_mesh_d3dx11::destroy_buffer_data()
{
	SAFE_DELETE(pvb);
	SAFE_DELETE(pib);
}

void tri_mesh_d3dx11::create_buffer_data(void* &vb_data, void* &ib_data, bool b_left_handed)
{
	D3D11_INPUT_ELEMENT_DESC	decls[64];
	int							decls_count, size_per_vert;
	get_layout(decls_count, size_per_vert, decls);

	//// Copy the attribute data
	//UINT* pSubset = new UINT[num_faces()];
	//for (Index i = 0; i<num_faces(); i++)
	//	pSubset[i] = face2group(i);
	//pd3dxmesh->SetAttributeData(pSubset);
	//SAFE_DELETE_ARR(pSubset);

	destroy_buffer_data();

	// Copy the index data
	pib = new UINT[get_face_number()*3];
	ib_data = pib;
	for (int i = 0; i < get_face_number(); i++)
	{
		const triangle_face &f = faces[i];
		// the order is inversed?
		pib[i*3]	= f.v3;
		pib[i*3+1]	= f.v2;
		pib[i*3+2]	= f.v1;
	}

	// Copy the vertex data
	pvb = new BYTE[get_vertex_number()*size_per_vert];
	vb_data = pvb;
	BYTE* pVertex = pvb;

	for (int i = 0; i < get_vertex_number(); i++)
	{
		if (attr & VERT_ATTR_POSITION)
		{
			const Vector3 &v = vertices[i];
			float vv[3] = { v.x, v.y, -v.z };

			if (!b_left_handed)
				vv[2] = v.z;

			memcpy(pVertex, &vv, 12);
			pVertex += 12;
		}
		if (attr & VERT_ATTR_NORMAL)
		{
			const Vector3 &v = normals[i];
			float vv[3] = { v.x, v.y, -v.z };

			if (!b_left_handed)
				vv[2] = v.z;

			memcpy(pVertex, &vv, 12);
			pVertex += 12;
		}

		if (attr & VERT_ATTR_TANGENT)
		{
			const Vector3 &v = tangents[i];
			float vv[3] = { v.x, v.y, -v.z };

			if (!b_left_handed)
				vv[2] = (float)v.z;

			memcpy(pVertex, &vv, 12);
			pVertex += 12;
		}
		/*
		if (attr & VERT_ATTR_BINORMAL)
		{
			const Vector3 &v = binormal(i);
			codex::math::vector::Vector3<float> vv((float)v.x, (float)v.y, -(float)v.z);

			if (!b_left_handed)
				vv.z = (float)v.z;

			memcpy(pVertex, &vv.v, 12);
			pVertex += 12;
		}*/

		if (attr & VERT_ATTR_UV1)
		{
			//for (int j = 0; j < num_uv(); j++)
			{
				const Vector2 &v = uvs[i];
				float vv[2] = {v.x, v.y};

				memcpy(pVertex, &vv, 8);
				pVertex += 8;
			}
		}

		/*
		if (num_uvw())
		{
			for (int j = 0; j<num_uvw(); j++)
			{
				const Vector3 &v = uvw(j, i);
				codex::math::vector::Vector3<float> vv((float)v.x, (float)v.y, (float)v.z);

				memcpy(pVertex, &vv.v, 12);
				pVertex += 12;
			}
		}

		if (num_uvwx())
		{
			for (int j = 0; j<num_uvwx(); j++)
			{
				const vector4 &v = uvwx(j, i);
				codex::math::vector::vector4<float> vv((float)v.x, (float)v.y, (float)v.z, (float)v.w);

				memcpy(pVertex, &vv.v, 16);
				pVertex += 16;
			}
		}
		*/
	}
}

void tri_mesh_d3dx11::set_vertex_attr(int attribute)
{
	attr = attribute;
}
