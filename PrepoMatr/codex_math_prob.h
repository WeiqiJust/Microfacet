//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   03-03-2007
//  File:         codex_math_prob.h
//  Content:      codex_math probability and statistics related functions
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
          OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma once

#include <vector>
#include <cassert>
#include "codex_math_matrix4x4.h"
#include "codex_math_utils.h"
namespace codex
{
namespace math
{
	namespace prob
	{

#define PI 3.1415926
		using codex::math::vector::vector2;
		using codex::math::vector::vector3;
		using codex::math::matrix::matrix4x4;
		using codex::math::utils::rand_real;
		using codex::math::utils::random_number_generator;

		//*****************************************************************************
		// probability distributions
		//*****************************************************************************

		template <class Number>
		void normal_rng_pair(Number &a, Number &b, random_number_generator<Number> &rng)
		{
			Number w, x1, x2;

			do {
				x1 = 2*rng.rand_real() - 1,
				x2 = 2*rng.rand_real() - 1;
				w = x1 * x1 + x2 * x2;
			} while (w >= 1 || w == 0);

			w = sqrt((-2*log(w)) / w);
			a = x1 * w;
			b = x2 * w;
		}

		template <class Number, class vec>
		class prob_distribution
		{
		public:
			virtual void	sample(vec &v, Number &pdf, const vec &rv) const = 0;
			//virtual Number	pdf(vec &v) const = 0;
		};


		template <class Number>
		class step_1D_prob
		{
		//private:
		public:
			std::vector<Number>	CDF;

			step_1D_prob() {}

			step_1D_prob(const std::vector<Number> &func)
			{
				Number total = 0;

				//FIX ME: check for 0-length and negative values
				CDF.resize(func.size()+1);
				CDF[0] = 0;
				for (int i = 1; i<(int)CDF.size(); i++)
				{
					CDF[i] = func[i-1];
					total += CDF[i];
					CDF[i] += CDF[i-1];
				}

				//Normalize
				for (int i = 0; i<(int)CDF.size(); i++)
				{
					CDF[i] /= total;
				}
			}

			virtual void sample(int &v, Number &pdf, const Number rv) const
			{
				if (CDF.size() == 0)
				{
					v	= -1;
					pdf = 0;
					return;
				}

				if (rv == 0)
				{
					v	= 0;				
					pdf = CDF[1] - CDF[0];
					return;
				}

				int l = 0, r = (int)CDF.size()-1, m;

				while (l <= r)
				{
					m = (l+r) / 2;

					if (rv <= CDF[m])
					{
						r = m-1;
					} else if (rv > CDF[m+1])
					{
						l = m+1;
					} else {
						v	= m;
						pdf = CDF[m+1] - CDF[m];
						return;
					}
				}
			}

			Number pdf(const int i) const
			{
				if (i >= 0 && i < (int)CDF.size()-1)
				{
					return CDF[i+1] - CDF[i];
				} else {
					//FIX ME: report error here
					return 0;
				}
			}

			void load(FILE* &fp)
			{
				int	size;

				fread(&size, sizeof(size), 1, fp);
				CDF.resize(size);
				for (int i = 0; i < size; i++)
					fread(&CDF[i], sizeof(Number), 1, fp);
			}

			void save(FILE* &fp) const
			{
				int	size = (int)CDF.size();

				fwrite(&size, sizeof(size), 1, fp);
				for (int i = 0; i < size; i++)
					fwrite(&CDF[i], sizeof(Number), 1, fp);
			}
		};

		//template <class Number>
		//void uniform_sample_rectangle(vector2<Number> &v, Number x = 1, Number y = 1)
		//{
		//	v.x = rand_real() * x;
		//	v.y = rand_real() * y;
		//}

		template <class Number>
		class uniform_rectangle : public prob_distribution<Number, vector2<Number> >
		{
		private:
			Number x, y;
		public:
			uniform_rectangle(const Number x = 1, const Number y = 1) 
				: x(x), y(y)
			{
			}

			virtual void sample(vector2<Number> &v, Number &pdf, const vector2<Number> &rv) const
			{
				v.x = rv.x * x;
				v.y = rv.y * y;
				pdf = 1 / (x*y);
			}

			//virtual Number pdf(vector2<Number> &v) const
			//{
			//	return 1 / (x*y);
			//}
		};

