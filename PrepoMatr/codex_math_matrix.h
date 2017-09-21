//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   02-20-2007
//  File:         codex_math_matrix.h
//  Content:      codex_math matrix class definition
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
          OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma once
#include <iostream>
#include "codex_math_vector.h"
#include "codex_math_vector2.h"
#include "codex_math_vector3.h"
#include "codex_math_vector4.h"
namespace codex
{
namespace math
{
	namespace matrix
	{
		template <class Number>
		class matrix;

		template <class Number, class matrix>
		void LU_decompose(matrix &a, const matrix &mat, int *indx, Number &d);

		template <class Number, class vector, class matrix>
		void LU_back_substitue(vector &b, const matrix &a, int *indx, const vector &v);

		using codex::math::vector::vector;

		//Output to ostream
		template <class Number>
		std::ostream& operator << (std::ostream &os, const matrix<Number>& m)
		{
			os << "(";
			
			for (int x = 1; x < m.num_rows(); x++)
			{
				for (int y = 1; y <= m.num_cols(); y++)
					os << m(x, y) << ",";
				os << endl;
			}

			if (m.num_rows() > 0)
			{
				for (int y = 1; y < m.num_cols(); y++)
					os << m(m.num_rows(), y) << ",";
				os << m(m.num_rows(), m.num_cols());
			}

			os << ")" << endl;

			return os;
		}

		template <class Number>
		bool multiplification_dim_equal(const matrix<Number>& m1, const matrix<Number>& m2)
		{
			if (m1.num_cols() != m2.num_rows())
			{
			#if defined(CODEX_MATH_DEBUG_VERBOSE)
				throw_error(ERROR_POS, EXP_DIM_MISMATCH, "matrix::multiplification_dim_equal");
			#endif
				return false;
			} else {
				return true;
			}
		}

		//Multiplication by a number
		template <class Number>
		inline matrix<Number> operator * (const Number s, const matrix<Number>& m)
		{
			return m*s;
		}

		//Multiplication by a vector
		template <class Number>
		inline matrix<Number> operator * (const vector<Number>& v, const matrix<Number>& m)
		{
			return (static_cast<const matrix<Number>>v).transposed_matrix() * m;
		}

		//Get an identity matrix
		template <class Number>
		inline void identity(matrix<Number>& m)
		{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
			if (!m.is_square())
				throw_error(ERROR_POS, EXP_NON_SQUARE_MATRIX, "matrix4x4::operator /");
#endif
			memset(m.m, 0, sizeof(Number)*m.num_rows()*m.num_cols());
			for (int i = 0; i<m.num_rows(); i++)
				m.m[i+i*m.num_cols()] = 1;
		}

		template <class Number>
		void transpose(matrix<Number>& m, const matrix<Number>& n)
		{
			if (m.alloc(n.num_cols(), n.num_rows()))
			{
				for (int x = 0; x<m.num_rows(); x++)
					for (int y = 0; y<m.num_cols(); y++)
						m.m[x*m.num_cols()+y] = n.m[y*n.num_cols()+x];
			}
		}

		template <class Number, class vector, class matrix>
		void matrix_inverse(matrix& m, const matrix& n)
		{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
			if (!n.is_square())
				throw_error(ERROR_POS, EXP_NON_SQUARE_MATRIX, "inverse matrix");
#endif

			int				*indx = NULL;
			matrix			a(n);
			vector			zerocol(a.num_rows()), col(a.num_rows());
			Number			d;

			if (m.alloc(a.num_rows(), a.num_cols()))
			{
				indx = new int[a.num_rows()+1];

				LU_decompose(a, indx, d);

				for (int j = 1; j <= a.num_rows(); j++)
				{
					col = zerocol;
					col(j) = 1;

					LU_back_substitue<Number, vector, matrix>(col, a, indx, col);
					for (int i = 1; i <= a.num_rows(); i++) 
						m.m[(i-1)*m.num_cols() + (j-1)] = col(i);
				}

				SAFE_DELETE_ARR(indx);
			}
		}

