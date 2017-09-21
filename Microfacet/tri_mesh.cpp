#include "tri_mesh.h"
#include <fstream>
#include <iostream>

int main_axis_(const Vector3 &p)
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

tri_mesh::tri_mesh(const char * path)
{
	attr = 0;
	if (!load_obj(path))
		cout << "Error Loading Obj File: " << path << endl;
}

tri_mesh::tri_mesh(const std::vector<Vector3> vertex, const std::vector<triangle_face> face) : vertices(vertex), faces(face)
{
	attr = 0;
	calculate_face_normal();
	calculate_normal();
	mark_attribute();
}

tri_mesh::tri_mesh(const std::vector<Vector3> vertex, const std::vector<Vector3> normal, const std::vector<triangle_face> face)
	: vertices(vertex), normals(normal), faces(face)
{
	attr = 0;
	calculate_face_normal();
	mark_attribute();
}

tri_mesh::tri_mesh(const std::vector<Vector3> vertex,
	const std::vector<Vector3> normal, const std::vector<Vector2> uv,
	const std::vector<Vector3> tangent, const std::vector<triangle_face> face)
	: vertices(vertex), normals(normal), uvs(uv), tangents(tangent), faces(face)
{
	attr = 0;
	calculate_face_normal();
	mark_attribute();
}

void tri_mesh::merge_mesh(const std::vector<Vector3> vertex, const std::vector<triangle_face> face)
{
	normals.clear();
	uvs.clear();
	tangents.clear();
	int size = vertices.size();
	vertices.insert(vertices.end(), vertex.begin(), vertex.end());
	for (int i = 0; i < face.size(); i++)
	{
		triangle_face f = face[i];
		f.v1 += size;
		f.v2 += size;
		f.v3 += size;
		faces.push_back(f);
	}
}

void tri_mesh::clear()
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	tangents.clear();
	faces.clear();
	face_normals.clear();
	face_areas.clear();
	attr = 0;
}

bool tri_mesh::load_obj(const char * path)
{
	//printf("Loading OBJ file %s...\n", path);

	FILE * file = fopen(path, "r");
	if (file == NULL){
		cout << "Impossible to open the obj file in load tri_mesh! " << path << endl;;
		return false;
	}
	vector<int> vertex_index, normal_index, uv_index;
	vector<Vector3> vertex_tempt, normal_tempt;
	vector<Vector2> uv_tempt;

	while (1)
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0)
		{
			Vector3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertex_tempt.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			Vector2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			uv_tempt.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			Vector3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal_tempt.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			for (int i = 0; i < 3; i++)
			{ 
				vertex_index.push_back(vertexIndex[i]);
				normal_index.push_back(normalIndex[i]);
				uv_index.push_back(uvIndex[i]);
			}
			//triangle_face face(vertexIndex[0] - 1, vertexIndex[1] - 1, vertexIndex[2] - 1);
			//faces.push_back(face);
		}
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}
	for (int i = 0; i < vertex_index.size()/3; i++)
	{
		for (int j = 0; j < 3; j++)
		{ 
			vertices.push_back(vertex_tempt[vertex_index[3 * i + j] - 1]);
			if (normal_tempt.size() > normal_index[3 * i + j] - 1)
				normals.push_back(normal_tempt[normal_index[3 * i + j] - 1]);
			if (uv_tempt.size() > uv_index[3 * i + j] - 1)
				uvs.push_back(uv_tempt[uv_index[3 * i + j] - 1]);
		}
		triangle_face face(3*i, 3*i+1, 3*i+2);
		faces.push_back(face);
	}
	calculate_face_normal();
	if (normals.size() != vertices.size())
	{
		cout << "Warning: obj file vertex size is not same to normal size, recomputing normals " << path<<endl;
		calculate_normal();
	}
	if (uvs.size() != 0 && uvs.size() != vertices.size())
	{
		cout << "Warning: obj file vertex size is not same to uv size!" << path<<endl;
	}
	calculate_tangent();
	mark_attribute();
	return true;
}

void tri_mesh::calculate_face_area()
{
	face_areas.clear();
	for (int i = 0; i < faces.size(); i++)
	{
		triangle_face face_iter = faces[i];
		Vector3 edge1 = vertices[face_iter.v2] - vertices[face_iter.v1];
		Vector3 edge2 = vertices[face_iter.v3] - vertices[face_iter.v1];
		float area = abs(Cross(edge1, edge2).Length()) * 0.5f;
		face_areas.push_back(area);
	}
}

