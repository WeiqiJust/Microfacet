//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   02-14-2007
//  File:         codex_math_matrix4x4.h
//  Content:      codex_math matrix4x4 class definition
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
          OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma once
#include "codex_math_matrix.h"
namespace codex
{
namespace math
{	
	namespace matrix
	{
		template <class Number>
		class matrix4x4;

		using codex::math::vector::vector4;
		using codex::math::vector::vector3;

		//Output to ostream
		template <class Number>
		std::ostream& operator << (std::ostream &os, const matrix4x4<Number>& m)
		{
			return os	<< "(" << m._11 << "," << m._12 << "," << m._13 << "," << m._14 << "," << endl 
				<< " " << m._21 << "," << m._22 << "," << m._23 << "," << m._24 << "," << endl 
				<< " " << m._31 << "," << m._32 << "," << m._33 << "," << m._34 << "," << endl 
				<< " " << m._41 << "," << m._42 << "," << m._43 << "," << m._44 << ")" << endl; 
		}

		//Multiplication by a number
		template <class Number>
		inline matrix4x4<Number> operator * (const Number s, const matrix4x4<Number>& m)
		{
			return m*s;
		}
		
		//Multiplication by a vector4
		template <class Number>
		inline vector4<Number> operator * (const vector4<Number>& v, const matrix4x4<Number>& m)
		{
			vector4<Number>(
				v.x*m._11 + v.y*m._21 + v.z*m._31 + v.w*m._41,
				v.x*m._12 + v.y*m._22 + v.z*m._32 + v.w*m._42,
				v.x*m._13 + v.y*m._23 + v.z*m._33 + v.w*m._43,
				v.x*m._14 + v.y*m._24 + v.z*m._34 + v.w*m._44
				);
		}

		//Multiplication by a 3d direction
		template <class Number>
		inline void transform_direction_3d  (vector3<Number> &r, const matrix4x4<Number>& m, const vector3<Number>& v)
		{
			r = vector3<Number>(
				v.x*m._11 + v.y*m._21 + v.z*m._31,
				v.x*m._12 + v.y*m._22 + v.z*m._32,
				v.x*m._13 + v.y*m._23 + v.z*m._33
				);
		}

		template <class Number>
		inline void transpose(matrix4x4<Number>& m, const matrix4x4<Number>& n)
		{
			for (int x = 0; x<m.num_rows(); x++)
				for (int y = 0; y<m.num_cols(); y++)
					m.n[x][y] = n.n[y][x];
		}

		template <class Number>
		void inverse(matrix4x4<Number>& m, const matrix4x4<Number>& n)
		{
			codex::math::matrix::matrix_inverse<Number, vector4<Number>, matrix4x4<Number> >(m, n);
		}

	//*****************************************************************************
	// matrix4x4 class and non-member functions definition
	//*****************************************************************************

		template <class Number>
		class matrix4x4
		{
		private:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------

			const int	n_rows, n_cols;

			//-----------------------------------------------------------------------------
			//Dummy interface to keep compatible with matrix class
			//-----------------------------------------------------------------------------
			inline bool alloc(const int rows, const int cols)
			{
				return true;
			}

			//basic matrix operations
			//-----------------------------------------------------------------------------
			inline void exchange_row(const int i, const int j)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_rows - 1 || j < 0 || j > n_cols - 1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix4x4::exchange_row");
#endif
					Number temp[4];

					memcpy(temp, n[i], sizeof(Number)*n_cols);
					memcpy(n[i], n[j], sizeof(Number)*n_cols);
					memcpy(n[j], temp, sizeof(Number)*n_cols);
				}
			}

			inline void exchange_row(const int i, const int j, const int start_col, const int finish_col)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_rows - 1 || j < 0 || j > n_cols - 1 || 
						start_col < 0 || start_col > n_cols-1 || finish_col < 0 || finish_col > n_cols-1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix4x4::exchange_row");
#endif
					Number temp[4];

