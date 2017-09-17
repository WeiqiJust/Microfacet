#include "utils.h"
#include "d3dcompiler.h"

void CatmullRom(Vector3 &q,
	const float t,
	const Vector3 &p0,
	const Vector3 &p1,
	const Vector3 &p2,
	const Vector3 &p3)
{
	//Q(s) = [(-s3 + 2s2 - s)p0 + (3s3 - 5s2 + 2)p1 + (-3s3 + 4s2 + s)p2 + (s3 - s2)p3] / 2
	float	t2 = t*t,
		t3 = t*t*t;

	q = ((-t3 + 2 * t2 - t)*p0 + (3 * t3 - 5 * t2 + 2)*p1 + (-3 * t3 + 4 * t2 + t)*p2 + (t3 - t2)*p3) / 2;
}

void projection_orthogonal(Matrix4 mat, float ax, float ay, float znear, float zfar)
{
	float m00 = 1.0f / ax;
	float m11 = 1.0f / ay;
	float m22 = -2.0f / (zfar - znear);
	float m23 = -(zfar + znear) / (zfar - znear);
	mat = Matrix4( m00, 0, 0, 0,
				   0, m11, 0, 0,
				   0, 0, m22, m23,
				   0, 0,  0,  1 );
}

void projection_perspective(Matrix4 mat, float ax, float ay, float znear, float zfar)
{
	float m00 = znear / ax;
	float m11 = znear / ay;
	float m22 = -(zfar + znear) / (zfar - znear);
	float m23 = -2.0f*zfar*znear / (zfar - znear);
	mat = Matrix4(m00, 0, 0, 0,
		0, m11, 0, 0,
		0, 0, m22, m23,
		0, 0, -1, 0);
}

void build_frame(Vector3 &t, Vector3 &b, const Vector3 &n)
{
	if (n.z != 1 && n.z != -1)
	{
		t.x = t.y = 0;
		t.z = 1;
	}
	else {
		t.x = 1;
		t.y = t.z = 0;
	}
	t = Cross(t, n);
	t = Normalize(t);
	b = Cross(t, n);
}

void matrix_lookat(Matrix4 &m, const Vector3 &eye, const Vector3 &lookat, const Vector3 &up)
{
	const Vector3	vat = Normalize(eye - lookat);
	const Vector3	vup = Normalize(up - (Dot(up, vat) * vat));
	const Vector3	vright = Cross(vup, vat);

	m = Matrix4(
		vright.x, vright.y, vright.z, Dot(-eye, vright),
		vup.x, vup.y, vup.z, Dot(-eye, vup),
		vat.x, vat.y, vat.z, Dot(-eye, vat),
		0, 0, 0, 1);
}

void uniform_disk_sampling(Vector2 & result, Vector2 rnd)
{
	result.x = sqrt(rnd.x)*cos(rnd.y);
	result.y = sqrt(rnd.x)*sin(rnd.y);
}

void uniform_hemisphere_sample(Vector3& result, float u1, float u2)
{
	const float r = sqrt(1.0f - u1 * u1);
	const float phi = 2 * PI * u2;
	result = Vector3(cos(phi) * r, sin(phi) * r, u1);
}

void uniform_sphere_sample(Vector3& result, float u1, float u2)
{
	float theta = 2 * PI*u1;
	float phi = acosf(2 * u2 - 1); 
	result = Vector3(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
}

float snorm2float(short a)
{
	a = (std::max)(a, (short)-0x7FFF);
	return a / float(0x7FFF);
}


step_1D_prob::step_1D_prob(std::vector<float> samples)
{
	float sum_of_elemens = std::accumulate(samples.begin(), samples.end(), 0.0f);
	for (int i = 0; i < samples.size(); i++)
	{
		data.push_back((float)samples[i] / sum_of_elemens);
	}
}

void step_1D_prob::sample(int& index, float rnd) const
{
	float minimum = (std::numeric_limits<float>::max)();
	index = 0;
	for (int i = 0; i < data.size(); i++)
	{
		if (abs(rnd - data[i]) <= minimum)
		{
			minimum = abs(rnd - data[i]);
			index = i;
		}
	}
}

random::random()
{
	srand(static_cast <unsigned> (100));
	std::mt19937_64 rng;
	// initialize the random number generator with time-dependent seed
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
	rng.seed(ss);
}

int random::get_random_int()
{
	return rand();
}

float random::get_random_float()
{

	// initialize a uniform distribution between 0 and 1
	std::uniform_real_distribution<float> unif(0, 1);
	// ready to generate random numbers
	return unif(rng);
}

double random::get_random_double()
{
	std::uniform_real_distribution<double> unif(0, 1);
	// ready to generate random numbers
	return unif(rng);
}