void tri_mesh::calculate_normal()
{
	normals.resize(vertices.size(), Vector3(0.0f));
	calculate_face_area();
	for (int i = 0; i < faces.size(); i++)
	{
		triangle_face face_iter = faces[i];
		//float area = face_areas[i];
		normals[face_iter.v1] = face_normals[i];
		normals[face_iter.v2] = face_normals[i];
		normals[face_iter.v3] = face_normals[i];
	}
	attr |= VERT_ATTR_NORMAL;
}

void tri_mesh::calculate_face_normal()
{
	for (int i = 0; i < faces.size(); i++)
	{
		triangle_face face_iter = faces[i];
		Vector3 edge1 = vertices[face_iter.v2] - vertices[face_iter.v1];
		Vector3 edge2 = vertices[face_iter.v3] - vertices[face_iter.v1];
		Vector3 face_normal = Normalize(Cross(edge1, edge2));
		face_normals.push_back(face_normal);
	}
}

void tri_mesh::calculate_tangent()
{
	if (uvs.size() != vertices.size())
	{ 
		//cout << "tri_mesh::calculate_tangent() Error: uv size does not match vertices size!" << endl;
		return;
	}
	/*
	tangents.resize(vertices.size(), Vector3(0.0f));
	std::vector<Vector3> bitangent = tangents;
	
	for (long a = 0; a < faces.size(); a++)
	{
		Vector3 v1 = vertices[faces[a].v1];
		Vector3 v2 = vertices[faces[a].v2];
		Vector3 v3 = vertices[faces[a].v3];

		Vector2 w1 = uvs[faces[a].v1];
		Vector2 w2 = uvs[faces[a].v2];
		Vector2 w3 = uvs[faces[a].v3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0f / (s1 * t2 - s2 * t1);
		if (s1 * t2 - s2 * t1 == 0.0f)
			cout << "Warning: Mesh tangent calculation r = INF!" << endl;
		Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tangents[faces[a].v1] += sdir;
		tangents[faces[a].v2] += sdir;
		tangents[faces[a].v3] += sdir;

		bitangent[faces[a].v1] += tdir;
		bitangent[faces[a].v2] += tdir;
		bitangent[faces[a].v3] += tdir;
	}

	for (long a = 0; a < tangents.size(); a++)
	{
		Vector3 n = normals[a];
		Vector3 t = tangents[a];

		// Gram-Schmidt orthogonalize
		tangents[a] = Normalize(t - n * Dot(n, t));
		tangents[a] *= (Dot(Cross(n, t), bitangent[a]) < 0.0F) ? -1.0F : 1.0F;
	}*/
	
	for (int i = 0; i < normals.size(); i++)
	{
		//Vector3 tangent, binormal;
		/*
		build_frame(tangent, binormal, normals[i]);
		tangent *= (Dot(Cross(normals[i], tangent), binormal) < 0.0F) ? -1.0F : 1.0F;*/

		Vector3 n = normals[i], t, b;

		t[(main_axis_(n) + 1) % 3] = 1;
		t = t ^ n;
		t.normalize();
		tangents.push_back(t);
	}

}

void tri_mesh::mark_attribute()
{
	if (get_vertex_number())
		attr |= VERT_ATTR_POSITION;
	if (get_normal_number())
		attr |= VERT_ATTR_NORMAL;
	if (get_uv_number())
		attr |= VERT_ATTR_UV1;
	if (get_tangent_number())
		attr |= VERT_ATTR_TANGENT;
}

void tri_mesh::save_obj(string filename)
{
	ofstream file;
	file.open(filename);
	for (int i = 0; i < vertices.size(); i++)
	{
		file << "v " << vertices[i];
	}
	for (int i = 0; i < normals.size(); i++)
	{
		file << "vn " << normals[i];
	}
	for (int i = 0; i < uvs.size(); i++)
	{
		file << "vt " << uvs[i];
	}
	for (int i = 0; i < faces.size(); i++)
	{
		file << "f " << faces[i].v1+1 << "/" << faces[i].v1+1 << "/" << faces[i].v1+1 << " "
			<< faces[i].v2 + 1 << "/" << faces[i].v2 + 1 << "/" << faces[i].v2 + 1 << " "
			<< faces[i].v3 + 1 << "/" << faces[i].v3 + 1 << "/" << faces[i].v3 + 1 << endl;
	}
	file.close();
}