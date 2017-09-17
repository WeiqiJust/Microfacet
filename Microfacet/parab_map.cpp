#include "parab_map.h"
#include "hammersley.h"

int parab_map(const Vector3 &v, const int dim, const int actual_dim, const bool b_upper)
{
	Vector3 n;
	n = v;
	if (b_upper)
		n.z += 1;
	else
		n.z = -n.z+1;

	int offset = (actual_dim-dim)/2;
	int x = (n.x/n.z+1)/2*dim+offset,
		y = (n.y/n.z+1)/2*dim+offset;
	return max(min(x, actual_dim-1), 0) + max(min(y, actual_dim-1), 0)*actual_dim;
}

void parab_map(Vector2 &result, const Vector3 &v, 
						   const int dim, const int actual_dim, const bool b_upper)
{
	Vector3 n;
	n = v;
	if (b_upper)
		n.z += 1;
	else
		n.z = -n.z+1;

	int offset = (actual_dim-dim)/2;
	result.x = (n.x/n.z+1)/2*dim+offset;
	result.y = (n.y/n.z+1)/2*dim+offset;
}

void back_parab_map(Vector3 &n, const Vector2 &coord, 
								const int dim, const int actual_dim, const bool b_upper)
{
	int offset = (actual_dim-dim)/2;
	float	a = ((coord.x-offset)/dim*2-1),
			b = ((coord.y-offset)/dim*2-1);

	float temp = a*a+b*b;
	n.z = (1-temp) / (1+temp);
	n.x = a*(n.z+1);
	n.y = b*(n.z+1);
	if (!b_upper)
		n.z = -n.z;
}