		//*****************************************************************************
		// matrix class and non-member functions definition
		//*****************************************************************************

		template <class Number>
		class matrix
		{
		private:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------

			int			n_rows, n_cols;
			Number		*m;

			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline bool dim_equal(const matrix<Number>& mat) const
			{
				if (num_rows() != mat.num_rows() || num_cols() != mat.num_cols())
				{
				#if defined(CODEX_MATH_DEBUG_VERBOSE)
					throw_error(ERROR_POS, EXP_DIM_MISMATCH, "matrix::dim_equal");
				#endif
					return false;
				} else {
					return true;
				}
			}

			inline bool alloc(const int rows, const int cols)
			{
				if (n_rows == 0 && n_cols == 0)
				{
					n_rows	= rows;
					n_cols	= cols;
					m		= new Number[n_rows*n_cols];
				} else if (n_rows != rows || n_cols != cols) {
				#if defined(CODEX_MATH_DEBUG_VERBOSE)
					throw_error(ERROR_POS, EXP_ALLOCATION, "matrix::alloc");
				#endif
					return false;
				}

				return true;
			}

			//-----------------------------------------------------------------------------
			//basic matrix operations
			//-----------------------------------------------------------------------------
			inline void exchange_row(const int i, const int j)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_rows - 1 || j < 0 || j > n_cols - 1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix::exchange_row");
#endif
					Number temp[n_cols];

					memcpy(temp, m[i], sizeof(Number)*n_cols);
					memcpy(m[i], m[j], sizeof(Number)*n_cols);
					memcpy(m[j], temp, sizeof(Number)*n_cols);
				}
			}

			inline void exchange_row(const int i, const int j, const int start_col, const int finish_col)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_rows - 1 || j < 0 || j > n_cols - 1 || 
						start_col < 0 || start_col > n_cols-1 || finish_col < 0 || finish_col > n_cols-1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix::exchange_row");
#endif
					Number temp[4];

					memcpy(&temp[start_col], &m[i][start_col], sizeof(Number)*(finish_col-start_col+1));
					memcpy(&m[i][start_col], &m[j][start_col], sizeof(Number)*(finish_col-start_col+1));
					memcpy(&m[j][start_col], &temp[start_col], sizeof(Number)*(finish_col-start_col+1));
				}
			}

			inline void exchange_column(const int i, const int j)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_cols || j < 0 || j > n_cols - 1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix::exchange_column");
#endif
					for (int k = 0; k<n_rows; k++)
					{
						Number temp;

						temp = m[k][i];
						m[k][i] = m[k][j];
						m[k][j] = temp;
					}
				}
			}

			inline void exchange_column(const int i, const int j, const int start_row, const int finish_row)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_rows-1 || j < 0 || j > n_cols - 1 || 
						start_row < 0 || start_row > n_rows-1 || finish_row < 0 || finish_row > n_rows-1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix::exchange_column");