					memcpy(&temp[start_col], &n[i][start_col], sizeof(Number)*(finish_col-start_col+1));
					memcpy(&n[i][start_col], &n[j][start_col], sizeof(Number)*(finish_col-start_col+1));
					memcpy(&n[j][start_col], &temp[start_col], sizeof(Number)*(finish_col-start_col+1));
				}
			}

			inline void exchange_column(const int i, const int j)
			{
				if (i != j)
				{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
					if (i <0 || i > n_cols || j < 0 || j > n_cols - 1)
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix4x4::exchange_column");
#endif
					for (int k = 0; k<n_rows; k++)
					{
						Number temp;

						temp = n[k][i];
						n[k][i] = n[k][j];
						n[k][j] = temp;
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
						throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix4x4::exchange_column");
#endif
					for (int k = start_row; k<=finish_row; k++)
					{
						Number temp;

						temp = n[k][i];
						n[k][i] = n[k][j];
						n[k][j] = temp;
					}
				}
			}

			//-----------------------------------------------------------------------------
			//friends
			//-----------------------------------------------------------------------------
			friend class matrix4x4<Number>;
			friend void codex::math::matrix::matrix_inverse<Number, vector4<Number>, matrix4x4<Number> >(matrix4x4<Number>& m, const matrix4x4<Number>& n);

			friend void codex::math::matrix::LU_decompose<Number, matrix4x4<Number> >(matrix4x4<Number> &a, const matrix4x4<Number> &mat, int *indx, Number &d);
			friend void codex::math::matrix::LU_back_substitue<Number, vector4<Number>, matrix4x4<Number> >(vector4<Number> &b, const matrix4x4<Number> &a, int *indx, const vector4<Number> &v);

		public:
			//-----------------------------------------------------------------------------
			//data members
			//-----------------------------------------------------------------------------
			union {
				struct {
					Number	_11, _12, _13, _14;
					Number	_21, _22, _23, _24;
					Number	_31, _32, _33, _34;
					Number	_41, _42, _43, _44;
				};
				Number m[16];
				Number n[4][4];
			};

			//-----------------------------------------------------------------------------
			//constructors
			//-----------------------------------------------------------------------------
			matrix4x4() 
			: n_rows(4), n_cols(4)
			{	
				memset(m, 0, sizeof(m)); 
			}

			matrix4x4(	const Number _11, const Number _12, const Number _13, const Number _14, 
						const Number _21, const Number _22, const Number _23, const Number _24, 
						const Number _31, const Number _32, const Number _33, const Number _34, 
						const Number _41, const Number _42, const Number _43, const Number _44)
					:	_11(_11), _12(_12), _13(_13), _14(_14),
						_21(_21), _22(_22), _23(_23), _24(_24),
						_31(_31), _32(_32), _33(_33), _34(_34),
						_41(_41), _42(_42), _43(_43), _44(_44),
						n_rows(4), n_cols(4)
			{
			}

			matrix4x4(const Number *n)
			: n_rows(4), n_cols(4)
			{ 
				memcpy(m, n, sizeof(m));
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
				matrix4x4<Number> temp;
				
				codex::math::matrix::transpose(temp, *this);

				*this = temp;
			}

			inline matrix4x4<Number> transposed_matrix() const
			{
				matrix4x4<Number> temp;

				codex::math::matrix::transpose(temp, *this);

				return temp;
			}

			inline void inverse()
			{
				codex::math::matrix::inverse(*this, *this);
			}

			inline matrix4x4<Number> inversed_matrix() const
			{
				matrix4x4<Number> temp;

				codex::math::matrix::inverse(temp, *this);

				return temp;
			}

			inline Number determinant() const
			{
				int				*indx = NULL;

				try
				{
					matrix4x4<Number>	a;
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
			}
			//-----------------------------------------------------------------------------
			//access operators
			//-----------------------------------------------------------------------------
			inline Number& operator () (int row, int col)
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (row <1 || row > n_rows || col < 1 || col > n_cols)
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix4x4::operator ()");
	#endif
				return n[row-1][col-1];
			}

			inline Number operator () (int row, int col) const
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (row <1 || row > n_rows || col < 1 || col > n_cols)
					throw_error(ERROR_POS, EXP_OUT_OF_BOUND, "matrix4x4::operator () const");
	#endif
				return n[row-1][col-1];
			}


			//-----------------------------------------------------------------------------
			//assignment operators
			//-----------------------------------------------------------------------------
			inline matrix4x4<Number>& operator = (const matrix4x4<Number>& mat)
			{
				if (&mat != this)
					memcpy(n, mat.n, sizeof(n));
				return *this;
			}

			inline matrix4x4<Number>& operator += (const matrix4x4<Number>& mat)
			{
				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						n[x][y] += mat.n[x][y];
				return *this;
			}

			inline matrix4x4<Number>& operator -= (const matrix4x4<Number>& mat)
			{
				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						n[x][y] -= mat.n[x][y];
				return *this;
			}

			inline matrix4x4<Number>& operator *= (const matrix4x4<Number>& mat)
			{
				matrix4x4<Number> temp = (*this) * mat;
				*this = temp;
				return *this;
			}

			inline matrix4x4<Number>& operator *= (const Number s)
			{
				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						m[x*n_cols+y] *= s;
				return *this;
			}

			inline matrix4x4<Number>& operator /= (const Number s)
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "matrix4x4::operator /");
	#endif

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						m[x*n_cols+y] /= s;
				return *this;
			}
			//-----------------------------------------------------------------------------
			//unary operators
			//-----------------------------------------------------------------------------
			inline matrix4x4<Number> operator + () const
			{
				return matrix4x4<Number>(m);
			}

			inline matrix4x4<Number> operator - () const
			{
				matrix4x4<Number> temp;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						temp.n[x][y] = -n[x][y];

				return temp;
			}

			//-----------------------------------------------------------------------------
			//binary operators
			//-----------------------------------------------------------------------------
			inline matrix4x4<Number> operator + (const matrix4x4<Number>& mat) const
			{
				matrix4x4<Number> temp;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						temp.n[x][y] = n[x][y] + mat.n[x][y];

				return temp;
			}

			inline matrix4x4<Number> operator - (const matrix4x4<Number>& mat) const
			{
				matrix4x4<Number> temp;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						temp.n[x][y] = n[x][y] - mat.n[x][y];

				return temp;
			}

			inline matrix4x4<Number> operator * (const matrix4x4<Number>& mat) const
			{
				matrix4x4<Number> temp;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						temp.n[x][y] = n[x][0]*mat.n[0][y] + n[x][1]*mat.n[1][y] + n[x][2]*mat.n[2][y] + n[x][3]*mat.n[3][y];

				return temp;
			}
			
			inline vector4<Number> operator * (const vector4<Number>& v) const
			{
				return vector4<Number>(
					v.x*_11 + v.y*_12 + v.z*_13 + v.w*_14,
					v.x*_21 + v.y*_22 + v.z*_23 + v.w*_24,
					v.x*_31 + v.y*_32 + v.z*_33 + v.w*_34,
					v.x*_41 + v.y*_42 + v.z*_43 + v.w*_44
					);
			}

			inline vector3<Number> operator * (const vector3<Number>& v) const
			{
				Number inv_w = 1 / (v.x*_41 + v.y*_42 + v.z*_43 + _44);

				return vector3<Number>(
					(v.x*_11 + v.y*_12 + v.z*_13 + _14) * inv_w,
					(v.x*_21 + v.y*_22 + v.z*_23 + _24) * inv_w,
					(v.x*_31 + v.y*_32 + v.z*_33 + _34) * inv_w
					);
			}

			inline matrix4x4<Number> operator * (const Number s) const
			{
				matrix4x4<Number> temp;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						temp.n[x][y] = n[x][y] * s;

				return temp;
			}

			inline matrix4x4<Number> operator / (const Number s) const
			{
	#if defined(CODEX_MATH_DEBUG_VERBOSE)
				if (codex::math::utils::equal<Number>(s, 0, RELATIVE_PRECISION))
					throw_error(ERROR_POS, EXP_DIVIDED_BY_ZERO, "matrix4x4::operator /");
	#endif
				matrix4x4<Number> temp;

				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						temp.n[x][y] = n[x][y] / s;

				return temp;
			}

			inline bool operator == (const matrix4x4<Number>& mat) const
			{
				for (int x = 0; x<n_rows; x++)
					for (int y = 0; y<n_cols; y++)
						if (codex::math::utils::not_equal(n[x][y], mat.n[x][y], RELATIVE_PRECISION))
							return false;

				return true;
			}

			inline bool operator != (const matrix4x4<Number>& n) const
			{
				return !(*this == n);
			}

		};
	}
}
}

