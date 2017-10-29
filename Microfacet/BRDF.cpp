#include "BRDF.h"
#define RED_SCALE	(1.0/1500.0)
#define GREEN_SCALE (1.15/1500.0)
#define BLUE_SCALE	(1.66/1500.0)
const int MEASURED_BRDF_SAMPLING_THETA_H = 90;
const int MEASURED_BRDF_SAMPLING_THETA_D = 90;
const int MEASURED_BRDF_SAMPLING_PHI_D = 360;

//void build_frame(Vector3 &t, Vector3 &b, const Vector3 &n)
//{
//	if (n.y != 1 && n.y != -1)
//	{
//		b.x = b.z = 0; 
//		b.y = 1;
//	} else {
//		t.x = 1;
//		t.y = t.z = 0;
//	}
//	t = b ^ n;
//	t.normalize();
//	b = t ^ n;
//}

void BRDF_interface::sample(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	float	dot_i = Dot(w_i, n), 
			dot_o = Dot(w_o, n);
	if (dot_i <= 0 || dot_o <= 0)
	{
		result.x = result.y = result.z = 0;
		return;
	}
	sample_brdf(result, w_i, w_o, n);
	result *= dot_i*dot_o;
#ifdef LOG_MATERIAL
	result.x = log(result.x+1.0f);
	result.y = log(result.y+1.0f);
	result.z = log(result.z+1.0f);
#endif // LOG_MATERIAL
}

//Lambert
void Lambert_interface::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	result.x = result.y = result.z = 1;
}

//BlinnPhong
BlinnPhong_interface::BlinnPhong_interface(const float e)
:ex(e)
{
}

void BlinnPhong_interface::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	Vector3 vhalf = w_i + w_o;
	vhalf = Normalize(vhalf);

	float cos_s = Dot(vhalf, n);
	if (cos_s > 0)
	{
		result.x = result.y = result.z = exp(log(cos_s)*ex);
	}
	else
		result.x = result.y = result.z = 0;
}

//CookTorrance
CT_interface::CT_interface(const float m, const float F0)
:m(m), F0(F0)
{
}

void CT_interface::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	Vector3 vhalf = w_i + w_o;
	vhalf = Normalize(vhalf);

	float	dot_h_n		= min(max(-1.0f, Dot(vhalf, n)), 1.0f),
			alpha		= acos(dot_h_n),
			tan_alpha	= tan(alpha),
			cos_alpha	= cos(alpha),
			dot_l_n		= Dot(w_i, n),
			dot_v_n		= Dot(w_o, n),
			dot_h_v		= Dot(vhalf, w_o);

	if (dot_h_v != 0 && cos_alpha != 0)
	{
		float	temp= 1-dot_h_v,
				F	= F0 + (1-F0)*temp*temp*temp*temp*temp,
				D	= exp(-tan_alpha*tan_alpha/(m*m)) / (m*m*cos_alpha*cos_alpha*cos_alpha*cos_alpha),
				G	= min(min(1.0f, 2*dot_h_n*dot_v_n/dot_h_v), 2*dot_h_n*dot_l_n/dot_h_v);

		result.x = result.y = result.z = D*F*G / (dot_l_n*dot_v_n);
	} else {
		result.x = result.y = result.z = 0;
	}
}

//Ward
Ward_interface::Ward_interface(const float a, Vector3 albedo_)
	:a(a), albedo(albedo_)
{
}

void Ward_interface::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	Vector3 vhalf = w_i + w_o;
	vhalf = Normalize(vhalf);
	
	float		theta_h		= acos(min(max(-1.0f, Dot(vhalf, n)), 1.0f)),
			tan_theta_h		= tan(theta_h),
				cos_theta_in	= Dot(w_i, n),
				cos_theta_out	= Dot(w_o, n);

	float	r = exp(-tan_theta_h*tan_theta_h/(a*a)) / 
				(sqrt(cos_theta_in*cos_theta_out)*4*PI*a*a);
	
	result.x = result.y = result.z = r;

	//for testing
	result.x = result.x * albedo.x;
	result.y = result.y * albedo.y;
	result.z = result.z * albedo.z;
}

//OrenNayar
OrenNayar_interface::OrenNayar_interface(const float alpha)
:alpha(alpha)
{
}

void OrenNayar_interface::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	Vector3 t, b;
	build_frame(t, b, n);

	float	alpha2		= alpha*alpha,
			theta_in	= acos(Dot(w_i, n)),
			theta_out	= acos(Dot(w_o, n)),
			phi_in		= atan2(Dot(w_i, t), Dot(w_i, b)),
			phi_out		= atan2(Dot(w_o, t), Dot(w_o, b)),
			A, B;

	A = 1.0f - 0.5f*alpha2/(alpha2+0.33f);
	B = 0.45f*alpha2/(alpha2+0.09f);

	result.x = result.y = result.z = (A + B*max(0.0f, cos(phi_in-phi_out))*sin(max(theta_in, theta_out))*tan(min(theta_in, theta_out)));
}


