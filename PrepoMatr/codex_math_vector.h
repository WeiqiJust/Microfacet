//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   02-19-2007
//  File:         codex_math_vector.h
//  Content:      codex_math vector class definition
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
	namespace matrix
	{
		template <class Number>
		class matrix;
	}

	namespace vector
	{
		template <class Number>
		class vector;

		using codex::math::matrix::matrix;

		//Output to ostream
		template <class Number>
		std::ostream& operator << (std::ostream &os, const vector<Number> &vec)
		{
			os << "("; 
			
			for (int i = 0; i < vec.num_dim() - 1; i++)
				os << vec.v[i] << ",";
			if (vec.num_dim() - 1 > 0)
				os << vec.v[vec.num_dim() - 1];

			os << ")"; 

			return os;
		}

		//Multiplication by a number
		template <class Number>
		inline vector<Number> operator * (const Number s, const vector<Number>& v)
		{
			return v*s;
		}

		//Normalization
		template <class Number>
		inline Number normalize(vector<Number> &vout, const vector<Number>& v)
		{
			Number l		= v.length();
			Number inv_l	= l > 0 ? 1/l : 0;

			vout = v * inv_l;

			return l;
		}

		//Dot Product
		template <class Number>
		Number operator * (const vector<Number>& v1, const vector<Number>& v2)
		{
			if (v1.dim_equal(v2))
			{
				Number result = 0;

				for (int i = 0; i < v1.num_dim(); i++)
					result += v1.v[i]*v2.v[i];

				return result;
			} else
				return 0;
		}

		//FIX ME : Implement this!
//		//Cross Product
//		template <class Number>
//		inline vector<Number> operator ^ (const vector<Number>& v1, const vector<Number>& v2)
//		{
//#if defined(CODEX_MATH_RIGHTHAND)
//			return vector<Number>();
//#else
//			return vector<Number>();
//#endif
//		}

		//*****************************************************************************
		// vector class and non-member functions definition
		//*****************************************************************************

		template <class Number>
		class vector
		{
		private:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------
			int		n_dim;
			Number	*v;

			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline bool alloc(const int dim)
			{
				if (n_dim == 0)
				{
					n_dim	= dim;
					v		= new Number[n_dim];
				} else if (n_dim != dim) {
				#if defined(CODEX_MATH_DEBUG_VERBOSE)
					throw_error(ERROR_POS, EXP_ALLOCATION, "vector::alloc");
				#endif
					return false;
				}

				return true;
			}

			//-----------------------------------------------------------------------------
			//friends
			//-----------------------------------------------------------------------------
			friend std::ostream& operator << <Number>(std::ostream &os, const vector<Number> &vec);
			friend Number operator * <Number>(const vector<Number>& v1, const vector<Number>& v2);

		protected:
			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline bool dim_equal(const vector<Number>& vec) const
			{
				if (num_dim() != vec.num_dim())
				{
				#if defined(CODEX_MATH_DEBUG_VERBOSE)
					throw_error(ERROR_POS, EXP_DIM_MISMATCH, "vector::dim_equal");
				#endif
					return false;
				} else {
					return true;
				}
			}

		public:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------

			//-----------------------------------------------------------------------------
			//constructors and destructor
			//-----------------------------------------------------------------------------
			vector() : n_dim(0), v(NULL) {}

			vector(const vector &vec) : n_dim(vec.n_dim)
			{
				v = new Number[n_dim];
				memcpy(v, vec.v, sizeof(Number)*n_dim);
			}

			vector(const int dim) : n_dim(dim)
			{
				v = new Number[n_dim];
				memset(v, 0, sizeof(Number)*n_dim);
			}

			vector(const int dim, Number *vec, const bool alloc_new = true) : n_dim(dim)
			{
				if (alloc_new)
				{
					v = new Number[n_dim];
					memcpy(v, vec, sizeof(Number)*n_dim);
				} else {
					v = vec;
				}
			}

			vector(const int dim, const Number *vec) : n_dim(dim)
			{
				v = new Number[n_dim];
				memcpy(v, vec, sizeof(Number)*n_dim);
			}

			virtual ~vector()
			{
				SAFE_DELETE(v);
			}
			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline void reset()
			{
				n_dim = 0;
				SAFE_DELETE(v);
			}

			inline const int num_dim() const
			{
				return n_dim;
			}

			inline Number length() const
			{
				return sqrt(length_sq());
			}

			inline Number length_sq() const
			{
				double len_sq = 0;

				for (int i = 0; i<n_dim; i++)
					len_sq += v[i]*v[i];

				return len_sq;
			}

			inline Number normalize()
			{
				return codex::math::vector::normalize(*this, *this);
			}


			// Performance warning!
			// avoid frequent using of this function
			// use normalize instead
			inline vector<Number> normalized_vector() const
			{
				vector<Number> temp;
				codex::math::vector::normalize(temp, *this);

				return temp;
			}

			//-----------------------------------------------------------------------------
			//access operators
			//-----------------------------------------------------------------------------
			inline Number& operator () (int comp)
			{
			#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (comp < 1 || comp > n_dim)
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "vector::operator ()");
			#endif
				return v[comp-1];
			}

			inline const Number operator () (int comp) const
			{
			#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (comp < 1 || comp > n_dim)
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "vector::operator () const");
			#endif
				return v[comp-1];
			}

			//-----------------------------------------------------------------------------
			//assignment operators
			//-----------------------------------------------------------------------------
			inline vector<Number>& operator = (const vector<Number>& vec)
			{
				if (&vec != this)
				{
					if (alloc(vec.num_dim()))
						memcpy(v, vec.v, sizeof(Number)*n_dim);
				}

				return *this;
			}

			inline vector<Number>& operator += (const vector<Number>& vec)
			{
				if (dim_equal(vec))
				{
					for (int i = 0; i<n_dim; i++)
						v[i] += vec.v[i];
				}

				return *this;
			}

			inline vector<Number>& operator -= (const vector<Number>& vec)
			{
				if (dim_equal(vec))
				{
					for (int i = 0; i<n_dim; i++)
						v[i] -= vec.v[i];
				}

				return *this;
			}

			inline vector<Number>& operator *= (const Number r)
			{
				for (int i = 0; i<n_dim; i++)
					v[i] *= r;

				return *this;
			}

			inline vector<Number>& operator /= (const Number r)
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(r, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector::operator /=");
#endif
				for (int i = 0; i<n_dim; i++)
					v[i] /= r;

				return *this;
			}

			//-----------------------------------------------------------------------------
			//unary operators
			//-----------------------------------------------------------------------------
			inline vector<Number> operator + () const
			{
				return vector<Number>(n_dim, v);
			}

			inline vector<Number> operator - () const
			{
				Number *value;

				value = new Number[n_dim];
				for (int i = 0; i < n_dim; i++)
					value[i] = -v[i];

				return vector<Number>(n_dim, value, false);
			}

			//-----------------------------------------------------------------------------
			//type conversion operators
			//-----------------------------------------------------------------------------
			operator const matrix<Number>() const 
			{ 
				return matrix<Number>(n_dim, 1, v);
			}

			//-----------------------------------------------------------------------------
			//binary operators
			//-----------------------------------------------------------------------------
			inline vector<Number> operator + (const vector<Number>& vec) const
			{
				Number *value;

				if (dim_equal(vec))
				{
					value = new Number[n_dim];
					for (int i = 0; i < n_dim; i++)
						value[i] = v[i] + vec.v[i];

					return vector<Number>(n_dim, value, false);
				} else 
					return vector<Number>();
			}

			inline vector<Number> operator - (const vector<Number>& vec) const
			{
				Number *value;

				if (dim_equal(vec))
				{
					value = new Number[n_dim];
					for (int i = 0; i < n_dim; i++)
						value[i] = v[i] - vec.v[i];

					return vector<Number>(n_dim, value, false);
				} else 
					return vector<Number>();
			}

			inline vector<Number> operator * (const Number s) const
			{
				Number *value;

				value = new Number[n_dim];
				for (int i = 0; i < n_dim; i++)
					value[i] = v[i] * s;

				return vector<Number>(n_dim, value, false);
			}

			inline vector<Number> operator / (const Number s) const
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "vector::operator /");
#endif
				Number *value;

				value = new Number[n_dim];
				for (int i = 0; i < n_dim; i++)
					value[i] = v[i] / s;

				return vector<Number>(n_dim, value, false);
			}

			inline bool operator == (const vector<Number>& vec) const
			{
				if (num_dim() != vec.num_dim())
					return false;
				else {
					for (int i = 0; i < n_dim; i++)
						if (codex::math::utils::not_equal(v[i], vec.v[i], RELATIVE_PRECISION))
							return false;

					return true;
				}
			}

			inline bool operator != (const vector<Number>& vec) const
			{
				return !(*this == vec);
			}
		};

	}

}
}