//parab frame
void parab_frame::load(FILE *&fp)
{
	int len;

	fread(&dim, sizeof(int), 1, fp);
	fread(&b_upper, sizeof(bool), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	idx.resize(len);
	fread(&idx[0], sizeof(int)*idx.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	back_idx.resize(len);
	fread(&back_idx[0], sizeof(int)*back_idx.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	area.resize(len);
	fread(&area[0], sizeof(float)*area.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	spherical_area.resize(len);
	if (len > 0)
		fread(&spherical_area[0], sizeof(float)*spherical_area.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	n.resize(len);
	for (int i = 0; i < n.size(); i++)
	{
		fread(&len, sizeof(len), 1, fp);
		n[i].resize(len);
		for (int j = 0; j < len; j++)
		{
			fread(&n[i][j].x, sizeof(float), 1, fp);
			fread(&n[i][j].y, sizeof(float), 1, fp);
			fread(&n[i][j].z, sizeof(float), 1, fp);
		}
	}
}

void parab_frame::save(FILE *&fp) const
{
	int len;

	fwrite(&dim, sizeof(int), 1, fp);
	fwrite(&b_upper, sizeof(bool), 1, fp);

	len = idx.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&idx[0], sizeof(int)*idx.size(), 1, fp);

	len = back_idx.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&back_idx[0], sizeof(int)*back_idx.size(), 1, fp);

	len = area.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&area[0], sizeof(float)*area.size(), 1, fp);

	len = spherical_area.size();
	fwrite(&len, sizeof(len), 1, fp);
	if (len > 0)
		fwrite(&spherical_area[0], sizeof(float)*spherical_area.size(), 1, fp);

	len = n.size();
	fwrite(&len, sizeof(len), 1, fp);
	for (int i = 0; i < n.size(); i++)
	{
		len = n[i].size();
		fwrite(&len, sizeof(len), 1, fp);
		for (int j = 0; j < len; j++)
		{
			fwrite(&n[i][j].x, sizeof(float), 1, fp);
			fwrite(&n[i][j].y, sizeof(float), 1, fp);
			fwrite(&n[i][j].z, sizeof(float), 1, fp);
		}
	}
}

void parab_frame::compute_spherical_area(const int num_samples)
{
	////DEBUG
	//spherical_area.clear();
	//spherical_area.resize(n.size(), 1.0f);
	//return;

	std::vector<int> count;
	count.resize(n.size(), 0);

	random rng;

	Vector3 v;
	float pdf;
	for (int i = 0; i < num_samples; i++)
	{
		uniform_hemisphere_sample(v, rng.get_random_float(), rng.get_random_float());
		int ii = parab_map(v, dim, dim, b_upper);
		if (ii >= 0 && idx[ii] >= 0)
			count[idx[ii]]++;
		if (i%10000000 == 0)
			cout << " i =  " << i << " total = " << num_samples << endl;
	}

	spherical_area.resize(count.size());
	for (int i = 0; i < spherical_area.size(); i++)
		spherical_area[i] = (float)PI * 2 * count[i] / num_samples;
}

void parab_frame::init(const int d, const int samples_per_texel, const int area_samples_per_texel, const bool b_up)
{
	dim		= d;
	b_upper = b_up;

	// initiate the vectors with random values for later sampling
	hammersley seq(samples_per_texel, 2);
	std::vector<Vector2> samples;
	samples.resize(samples_per_texel);
	for (int i = 0; i < samples_per_texel; i++)
	{ 
		float* result = seq.get_sample();
		samples[i] = Vector2(result[0], result[1]);
	}

	hammersley seq_area(area_samples_per_texel, 2);
	std::vector<Vector2> samples_area;
	samples_area.resize(area_samples_per_texel);
	for (int i = 0; i < area_samples_per_texel; i++)
	{ 
		float* result = seq_area.get_sample();
		samples_area[i] = Vector2(result[0], result[1]);
	}

	n.clear();
	idx.clear();
	idx.resize(dim*dim, -1);
	back_idx.clear();

	// uniformly map each 2d grid (with randomness) back to 3d and save the mapping used for later lookup
	int ii = 0;
	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
		{
			Vector3 normal;
			std::vector<Vector3> temp_n;

			for (int i = 0; i < samples.size(); i++)
			{
				Vector2 s = samples[i];
				s.x += x;
				s.y += y;

				back_parab_map(normal, s, dim, dim, b_upper);
				if (normal.z > 0)
					temp_n.push_back(normal);
			}

			int num_hit = 0;
			for (int i = 0; i < samples_area.size(); i++)
			{
				Vector2 s = samples_area[i];
				s.x += x;
				s.y += y;

				back_parab_map(normal, s, dim, dim, b_upper);
				if (normal.z > 0)
					num_hit++;
			}

			if (temp_n.size() > 0)
			{
				idx[x+y*dim] = ii;
				back_idx.push_back(x+y*dim);
				n.push_back(temp_n);
				area.push_back((float)num_hit / area_samples_per_texel);
				ii++;
			}
		}
}

void parab_frame::normalize_n()
{
	for (int i = 0; i < n.size(); i++)
	{
		Vector3 sum;
		for (int j = 0; j < n[i].size(); j++)
			sum += n[i][j];
		sum = Normalize(sum);
		n[i].resize(1);
		n[i][0] = sum;
	}
}

int parab_frame::map(const Vector3 &n) const
{
	int i = parab_map(n, dim, dim, b_upper);

	if (i >= 0)
		return idx[i];
	else
		return -1;
}

void parab_frame::lerp(int &num, int *ridx, float *weight, const Vector3 &n,
					   const bool area_weighted, const bool b_normalize) const
{
	Vector2 xy;
	parab_map(xy, n, dim, dim, b_upper);

	int x[2], y[2];
	x[0] = max(int(xy.x - 0.5f), 0);
	y[0] = max(int(xy.y - 0.5f), 0);
	x[1] = x[0]+1;
	y[1] = y[0]+1;

	float wx[2], wy[2];
	wx[0] = x[0]+1-(xy.x-0.5f);
	wx[1] = 1-wx[0];
	wy[0] = y[0]+1-(xy.y-0.5f);
	wy[1] = 1-wy[0];

	float total_weight = 0.0f;
	num = 0;
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < 2; i++)
			if (x[i] < dim && y[j] < dim)
			{
				int ii = idx[x[i]+y[j]*dim];
				if (ii >= 0)
				{
					ridx[num]	= ii;
					weight[num] = wx[i]*wy[j];
					if (area_weighted) 
						weight[num] *= area[ii];
					if (b_normalize)
						total_weight += weight[num];
					num++;
				}
			}

	if (b_normalize)
		for (int i = 0; i < num; i++)
			weight[i] /= total_weight;
}

//double parab frame
int dbl_parab_neighbor_pixel[8*2] =
{
	-1, -1, -1, 0, -1, 1,
	0, -1, 0, 1,
	1, -1, 1, 0, 1, 1
};

void double_parab_frame::load(FILE *&fp)
{
	int len;

	fread(&dim, sizeof(int), 1, fp);
	fread(&actual_dim, sizeof(int), 1, fp);
	fread(&size_single_hemi, sizeof(int), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	idx.resize(len);
	fread(&idx[0], sizeof(int)*idx.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	back_idx.resize(len);
	fread(&back_idx[0], sizeof(int)*back_idx.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	spherical_area.resize(len);
	if (len > 0)
		fread(&spherical_area[0], sizeof(float)*spherical_area.size(), 1, fp);

	fread(&len, sizeof(len), 1, fp);
	n.resize(len);
	for (int i = 0; i < n.size(); i++)
	{
		fread(&len, sizeof(len), 1, fp);
		n[i].resize(len);
		for (int j = 0; j < len; j++)
		{
			fread(&n[i][j].x, sizeof(float), 1, fp);
			fread(&n[i][j].y, sizeof(float), 1, fp);
			fread(&n[i][j].z, sizeof(float), 1, fp);
		}
	}
}

void double_parab_frame::save(FILE *&fp) const
{
	int len;

	fwrite(&dim, sizeof(int), 1, fp);
	fwrite(&actual_dim, sizeof(int), 1, fp);
	fwrite(&size_single_hemi, sizeof(int), 1, fp);

	len = idx.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&idx[0], sizeof(int)*idx.size(), 1, fp);

	len = back_idx.size();
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(&back_idx[0], sizeof(int)*back_idx.size(), 1, fp);

	len = spherical_area.size();
	fwrite(&len, sizeof(len), 1, fp);
	if (len > 0)
		fwrite(&spherical_area[0], sizeof(float)*spherical_area.size(), 1, fp);

	len = n.size();
	fwrite(&len, sizeof(len), 1, fp);
	for (int i = 0; i < n.size(); i++)
	{
		len = n[i].size();
		fwrite(&len, sizeof(len), 1, fp);
		for (int j = 0; j < len; j++)
		{
			fwrite(&n[i][j].x, sizeof(float), 1, fp);
			fwrite(&n[i][j].y, sizeof(float), 1, fp);
			fwrite(&n[i][j].z, sizeof(float), 1, fp);
		}
	}
}

void double_parab_frame::compute_spherical_area(const int num_samples)
{
	////DEBUG
	//spherical_area.clear();
	//spherical_area.resize(n.size(), 1.0f);
	//return;

	std::vector<int> count;
	count.resize(n.size(), 0);
	random rng;

	Vector3 v;
	float pdf;
	for (int i = 0; i < num_samples; i++)
	{
		uniform_hemisphere_sample(v, rng.get_random_float(), rng.get_random_float());
		bool b_up = (v.z >= 0);
		int ii = parab_map(v, dim, actual_dim, b_up);
		if (!b_up)
			ii += actual_dim*actual_dim;
		if (ii >= 0)
			count[idx[ii]]++;
	}

	spherical_area.resize(count.size());
	for (int i = 0; i < spherical_area.size(); i++)
		spherical_area[i] = (float)PI * 4 * count[i] / num_samples;
}

void double_parab_frame::init(const int d, const int samples_per_texel)
{
	dim	= d;
	actual_dim = dim+2;

	hammersley seq(samples_per_texel, 2);
	std::vector<Vector2> samples;
	samples.resize(samples_per_texel);
	for (int i = 0; i < samples_per_texel; i++)
	{ 
		float* result = seq.get_sample();
		samples[i] = Vector2(result[0], result[1]);
	}

	n.clear();
	idx.clear();
	idx.resize(actual_dim*actual_dim*2, -1);
	back_idx.clear();

	int *pidx, ii;
	//upper hemisphere
	pidx = &idx[0];
	ii = 0;
	for (int y = 0; y < actual_dim; y++)
		for (int x = 0; x < actual_dim; x++)
		{
			Vector3 normal;
			std::vector<Vector3> temp_n;

			for (int i = 0; i < samples.size(); i++)
			{
				Vector2 s = samples[i];
				s.x += x;
				s.y += y;

				back_parab_map(normal, s, dim, actual_dim, true);
				if (normal.z >= 0)
					temp_n.push_back(normal);
			}

			if (temp_n.size() > 0)
			{
				pidx[x+y*actual_dim] = ii;
				back_idx.push_back(x+y*actual_dim);
				n.push_back(temp_n);
				ii++;
			}
		}
	size_single_hemi = n.size();

	//lower hemisphere
	pidx = &idx[actual_dim*actual_dim];
	for (int y = 0; y < actual_dim; y++)
		for (int x = 0; x < actual_dim; x++)
		{
			Vector3 normal;
			std::vector<Vector3> temp_n;

			for (int i = 0; i < samples.size(); i++)
			{
				Vector2 s = samples[i];
				s.x += x;
				s.y += y;

				back_parab_map(normal, s, dim, actual_dim, false);
				if (normal.z < 0)
					temp_n.push_back(normal);
			}

			if (temp_n.size() > 0)
			{
				pidx[x+y*actual_dim] = ii;
				back_idx.push_back(x+y*actual_dim);
				n.push_back(temp_n);
				ii++;
			}
		}
	
	std::vector<int> temp_idx;
	//inflate upper hemisphere
	temp_idx.clear();
	temp_idx.resize(actual_dim*actual_dim, -1);
	pidx = &idx[0];
	for (int y = 0; y < actual_dim; y++)
		for (int x = 0; x < actual_dim; x++)
			if (pidx[x+y*actual_dim] < 0)
			{
				bool b_compute = false;

				for (int i = 0; i < 8; i++)
				{
					int xx = x + dbl_parab_neighbor_pixel[i*2],
						yy = y + dbl_parab_neighbor_pixel[i*2+1];
					if (xx >= 0 && xx < actual_dim && 
						yy >= 0 && yy < actual_dim &&
						pidx[xx+yy*actual_dim] >= 0)
					{
						b_compute = true;
						break;
					}
				}

				if (b_compute)
				{
					int num = 0;
					Vector2 avg_s;
					Vector3 normal;
					for (int i = 0; i < samples.size(); i++)
					{
						Vector2 s = samples[i];
						s.x += x;
						s.y += y;

						back_parab_map(normal, s, dim, actual_dim, true);
						if (normal.z < 0)
						{
							avg_s += s;
							num++;
						}
					}
					avg_s /= num;

					back_parab_map(normal, avg_s, dim, actual_dim, true);
					temp_idx[x+y*actual_dim] = map(normal);
				}
			}
	for (int i = 0; i < temp_idx.size(); i++)
		if (temp_idx[i] != -1)
			pidx[i] = temp_idx[i];

	//inflate lower hemisphere
	temp_idx.clear();
	temp_idx.resize(actual_dim*actual_dim, -1);
	pidx = &idx[actual_dim*actual_dim];
	for (int y = 0; y < actual_dim; y++)
		for (int x = 0; x < actual_dim; x++)
			if (pidx[x+y*actual_dim] < 0)
			{
				bool b_compute = false;

				for (int i = 0; i < 8; i++)
				{
					int xx = x + dbl_parab_neighbor_pixel[i*2],
						yy = y + dbl_parab_neighbor_pixel[i*2+1];
					if (xx >= 0 && xx < actual_dim && 
						yy >= 0 && yy < actual_dim &&
						pidx[xx+yy*actual_dim] >= 0)
					{
						b_compute = true;
						break;
					}
				}

				if (b_compute)
				{
					int num = 0;
					Vector2 avg_s;
					Vector3 normal;
					for (int i = 0; i < samples.size(); i++)
					{
						Vector2 s = samples[i];
						s.x += x;
						s.y += y;

						back_parab_map(normal, s, dim, actual_dim, false);
						if (normal.z >= 0)
						{
							avg_s += s;
							num++;
						}
					}
					avg_s /= num;

					back_parab_map(normal, avg_s, dim, actual_dim, false);
					temp_idx[x+y*actual_dim] = map(normal);
				}
			}
	for (int i = 0; i < temp_idx.size(); i++)
		if (temp_idx[i] != -1)
			pidx[i] = temp_idx[i];
}

void double_parab_frame::normalize_n()
{
	for (int i = 0; i < n.size(); i++)
	{
		Vector3 sum;
		for (int j = 0; j < n[i].size(); j++)
			sum += n[i][j];
		sum = Normalize(sum);
		n[i].resize(1);
		n[i][0] = sum;
	}
}

void double_parab_frame::cluster_vec(const int num_clusters, 
									 const int max_iter,
									 const std::vector<Vector3> &input, 
									 std::vector<Vector3> &output)
{
	random rng;
	std::vector<Vector3> centers;

	//init centers by random sampling
	std::vector<int> v_init_idx;
	for (int i = 0; i < num_clusters; i++)
	{
		while (true)
		{
			int idx = int(rng.get_random_float()*input.size());
			bool b_found = false;
			for (int j = 0; j < v_init_idx.size(); j++)
				if (v_init_idx[j] == idx)
				{
					b_found = true;
					break;
				}
			if (!b_found)
			{
				v_init_idx.push_back(idx);
				break;
			}
		}

		centers.push_back(input[v_init_idx[v_init_idx.size()-1]]);
	}

	//clustering
	bool b_changed = true;
	int iter = 0;
	std::vector<int> cluster;
	std::vector<Vector3> new_centers;
	cluster.resize(input.size(), -1);
	
	while (b_changed && iter < max_iter)
	{
		b_changed = false;
		new_centers.clear();
		new_centers.resize(centers.size());
		
		for (int i = 0; i < input.size(); i++)
		{
			//assign to one cluster
			float dist = 1e30f;
			int id = -1;
			for (int j = 0; j < centers.size(); j++)
			{
				Vector3 delta = input[i]-centers[j];
				float len = delta.LengthSquared();
				if (len < dist)
				{
					dist = len;
					id = j;
				}
			}

			if (cluster[i] != id)
				b_changed = true;
			cluster[i] = id;
			new_centers[id] += input[i];
		}

		//compute new centers
		for (int i = 0; i < new_centers.size(); i++)
			new_centers[i] = Normalize(new_centers[i]);
		centers = new_centers;

		iter++;
	}

	output = new_centers;
}

void double_parab_frame::reduce_n(const int samples_per_texel)
{
	for (int i = 0; i < n.size(); i++)
		if (n[i].size() > samples_per_texel)
		{
			std::vector<Vector3> temp;
			cluster_vec(samples_per_texel, 100, n[i], temp);
			n[i] = temp;
		}
}


int double_parab_frame::map(const Vector3 &n) const
{
	if (n.z >= 0)
		return idx[parab_map(n, dim, actual_dim, true)];
	else
		return idx[actual_dim*actual_dim+parab_map(n, dim, actual_dim, false)];
}

void double_parab_frame::lerp(int &num, int *ridx, float *weight, const Vector3 &n,
							  const bool b_normalize) const
{
	bool b_upper = (n.z >= 0);
	Vector2 xy;
	parab_map(xy, n, dim, actual_dim, b_upper);

	int x[2], y[2];
	x[0] = int(xy.x - 0.5f);
	y[0] = int(xy.y - 0.5f);
	x[1] = x[0]+1;
	y[1] = y[0]+1;

	float wx[2], wy[2];
	wx[0] = x[0]+1-(xy.x-0.5f);
	wx[1] = 1-wx[0];
	wy[0] = y[0]+1-(xy.y-0.5f);
	wy[1] = 1-wy[0];

	float total_weight = 0.0f;
	int offset = b_upper ? 0 : size_single_hemi;
	num = 0;
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < 2; i++)
			if (x[i] < actual_dim && y[j] < actual_dim)
			{
				int ii = idx[offset+x[i]+y[j]*actual_dim];
				if (ii >= 0)
				{
					ridx[num]	= ii;
					weight[num] = wx[i]*wy[j];
					if (b_normalize)
						total_weight += weight[num];
					num++;
				}
			}

	if (b_normalize)
		for (int i = 0; i < num; i++)
			weight[i] /= total_weight;
}