#endif
					for (int k = start_row; k<=finish_row; k++)
					{
						Number temp;

						temp = m[k][i];
						m[k][i] = m[k][j];
						m[k][j] = temp;
					}
				}
			}

			//-----------------------------------------------------------------------------
			//friends
			//-----------------------------------------------------------------------------
			friend class matrix<Number>;
			friend bool multiplification_dim_equal<Number>(const matrix<Number>& m1, const matrix<Number>& m2);
			friend void transpose<Number>(matrix<Number>& m, const matrix<Number>& n);
			friend void matrix_inverse<Number, vector<Number>, matrix<Number> >(matrix<Number>& m, const matrix<Number>& n);

			friend void LU_decompose<Number, matrix<Number> >(matrix &a, const matrix<Number> &mat, int *indx, Number &d);
			friend void LU_back_substitue<Number, vector<Number>, matrix<Number> >(vector<Number> &b, const matrix<Number> &a, int *indx, const vector<Number> &v);

		public:
			//-----------------------------------------------------------------------------
			//constructors and destructor
			//-----------------------------------------------------------------------------
			matrix() : n_rows(0), n_cols(0), m(NULL) {}

			matrix(const matrix<Number> &mat) : n_rows(mat.num_rows()), n_cols(mat.num_cols())
			{
				m = new Number[n_rows*n_cols];
				memcpy(m, mat.m, sizeof(Number)*n_rows*n_cols);
			}

			matrix(const int rows, const int cols) : n_rows(rows), n_cols(cols)
			{
				m = new Number[n_rows*n_cols];
				memset(m, 0, sizeof(Number)*n_rows*n_cols);
			}

			matrix(const int rows, const int cols, Number *vec, const bool alloc_new = true) : n_rows(rows), n_cols(cols)
			{
				if (alloc_new)
				{
					m = new Number[n_rows*n_cols];
					memcpy(m, vec, sizeof(Number)*n_rows*n_cols);
				} else {
					m = vec;
				}
			}

			virtual ~matrix()
			{
				SAFE_DELETE(m);
			}
			//-----------------------------------------------------------------------------
			//function members
			//-----------------------------------------------------------------------------
			inline int num_rows() const
			{
				return n_rows;
			}

			inline int num_cols() const
			{
				return n_cols;
			}

			inline bool is_square() const
			{
				return n_rows == n_cols;
			}

			inline void transpose()
			{
				matrix<Number> temp;

				codex::math::matrix::transpose(temp, *this);

				*this = temp;
			}

			inline matrix<Number> transposed_matrix() const
			{
				matrix<Number> temp;

				codex::math::matrix::transpose(temp, *this);

				return matrix<Number>(temp);
			}

			inline void inverse()
			{
				codex::math::matrix::matrix_inverse(*this, *this);
			}

			inline matrix<Number> inversed_matrix() const
			{
				matrix<Number> temp;

				codex::math::matrix::matrix_inverse<Number, vector<Number>, matrix<Number> >(temp, *this);

				return matrix<Number>(temp);
			}

			inline Number determinant() const
			{
				if (is_square())
				{
					int				*indx = NULL;

					try
					{
						matrix<Number>	a;
						Number			d;

						indx = new int[num_rows()+1];

						LU_decompose(a, *this, indx, d);
						for (int j = 1; j <= a.num_rows(); j++)
							d *= a.m[j-1 + (j-1)*a.num_rows()];

						SAFE_DELETE_ARR(indx);
						
						return d;
					}
					catch (generic_exception e)
					{
						if (e.get_error_code() == EXP_NON_INVERTIBLE_MATRIX)
						{
							SAFE_DELETE_ARR(indx);
							return 0;
						} else
							throw;
					}
				} else {
					return 0;
				}
			}

			//-----------------------------------------------------------------------------
			//access operators
			//-----------------------------------------------------------------------------
			inline Number& operator () (int row, int col)
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (row <1 || row > n_rows || col < 1 || col > n_cols)
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix::operator ()");
#endif
				return m[(row-1)*num_cols()+col-1];
			}

			inline Number operator () (int row, int col) const
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (row <1 || row > n_rows || col < 1 || col > n_cols)
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix::operator () const");
#endif
				return m[(row-1)*num_cols()+col-1];
			}


			//-----------------------------------------------------------------------------
			//assignment operators
			//-----------------------------------------------------------------------------
			inline matrix<Number>& operator = (const matrix<Number>& mat)
			{
				if (&mat != this)
				{
					if (alloc(mat.num_rows(), mat.num_cols()))
						memcpy(m, mat.m, sizeof(Number)*num_rows()*num_cols());
				}

				return *this;
			}

			inline matrix<Number>& operator += (const matrix<Number>& mat)
			{
				if (dim_equal(mat))
				{
					for (int i = 0; i<n_rows*n_cols; i++)
							m[i] += mat.m[i];
				}
				return *this;
			}

			inline matrix<Number>& operator -= (const matrix<Number>& mat)
			{
				if (dim_equal(mat))
				{
					for (int i = 0; i<n_rows*n_cols; i++)
						m[i] -= mat.m[i];
				}
				return *this;
			}

			inline matrix<Number>& operator *= (const matrix<Number>& mat)
			{
				matrix<Number> temp = (*this) * mat;
				*this = temp;
				return *this;
			}

			inline matrix<Number>& operator *= (const Number s)
			{
				for (int i = 0; i<n_rows*n_cols; i++)
					m[i] *= s;
				return *this;
			}

			inline matrix<Number>& operator /= (const Number s)
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "matrix::operator /");
#endif

				for (int i = 0; i<n_rows*n_cols; i++)
					m[i] /= s;
				return *this;
			}
			//-----------------------------------------------------------------------------
			//unary operators
			//-----------------------------------------------------------------------------
			inline matrix<Number> operator + () const
			{
				return matrix<Number>(num_rows(), num_cols(), m);
			}

			inline matrix<Number> operator - () const
			{
				Number *value;

				value = new Number[num_rows()*num_cols()];
				for (int i = 0; i<num_rows()*num_cols(); i++)
					value[i] = -m[i];

				return matrix<Number>(num_rows(), num_cols(), value, false);
			}

			//-----------------------------------------------------------------------------
			//binary operators
			//-----------------------------------------------------------------------------
			inline matrix<Number> operator + (const matrix<Number>& n) const
			{
				if (dim_equal(n))
				{
					Number *value;

					value = new Number[num_rows()*num_cols()];
					for (int i = 0; i<num_rows()*num_cols(); i++)
						value[i] = m[i] + n.m[i];

					return matrix<Number>(num_rows(), num_cols(), value, false);
				} else 
					return matrix<Number>();
			}

			inline matrix<Number> operator - (const matrix<Number>& n) const
			{
				if (dim_equal(n))
				{
					Number *value;

					value = new Number[num_rows()*num_cols()];
					for (int i = 0; i<num_rows()*num_cols(); i++)
						value[i] = m[i] - n.m[i];

					return matrix<Number>(num_rows(), num_cols(), value, false);
				} else 
					return matrix<Number>();
			}

			inline matrix<Number> operator * (const matrix<Number>& n) const
			{
				if (multiplification_dim_equal(*this, n))
				{
					Number *value;

					value = new Number[num_rows()*n.num_cols()];

					for (int x = 0; x<num_rows(); x++)
						for (int y = 0; y<n.num_cols(); y++)
						{
							double v = 0;
							
							for (int i = 0; i<num_cols(); i++)
								v += m[x*num_cols() + i] * n.m[i*n.num_cols() + y];

							value[x*n.num_cols()+y] = v;
						}

					return matrix<Number>(num_rows(), n.num_cols(), value, false);
				} else 
					return matrix<Number>();
			}

			inline codex::math::vector::vector<Number> operator * (const vector<Number>& v) const
			{
				return vector<Number>(v.num_dim(), (*this * static_cast<const matrix<Number>>(v)).m);
			}

			inline matrix<Number> operator * (const Number s) const
			{
				Number *value;

				value = new Number[num_rows()*num_cols()];
				for (int i = 0; i<num_rows()*num_cols(); i++)
					value[i] = m[i]*s;

				return matrix<Number>(num_rows(), num_cols(), value, false);
			}

			inline matrix<Number> operator / (const Number s) const
			{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "matrix::operator /");
