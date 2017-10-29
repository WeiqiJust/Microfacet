#pragma once
#include "utils.h"
class BRDF_interface
{
protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const = 0;
public:
	virtual ~BRDF_interface() {};
	void sample(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;
};

class Lambert_interface : public BRDF_interface
{
protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;
};

class BlinnPhong_interface : public BRDF_interface
{
private:
	float ex;
protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;
public:
	BlinnPhong_interface(const float e);
};

class CT_interface : public BRDF_interface
{
private:
	float m, F0;
protected:
	friend class LambertAndCT;
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;
public:
	CT_interface(const float m, const float F0);
};

class Ward_interface : public BRDF_interface
{
private:
	float a;
	Vector3 albedo;
protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;
public:
	Ward_interface(const float a, Vector3 albedo_ = Vector3(1.0f, 1.0f, 1.0f));
	void setAlbedo(Vector3 albedo_) { albedo = albedo_; }
};

class OrenNayar_interface : public BRDF_interface
{
private:
	float alpha;
protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;
public:
	OrenNayar_interface(const float alpha);
};

class measured_isotropic_BRDF
{
private:
	double *data;//[MEASURED_BRDF_SAMPLING_THETA_H * MEASURED_BRDF_SAMPLING_THETA_D * MEASURED_BRDF_SAMPLING_PHI_D / 2][3];

	inline int theta_half_index(const double theta_half) const;
	inline int theta_diff_index(const double theta_diff) const;
	inline int phi_diff_index(const double phi_diff) const;

	std::vector<step_1D_prob>	prob;

public:
	measured_isotropic_BRDF();
	measured_isotropic_BRDF(const char *filename, bool b_need_CDF);
	virtual ~measured_isotropic_BRDF();

	void load_data(const char *filename, bool b_need_CDF);
	virtual void get_BRDF(Vector3 &result, const Vector3 &wi, const Vector3 &wo) const;
};


class measured_BRDF : public BRDF_interface
{
private:
	measured_isotropic_BRDF *brdf;

protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;

public:
	measured_BRDF();
	measured_BRDF(const char *filename);
	virtual ~measured_BRDF();

	void init(const char *filename);
	void release();
};

class LambertAndCT : public BRDF_interface
{
private:
	CT_interface	ct;
	float			rd, gd, bd, rs, gs, bs;

protected:
	virtual void sample_brdf(Vector3 &result, const Vector3 &w_i, const Vector3 &w_o, const Vector3 &n) const;

public:
	LambertAndCT(float rd, float gd, float bd, float rs, float gs, float bs, float m, float F0);
};

class BRDF_factory
{
public:
	static BRDF_interface* produce(const char *name, const char *param, Vector3 albedo = Vector3(1.0f, 1.0f, 1.0f));
};