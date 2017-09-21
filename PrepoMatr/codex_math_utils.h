//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   03-03-2007
//  File:         codex_math_utils.h
//  Content:      codex_math helper functions
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
          OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


//////////////////////////////////////////////////////////////////////////////
// Our library uses the following code:
//////////////////////////////////////////////////////////////////////////////
// Random Number State
/*
Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The names of its contributors may not be used to endorse or promote
products derived from this software without specific prior written
permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "codex_math_vector.h"
#include "codex_math_vector2.h"
#include "codex_math_vector3.h"
#include "codex_math_vector4.h"
namespace codex
{
namespace math
{
	namespace utils
	{
		using codex::math::vector::vector2;
		using codex::math::vector::vector3;
		using codex::math::vector::vector4;
		using codex::math::vector::vector;

		const int	QUASI_MONTE_CARLO_MAX_DIM = 10;
		const int	prime_quasi[QUASI_MONTE_CARLO_MAX_DIM] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

		//basic operators

		//function declarations changed for Intel compiler
		template <class Number>
		inline bool equal(const Number a, const Number b, const Number precision)
		{
			if (b>1)
				return abs(a-b) <= precision*b;
			else
				return abs(a-b) <= precision;
		}

		template <class Number>
		inline bool not_equal(const Number a, const Number b, const Number precision)
		{
			if (b>1)
				return abs(a-b) > precision*b;
			else
				return abs(a-b) > precision;
		}

		//template <class Number, Number precision>
		//inline bool equal(const Number a, const Number b)
		//{
		//	if (b>1)
		//		return abs(a-b) <= precision*b;
		//	else
		//		return abs(a-b) <= precision;
		//}

		//template <class Number, Number precision>
		//inline bool not_equal(const Number a, const Number b)
		//{
		//	if (b>1)
		//		return abs(a-b) > precision*b;
		//	else
		//		return abs(a-b) > precision;
		//}

		template <class Number>
		inline Number sgn(const Number a, const Number precision)
		{
			if (a > - precision && a < precision)
				return 0;
			else if (a > 0)
				return 1;
			else
				return -1;
		}

		template <class Number>
		inline Number clamp(const Number a, const Number low, const Number high)
		{
			if (a > high)
				return high;
			else if (a<low)
				return low;
			else
				return a;
		}

		template <class Number>
		inline Number lerp(const Number t, const Number a1, const Number a2)
		{
#if defined(CODEX_MATH_DEBUG_VERBOSE)
			if (t < 0 || t > 1)
				throw_error(ERROR_POS, EXP_OUT_OF_RANGE, "utils::lerp");
#endif
			return (1-t) * a1 + t * a2;
		}

		//conversion utilities
		template <class Number>
		inline Number radian_to_degree(const Number radian)
		{
			return radian / PI * 180;
		}

		template <class Number>
		inline Number degree_to_radian(const Number degree)
		{
			return degree / 180 * PI;
		}

		// random number utilities

		// Below are thread-unsafe routines
		void			rand_seed(unsigned long seed);
		unsigned long	rand_int();

		template <class Number>
		inline Number rand_real_open() // [0, 1)
		{
			return (Number)rand_int() / (Number)4294967296.0;
		}

		template <class Number>
		inline Number rand_real() // [0, 1]
		{
			return (Number)rand_int() / (Number)4294967295.0;
		}

		// For thread-safety concern, use random number generator class
		class random_number_generator_state
		{
		public:
			static const int N = 624;

			unsigned long mt[N];	/* the array for the state vector  */
			int mti;				/* mti==N+1 means mt[N] is not initialized */
		};

		template <class Number>
		class random_number_generator
		{
		private:
			static const int N = 624;
			static const int M = 397;
			static const unsigned long MATRIX_A		= 0x9908b0dfUL;	/* constant vector a */
			static const unsigned long UPPER_MASK	= 0x80000000UL;	/* most significant w-r bits */
			static const unsigned long LOWER_MASK	= 0x7fffffffUL; /* least significant r bits */

			unsigned long mt[N];	/* the array for the state vector  */
			int mti;				/* mti==N+1 means mt[N] is not initialized */

			unsigned long	rand_int()
			{
				unsigned long y;
				static unsigned long mag01[2]={0x0UL, MATRIX_A};
				/* mag01[x] = x * MATRIX_A  for x=0,1 */

				if (mti >= N) { /* generate N words at one time */
					int kk;

					if (mti == N+1)   /* if init_genrand() has not been called, */
						rand_seed(5489UL); /* default initial seed */

					for (kk=0;kk<N-M;kk++) {
						y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
						mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
					}
					for (;kk<N-1;kk++) {
						y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
						mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
					}
					y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
					mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

					mti = 0;
				}

				y = mt[mti++];

				/* Tempering */
				y ^= (y >> 11);
				y ^= (y << 7) & 0x9d2c5680UL;
				y ^= (y << 15) & 0xefc60000UL;
				y ^= (y >> 18);

				return y;
			}

		public:
			random_number_generator(unsigned long seed = 0x0272408)
			: mti(N+1)
			{
				rand_seed(seed);
			}

			void rand_seed(unsigned long seed)
			{
				mt[0]= seed & 0xffffffffUL;
				for (mti=1; mti<N; mti++) {
					mt[mti] =
						(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
					/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
					/* In the previous versions, MSBs of the seed affect   */
					/* only MSBs of the array mt[].                        */
					/* 2002/01/09 modified by Makoto Matsumoto             */
					mt[mti] &= 0xffffffffUL;
					/* for >32 bit machines */
				}
			}

			inline Number rand_real_open() // [0, 1)
			{
				return (Number)rand_int() / (Number)4294967296.0;
			}

			inline Number rand_real() // [0, 1]
			{
				return (Number)rand_int() / (Number)4294967295.0;
			}

			//for debug purpose only
			void load_state(const random_number_generator_state &state)
			{
				mti = state.mti;
				memcpy(mt, state.mt, sizeof(mt));
			}

			void save_state(random_number_generator_state &state)
			{
				state.mti = mti;
				memcpy(state.mt, mt, sizeof(mt));
			}
		};


		//low-discrepancy sequence generator

		template <class Number>
		inline Number radical_inverse(int n, int base)
		{
			Number val = 0;
			Number inv_base = (Number)1 / base, inv_bi = inv_base;

			while (n > 0)
			{
				int d_i = n % base;
				val += d_i * inv_bi;
				n /= base;
				inv_bi *= inv_base;
			}

			return val;
		}

		template <class Number>
		inline Number folded_radical_inverse(int n, int base)
		{
			Number	val = 0;
			Number	inv_base = (Number)1 / base, inv_bi = inv_base;
			int		mod_offset = 0;

			while (val + base * inv_bi != val)
			{
				int digit = (n + mod_offset) % base;

				val		+= digit * inv_bi;
				n		/= base;
				inv_bi	*= inv_base;
				mod_offset++;
			}

			return val;
		}

		template<class Number>
		class Hammersley_seq
		{
		private:
			int	i, n;

		public:
			Hammersley_seq(int n) : n(n), i(0) {};

			void get_sample(Number &r)
			{
				r = (Number) i / n;
				i++;
			}

			void get_sample(vector2<Number> &r)
			{
				r.x = (Number) i / n;
				r.y = radical_inverse<Number>(i, 2);
				i++;
			}

			void get_sample(vector3<Number> &r)
			{
				r.x = (Number) i / n;
				r.y = radical_inverse<Number>(i, 2);
				r.z = radical_inverse<Number>(i, 3);
				i++;
			}

			void get_sample(vector4<Number> &r)
			{
				r.x = (Number) i / n;
				r.y = radical_inverse<Number>(i, 2);
				r.z = radical_inverse<Number>(i, 3);
				r.w = radical_inverse<Number>(i, 5);
				i++;
			}

			void get_sample(vector<Number> &r, const int dim)
			{
				if (dim > 0 && dim <= QUASI_MONTE_CARLO_MAX_DIM)
				{
					vector<Number> v(dim);

					v(1) = (Number) i / n;
					for (int j = 2; j <= dim; j++)
						v(j) = radical_inverse<Number>(i, prime_quasi[j-2]);
					r = v;

					i++;
				} else {
					//FIX ME : report error
				}
			}
		};

		template<class Number>
		class Hammersley_Zaremba_seq
		{
		private:
			int	i, n;

		public:
			Hammersley_Zaremba_seq(int n) : n(n), i(0) {};

			void get_sample(Number &r)
			{
				r = (Number) i / n;
				i++;
			}

			void get_sample(vector2<Number> &r)
			{
				r.x = (Number) i / n;
				r.y = folded_radical_inverse<Number>(i, 2);
				i++;
			}

			void get_sample(vector3<Number> &r)
			{
				r.x = (Number) i / n;
				r.y = folded_radical_inverse<Number>(i, 2);
				r.z = folded_radical_inverse<Number>(i, 3);
				i++;
			}

			void get_sample(vector4<Number> &r)
			{
				r.x = (Number) i / n;
				r.y = folded_radical_inverse<Number>(i, 2);
				r.z = folded_radical_inverse<Number>(i, 3);
				r.w = folded_radical_inverse<Number>(i, 5);
				i++;
			}

			void get_sample(vector<Number> &r, const int dim)
			{
				if (dim > 0 && dim <= QUASI_MONTE_CARLO_MAX_DIM)
				{
					vector<Number> v(dim);

					v(1) = (Number) i / n;
					for (int j = 2; j <= dim; j++)
						v(j) = folded_radical_inverse<Number>(i, prime_quasi[j-2]);
					r = v;

					i++;
				} else {
					//FIX ME : report error
				}
			}
		};

		template<class Number>
		class Halton_seq
		{
		private:
			int	i;

		public:
			Halton_seq(int i = 1) : i(max(i, 1)) {};

			void get_sample(Number &r)
			{
				r = radical_inverse<Number>(i, 2);
				i++;
			}

			void get_sample(vector2<Number> &r)
			{
				r.x = radical_inverse<Number>(i, 2);
				r.y = radical_inverse<Number>(i, 3);
				i++;
			}

			void get_sample(vector3<Number> &r)
			{
				r.x = radical_inverse<Number>(i, 2);
				r.y = radical_inverse<Number>(i, 3);
				r.z = radical_inverse<Number>(i, 5);
				i++;
			}

			void get_sample(vector4<Number> &r)
			{
				r.x = radical_inverse<Number>(i, 2);
				r.y = radical_inverse<Number>(i, 3);
				r.z = radical_inverse<Number>(i, 5);
				r.w = radical_inverse<Number>(i, 7);
				i++;
			}

			void get_sample(vector<Number> &r, const int dim)
			{
				if (dim > 0 && dim <= QUASI_MONTE_CARLO_MAX_DIM)
				{
					vector<Number> v(dim);

					for (int j = 1; j <= dim; j++)
						v(j) = radical_inverse<Number>(i, prime_quasi[j-1]);
					r = v;

					i++;
				} else {
					//FIX ME : report error
				}
			}
		};

		template<class Number>
		class Halton_Zaremba_seq
		{
		private:
			int	i;

		public:
			Halton_Zaremba_seq(int i = 1) : i(max(i, 1)) {};

			void get_sample(Number &r)
			{
				r = folded_radical_inverse<Number>(i, 2);
				i++;
			}

			void get_sample(vector2<Number> &r)
			{
				r.x = folded_radical_inverse<Number>(i, 2);
				r.y = folded_radical_inverse<Number>(i, 3);
				i++;
			}

			void get_sample(vector3<Number> &r)
			{
				r.x = folded_radical_inverse<Number>(i, 2);
				r.y = folded_radical_inverse<Number>(i, 3);
				r.z = folded_radical_inverse<Number>(i, 5);
				i++;
			}

			void get_sample(vector4<Number> &r)
			{
				r.x = folded_radical_inverse<Number>(i, 2);
				r.y = folded_radical_inverse<Number>(i, 3);
				r.z = folded_radical_inverse<Number>(i, 5);
				r.w = folded_radical_inverse<Number>(i, 7);
				i++;
			}

			void get_sample(vector<Number> &r, const int dim)
			{
				if (dim > 0 && dim <= QUASI_MONTE_CARLO_MAX_DIM)
				{
					vector<Number> v(dim);

					for (int j = 1; j <= dim; j++)
						v(j) = folded_radical_inverse<Number>(i, prime_quasi[j-1]);
					r = v;

					i++;
				} else {
					//FIX ME : report error
				}
			}
		};


		/* seedx, seedy is point on [0,1]^2.  x, y is point on radius 1 disk */
		template<class Number>
		void square_to_disk(Number &x, Number &y, const Number &seedx, const Number &seedy)
		{

			Number phi, r;

			Number a = 2*seedx - 1;   /* (a,b) is now on [-1,1]^2 */
			Number b = 2*seedy - 1;

			if (a > -b) {     /* region 1 or 2 */
				if (a > b) {  /* region 1, also |a| > |b| */
					r = a;
					phi = (PI/4) * (b/a);
				}
				else       {  /* region 2, also |b| > |a| */
					r = b;
					phi = (PI/4) * (2 - (a/b));
				}
			}
			else {        /* region 3 or 4 */
				if (a < b) {  /* region 3, also |a| >= |b|, a != 0 */
					r = -a;
					phi = (PI/4) * (4 + (b/a));
				}
				else       {  /* region 4, |b| >= |a|, but a==0 and b==0 could occur. */
					r = -b;
					if (b != 0)
						phi = (PI/4) * (6 - (a/b));
					else
						phi = 0;
				}
			}

			x = r * cos(phi);
			y = r * sin(phi);
		}

		/* diskx, disky is point on radius 1 disk.  x, y is point on [0,1]^2 */
		template<class Number>
		void disk_to_square(Number &x, Number &y, const Number &diskx, const Number &disky)
		{
			Number r = sqrt( diskx*diskx + disky*disky );
			Number phi = atan2( disky, diskx );
			Number a, b;
			if (phi < -PI/4) phi += 2*PI;  /* in range [-pi/4,7pi/4] */
			if (phi < PI/4) {         /* region 1 */
				a = r;
				b = phi * a / (PI/4);
			}
			else if (phi < 3*PI/4) { /* region 2 */
				b = r;
				a = -(phi - PI/2) * b / (PI/4);
			}
			else if (phi < 5*PI/4) { /* region 3 */
				a = -r;
				b = (phi - PI) * a / (PI/4);
			}
			else {                       /* region 4 */
				b = -r;
				a = -(phi - 3*PI/2) * b / (PI/4);
			}

			x = (a + 1) / 2;
			y = (b + 1) / 2;
		}

		template<class Number>
		void build_frame(vector3<Number> &t, vector3<Number> &b, const vector3<Number> &n)
		{
			if (n.z != 1 && n.z != -1)
			{
				t.x = t.y = 0;
				t.z = 1;
			}
			else {
				t.x = 1;
				t.y = t.z = 0;
			}
			t = t ^ n;
			t.normalize();
			b = n ^ t;
		}
	}
}
}