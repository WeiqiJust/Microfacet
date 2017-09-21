//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   02-20-2007
//  File:         codex_math_matrix_utils.h
//  Content:      matrix utility functions definition
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
          OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma once

namespace codex
{
namespace math
{
	namespace matrix
	{
		using codex::math::vector::vector3;

		//Get an identity matrix
		template <class matrix>
		inline void identity(matrix& m)
		{
			memset(m.n, 0, sizeof(m.n));
			m._11 = m._22 = m._33 = m._44 = 1;
		}

		//Get a translation matrix
		template <class matrix, class Number>
		inline void translation(matrix& m, 
			const Number x, const Number y, const Number z)
		{
			identity(m);

			m._14 = x;
			m._24 = y;
			m._34 = z;
		}

		//Get a scaling matrix
		template <class matrix, class Number>
		inline void scaling(matrix& m, 
			const Number sx, const Number sy, const Number sz)
		{
			memset(m.n, 0, sizeof(m.n));
			m._44	= 1;

			m._11 = sx;
			m._22 = sy;
			m._33 = sz;
		}

		//Get an rotation matrix along x-axis
		template <class matrix, class Number>
		inline void rotation_x(matrix& m, const Number cos_theta, const Number sin_theta)
		{
			memset(m.n, 0, sizeof(m.n));
			m._44	= 1;
			m._11	= 1;
			m._22	= cos_theta;
			m._32	= sin_theta;
			m._23	= -sin_theta;
			m._33	= cos_theta;
		}

		template <class matrix, class Number>
		inline void rotation_x(matrix& m, const Number t)
		{
		#if defined(CODEX_MATH_RIGHTHAND)
			Number theta = t;
		#else
			Number theta = -t;
		#endif
			Number cos_theta = cos(theta), sin_theta = sin(theta);
			rotation_x(m, cos_theta, sin_theta);
		}

		//Get an rotation matrix along y-axis
		template <class matrix, class Number>
		inline void rotation_y(matrix& m, const Number cos_theta, const Number sin_theta)
		{
			memset(m.n, 0, sizeof(m.n));
			m._44	= 1;
			m._22	= 1;
			m._11	= cos_theta;
			m._31	= -sin_theta;
			m._13	= sin_theta;
			m._33	= cos_theta;
		}

		template <class matrix, class Number>
		inline void rotation_y(matrix& m, const Number t)
		{
		#if defined(CODEX_MATH_RIGHTHAND)
			Number theta = t;
		#else
			Number theta = -t;
		#endif
			Number cos_theta = cos(theta), sin_theta = sin(theta);
			rotation_y(m, cos_theta, sin_theta);
		}

		//Get an rotation matrix along z-axis
		template <class matrix, class Number>
		inline void rotation_z(matrix& m, const Number cos_theta, const Number sin_theta)
		{
			memset(m.n, 0, sizeof(m.n));
			m._44	= 1;
			m._33	= 1;
			m._11	= cos_theta;
			m._12	= -sin_theta;
			m._21	= sin_theta;
			m._22	= cos_theta;
		}

		template <class matrix, class Number>
		inline void rotation_z(matrix& m, const Number t)
		{
		#if defined(CODEX_MATH_RIGHTHAND)
			Number theta = t;
		#else
			Number theta = -t;
		#endif
			Number cos_theta = cos(theta), sin_theta = sin(theta);
			rotation_z(m, cos_theta, sin_theta);
		}

		//Get an rotation matrix along an axis
		template <class matrix, class Number>
		inline void rotation_axis(matrix& m, const vector3<Number> &v, const Number t)
		{
		//#if defined(CODEX_MATH_DEBUG_VERBOSE)
		//	if (codex::math::utils::not_equal<Number>(v.length(), 1, RELATIVE_PRECISION))
		//		throw_error(ERROR_POS, EXP_NON_UNIT_VECTOR, "rotation_axis");
		//#endif
		#if defined(CODEX_MATH_RIGHTHAND)
			Number theta = t;
		#else
			Number theta = -t;
		#endif
			Number c = cos(theta), s = sin(theta);
			rotation_axis(m, v, c, s);
		}

		template <class matrix, class Number>
		inline void rotation_axis(matrix& m, const vector3<Number> &v, const Number c, const Number s)
		{
			m._14 = m._24 = m._34 = m._41 = m._42 = m._43 = 0;
			m._44 = 1;

			m._11 = c + (1-c)*v.x*v.x;
			m._12 = (1 - c)*v.x*v.y - s*v.z;
			m._13 = (1 - c)*v.x*v.z + s*v.y;

			m._21 = (1 - c)*v.x*v.y + s*v.z;
			m._22 = c + (1-c)*v.y*v.y;
			m._23 = (1 - c)*v.y*v.z - s*v.x;

			m._31 = (1 - c)*v.z*v.x - s*v.y;
			m._32 = (1 - c)*v.z*v.y + s*v.x;
			m._33 = c + (1-c)*v.z*v.z;
		}

		template <class matrix, class Number>
		inline void projection_orthogonal(matrix& m, 
			const Number w, const Number h, const Number zn, const Number zf)
		{
			identity(m);

			m._11 = (Number)2 / w;
			m._22 = (Number)2 / h;
		//#if defined(CODEX_MATH_RIGHTHAND)
		//	m._33 = (Number)1 / (zn - zf);
		//#else
		//	m._33 = (Number)1 / (zf - zn)
		//#endif
			m._33 = (Number)1 / (zf - zn);
			m._34 = zn / (zn - zf);
		}

		template <class matrix, class Number>
		inline void projection_perspective(matrix& m, 
			const Number w, const Number h, const Number zn, const Number zf)
		{
			identity(m);

			m._11 = -(Number)2 * zn / w;
			m._22 = -(Number)2 * zn / h;
		//#if defined(CODEX_MATH_RIGHTHAND)
		//	m._33 = zf / (zn - zf);
		//	m._34 = -1;
		//#else
		//	m._33 = zf / (zf - zn);
		//	m._34 = 1;
		//#endif

			m._33 = -zf / (zf - zn);
			m._43 = -1;

			m._34 = -zn * zf / (zn - zf);
			m._44 = 0;
		}

		template <class matrix, class vec>
		inline void lookat(matrix &m, const vec &eye, const vec &lookat, const vec &up)
		{
			const vec	vat		= (eye - lookat).normalized_vector();
			const vec	vup		= (up - (up * vat) * vat).normalized_vector();
			const vec	vright	= vup ^ vat;

			m = matrix(
				vright.x,	vright.y,	vright.z,	-eye*vright,
				vup.x,		vup.y,		vup.z,		-eye*vup,
				vat.x,		vat.y,		vat.z,		-eye*vat,
				0,			0,			0,			1);
		}
	}
}
}