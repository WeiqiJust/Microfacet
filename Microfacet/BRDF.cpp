#include "BRDF.h"

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
Ward_interface::Ward_interface(const float a)
:a(a)
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

/*
//measured_BRDF
measured_BRDF::measured_BRDF()
:brdf(NULL)
{
	brdf = new zrt::measured_isotropic_BRDF;
}

measured_BRDF::measured_BRDF(const char *filename)
:brdf(NULL)
{
	brdf = new zrt::measured_isotropic_BRDF;
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
	my_build_frame(t, b, n);

	spectrum c;
	brdf->get_BRDF(c, vector3(Dot(w_i, t), Dot(w_i, b), Dot(w_i, n)), vector3(Dot(w_o, t), Dot(w_o, b), Dot(w_o, n)));
	result.x = (float)c(1);
	result.y = (float)c(2);
	result.z = (float)c(3);
}
*/

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
BRDF_interface* BRDF_factory::produce(const char *name, const char *param)
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
		p = new Ward_interface(a);
	/*} else if (strcmp(name, "MERL") == 0)
	{
		p = new measured_BRDF(param);*/
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
