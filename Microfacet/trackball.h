#include "utils.h"

struct trackball_param
{
	Vector3 eye, lookat, up;
};

class trackball
{
public:
	void init(const Vector3 eye, const Vector3 lookat, const Vector3 up)
	{
		param.eye = eye;
		param.lookat = lookat;
		param.up = up;
	}

	trackball_param get_params() { return param; };

	void set_params(const trackball_param p)
	{
		param.eye = p.eye;
		param.lookat = p.lookat;
		param.up = p.up;
	}

private:
	trackball_param param;

};