		template <class Number>
		class uniform_box : public prob_distribution<Number, vector3<Number> >
		{
		private:
			Number x, y, z;
		public:
			uniform_box(const Number x = 1, const Number y = 1, const Number z = 1) 
				: x(x), y(y), z(z)
			{
			}

			virtual void sample(vector3<Number> &v, Number &pdf, const vector3<Number> &rv) const
			{
				v.x = rv.x * x;
				v.y = rv.y * y;
				v.z = rv.z * z;
				pdf = 1 / (x*y*z);
			}

			//virtual Number pdf(vector3<Number> &v) const
			//{
			//	return 1 / (x*y*z);
			//}
		};


		template <class Number>
		class uniform_disk : public prob_distribution<Number, vector2<Number> >
		{
		private:
			Number radius;
		public:
			uniform_disk(const Number radius = 1) : radius(radius)	{}

			virtual void sample(vector2<Number> &v, Number &pdf, const vector2<Number> &rv) const
			{
				Number r, theta;

				r		= (Number)sqrt(rv.x) * radius;
				theta	= rv.y * 2 * (Number)PI;

				v.x		= r * cos(theta);
				v.y		= r * sin(theta);
				pdf		= 1 / (radius*radius*(Number)PI);
			}

			//virtual Number pdf(vector2<Number> &v) const
			//{
			//	return 1 / (radius*radius*PI);
			//}
		};

		template <class Number>
		class uniform_direction_2d : public prob_distribution<Number, vector2<Number> >
		{
		public:
			uniform_direction_2d()	{}

			virtual void sample(vector2<Number> &v, Number &pdf, const vector2<Number> &rv) const
			{
				Number theta;

				theta	= rv.x * 2 * (Number)PI;
				v.x		= cos(theta);
				v.y		= sin(theta);
				pdf		= 1 / (2*(Number)PI);
			}

			//virtual Number pdf(vector2<Number> &v) const
			//{
			//	return 1 / (2*PI);
			//}
		};

		template <class Number>
		class uniform_direction_3d : public prob_distribution<Number, vector3<Number> >
		{
		public:
			uniform_direction_3d()	{}

			virtual void sample(vector3<Number> &v, Number &pdf, const vector3<Number> &rv) const
			{
				Number	z, r, theta;

				z		= 1 - rv.x * 2;
				r		= (Number)sqrt(1 - z*z);
				theta	= rv.y * 2 * (Number)PI;
				v.x		= r * cos(theta);
				v.y		= r * sin(theta);
				v.z		= z;
				pdf		= 1 / (4*(Number)PI);
			}

			//virtual Number pdf(vector3<Number> &v) const
			//{
			//	return 1 / (4*PI);
			//}
		};

		template <class Number>
		class uniform_hemi_direction_3d : public prob_distribution<Number, vector3<Number> >
		{
		public:
			uniform_hemi_direction_3d()	{}

			virtual void sample(vector3<Number> &v, Number &pdf, const vector3<Number> &rv) const
			{
				Number	z, r, theta;

				z		= rv.x;
				r		= (Number)sqrt(1 - z*z);
				theta	= rv.y * 2 * (Number)PI;
				v.x		= r * cos(theta);
				v.y		= r * sin(theta);
				v.z		= z;
				pdf		= 1 / (2*(Number)PI);
			}

			Number pdf(vector3<Number> &v) const
			{
				return 1 / (2*(Number)PI);
			}
		};

		template <class Number>
		class uniform_cos_hemi_direction_3d : public prob_distribution<Number, vector3<Number> >
		{
			//private:
			//uniform_disk<Number> disk_dist;
		public:
			uniform_cos_hemi_direction_3d()// : disk_dist(1) 
			{}

			virtual void sample(vector3<Number> &v, Number &pdf, const vector3<Number> &rv) const
			{
				vector2<Number> xy;

				// Map uniform random numbers to $[-1,1]^2$
				Number sx = 2 * rv.x - 1;
				Number sy = 2 * rv.y - 1;
				// Map square to $(r,\theta)$
				// Handle degeneracy at the origin
				if (sx == 0.0 && sy == 0.0) 
				{
				} else {
					Number r, theta;

					if (sx >= -sy) {
						if (sx > sy) {
							// Handle first region of disk
							r = sx;
							if (sy > 0)
								theta = sy/r;
							else
								theta = 8 + sy/r;
						}
						else {
							// Handle second region of disk
							r = sy;
							theta = 2 - sx/r;
						}
					}
					else {
						if (sx <= sy) {
							// Handle third region of disk
							r = -sx;
							theta = 4 - sy/r;
						}
						else {
							// Handle fourth region of disk
							r = -sy;
							theta = 6 + sx/r;
						}
					}
					theta *= (Number)PI / (Number)4.0;
					xy.x = r*cos(theta);
					xy.y = r*sin(theta);
				}

				//Number			temp;

				//disk_dist.sample(xy, temp, vector2<Number>(rv.x, rv.y));

				v.x = xy.x;
				v.y = xy.y;
				v.z = (Number)sqrt(1 - v.x*v.x - v.y*v.y);
				pdf = v.z / (Number)PI;
			}

