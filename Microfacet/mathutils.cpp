#include "mathutils.h"

#include <string>
using namespace std;

Matrix4::Matrix4(float mat[4][4]) 
{
    memcpy(m, mat, 16*sizeof(float));
}


Matrix4::Matrix4(float t00, float t01, float t02, float t03,
                     float t10, float t11, float t12, float t13,
                     float t20, float t21, float t22, float t23,
                     float t30, float t31, float t32, float t33)
{
    m[0][0] = t00; m[0][1] = t01; m[0][2] = t02; m[0][3] = t03;
    m[1][0] = t10; m[1][1] = t11; m[1][2] = t12; m[1][3] = t13;
    m[2][0] = t20; m[2][1] = t21; m[2][2] = t22; m[2][3] = t23;
    m[3][0] = t30; m[3][1] = t31; m[3][2] = t32; m[3][3] = t33;
}

void Matrix4::Print()
{
	for(int x=0; x<4; x++){
		for(int y=0; y<4; y++){
			printf("%.2f ",m[x][y]);
		}
		printf("\n");
	}
	printf("\n");
}

Matrix4 Transpose(const Matrix4 &m)
{
   return Matrix4(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
                    m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
                    m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
                    m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
}

Matrix4 Inverse(const Matrix4 &m)
{
    int indxc[4], indxr[4];
    int ipiv[4] = { 0, 0, 0, 0 };
    float minv[4][4];
    memcpy(minv, m.m, 4*4*sizeof(float));
    for (int i = 0; i < 4; i++) 
	{
        int irow = -1, icol = -1;
        float big = 0.;
        // Choose pivot
        for (int j = 0; j < 4; j++)
		{
            if (ipiv[j] != 1) {
                for (int k = 0; k < 4; k++)
				{
                    if (ipiv[k] == 0) {
                        if (fabsf(minv[j][k]) >= big)
						{
                            big = float(fabsf(minv[j][k]));
                            irow = j;
                            icol = k;
                        }
                    }
                    else if (ipiv[k] > 1)
					{
                        fprintf(stdout, "Singular matrix in MatrixInvert\n");
						return Matrix4();
					}
                }
            }
        }
        ++ipiv[icol];
        // Swap rows _irow_ and _icol_ for pivot
        if (irow != icol)
		{
            for (int k = 0; k < 4; ++k)
                swap(minv[irow][k], minv[icol][k]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        if (minv[icol][icol] == 0.)
		{
            fprintf(stdout, "Singular matrix in MatrixInvert\n");
			return Matrix4();
		}

        // Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
        float pivinv = 1.f / minv[icol][icol];
        minv[icol][icol] = 1.f;
        for (int j = 0; j < 4; j++)
            minv[icol][j] *= pivinv;

        // Subtract this row from others to zero out their columns
        for (int j = 0; j < 4; j++)
		{
            if (j != icol)
			{
                float save = minv[j][icol];
                minv[j][icol] = 0;
                for (int k = 0; k < 4; k++)
                    minv[j][k] -= minv[icol][k]*save;
            }
        }
    }
    // Swap columns to reflect permutation
    for (int j = 3; j >= 0; j--)
	{
        if (indxr[j] != indxc[j])
		{
            for (int k = 0; k < 4; k++)
                swap(minv[k][indxr[j]], minv[k][indxc[j]]);
        }
    }
    return Matrix4(minv);
}

Matrix4 Identity()
{
	return Matrix4( 1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
}

Matrix4 Translate(const Vector3 &delta)
{
    return Matrix4(1, 0, 0, delta.x,
				   0, 1, 0, delta.y,
				   0, 0, 1, delta.z,
				   0, 0, 0,       1);
}


Matrix4 Scale(float x, float y, float z)
{
    return Matrix4(x, 0, 0, 0,
				   0, y, 0, 0,
				   0, 0, z, 0,
				   0, 0, 0, 1);
}


Matrix4 RotateX(float angle)
{
	float sin_t = sinf(DegToRad(angle));
    float cos_t = cosf(DegToRad(angle));
    return Matrix4(1,     0,      0, 0,
				   0, cos_t, -sin_t, 0,
				   0, sin_t,  cos_t, 0,
				   0,     0,      0, 1);
}


Matrix4 RotateY(float angle)
{
    float sin_t = sinf(DegToRad(angle));
    float cos_t = cosf(DegToRad(angle));
    return Matrix4( cos_t,   0,  sin_t, 0,
                    0,   1,      0, 0,
                   -sin_t,   0,  cos_t, 0,
                    0,   0,   0,    1);
}



Matrix4 RotateZ(float angle)
{
    float sin_t = sinf(DegToRad(angle));
    float cos_t = cosf(DegToRad(angle));
    return Matrix4(cos_t, -sin_t, 0, 0,
                   sin_t,  cos_t, 0, 0,
                   0,      0, 1, 0,
                   0,      0, 0, 1);
}


Matrix4 Rotate(float angle, const Vector3 &axis)
{
    Vector3 a = Normalize(axis);
    float s = sinf(DegToRad(angle));
    float c = cosf(DegToRad(angle));
    float m[4][4];

    m[0][0] = a.x * a.x + (1.f - a.x * a.x) * c;
    m[0][1] = a.x * a.y * (1.f - c) - a.z * s;
    m[0][2] = a.x * a.z * (1.f - c) + a.y * s;
    m[0][3] = 0;

    m[1][0] = a.x * a.y * (1.f - c) + a.z * s;
    m[1][1] = a.y * a.y + (1.f - a.y * a.y) * c;
    m[1][2] = a.y * a.z * (1.f - c) - a.x * s;
    m[1][3] = 0;

    m[2][0] = a.x * a.z * (1.f - c) - a.y * s;
    m[2][1] = a.y * a.z * (1.f - c) + a.x * s;
    m[2][2] = a.z * a.z + (1.f - a.z * a.z) * c;
    m[2][3] = 0;

    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;

    return Matrix4(m);
}


Matrix4 LookAt(const Vector3 &eye, const Vector3 &lookAt, const Vector3 &up)
{
    float m[4][4];
    // Initialize fourth column of viewing matrix
    m[0][3] = eye.x;
    m[1][3] = eye.y;
    m[2][3] = eye.z;
    m[3][3] = 1;

    // Initialize first three columns of viewing matrix
    Vector3 dir = Normalize(lookAt - eye);
    Vector3 left = Normalize(Cross(Normalize(up), dir));
    Vector3 newUp = Cross(dir, left);
    m[0][0] = left.x;
    m[1][0] = left.y;
    m[2][0] = left.z;
    m[3][0] = 0.;
    m[0][1] = newUp.x;
    m[1][1] = newUp.y;
    m[2][1] = newUp.z;
    m[3][1] = 0.;
    m[0][2] = dir.x;
    m[1][2] = dir.y;
    m[2][2] = dir.z;
    m[3][2] = 0.;
    Matrix4 camToWorld(m);
	return Inverse(camToWorld);
}


Matrix4 Orthographic(float znear, float zfar)
{
    return Scale(1.f, 1.f, 1.f / (zfar-znear)) *
           Translate(Vector3(0.f, 0.f, -znear));
}


Matrix4 Perspective(float fov, float n, float f)
{
    // Perform projective divide
    Matrix4 persp = Matrix4(1, 0,           0,              0,
                                0, 1,           0,              0,
                                0, 0, f / (f - n), -f*n / (f - n),
                                0, 0,           1,              0);

    // Scale to canonical viewing volume
	float invTanAng = 1.f / tanf(DegToRad(fov) / 2.f);
    return Scale(invTanAng, invTanAng, 1) * persp;
}

Matrix4 InitWithBasis(Vector3 u, Vector3 v, Vector3 w, Vector3 t)
{
   float m[4][4];
    m[0][0] = u.x;
    m[0][1] = v.x;
    m[0][2] = w.x;
    m[0][3] = t.x;
    
    m[1][0] = u.y;
    m[1][1] = v.y;
    m[1][2] = w.y;
    m[1][3] = t.y;

    m[2][0] = u.z;
    m[2][1] = v.z;
    m[2][2] = w.z;
    m[2][3] = t.z;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;

    return Matrix4( m );
}