measured_isotropic_BRDF::measured_isotropic_BRDF(): data(NULL)
{

}

measured_isotropic_BRDF::measured_isotropic_BRDF(const char *filename, bool b_need_CDF)
	: data(NULL)
{
	load_data(filename, b_need_CDF);
}

measured_isotropic_BRDF::~measured_isotropic_BRDF()
{
	SAFE_DELETE_ARR(data);
}


void cross_product(double* v1, double* v2, double* out)
{
	out[0] = v1[1] * v2[2] - v1[2] * v2[1];
	out[1] = v1[2] * v2[0] - v1[0] * v2[2];
	out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

// normalize vector
void normalize(double* v)
{
	// normalize
	double len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / len;
	v[1] = v[1] / len;
	v[2] = v[2] / len;
}

// rotate vector along one axis
void rotate_vector(double* vector, double* axis, double angle, double* out)
{
	double temp;
	double cross[3];
	double cos_ang = cos(angle);
	double sin_ang = sin(angle);

	out[0] = vector[0] * cos_ang;
	out[1] = vector[1] * cos_ang;
	out[2] = vector[2] * cos_ang;

	temp = axis[0] * vector[0] + axis[1] * vector[1] + axis[2] * vector[2];
	temp = temp*(1.0 - cos_ang);

	out[0] += axis[0] * temp;
	out[1] += axis[1] * temp;
	out[2] += axis[2] * temp;

	cross_product(axis, vector, cross);

	out[0] += cross[0] * sin_ang;
	out[1] += cross[1] * sin_ang;
	out[2] += cross[2] * sin_ang;
}

void measured_isotropic_BRDF::load_data(const char *filename, bool b_need_CDF)
{
	FILE *f = fopen(filename, "rb");
	if (!f)
		return;

	int dims[3];
	fread(dims, sizeof(int), 3, f);
	int n = dims[0] * dims[1] * dims[2];
	if (n != MEASURED_BRDF_SAMPLING_THETA_H *
		MEASURED_BRDF_SAMPLING_THETA_D *
		MEASURED_BRDF_SAMPLING_PHI_D / 2)
	{
		fprintf(stderr, "Dimensions don't match\n");
		fclose(f);
		return;
	}

	data = (double*)malloc(sizeof(double) * 3 * n);
	fread(data, sizeof(double), 3 * n, f);

	fclose(f);

}


inline int measured_isotropic_BRDF::theta_half_index(const double theta_half) const
{
	if (theta_half <= 0.0)
		return 0;
	double theta_half_deg = ((theta_half / (PI / 2.0))*MEASURED_BRDF_SAMPLING_THETA_H);
	double temp = theta_half_deg*MEASURED_BRDF_SAMPLING_THETA_H;
	temp = sqrt(temp);
	int ret_val = (int)temp;
	if (ret_val < 0) ret_val = 0;
	if (ret_val >= MEASURED_BRDF_SAMPLING_THETA_H)
		ret_val = MEASURED_BRDF_SAMPLING_THETA_H - 1;
	return ret_val;
}

inline int measured_isotropic_BRDF::theta_diff_index(const double theta_diff) const
{
	int tmp = int(theta_diff / (PI * 0.5) * MEASURED_BRDF_SAMPLING_THETA_D);
	if (tmp < 0)
		return 0;
	else if (tmp < MEASURED_BRDF_SAMPLING_THETA_D - 1)
		return tmp;
	else
		return MEASURED_BRDF_SAMPLING_THETA_D - 1;
}

inline int measured_isotropic_BRDF::phi_diff_index(const double phi_d) const
{
	double phi_diff = phi_d;
	if (phi_d < 0.0)
		phi_diff += PI;

	// In: phi_diff in [0 .. pi]
	// Out: tmp in [0 .. 179]
	int tmp = int(phi_diff / PI * MEASURED_BRDF_SAMPLING_PHI_D / 2);
	if (tmp < 0)
		return 0;
	else if (tmp < MEASURED_BRDF_SAMPLING_PHI_D / 2 - 1)
		return tmp;
	else
		return MEASURED_BRDF_SAMPLING_PHI_D / 2 - 1;
}

void measured_isotropic_BRDF::get_BRDF(Vector3 &result, const Vector3 &wi, const Vector3 &wo) const
{
	double half_x = (wi.x + wo.x) / 2.0f;
	double half_y = (wi.y + wo.y) / 2.0f;
	double half_z = (wi.z + wo.z) / 2.0f;
	double half[3] = { half_x, half_y, half_z };
	normalize(half);

	// compute  theta_half, fi_half
	double theta_half = acos(half[2]);
	double fi_half = atan2(half[1], half[0]);


	double bi_normal[3] = { 0.0, 1.0, 0.0 };
	double normal[3] = { 0.0, 0.0, 1.0 };
	double temp[3];
	double diff[3];

	// compute diff vector
	double in[3] = { wi.x, wi.y, wi.z };
	rotate_vector(in, normal, -fi_half, temp);
	rotate_vector(temp, bi_normal, -theta_half, diff);

	// compute  theta_diff, fi_diff	
	double theta_diff = acos(diff[2]);
	double fi_diff = atan2(diff[1], diff[0]);

	int ind = phi_diff_index(fi_diff) +
		theta_diff_index(theta_diff) * MEASURED_BRDF_SAMPLING_PHI_D / 2 +
		theta_half_index(theta_half) * MEASURED_BRDF_SAMPLING_PHI_D / 2 *
		MEASURED_BRDF_SAMPLING_THETA_D;

	result.x = data[ind] * RED_SCALE;
	result.y = data[ind + MEASURED_BRDF_SAMPLING_THETA_H*MEASURED_BRDF_SAMPLING_THETA_D*MEASURED_BRDF_SAMPLING_PHI_D / 2] * GREEN_SCALE;
	result.z = data[ind + MEASURED_BRDF_SAMPLING_THETA_H*MEASURED_BRDF_SAMPLING_THETA_D*MEASURED_BRDF_SAMPLING_PHI_D] * BLUE_SCALE;

}



//measured_BRDF
measured_BRDF::measured_BRDF()
:brdf(NULL)
{
	brdf = new measured_isotropic_BRDF;
}

measured_BRDF::measured_BRDF(const char *filename)
:brdf(NULL)
{
	brdf = new measured_isotropic_BRDF;
	init(filename);
}

measured_BRDF::~measured_BRDF()
{
	release();
}

void measured_BRDF::init(const char *filename)
{
	brdf->load_data(filename, false);
}

void measured_BRDF::release()
{
	SAFE_DELETE(brdf)
}

void measured_BRDF::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	Vector3 t, b;
	build_frame(t, b, n);

	brdf->get_BRDF(result, Vector3(Dot(w_i, t), Dot(w_i, b), Dot(w_i, n)), Vector3(Dot(w_o, t), Dot(w_o, b), Dot(w_o, n)));

}