			Number pdf(vector3<Number> &v) const
			{
				return v.z / (Number)PI;
			}
		};


		template <class Number>
		class uniform_cos_cone_direction_3d : public prob_distribution<Number, vector3<Number> >
		{
		private:
			const Number cos_theta_max;
		public:
			uniform_cos_cone_direction_3d() : cos_theta_max(0) {}
			uniform_cos_cone_direction_3d(const Number cos_theta) : cos_theta_max(cos_theta) {}

			virtual void sample(vector3<Number> &v, Number &pdf, const vector3<Number> &rv) const
			{
				Number cos_theta = codex::math::utils::lerp<Number>(rv.x, cos_theta_max, 1);
				Number sin_theta = sqrt(1 - cos_theta*cos_theta);
				Number phi = rv.y * 2 * (Number)PI;

				v.x = cos(phi) * sin_theta;
				v.y = sin(phi) * sin_theta;
				v.z = cos_theta;

				pdf = (Number)1.0 / (Number)(2.0 * (Number)PI * (1.0 - cos_theta_max));
			}
		};


		template <class Number>
		class uniform_triangle_barycentric : public prob_distribution<Number, vector2<Number> >
		{
		public:
			uniform_triangle_barycentric()	{}

			virtual void sample(vector2<Number> &v, Number &pdf, const vector2<Number> &rv) const
			{
				Number rt_u = (Number)sqrt(rv.x);

				v.x = 1 - rt_u;
				v.y = rv.y * rt_u;
			}

			//FIX ME:
			//virtual Number pdf(vector2<Number> &v)
			//{
			//	return 1 / (2*PI);
			//}
		};

		//*****************************************************************************
		// probability distribution transformations
		//*****************************************************************************
		template <class Number>
		void transform_hemi_direction_3d(vector3<Number> &p, const vector3<Number> &n, 
				const vector3<Number> &orig_p, const vector3<Number> &nz = vector3<Number>(0, 0, 1))
		{
			vector3<Number> axis;
			Number			axis_len;
			
			axis		= nz ^ n;
			axis_len	= axis.normalize();

			if (codex::math::utils::equal<Number>(axis_len, 0, RELATIVE_PRECISION))
			{
				if (n*nz > 0)
					p = orig_p;
				else
					p = -orig_p;
				return;
			} else {
				Number		angle = acos(nz * n);
				matrix4x4<Number>
							mat_rot;

				if (angle < 0) angle = angle + (Number)PI;

				rotation_axis(mat_rot, axis, angle);

				p = mat_rot * orig_p;
			}
		}

		//*****************************************************************************
		// heuristic weighting functions for multiple importance sampling
		//*****************************************************************************
		template <class Number>
		inline Number MIS_balance_heuristic(const int nf, const Number pdf_f, const int ng, const Number pdf_g)
		{
			return (nf * pdf_f) / (nf * pdf_f + ng * pdf_g);
		}
		
		// beta = 2
		template <class Number>
		inline Number MIS_power_heuristic_beta_2(const int nf, const Number pdf_f, const int ng, const Number pdf_g)
		{
			Number f = nf * pdf_f;
			Number g = ng * pdf_g;

			return (f*f) / (f*f + g*g);
		}

		template <class Number>
		void generate_permutation(std::vector<int> &perm,
			const int n,
			random_number_generator<Number> &rng,
			const int len = -1)
		{
			assert(len <= n);

			perm.resize(n);
			for (int i = 0; i < n; i++)
				perm[i] = i;

			for (int i = 0; i < ((len <= 0) ? n-1 : min(len, n-1)); i++)
			{
				int p = i+(int)(rng.rand_real_open()*(n-i));
				int temp;
				temp = perm[i];
				perm[i] = perm[p];
				perm[p] = temp;
			}

			if (len > 0)
				perm.resize(len);
		}
	}
}
}