//			inline void add_row(const int to, const int from)
//			{
//#if defined(CODEX_MATH_DEBUG_VERBOSE)
//				if (to <0 || to > 3 || from < 0 || from > 3)
//					throw_error(ERROR_POS, "matrix4x4::add_row", EXP_OUT_OF_BOUND);
//#endif
//				for (int k = 0; k<4; k++)
//				{
//					m[to][k] += m[from][k];
//				}
//			}
//
//			inline void add_row(const int to, const int from, const int start_col, const int finish_col)
//			{
//#if defined(CODEX_MATH_DEBUG_VERBOSE)
//				if (to <0 || to > 3 || from < 0 || from > 3 ||
//					start_col < 0 || start_col > 3 || finish_col < 0 || finish_col > 3)
//					throw_error(ERROR_POS, "matrix4x4::add_row", EXP_OUT_OF_BOUND);
//#endif
//				for (int k = start_col; k<=finish_col; k++)
//				{
//					m[to][k] += m[from][k];
//				}
//			}
//
//			inline void add_row_scaled(const int to, const int from, const Number s)
//			{
//#if defined(CODEX_MATH_DEBUG_VERBOSE)
//				if (to <0 || to > 3 || from < 0 || from > 3)
//					throw_error(ERROR_POS, "matrix4x4::add_row_scaled", EXP_OUT_OF_BOUND);
//#endif
//				for (int k = 0; k<4; k++)
//				{
//					m[to][k] += m[from][k]*s;
//				}
//			}
//
//			inline void add_row_scaled(const int to, const int from, const Number s, const int start_col, const int finish_col)
//			{
//#if defined(CODEX_MATH_DEBUG_VERBOSE)
//				if (to <0 || to > 3 || from < 0 || from > 3 ||
//					start_col < 0 || start_col > 3 || finish_col < 0 || finish_col > 3)
//					throw_error(ERROR_POS, "matrix4x4::add_row_scaled", EXP_OUT_OF_BOUND);
//#endif
//				for (int k = start_col; k<=finish_col; k++)
//				{
//					m[to][k] += m[from][k]*s;
//				}
//			}
//
//			inline void multiply_row(const int i, const Number s)
//			{
//#if defined(CODEX_MATH_DEBUG_VERBOSE)
//				if (i <0 || i > 3)
//					throw_error(ERROR_POS, "matrix4x4::multiply_row", EXP_OUT_OF_BOUND);
//#endif
//				for (int k = 0; k<4; k++)
//				{
//					m[i][k] *= s;
//				}
//			}
//
//			inline void multiply_row(const int i, const Number s, const int start_col, const int finish_col)
//			{
//#if defined(CODEX_MATH_DEBUG_VERBOSE)
//				if (i <0 || i > 3 ||
//					start_col < 0 || start_col > 3 || finish_col < 0 || finish_col > 3)
//					throw_error(ERROR_POS, "matrix4x4::multiply_row", EXP_OUT_OF_BOUND);
//#endif
//				for (int k = start_col; k<=finish_col; k++)
//				{
//					m[i][k] *= s;
//				}
//			}


