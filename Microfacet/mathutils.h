#pragma once

#include <stdio.h>
#include <math.h>
#include <iostream>
using namespace std;
#define PI 3.14159265358979323846f

// Vector2
class Vector2
{
public:
	Vector2() { x = y = 0.f; }

	Vector2(float f) { x = y = f; }

	Vector2(float xx, float yy) : x(xx), y(yy) {}

	Vector2(const Vector2 &v)
	{
		x = v.x; y = v.y;
	}

	Vector2 &operator=(const Vector2 &v)
	{
		x = v.x; y = v.y;
		return *this;
	}

	Vector2 operator+(const Vector2 &v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2& operator+=(const Vector2 &v)
	{
		x += v.x; y += v.y;
		return *this;
	}

	Vector2 operator-(const Vector2 &v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	Vector2& operator-=(const Vector2 &v)
	{
		x -= v.x; y -= v.y;
		return *this;
	}

	Vector2 operator*(float f) const { return Vector2(f*x, f*y); }

	Vector2 &operator*=(float f)
	{
		x *= f; y *= f;
		return *this;
	}

	Vector2 operator/(float f) const
	{
		float inv = 1.f / f;
		return Vector2(x * inv, y * inv);
	}

	Vector2 &operator/=(float f)
	{
		float inv = 1.f / f;
		x *= inv; y *= inv;
		return *this;
	}

	Vector2 operator-() const { return Vector2(-x, -y); }

	float operator[](int i) const
	{
		return (&x)[i];
	}

	float &operator[](int i)
	{
		return (&x)[i];
	}

	float LengthSquared() const { return x*x + y*y; }

	float Length() const { return sqrtf(LengthSquared()); }

	bool operator==(const Vector2 &v) const
	{
		return x == v.x && y == v.y;
	}

	bool operator!=(const Vector2 &v) const
	{
		return x != v.x || y != v.y;
	}

	ostream& print(ostream& out) const
	{
		out << x << " " << y << "\n";
		return out;
	}

	float x, y;
};

inline ostream& operator<<(ostream& out, const Vector2& v) { return v.print(out); }

inline Vector2 operator*(float f, const Vector2 &v) { return v*f; }

inline float Dot(const Vector2 &v1, const Vector2 &v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

inline float AbsDot(const Vector2 &v1, const Vector2 &v2) 
{
	return fabsf(Dot(v1, v2));
}

inline Vector2 Normalize(const Vector2 &v) { return v / v.Length(); }

// Vector3
class Vector3
{
public:
	Vector3() { x = y = z = 0.f; }

	Vector3(float f) { x = y = z = f; }

	Vector3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

	Vector3(const Vector3 &v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	Vector3 &operator=(const Vector3 &v)
	{
		x = v.x; y = v.y; z = v.z;
		return *this;
	}

	Vector3 operator+(const Vector3 &v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3& operator+=(const Vector3 &v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	Vector3 operator-(const Vector3 &v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3& operator-=(const Vector3 &v)
	{
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	Vector3 operator*(float f) const { return Vector3(f*x, f*y, f*z); }

	float operator*(Vector3 f) const 
	{ return x*f.x + y*f.y + z*f.z; 
	}

	Vector3 operator^(const Vector3 v)
	{
		return Vector3((y * v.z) - (z * v.y),
			(z * v.x) - (x * v.z),
			(x * v.y) - (y * v.x));
	}

	Vector3 &operator*=(float f)
	{
		x *= f; y *= f; z *= f;
		return *this;
	}

	Vector3 operator/(float f) const
	{
		float inv = 1.f / f;
		return Vector3(x * inv, y * inv, z * inv);
	}

	Vector3 &operator/=(float f)
	{
		float inv = 1.f / f;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}

	Vector3 operator-() const { return Vector3(-x, -y, -z); }

	float operator[](int i) const
	{
		return (&x)[i];
	}

	float &operator[](int i)
	{
		return (&x)[i];
	}

	float LengthSquared() const { return x*x + y*y + z*z; }

	float Length() const { return sqrtf(LengthSquared()); }

	bool operator==(const Vector3 &v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator!=(const Vector3 &v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	ostream& print(ostream& out) const
	{
		out << x << " " << y << " " << z << "\n";
		return out;
	}

	Vector3 normalize()
	{
		float len = Length();
		x = x / len;
		y = y / len;
		z = z / len;
		return *this;
	}

	float x, y, z;
};

inline ostream& operator<<(ostream& out, const Vector3& v) { return v.print(out); }

inline Vector3 operator*(float f, const Vector3 &v) { return v*f; }

inline float Dot(const Vector3 &v1, const Vector3 &v2) 
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}


inline float AbsDot(const Vector3 &v1, const Vector3 &v2) 
{
	return fabsf(Dot(v1, v2));
}

inline Vector3 Cross(const Vector3 &v1, const Vector3 &v2) 
{
	float v1x = v1.x, v1y = v1.y, v1z = v1.z;
	float v2x = v2.x, v2y = v2.y, v2z = v2.z;
	return Vector3((v1y * v2z) - (v1z * v2y),
		(v1z * v2x) - (v1x * v2z),
		(v1x * v2y) - (v1y * v2x));
}

inline Vector3 Normalize(const Vector3 &v) { return v / v.Length(); }

//Matix4 declaration

class Matrix4
{
public:
	Matrix4() { m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f;
		m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] = m[1][3] = m[2][0] = m[2][1] = m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.f; }

	Matrix4(float mat[4][4]);

	Matrix4(float t00, float t01, float t02, float t03,
		float t10, float t11, float t12, float t13,
		float t20, float t21, float t22, float t23,
		float t30, float t31, float t32, float t33);

	bool operator==(const Matrix4 &m2) const
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (m[i][j] != m2.m[i][j]) return false;
		return true;
	}

	bool operator!=(const Matrix4 &m2) const
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (m[i][j] != m2.m[i][j]) return true;
		return false;
	}

	Matrix4 operator*(const Matrix4 &m2) const
	{
		Matrix4 r;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				r.m[i][j] = m[i][0] * m2.m[0][j] +
				m[i][1] * m2.m[1][j] +
				m[i][2] * m2.m[2][j] +
				m[i][3] * m2.m[3][j];
		return r;
	}

	Vector3 operator*(const Vector3 &v) 
	{
		float x = v.x, y = v.y, z = v.z;
		return Vector3(m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3],
			m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3],
			m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3]);

	}

	Vector3 operator*(Vector3 v) const
	{
		float x = v.x, y = v.y, z = v.z;
		return Vector3(m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3],
			m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3],
			m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3]);

	}

