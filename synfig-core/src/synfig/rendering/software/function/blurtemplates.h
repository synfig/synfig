/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/blurtemplates.h
**	\brief Blur Templates Header
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_BLURTEMPLATES_H
#define __SYNFIG_RENDERING_SOFTWARE_BLURTEMPLATES_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include <algorithm>
#include <deque>

#include "array.h"

#include <synfig/angle.h>
#include <synfig/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace software
{

class BlurTemplates
{
public:
	template<typename T>
	static T amplifier_box() { return T(0.25)*sqrt(T(PI)); }
	template<typename T>
	static T amplifier_cross() { return T(1.0); }
	template<typename T>
	static T amplifier_disk() { return T(0.5); }
	template<typename T>
	static T amplifier_gauss() { return T(3.0)/T(8.0); }
	template<typename T>
	static T amplifier_fastgauss() { return amplifier_gauss<T>(); }


	template<typename T>
	struct Abs { T operator() (const T &x) { return abs(x); } };

	template<typename T>
	static T gauss(const T &x, const T &r)
	{
		static const T precision(1e-10);
		static const T k( T(1.0)/sqrt(T(2.0)*PI) );
		if (fabs(r) < precision)
			return fabs(x) < precision ? T(1.0) : T(0.0);
		return k*exp(T(-0.5)*x*x/(r*r))/r;
	}

	template<typename T>
	static T ungauss(const T &x)
	{
		static const T k( T(1.0)/sqrt(T(2.0)*PI) );
		return sqrt(-T(2.0)*log(x/k));
	}

	template<typename T>
	static void fill_pattern_box(const Array<T, 1> &x, const T &size)
	{
		typedef Array<T, 1> A;

		const T precision(1e-10);
		T s(fabs(size));
		int sizei = 1 + (int)floor(s + precision);

		/// assume that pattern already contains zeros
		for(typename A::Iterator i(x, 0, std::min(sizei, x.count)); i; ++i)
			*i = T(1.0);

		// antialiasing
		if (sizei < x.count)
			x[sizei] = T(sizei-1) - s;
	}

	template<typename T>
	static void fill_pattern_gauss(const Array<T, 1> &x, T size)
	{
		if (x.count <= 0) return;
		T s = T(0.5)+fabs(size);
		x[0] = gauss(T(0.0), s);
		for(int i = 0; i < x.count; ++i)
			x[i] = gauss(T(i), s);
	}

	template<typename T>
	static void fill_pattern_2d_disk(const Array<T, 2> &x, const T &size_x, const T &size_y)
	{
		const T precision(1e-10);

		T sx(T(0.5) + fabs(size_x));
		T sy(T(0.5) + fabs(size_y));

		int sizec = std::min(x.get_count(1)-1, (int)ceil(sx - precision));
		int sizer = std::min(x.get_count(0)-1, (int)ceil(sy - precision));

		// draw sector
		T k0x(sx - T(0.5)), k0y(sy - T(0.5));
		T k1x(sx + T(0.5)), k1y(sy + T(0.5));
		T kk0x( k0x > precision ? T(1.0)/k0x : T(0.0) );
		T kk0y( k0y > precision ? T(1.0)/k0y : T(0.0) );
		T kk1x( k1x > precision ? T(1.0)/k1x : T(0.0) );
		T kk1y( k1y > precision ? T(1.0)/k1y : T(0.0) );
		T pp0x(0.0), pp0y(0.0);
		T pp1x(0.0), pp1y(0.0);
		for(int r = 0; r <= sizer; ++r)
		{
			for(int c = 0; c <= sizec; ++c)
			{
				T &v = x[r][c];
				T r1_squared = pp1x*pp1x + pp1y*pp1y;
				/// assume that pattern already contains zeros
				if (r1_squared < T(1.0))
				{
					T r0_squared = pp0x*pp0x + pp0y*pp0y;
					if (r0_squared <= T(1.0))
					{
						v = T(1.0);
					}
					else
					{
						T rr0 = sqrt(r0_squared);
						T rr1 = sqrt(r1_squared);
						T drr = rr0 - rr1;
						v = drr > precision ? rr0*(1.0 - rr1)/drr : T(0.0);
					}
				}
				pp0x += kk0x;
				pp1x += kk1x;
			}
			pp0x = T(0.0); pp0y += kk0y;
			pp1x = T(0.0); pp1y += kk1y;
		}

		/// assume that pattern already contains zeros
		/// and don't touch other cells
	}

	template<typename T>
	static void mirror_pattern(const Array<T, 1> &x)
	{
		typedef Array<T, 1> A;
		int count = (x.count-1)/2;
		typename A::ReverseIterator j(x);
		for(typename A::Iterator i(x, 1, 1 + count); i; ++i, ++j)
			*j = *i;
	}

	template<typename T>
	static void mirror_pattern_2d(const Array<T, 2> &x)
	{
		typedef Array<T, 2> A;
		int count = (x.count-1)/2;
		for(typename A::Iterator i(x, 1, 1 + count); i; ++i)
			mirror_pattern(*i);
		for(typename A::Iterator i(x.reorder(1, 0)); i; ++i)
			mirror_pattern(*i);
	}

	template<typename T>
	static void normalize_full_pattern(const Array<T, 1> &x)
	{
		const T precision(1e-10);
		typedef Array<T, 1> A;
		T sum = T(0.0);
		for(typename A::Iterator i(x); i; ++i)
			sum += *i;
		if (sum > precision)
		{
			T k = T(1.0)/sum;
			for(typename A::Iterator i(x); i; ++i)
				*i *= k;
		}
	}

	template<typename T>
	static void normalize_half_pattern(const Array<T, 1> &x)
	{
		if (x.count <= 0) return;
		const T precision(1e-10);
		typedef Array<T, 1> A;
		T sum = x[0];
		for(typename A::Iterator i(x, 1); i; ++i)
			sum += T(2.0)*(*i);
		if (sum > precision)
		{
			T k = T(1.0)/sum;
			for(typename A::Iterator i(x); i; ++i)
				*i *= k;
		}
	}

	template<typename T>
	static void normalize_full_pattern_2d(const Array<T, 2> &x)
	{
		if (x.get_count(0) <= 0 || x.get_count(1) <= 0) return;
		const T precision(1e-10);
		typedef Array<T, 2> A;
		typedef Array<T, 1> B;
		T sum = T(0.0);
		for(typename A::Iterator i(x); i; ++i)
		for(typename B::Iterator j(*i); j; ++j)
			sum += *j;
		if (sum > precision)
		{
			T k = T(1.0)/sum;
			for(typename A::Iterator i(x); i; ++i)
			for(typename B::Iterator j(*i); j; ++j)
				*j *= k;
		}
	}

	template<typename T>
	static void normalize_half_pattern_2d(const Array<T, 2> &x)
	{
		if (x.get_count(0) <= 0 || x.get_count(1) <= 0) return;
		const T precision(1e-10);
		typedef Array<T, 2> A;
		typedef Array<T, 1> B;
		T sum = x[0][0];
		for(typename A::Iterator i(x, 1); i; ++i)
			sum += T(2.0)*(*i)[0];
		for(typename B::Iterator j(x[0], 1); j; ++j)
			sum += T(2.0)*(*j);
		for(typename A::Iterator i(x, 1); i; ++i)
		for(typename B::Iterator j(*i, 1); j; ++j)
			sum += T(4.0)*(*j);
		if (sum > precision)
		{
			T k = T(1.0)/sum;
			for(typename A::Iterator i(x); i; ++i)
			for(typename B::Iterator j(*i); j; ++j)
				*j *= k;
		}
	}

	template<typename T>
	static void blur_pattern(const Array<T, 1> &dst, const Array<T, 1> &src, const Array<T, 1> &pattern)
	{
		typedef Array<T, 1> A;
		if (pattern.count <= 0)
		{
			dst.assign(src);
			return;
		}

		int pattern_size = pattern.count - 1;
		int end = std::min(src.count, dst.count) - pattern_size;

		int si = pattern_size;
		for(typename A::Iterator di(dst, pattern_size, end); di; ++di, ++si)
		{
			(*di) += src[si] * pattern[0];
			for(int i = 1; i <= pattern_size; ++i)
				(*di) += (src[si - i] + src[si + i])*pattern[i];
		}
	}

	template<typename T>
	static void blur_2d_pattern(const Array<T, 2> &dst, const Array<T, 2> &src, const Array<T, 2> &pattern)
	{
		typedef Array<T, 2> A;
		typedef Array<T, 1> B;
		if (pattern.get_count(0) <= 0 || pattern.get_count(1) <= 0)
		{
			dst.assign(src);
			return;
		}

		int pattern_size0 = pattern.get_count(0) - 1;
		int pattern_size1 = pattern.get_count(1) - 1;
		int end0 = std::min(src.get_count(0), dst.get_count(0)) - pattern_size0;
		int end1 = std::min(src.get_count(1), dst.get_count(1)) - pattern_size1;

		int si0 = pattern_size0;
		int si1 = pattern_size1;
		for(typename A::Iterator di0(dst, pattern_size0, end0); di0; ++di0, ++si0, si1 = pattern_size1)
		for(typename B::Iterator di1(*di0, pattern_size1, end1); di1; ++di1, ++si1)
		{
			(*di1) += src[si0][si1]*pattern[0][0];

			for(int i0 = 1; i0 <= pattern_size0; ++i0)
				(*di1) += (src[si0 - i0][si1] + src[si0 + i0][si1])*pattern[i0][0];
			for(int i1 = 1; i1 <= pattern_size1; ++i1)
				(*di1) += (src[si0][si1 - i1] + src[si0][si1 + i1])*pattern[0][i1];

			for(int i0 = 1; i0 <= pattern_size0; ++i0)
			for(int i1 = 1; i1 <= pattern_size1; ++i1)
				(*di1) += pattern[i0][i1]*( src[si0 - i0][si1 - i1]
						                  + src[si0 - i0][si1 + i1]
										  + src[si0 + i0][si1 - i1]
										  + src[si0 + i0][si1 + i1] );
		}
	}

	template<typename T>
	static void blur_box_discrete(const Array<T, 1> &x, std::deque<T> &q, const int size)
	{
		typedef Array<T, 1> A;
		if (size == 0) return;

		int s = abs(size);
		int full_size = 1 + 2*s;
		if (x.count < full_size) return;
		T w(T(1.0)/T(full_size));
		T sum(0.0);
		q.clear();
		for(typename A::Iterator i(x, 0, full_size); i; ++i)
			{ q.push_back(*i); sum += *i; }
		for(typename A::Iterator i(x, full_size), j(x, s); i; ++i, ++j)
		{
			*j = w*sum;
			sum += *i - q.front();
			q.pop_front();
			q.push_back(*i);
		}
	}

	template<typename T>
	static void blur_box_discrete(const Array<T, 1> &dst, const Array<const T, 1> &src, const int size, const int offset)
	{
		typedef Array<T, 1> A;
		typedef Array<const T, 1> B;

		int s = abs(size);
		int o0 = std::max(offset - s, 0);
		int o1 = std::max(s - offset, 0);
		if (o0 >= src.count) return;
		if (o1 >= dst.count) return;

		if (s == 0)
		{
			// copy
			typename B::Iterator j(dst, o1);
			for(typename A::Iterator i(src, o0); i && j; ++i, ++j)
				*j = *i;
		}
		else
		{
			int full_size = 1 + 2*s;
			if (full_size + o0 > src.count) return;
			T w(T(1.0)/T(full_size));
			T sum(0.0);
			for(typename A::Iterator i(src, o0, full_size+o0); i; ++i)
				sum += *i;
			typename B::Iterator j(dst, o1);
			for(typename A::Iterator i0(src, o0), i1(src, full_size+o0); i1 && j; ++i0, ++j, ++i1)
			{
				*j = w*sum;
				sum += *i1 - *i0;
			}
		}
	}

	template<typename T>
	static void blur_box_aa(const Array<T, 1> &x, std::deque<T> &q, const T &size)
	{
		typedef Array<T, 1> A;

		const T precision(1e-10);
		T s(fabs(size));
		if (s < precision) return;

		int sizei = 1 + (int)floor(s + precision);
		if (x.count <= 2*sizei) return;

		T w(T(1.0)/(T(1.0) + T(2.0)*s));
		T aa(T(sizei) - s);
		T sum(0.0);
		T aa_out = x[0];
		q.clear();
		for(typename A::Iterator i(x, 1, 2*sizei); i; ++i)
			{ q.push_back(*i); sum += *i; }
		q.push_back(x[2*sizei]);
		sum += aa*(aa_out + q.back());
		for(typename A::Iterator i(x, 2*sizei+1), j(x, sizei); i; ++i, ++j)
		{
			*j = w*sum;
			T d = q.back() - q.front();
			sum += aa*(*i - aa_out - d) + d;
			aa_out = q.front();
			q.pop_front();
			q.push_back(*i);
		}
	}

	template<typename T>
	static void blur_box_aa(const Array<T, 1> &dst, const Array<const T, 1> &src, const T &size, const int offset)
	{
		typedef Array<T, 1> A;
		typedef Array<const T, 1> B;
		const T precision(1e-10);

		T s(fabs(size));
		int sizei = 1 + (int)floor(s + precision);
		int o0 = std::max(offset - sizei - 1, 0);
		int o1 = std::max(sizei + 1 - offset, 0);
		if (o0 >= src.count) return;
		if (o1 >= dst.count) return;

		if (s < precision)
		{
			// copy
			typename B::Iterator j(dst, o1);
			for(typename A::Iterator i(src, o0); i && j; ++i, ++j)
				*j = *i;
		}
		else
		{
			if (2*sizei + o0 >= src.count) return;
			T w(T(1.0)/(T(1.0) + T(2.0)*s));
			T aa(T(sizei) - s);
			T sum(0.0);
			T aa_out = src[o0];
			for(typename A::Iterator i(src, o0, 2*sizei+o0); i; ++i)
				sum += *i;
			T aa_in = src[2*sizei+o0];
			sum += aa*(aa_out + aa_in);
			typename B::Iterator j(dst, o1);
			for(typename A::Iterator i0(src, o0+1), i1(src, 2*sizei+o0+1); i1 && j; ++i0, ++j, ++i1)
			{
				*j = w*sum;
				T d = aa_in - *i0;
				sum += aa(*i1 - aa_out - d) + d;
				aa_out = *i0;
				aa_in = *i1;
			}
		}
	}

	template<typename T>
	static void blur_box(const Array<T, 1> &x, std::deque<T> &q, const T &size)
	{
		const T precision(1e-5);
		if (fabs(size - round(size)) < precision)
			blur_box_discrete(x, q, (int)round(size));
		else
			blur_box_aa(x, q, size);
	}

	template<typename T>
	static void blur_box(const Array<T, 1> &dst, const Array<const T, 1> &src, const T &size, const int offset)
	{
		const T precision(1e-5);
		if (fabs(size - round(size)) < precision)
			blur_box_discrete(dst, src, (int)round(size), offset);
		else
			blur_box_aa(dst, src, size, offset);
	}

	template<typename T>
	static void blur_iir(const Array<T, 1> &x, const T &k0, const T &k1, const T &k2, const T &k3)
	{
		typedef Array<T, 1> A;
		T d3(0.0), d2(0.0), d1(0.0), d0;
		for(typename A::Iterator i(x); i; ++i)
			*i = d0 = k0*(*i) + k1*d1 + k2*d2 + k3*d3, d3 = d2, d2 = d1, d1 = d0;
		d3 = d2 = d1 = T(0.0);
		for(typename A::ReverseIterator i(x); i; ++i)
			*i = d0 = k0*(*i) + k1*d1 + k2*d2 + k3*d3, d3 = d2, d2 = d1, d1 = d0;
	}

	template<typename T>
	static void blur_iir(const Array<T, 1> &dst, const Array<T, 1> &src, const T &k0, const T &k1, const T &k2, const T &k3)
	{
		typedef Array<T, 1> A;
		int count = std::min(dst.count, src.count);
		T d3(0.0), d2(0.0), d1(0.0), d0;
		dst[0] = dst[1] = dst[2] = dst[dst.count - 1] = dst[dst.count - 2] = dst[dst.count - 3] = 0.0;
		for(typename A::Iterator j(dst, count), i(src, count); j; ++j, ++i)
			*j = d0 = k0*(*i) + k1*d1 + k2*d2 + k3*d3, d3 = d2, d2 = d1, d1 = d0;
		d3 = d2 = d1 = T(0.0);
		for(typename A::ReverseIterator j(dst, dst.count - count), i(src, src.count - count); j; ++j, ++i)
			*j = d0 = k0*(*i) + k1*d1 + k2*d2 + k3*d3, d3 = d2, d2 = d1, d1 = d0;
	}

	static void surface_as_array(Array<const Color, 2> &a, const synfig::Surface &src, const RectInt &r)
	{
		assert(src.is_valid() && r.is_valid());
		assert(r.minx >= 0 && r.miny >= 0);
		assert(r.maxx <= src.get_w() && r.miny <= src.get_h());
		a.pointer = &src[r.miny][r.minx];
		a.set_dim(r.maxy - r.miny, src.get_pitch()/sizeof(Color))
		 .set_dim(r.maxx - r.minx, 1);
	}

	static void surface_as_array(Array<Color, 2> &a, synfig::Surface &src, const RectInt &r)
	{
		assert(src.is_valid() && r.is_valid());
		assert(r.minx >= 0 && r.miny >= 0);
		assert(r.maxx <= src.get_w() && r.miny <= src.get_h());
		a.pointer = &src[r.miny][r.minx];
		a.set_dim(r.maxy - r.miny, src.get_pitch()/sizeof(Color))
		 .set_dim(r.maxx - r.minx, 1);
	}

	static Array<Color, 2> surface_as_array(synfig::Surface &src)
		{ Array<Color, 2> a; surface_as_array(a, src, RectInt(0, 0, src.get_w(), src.get_h())); return a; }
	static Array<Color, 2> surface_as_array(synfig::Surface &src, const RectInt &r)
		{ Array<Color, 2> a; surface_as_array(a, src, r); return a; }

	static Array<const Color, 2> surface_as_array(const synfig::Surface &src)
		{ Array<const Color, 2> a; surface_as_array(a, src, RectInt(0, 0, src.get_w(), src.get_h())); return a; }
	static Array<const Color, 2> surface_as_array(const synfig::Surface &src, const RectInt &r)
		{ Array<const Color, 2> a; surface_as_array(a, src, r); return a; }

	template<typename T>
	static void surface_read(
		const Array<T, 3> &dst,
		const Array<const Color, 2> &src )
	{
		assert(dst.count == src.count);
		assert(dst.sub().count == src.sub().count);
		assert(dst.sub().sub().count == 4);
		Array<const Color, 2>::Iterator rr(src);
		for(typename Array<T, 3>::Iterator r(dst); r; ++r, ++rr)
		{
			Array<const Color, 1>::Iterator cc(*rr);
			for(typename Array<T, 2>::Iterator c(*r); c; ++c, ++cc)
			{
				T a = cc->get_a();
				(*c)[0] = cc->get_r()*a;
				(*c)[1] = cc->get_g()*a;
				(*c)[2] = cc->get_b()*a;
				(*c)[3] = a;
			}
		}
	}

	template<typename T>
	static void surface_read(
		const Array<T, 3> &dst,
		const synfig::Surface &src,
		const VectorInt &dst_offset,
		const RectInt &src_rect )
	{
		RectInt dst_rect = src_rect - src_rect.get_min() + dst_offset;
		Array<T, 3> dst_range = dst.get_range(1, dst_rect.minx, dst_rect.maxx)
								   .get_range(0, dst_rect.miny, dst_rect.maxy);
		Array<const Color, 2> src_range = surface_as_array(src, src_rect);
		surface_read(dst_range, src_range);
	}

	template<typename T>
	static void surface_write(
		const Array<Color, 2> &dst,
		const Array<T, 3> &src )
	{
		assert(dst.count == src.count);
		assert(dst.sub().count == src.sub().count);
		assert(src.sub().sub().count == 4);
		const ColorReal precision = 1e-10;
		Array<Color, 2>::Iterator rr(dst);
		for(typename Array<T, 3>::Iterator r(src); r; ++r, ++rr)
		{
			Array<Color, 1>::Iterator cc(*rr);
			for(typename Array<T, 2>::Iterator c(*r); c; ++c, ++cc)
			{
				Real a = (*c)[3];
				Real one_div_a = fabs(a) < precision ? 0.0 : 1.0/a;
				Color color((*c)[0]*one_div_a, (*c)[1]*one_div_a, (*c)[2]*one_div_a, a);
				*cc = color;
			}
		}
	}

	template<typename T>
	static void surface_write(
		const Array<Color, 2> &dst,
		const Array<T, 3> &src,
		Color::BlendMethod blend_method,
		ColorReal amount )
	{
		assert(dst.count == src.count);
		assert(dst.sub().count == src.sub().count);
		assert(src.sub().sub().count == 4);
		const ColorReal precision = 1e-10;
		Array<Color, 2>::Iterator rr(dst);
		for(typename Array<T, 3>::Iterator r(src); r; ++r, ++rr)
		{
			Array<Color, 1>::Iterator cc(*rr);
			for(typename Array<T, 2>::Iterator c(*r); c; ++c, ++cc)
			{
				ColorReal a = (*c)[3];
				Real one_div_a = fabs(a) < precision ? 0.0 : 1.0/a;
				Color color((*c)[0]*one_div_a, (*c)[1]*one_div_a, (*c)[2]*one_div_a, a);
				*cc = Color::blend(color, *cc, amount, blend_method);
			}
		}
	}

	template<typename T>
	static void surface_write(
		synfig::Surface &dst,
		const Array<T, 3> &src,
		const RectInt &dst_rect,
		const VectorInt &src_offset,
		bool blend,
		Color::BlendMethod blend_method,
		ColorReal amount )
	{
		RectInt src_rect = dst_rect - dst_rect.get_min() + src_offset;
		Array<Color, 2> dst_range = surface_as_array(dst, dst_rect);
		Array<T, 3> src_range = src.get_range(1, src_rect.minx, src_rect.maxx)
								   .get_range(0, src_rect.miny, src_rect.maxy);
		if (blend) surface_write(dst_range, src_range, blend_method, amount);
			  else surface_write(dst_range, src_range);
	}
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
