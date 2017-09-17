#include "utils.h"

struct trackball_param
{
	Vector3 eye, lookat;
};

class trackball
{
public:
	void init(const Vector3 eye, const Vector3 lookat)
	{
		param.eye = eye;
		param.lookat = lookat;
	}

	trackball_param get_params() { return param; };

	void set_params(const trackball_param p)
	{
		param.eye = p.eye;
		param.lookat = p.lookat;
	}

private:
	trackball_param param;

};