//LambertAndCT
void LambertAndCT::sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const
{
	Vector3 specular;
	ct.sample_brdf(specular, w_i, w_o, n);

	result.x = rd + specular.x*rs;
	result.y = gd + specular.y*gs;
	result.z = bd + specular.z*bs;
}

LambertAndCT::LambertAndCT(float rd, float gd, float bd, float rs, float gs, float bs, float m, float F0)
: ct(m, F0), rd(rd), gd(gd), bd(bd), rs(rs), gs(gs), bs(bs)
{
	printf_s("%g %g %g %g %g %g %g %g\n", rd, gd, bd, rs, gs, bs, m, F0);
}

//BRDF factory
BRDF_interface* BRDF_factory::produce(const char *name, const char *param, Vector3 albedo)
{
	BRDF_interface *p = NULL;
	if (strcmp(name, "Lambertian") == 0)
	{
		p = new Lambert_interface();
	} else if (strcmp(name, "BlinnPhong") == 0)
	{
		float e;
		sscanf_s(param, "%f", &e);
		p = new BlinnPhong_interface(e);
	} else if (strcmp(name, "CookTorrance") == 0)
	{
		float m, F0;
		sscanf_s(param, "%f %f", &m, &F0);
		p = new CT_interface(m, F0);
	} else if (strcmp(name, "OrenNayar") == 0)
	{
		float a;
		sscanf_s(param, "%f", &a);
		p = new OrenNayar_interface(a);
	} else if (strcmp(name, "Ward") == 0)
	{
		float a;
		sscanf_s(param, "%f", &a);
		p = new Ward_interface(a, albedo);
	} else if (strcmp(name, "MERL") == 0)
	{
		p = new measured_BRDF(param);
	} else if (strcmp(name, "LambertAndCT") == 0)
	{
		float alpha, rd, gd, bd, rs, gs, bs, m, F0;
		sscanf_s(param, "%f %f %f %f %f %f %f %f %f", 
			&alpha, &rd, &gd, &bd, &rs, &gs, &bs, &m, &F0);
		rd *= alpha;
		gd *= alpha;
		bd *= alpha;
		rs *= alpha;
		gs *= alpha;
		bs *= alpha;
		p = new LambertAndCT(rd, gd, bd, rs, gs, bs, m, F0);
	}	

	return p;
}
