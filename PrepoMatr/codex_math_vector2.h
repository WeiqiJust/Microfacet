//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   03-12-2007
//  File:         codex_math_vector2.h
//  Content:      codex_math vector2 class definition
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
	namespace vector
	{
		template <class Number>
		class vector2;

		template <class Number>
		class vector;

		//Output to ostream
		template <class Number>
		std::ostream& operator << (std::ostream &os, const vector2<Number> &v)
		{
			return os << "(" << v.x << "," << v.y << ")";
		}

		//Multiplication by a number
		template <class Number>
		inline vector2<Number> operator * (const Number s, const vector2<Number>& v)
		{
			return v*s;
		}

		//Normalization
		template <class Number>
		inline Number normalize(vector2<Number> &vout, const vector2<Number>& v)
		{
			Number l		= v.length();
			Number inv_l	= l > 0 ? 1/l : 0;

			vout = v * inv_l;

			return l;
		}

		//Dot Product
		template <class Number>
		inline Number operator * (const vector2<Number>& v1, const vector2<Number>& v2)
		{
			return v1.x*v2.x + v1.y*v2.y;
		}

		//*****************************************************************************
		// vector2 class and non-member functions definition
		//*****************************************************************************

		template <class Number>
		class vector2
		{
		public:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------
			union {
				struct {
					Number	x, y;
				};
				struct {
					Number	s, t;
				};
				Number v[2];
			};

			//-----------------------------------------------------------------------------
			//constructors
			//-----------------------------------------------------------------------------
			vector2() : x(0), y(0) {}
			vector2(const Number x, const Number y) : x(x), y(y) {}
			vector2(const Number *v) : x(v[0]), y(v[1]) {}

			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline const int num_dim() const
			{
				return 2;
			}

			inline Number length() const
			{
				return sqrt(length_sq());
			}

			inline Number length_sq() const
			{
				return (x*x + y*y);
			}

			inline Number normalize()
			{
				return codex::math::vector::normalize(*this, *this);
			}


			// Performance warning!
			// avoid frequent using of this function
			// use normalize instead
			inline vector2<Number> normalized_vector() const
			{
				vector2<Number> temp;
				codex::math::vector::normalize(temp, *this);

				return temp;
			}

			//-----------------------------------------------------------------------------
			//assignment operators
			//-----------------------------------------------------------------------------
			inline vector2<Number>& operator = (const vector2<Number>& v)
			{
				if (&v != this)
				{
					x = v.x;
					y = v.y;
				}

				return *this;
			}

			inline vector2<Number>& operator += (const vector2<Number>& v)
			{
				x += v.x;
				y += v.y;

				return *this;
			}

			inline vector2<Number>& operator -= (const vector2<Number>& v)
			{
				x -= v.x;
				y -= v.y;

				return *this;
			}

			inline vector2<Number>& operator *= (const Number r)
			{
				x *= r;
				y *= r;

				return *this;
			}

			inline vector2<Number>& operator /= (const Number r)
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(r, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector2::operator /=");
#endif
				x /= r;
				y /= r;

				return *this;
			}

			//-----------------------------------------------------------------------------
			//unary operators
			//-----------------------------------------------------------------------------
			inline vector2<Number> operator + () const
			{
				return vector2<Number>(x, y);
			}

			inline vector2<Number> operator - () const
			{
				return vector2<Number>(-x, -y);
			}

			//-----------------------------------------------------------------------------
			//type conversion operators
			//-----------------------------------------------------------------------------
			operator const vector<Number>() const 
			{ 
				return vector<Number>(2, v);
			}

			//-----------------------------------------------------------------------------
			//binary operators
			//-----------------------------------------------------------------------------
			inline vector2<Number> operator + (const vector2<Number>& v) const
			{
				return vector2<Number>(x + v.x, y + v.y);
			}

			inline vector2<Number> operator - (const vector2<Number>& v) const
			{
				return vector2<Number>(x - v.x, y - v.y);
			}

			inline vector2<Number> operator * (const Number s) const
			{
				return vector2<Number>(x*s, y*s);
			}

			inline vector2<Number> operator / (const Number s) const
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector2::operator /");
#endif
				return vector2<Number>(x/s, y/s);
			}

			inline bool operator == (const vector2<Number>& v) const
			{
				return (codex::math::utils::equal(x, v.x, RELATIVE_PRECISION) && codex::math::utils::equal(y, v.y, RELATIVE_PRECISION));
			}

			inline bool operator != (const vector2<Number>& v) const
			{
				return !(*this == v);
			}
		};

	}

}
}