//template <class Number>
//void inverse(matrix4x4<Number>& m, const matrix4x4<Number>& n)
//{
//	matrix4x4<Number> temp = n;

//	identity(m);

//	Number s[4];

//	for (int i = 0; i<3; i++)
//	{
//		for (int x = i; x<4; x++)
//		{
//			s[x] = 0;
//			for (int y = i; y<4; y++)
//			{
//				s[x] = max(s[x], abs(temp.n[x][y]));
//			}

//			if (codex::math::utils::equal<Number, RELATIVE_PRECISION>(s[x], 0))
//				throw_error(ERROR_POS, "matrix4x4::inverse", EXP_NON_INVERTIBLE_MATRIX);
//		}


//		Number	max_col_value	= 0;
//		int		max_row			= -1;

//		for (int j = i; j<4; j++)
//			if (abs(temp.n[j][i] / s[j]) > max_col_value)
//			{
//				max_col_value	= abs(temp.n[j][i] / s[j]);
//				max_row			= j;
//			}

//		temp.exchange_row(max_row, i, i, 3);
//		m.exchange_row(max_row, i);

//		Number head = temp.n[i][i];

//		temp.nultiply_row(i, 1/head, i, 3);
//		m.nultiply_row(i, 1/head);

//		for (int j = i+1; j<4; j++)
//		{
//			Number head = temp.n[j][i];
//			temp.add_row_scaled(j, i, -head, i, 3);
//			m.add_row_scaled(j, i, -head);
//		}
//	}

//	Number head = temp.n[3][3];
//	temp.nultiply_row(3, 1/head, 3, 3);
//	m.nultiply_row(3, 1/head);

//	for (int i = 3; i>=1; i--)
//		for (int j = i-1; j>=0; j--)
//		{
//			Number tail = temp.n[j][i];

//			temp.add_row_scaled(j, i, -tail, i, i);
//			m.add_row_scaled(j, i, -tail);
//		}
//}

////with scaled partial pivoting
//inline Number determinant() const
//{
//	matrix4x4<Number> temp = *this;
//	Number s[4];
//	Number det = 1;

//	for (int i = 0; i<3; i++)
//	{
//		for (int x = i; x<4; x++)
//		{
//			s[x] = 0;
//			for (int y = i; y<4; y++)
//			{
//				s[x] = max(s[x], abs(temp.n[x][y]));
//			}

//			if (codex::math::utils::equal<Number, RELATIVE_PRECISION>(s[x], 0)) return 0;
//		}


//		Number	max_col_value	= 0;
//		int		max_row			= -1;

//		for (int j = i; j<4; j++)
//			if (abs(temp.n[j][i] / s[j]) > max_col_value)
//			{
//				max_col_value	= abs(temp.n[j][i] / s[j]);
//				max_row			= j;
//			}

//		temp.exchange_row(max_row, i, i, 3);

//		det *= temp.n[i][i];

//		for (int j = i+1; j<4; j++)
//		{
//			temp.add_row_scaled(j, i, -temp.n[j][i]/temp.n[i][i], i, 3);
//		}
//	}

//	det *= temp.n[3][3];

//	return det;
//}