	void Print();

	friend Matrix4 Transpose(const Matrix4 &);
	friend Matrix4 Inverse(const Matrix4 &);

	float m[4][4];
};

Matrix4 Identity();
Matrix4 Translate(const Vector3 &delta);
Matrix4 Scale(float x, float y, float z);
Matrix4 RotateX(float angle);
Matrix4 RotateY(float angle);
Matrix4 RotateZ(float angle);
Matrix4 Rotate(float angle, const Vector3 &axis);
Matrix4 LookAt(const Vector3 &eye, const Vector3 &lookAt, const Vector3 &up);

Matrix4 Orthographic(float znear, float zfar);
Matrix4 Perspective(float fov, float n, float f);

Matrix4 InitWithBasis(Vector3 u, Vector3 v, Vector3 w, Vector3 t);

inline float DegToRad(float angle)
{
	return ((float)PI / 180.f) * angle;
}

inline float RadToDeg(float angle)
{
	return ((float)180.f / PI) * angle;
}

inline Vector3 hsv2rgb(float H, float S, float V)
{
	float      hh, p, q, t, ff;
	long        i;
	Vector3 rgb;

	if (S <= 0.0f) // < is bogus, just shuts up warnings
	{
		rgb.x = V;
		rgb.y = V;
		rgb.z = V;
		return rgb;
	}
	hh = H;
	if (hh >= 360.0f) hh = 0.0f;
	hh /= 60.0f;
	i = (long)hh;
	ff = hh - i;
	p = V * (1.0f - S);
	q = V * (1.0f - (S * ff));
	t = V * (1.0f - (S * (1.0f - ff)));

	switch (i) {
	case 0:
		rgb.x = V;
		rgb.y = t;
		rgb.z = p;
		break;
	case 1:
		rgb.x = q;
		rgb.y = V;
		rgb.z = p;
		break;
	case 2:
		rgb.x = p;
		rgb.y = V;
		rgb.z = t;
		break;

	case 3:
		rgb.x = p;
		rgb.y = q;
		rgb.z = V;
		break;
	case 4:
		rgb.x = t;
		rgb.y = p;
		rgb.z = V;
		break;
	case 5:
	default:
		rgb.x = V;
		rgb.y = p;
		rgb.z = q;
		break;
	}
	return rgb;
}


