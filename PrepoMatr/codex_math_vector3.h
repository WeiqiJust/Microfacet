//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   02-14-2007
//  File:         codex_math_vector3.h
//  Content:      codex_math vector3 class definition
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
          OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma once
#include <iostream>
namespace codex
{
namespace math
{
	namespace vector
	{
		template <class Number>
		class vector3;

		template <class Number>
		class vector;

		//Output to ostream
		template <class Number>
		std::ostream& operator << (std::ostream &os, const vector3<Number> &v)
		{
			return os << "(" << v.x << "," << v.y << "," << v.z << ")";
		}

		//Multiplication by a number
		template <class Number>
		inline vector3<Number> operator * (const Number s, const vector3<Number>& v)
		{
			return v*s;
		}

		//Normalization
		template <class Number>
		inline Number normalize(vector3<Number> &vout, const vector3<Number>& v)
		{
			Number l		= v.length();
			Number inv_l	= l > 0 ? 1/l : 0;

			vout = v * inv_l;

			return l;
		}

		//Dot Product
		template <class Number>
		inline Number operator * (const vector3<Number>& v1, const vector3<Number>& v2)
		{
			return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
		}

		//Cross Product
		template <class Number>
		inline vector3<Number> operator ^ (const vector3<Number>& v1, const vector3<Number>& v2)
		{
#if defined(CODEX_MATH_RIGHTHAND)
			return vector3<Number>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
#else
			return vector3<Number>(v2.y*v1.z - v2.z*v1.y, v2.z*v1.x - v2.x*v1.z, v2.x*v1.y - v2.y*v1.x);
#endif
		}

		//Compute reflection vector
		//v			: incoming direction
		//n			: normal
		//result	: outgoing direction
		template <class Number>
		inline void reflect(vector3<Number>& vout, const vector3<Number>& vin, const vector3<Number>& n)
		{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
			if (codex::math::utils::not_equal<Number>(vin.length(), 1, RELATIVE_PRECISION) || codex::math::utils::not_equal<Number>(n.length(), 1, RELATIVE_PRECISION))
				throw_error(ERROR_POS, EXP_NON_UNIT_VECTOR, "reflect");
#endif

			vout = Number(2.0)*((vin*n) * n) - vin;
		}

	//*****************************************************************************
	// vector3 class and non-member functions definition
	//*****************************************************************************

		template <class Number>
		class vector3
		{
		public:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------
			union {
				struct {
					Number	x, y, z;
				};
				Number v[3];
			};

			//-----------------------------------------------------------------------------
			//constructors
			//-----------------------------------------------------------------------------
			vector3() : x(0), y(0), z(0) {}
			vector3(const Number x, const Number y, const Number z) :x(x), y(y), z(z) {}
			vector3(const Number *v) : x(v[0]), y(v[1]), z(v[2]) {}

			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline const int num_dim() const
			{
				return 3;
			}

			inline Number length() const
			{
				return sqrt(length_sq());
			}

			inline Number length_sq() const
			{
				return (x*x + y*y + z*z);
			}

			inline Number normalize()
			{
				return codex::math::vector::normalize(*this, *this);
			}


			// Performance warning!
			// avoid frequent using of this function
			// use normalize instead
			inline vector3<Number> normalized_vector() const
			{
				vector3<Number> temp;
				codex::math::vector::normalize(temp, *this);

				return temp;
			}

			//-----------------------------------------------------------------------------
			//assignment operators
			//-----------------------------------------------------------------------------
			inline vector3<Number>& operator = (const vector3<Number>& v)
			{
				if (&v != this)
				{
					x = v.x;
					y = v.y;
					z = v.z;
				}

				return *this;
			}

			inline vector3<Number>& operator += (const vector3<Number>& v)
			{
				x += v.x;
				y += v.y;
				z += v.z;

				return *this;
			}

			inline vector3<Number>& operator -= (const vector3<Number>& v)
			{
				x -= v.x;
				y -= v.y;
				z -= v.z;

				return *this;
			}

			inline vector3<Number>& operator *= (const Number r)
			{
				x *= r;
				y *= r;
				z *= r;

				return *this;
			}

			inline vector3<Number>& operator /= (const Number r)
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(r, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector3::operator /=");
	#endif
				x /= r;
				y /= r;
				z /= r;

				return *this;
			}

			//-----------------------------------------------------------------------------
			//unary operators
			//-----------------------------------------------------------------------------
			inline vector3<Number> operator + () const
			{
				return vector3<Number>(x, y, z);
			}

			inline vector3<Number> operator - () const
			{
				return vector3<Number>(-x, -y, -z);
			}

			//-----------------------------------------------------------------------------
			//type conversion operators
			//-----------------------------------------------------------------------------
			operator const vector<Number>() const 
			{ 
				return vector<Number>(3, v);
			}

			//-----------------------------------------------------------------------------
			//binary operators
			//-----------------------------------------------------------------------------
			inline vector3<Number> operator + (const vector3<Number>& v) const
			{
				return vector3<Number>(x + v.x, y + v.y, z + v.z);
			}

			inline vector3<Number> operator - (const vector3<Number>& v) const
			{
				return vector3<Number>(x - v.x, y - v.y, z - v.z);
			}

			inline vector3<Number> operator * (const Number s) const
			{
				return vector3<Number>(x*s, y*s, z*s);
			}

			inline vector3<Number> operator / (const Number s) const
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector3::operator /");
	#endif
				return vector3<Number>(x/s, y/s, z/s);
			}

			inline bool operator == (const vector3<Number>& v) const
			{
				return (codex::math::utils::equal(x, v.x, RELATIVE_PRECISION) && codex::math::utils::equal(y, v.y, RELATIVE_PRECISION) && codex::math::utils::equal(z, v.z, RELATIVE_PRECISION));
			}

			inline bool operator != (const vector3<Number>& v) const
			{
				return !(*this == v);
			}
		};

	}

}
}