#endif
				Number *value;

				value = new Number[num_rows()*num_cols()];
				for (int i = 0; i<num_rows()*num_cols(); i++)
					value[i] = m[i]/s;

				return matrix<Number>(num_rows(), num_cols(), value, false);
			}

			inline bool operator == (const matrix<Number>& n) const
			{
				if (num_rows() != n.num_rows() || num_cols() != n.num_cols())
					return false;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						if (not_equal(m[x][y], n.m[x][y], RELATIVE_PRECISION))
							return false;

				return true;
			}

			inline bool operator != (const matrix<Number>& n) const
			{
				return !(*this == n);
			}

		};

		template <class Number, class matrix>
		void LU_decompose(matrix &a, int *indx, Number &d)
		{
			int				i, imax, j, k, n;
			Number			big, dum, sum, temp;
			Number			*vv;

			n	= a.num_rows();
			vv	= new Number[n+1];
			
			d = 1;
			for (i = 1; i <= n; i++) 
			{
				big = (Number)0.0;
				for (j = 1; j <= n; j++)
					if ((temp = fabs(a.m[(i-1)*a.num_cols() + j-1])) > big) 
						big=temp;
				
				if (big == (Number)0.0)
					throw_error(ERROR_POS, EXP_NON_INVERTIBLE_MATRIX, "LU_decomp");

				vv[i] = (Number)1.0 / big;
			}

			for (j = 1; j <= n; j++) 
			{
				for (i = 1; i<j; i++) 
				{
					sum = a.m[(i-1)*a.num_cols() + j-1];
					for (k = 1; k < i; k++) 
						sum -= a.m[(i-1)*a.num_cols() + k-1]*a.m[(k-1)*a.num_cols() + j-1];
					a.m[(i-1)*a.num_cols() + j-1] = sum;
				}

				big = (Number)0.0;
				
				for (i = j; i <= n; i++) 
				{
					sum = a.m[(i-1)*a.num_cols() + j-1];
					for (k = 1; k < j; k++)
						sum -= a.m[(i-1)*a.num_cols() + k-1]*a.m[(k-1)*a.num_cols() + j-1];
					a.m[(i-1)*a.num_cols() + j-1] = sum;

					if ((dum = vv[i]*fabs(sum)) >= big) 
					{
						big	= dum;
						imax= i;
					}
				}

				if (j != imax) 
				{
					for (k = 1; k <= n; k++) 
					{
						dum			= a.m[(imax-1)*a.num_cols() + k-1];
						a.m[(imax-1)*a.num_cols() + k-1]	= a.m[(j-1)*a.num_cols() + k-1];
						a.m[(j-1)*a.num_cols() + k-1]		= dum;
					}
					d		= -d;
					vv[imax]= vv[j];
				}

				indx[j] = imax;
				if (a.m[(j-1)*a.num_cols() + j-1] == (Number)0.0) 
					a.m[(j-1)*a.num_cols() + j-1] = (Number)TINY;
				if (j != n) 
				{
					dum = (Number)1.0/(a.m[(j-1)*a.num_cols() + j-1]);
					for (i = j+1; i<=n; i++) 
						a.m[(i-1)*a.num_cols() + j-1] *= dum;
				}
			}

			delete[] vv;
		}

		template <class Number, class vector, class matrix>
		void LU_back_substitue(vector &b, const matrix &a, int *indx, const vector &v)
		{
			int		i, ii, ip, j, n;
			Number	sum;

			ii	= 0;
			n	= a.num_rows();

			if (n != b.num_dim())
				throw_error(ERROR_POS, EXP_DIM_MISMATCH, "LU_back_substitute");

			b = v;

			for (i = 1; i <= n; i++) 
			{
				ip		= indx[i];
				sum		= b(ip);
				b(ip)	= b(i);

				if (ii)
				{
					for (j = ii; j <= i-1; j++) 
						sum -= a.m[(i-1)*a.num_cols() + j-1]*b(j);
				} else {
					if (sum) ii = i;
				}
				
				b(i)= sum;
			}

			for (i = n; i >= 1; i--) 
			{
				sum = b(i);
				for (j = i+1; j <= n; j++) 
					sum -= a.m[(i-1)*a.num_cols() + j-1]*b(j);
				b(i) = sum/a.m[(i-1)*a.num_cols() + i-1];
			}
		}
	}
}
}