// This is a subfunction of HSLtoRGB
inline float HSLtoRGB_Subfunction(float temp1, float temp2, float temp3)
{
	float result;
	if ((temp3 * 6) < 1)
		result = ((temp2 + (temp1 - temp2) * 6 * temp3) * 100);
	else
		if ((temp3 * 2) < 1)
			result = (temp1 * 100);
		else
			if ((temp3 * 3) < 2)
				result = ((temp2 + (temp1 - temp2)*(.66666f - temp3) * 6.0f) * 100.0f);
			else
				result = (temp2 * 100);
	return result;
}

inline Vector3 balanceError(Vector3& v)
{
	if (abs(v.x) <= 0.0001f)
		v.x = 0.0f;
	if (abs(v.y) <= 0.0001f)
		v.y = 0.0f;
	if (abs(v.z) <= 0.0001f)
		v.z = 0.0f;
	return v;
}

// input is 0-1 scale
inline Vector3 RGBtoHSL(Vector3 rgb)
{
	float r_percent = rgb.x;
	float g_percent = rgb.y;
	float b_percent = rgb.z;

	float max_color = 0;
	if ((r_percent >= g_percent) && (r_percent >= b_percent))
		max_color = r_percent;
	if ((g_percent >= r_percent) && (g_percent >= b_percent))
		max_color = g_percent;
	if ((b_percent >= r_percent) && (b_percent >= g_percent))
		max_color = b_percent;

	float min_color = 0;
	if ((r_percent <= g_percent) && (r_percent <= b_percent))
		min_color = r_percent;
	if ((g_percent <= r_percent) && (g_percent <= b_percent))
		min_color = g_percent;
	if ((b_percent <= r_percent) && (b_percent <= g_percent))
		min_color = b_percent;

	float L = 0;
	float S = 0;
	float H = 0;

	L = (max_color + min_color) / 2;

	if (max_color == min_color)
	{
		S = 0;
		H = 0;
	}
	else
	{
		if (L < .50)
		{
			S = (max_color - min_color) / (max_color + min_color);
		}
		else
		{
			S = (max_color - min_color) / (2 - max_color - min_color);
		}
		if (max_color == r_percent)
		{
			H = (g_percent - b_percent) / (max_color - min_color);
		}
		if (max_color == g_percent)
		{
			H = 2 + (b_percent - r_percent) / (max_color - min_color);
		}
		if (max_color == b_percent)
		{
			H = 4 + (r_percent - g_percent) / (max_color - min_color);
		}
	}

	H = H * 60;
	if (H < 0)
		H += 360;
	H = H / 360;
	Vector3 HSL;
	HSL.x = H;
	HSL.y = S;
	HSL.z = L;
	return balanceError(HSL);
}

inline Vector3 HSLtoRGB(Vector3 hsl)
{
	float r = 0;
	float g = 0;
	float b = 0;
	Vector3 color;
	float L = hsl.z;
	float S = hsl.y;
	float H = hsl.x;

	if (S == 0.0f)
	{
		color.x = L;
		color.y = L;
		color.z = L;
		return balanceError(color);
	}
	else
	{
		float temp1 = 0;
		if (L < .50)
		{
			temp1 = L*(1 + S);
		}
		else
		{
			temp1 = L + S - (L*S);
		}

		float temp2 = 2 * L - temp1;

		float temp3 = 0;
		for (int i = 0; i < 3; i++)
		{
			switch (i)
			{
			case 0: // red
			{
				temp3 = H + .33333f;
				if (temp3 > 1)
					temp3 -= 1;
				r = HSLtoRGB_Subfunction(temp1, temp2, temp3);
				break;
			}
			case 1: // green
			{
				temp3 = H;
				g = HSLtoRGB_Subfunction(temp1, temp2, temp3);
				break;
			}
			case 2: // blue
			{
				temp3 = H - .33333f;
				if (temp3 < 0)
					temp3 += 1;
				b = HSLtoRGB_Subfunction(temp1, temp2, temp3);
				break;
			}
			default:
			{

			}
			}
		}
	}

	color.x = r / 100;
	color.y = g / 100;
	color.z = b / 100;
	return balanceError(color);
}

