#pragma once
#include "my_float_types.h"


class BRDF_interface
{
protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const = 0;
public:
	virtual ~BRDF_interface() {};
	void sample(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;
};

class Lambert_interface : public BRDF_interface
{
protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;
};

class BlinnPhong_interface : public BRDF_interface
{
private:
	float ex;
protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;
public:
	BlinnPhong_interface(const float e);
};

class CT_interface : public BRDF_interface
{
private:
	float m, F0;
protected:
	friend class LambertAndCT;
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;
public:
	CT_interface(const float m, const float F0);
};

class Ward_interface : public BRDF_interface
{
private:
	float a;
protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;
public:
	Ward_interface(const float a);
};

class OrenNayar_interface : public BRDF_interface
{
private:
	float alpha;
protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;
public:
	OrenNayar_interface(const float alpha);
};

/*
class measured_BRDF : public BRDF_interface
{
private:
	//zrt::measured_isotropic_BRDF *brdf;

protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;

public:
	measured_BRDF();
	measured_BRDF(const char *filename);
	virtual ~measured_BRDF();

	void init(const char *filename);
	void release();
};*/

class LambertAndCT : public BRDF_interface
{
private:
	CT_interface	ct;
	float			rd, gd, bd, rs, gs, bs;

protected:
	virtual void sample_brdf(vector3f &result, const vector3f &w_i, const vector3f &w_o, const vector3f &n) const;

public:
	LambertAndCT(float rd, float gd, float bd, float rs, float gs, float bs, float m, float F0);
};

class BRDF_factory
{
public:
	static BRDF_interface* produce(const char *name, const char *param);
};
