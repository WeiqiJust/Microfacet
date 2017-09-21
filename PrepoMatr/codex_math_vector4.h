//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   02-14-2007
//  File:         codex_math_vector4.h
//  Content:      codex_math vector4 class definition
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
		class vector4;

		template <class Number>
		class vector;

		//Output to ostream
		template <class Number>
		std::ostream& operator << (std::ostream &os, const vector4<Number> &v)
		{
			return os << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
		}

		//Multiplication by a number
		template <class Number>
		inline vector4<Number> operator * (const Number r, const vector4<Number>& v)
		{
			return v*r;
		}

		//Normalization
		template <class Number>
		inline Number normalize(vector4<Number> &vout, const vector4<Number>& v)
		{
			Number l		= v.length();
			Number inv_l	= l > 0 ? 1/l : 0;

			vout = v * inv_l;

			return l;
		}

		//Dot Product
		template <class Number>
		inline Number operator * (const vector4<Number>& v1, const vector4<Number>& v2)
		{
			return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
		}

		//FIX ME : Implement this!
//		//Cross Product
//		template <class Number>
//		inline vector4<Number> operator ^ (const vector4<Number>& v1, const vector4<Number>& v2)
//		{
//#if defined(CODEX_MATH_RIGHTHAND)
//			return vector4<Number>();
//#else
//			return vector4<Number>();
//#endif
//		}

	//*****************************************************************************
	// vector4 class and non-member functions definition
	//*****************************************************************************

		template <class Number>
		class vector4
		{
		public:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------
			union {
				struct {
					Number	x, y, z, w;
				};
				struct {
					Number	r, g, b, a;
				};
				struct {
					Number	r, g, b, e;
				};
				Number v[4];
			};

			//-----------------------------------------------------------------------------
			//constructors
			//-----------------------------------------------------------------------------
			vector4() : x(0), y(0), z(0), w(0) {}
			vector4(int) : x(0), y(0), z(0), w(0) {}
			vector4(const Number x, const Number y, const Number z, const Number w) : x(x), y(y), z(z), w(w) {}
			vector4(const Number *v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline const int num_dim() const
			{
				return 4;
			}

			inline Number length() const
			{
				return sqrt(length_sq());
			}

			inline Number length_sq() const
			{
				return (x*x + y*y + z*z + w*w);
			}

			inline Number normalize()
			{
				return normalize(*this, *this);
			}

			// Performance warning!
			// avoid frequent using of this function
			// use normalize instead
			inline vector4<Number> normalized_vector() const
			{
				vector4<Number> temp;

				normalize(temp, *this);
			}

			//-----------------------------------------------------------------------------
			//assignment operators
			//-----------------------------------------------------------------------------
			inline vector4<Number>& operator = (const vector4<Number>& v)
			{
				if (&v != this)
				{
					x = v.x;
					y = v.y;
					z = v.z;
					w = v.w;
				}

				return *this;
			}

			inline vector4<Number>& operator += (const vector4<Number>& v)
			{
				x += v.x;
				y += v.y;
				z += v.z;
				w += v.w;

				return *this;
			}

			inline vector4<Number>& operator -= (const vector4<Number>& v)
			{
				x -= v.x;
				y -= v.y;
				z -= v.z;
				w -= v.w;

				return *this;
			}

			inline vector4<Number>& operator *= (const Number s)
			{
				x *= s;
				y *= s;
				z *= s;
				w *= s;

				return *this;
			}

			inline vector4<Number>& operator /= (const Number s)
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector4::operator /=")
	#endif
				x /= s;
				y /= s;
				z /= s;
				w /= s;

				return *this;
			}
			//-----------------------------------------------------------------------------
			//access operators
			//-----------------------------------------------------------------------------
			inline Number& operator () (int comp)
			{
				#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (comp < 1 || comp > num_dim())
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "vector4::operator ()");
				#endif
				return v[comp-1];
			}

			inline const Number operator () (int comp) const
			{
				#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (comp < 1 || comp > num_dim())
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "vector4::operator () const");
				#endif
				return v[comp-1];
			}


			//-----------------------------------------------------------------------------
			//unary operators
			//-----------------------------------------------------------------------------
			inline vector4<Number> operator + () const
			{
				return vector4<Number>(x, y, z, w);
			}

			inline vector4<Number> operator - () const
			{
				return vector4<Number>(-x, -y, -z, -w);
			}

			//-----------------------------------------------------------------------------
			//type conversion operators
			//-----------------------------------------------------------------------------
			operator const vector<Number>() const 
			{ 
				return vector<Number>(4, v);
			}

			//-----------------------------------------------------------------------------
			//binary operators
			//-----------------------------------------------------------------------------
			inline vector4<Number> operator + (const vector4<Number>& v) const
			{
				return vector4<Number>(x + v.x, y + v.y, z + v.z, w + v.w);
			}

			inline vector4<Number> operator - (const vector4<Number>& v)
			{
				return vector4<Number>(x - v.x, y - v.y, z - v.z, w - v.w);
			}

			inline vector4<Number> operator * (const Number s) const
			{
				return vector4<Number>(x*s, y*s, z*s, w*s);
			}

			inline vector4<Number> operator / (const Number s) const
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector4::operator /");
	#endif
				return vector4<Number>(x/s, y/s, z/s, w/s);
			}

			inline bool operator == (const vector4<Number>& v) const
			{
				return (codex::math::utils::equal(x, v.x, RELATIVE_PRECISION) && codex::math::utils::equal(y, v.y) && codex::math::utils::equal(z, v.z) && codex::math::utils::equal(w, v.w, RELATIVE_PRECISION));
			}

			inline bool operator != (const vector4<Number>& v) const
			{
				return !(*this == v);
			}
		};

	}
}
}