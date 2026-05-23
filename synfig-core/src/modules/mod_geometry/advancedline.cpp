/* === S Y N F I G ========================================================= */
/*!	\file advancedline.cpp
**	\brief Implementation of the Advanced Line primitive
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
**	......... ... 2019 Ivan Mahonin
**	......... ... 2025 Synfig Contributors
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "advancedline.h"

#include <synfig/curve.h>
#endif

/* === M A C R O S ========================================================= */

#if defined(__has_cpp_attribute)
# if __has_cpp_attribute(fallthrough)
#  define fallthrough__ [[fallthrough]]
# endif
#endif

#ifndef fallthrough__
# if __GNUC__ >= 7
#  define fallthrough__ __attribute__((fallthrough))
# elif __clang__
#  define fallthrough__ [[clang::fallthrough]]
# else
#  define fallthrough__ ((void)0)
# endif
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
synfig::AdvancedLine::add(Real p, Real w, WidthPoint::SideType side0, WidthPoint::SideType side1, AddAction action)
{
	w = fabs(w);
	if (approximate_zero(w)) {
		AdvancedPoint &ap = (*this)[p];
		ap.w = 0;
		ap.side0 = ap.side1 = WidthPoint::TYPE_FLAT;
	} else {
		if (action != AddAction::REPLACE_IF_EXISTS) {
			auto it = find(p);
			if (it != end()) {
				AdvancedPoint &ap = it->second;
				if (action == AddAction::APPEND) {
					ap.w = w;
					ap.side1 = side1;
					return;
				} else if (action == AddAction::PREPEND) {
					ap.w = w;
					ap.side0 = side0;
					return;
				}
			}
		}
		AdvancedPoint& ap = (*this)[p];
		ap.w = w;
		ap.side0 = side0;
		ap.side1 = side1;
	}
}

void
synfig::AdvancedLine::calc_tangents(Real smoothness)
{
	if (empty()) return;

	const Real kx = Real(1)/3;
	iterator i2 = begin(), i0 = i2++, i1;
	if (i2 != end()) {
		Real s0 = clamp(smoothness, Real(0), Real(1));
		Real s1 = 1 - s0;

		for(i1 = i2++; i2 != end(); i0 = i1, i1 = i2++) {
			Vector p0(i0->first, i0->second.y1());
			Vector p1(i1->first, i1->second.y0());
			Vector p2(i2->first, i2->second.y0());

			Vector d0 = p1 - p0;
			Vector d1 = p2 - p1;
			Vector dd0 = d0*kx;
			Vector dd1 = d1*kx;
			i1->second.pp0[0] = p1[0] - dd0[0];
			i1->second.pp1[0] = p1[0] + dd1[0];

			if (p1[1] == i1->second.y1()) {
				Real ky0 = d0[1]/d0[0];
				Real ky1 = d1[1]/d1[0];
				Real ky = (ky0 + ky1)*0.5;
				ky = ky0 > 0 && ky1 > 0 ? std::min(ky, std::min(ky0, ky1)*3)
					 : ky0 < 0 && ky1 < 0 ? std::max(ky, std::max(ky0, ky1)*3)
										  : 0;
				ky *= s0;
				i1->second.pp0[1] = p1[1] - (dd0[0]*ky + s1*dd0[1]);
				i1->second.pp1[1] = p1[1] + (dd1[0]*ky + s1*dd1[1]);
			} else {
				i1->second.pp0[1] = p1[1] - s1*dd0[1];
				Real y = i1->second.y1();
				i1->second.pp1[1] = y + s1*kx*(p2[1] - y);
			}
		}

		Real ky = kx*s1;
		// i0: the first vertex, i1: the second one
		i1 = begin(); i0 = i1++;
		i0->second.pp1[0] = i0->first + (i1->first - i0->first)*kx;
		i0->second.pp1[1] = i0->second.y1() + (i1->second.y0() - i0->second.y1())*ky;

		// i0: the penultimate vertex, i1: the last one
		i0 = end(); i1 = (--i0)--;
		i1->second.pp0[0] = i1->first - (i1->first - i0->first)*kx;
		i1->second.pp0[1] = i1->second.y0() - (i1->second.y0() - i0->second.y1())*ky;
	}

	i0 = begin(); i1 = end(); --i1;
	i0->second.pp0 = Vector(i0->first, i0->second.y0());
	i1->second.pp1 = Vector(i1->first, i1->second.y1());
}

void
synfig::AdvancedLine::trunc_left(Real p, WidthPoint::SideType side)
{
	iterator i1 = lower_bound(p);
	if (i1 == end()) {
		clear();
		return;
	}

	if (approximate_equal(i1->first, p)) {
		i1->second.side0 = side;
		i1->second.pp0 = Vector(p, 0);
		erase(begin(), i1);
		return;
	}

	Bezier b;
	if (i1 == begin()) {
		b = Bezier(
			Vector(p, i1->second.y0()),
			Vector(i1->first, i1->second.y0()) );
	} else {
		iterator i0 = i1; --i0;
		b.p0 = Vector(i0->first, i0->second.y1());
		b.p1 = Vector(i1->first, i1->second.y0());
		b.pp0 = i0->second.pp1;
		b.pp1 = i1->second.pp0;
	}

	erase(begin(), i1);
	if (approximate_zero(b.p0[1]) && approximate_zero(b.p1[1])){
		if (!empty() && approximate_equal(begin()->first, p)) {
			begin()->second.side0 = side;
		}
		return;
	}

	Real k = (b.p1[0] - b.p0[0]);
	k = approximate_zero_hp(k) ? 0 : 1/k;
	b.split( (p - b.p0[0])*k, nullptr, &b );

	bool point_already_exists = find(p) != end();

	AdvancedPoint &ap = (*this)[p];
	ap.w = b.p0[1];
	ap.pp0 = Vector(p, 0);
	ap.pp1 = b.pp0;
	ap.side0 = side;
	if (!point_already_exists)
		ap.side1 = WidthPoint::TYPE_INTERPOLATE;

	i1->second.pp0 = b.pp1;
}

void synfig::AdvancedLine::trunc_right(Real p, WidthPoint::SideType side)
{
	iterator i1 = upper_bound(p);
	if (i1 == begin())
	{ clear(); return; }
	iterator i0 = i1; --i0;

	if (approximate_equal(i0->first, p)) {
		i0->second.side1 = side;
		i0->second.pp1 = Vector(p, 0);
		erase(i1, end());
		return;
	}

	Bezier b;
	if (i1 == end()) {
		b = Bezier(
			Vector(i0->first, i0->second.y1()),
			Vector(p, i0->second.y1()) );
	} else {
		iterator i0 = i1; --i0;
		b.p0 = Vector(i0->first, i0->second.y1());
		b.p1 = Vector(i1->first, i1->second.y0());
		b.pp0 = i0->second.pp1;
		b.pp1 = i1->second.pp0;
	}

	erase(i1, end());
	if (approximate_zero(b.p0[1]) && approximate_zero(b.p1[1])) {
		if (!empty()) {
			iterator last = end(); --last;
			if (approximate_equal(last->first, p))
				last->second.side1 = side;
		}
		return;
	}

	Real k = (b.p1[0] - b.p0[0]);
	k = approximate_zero_hp(k) ? 0 : 1/k;
	b.split( (p - b.p0[0])*k, &b, nullptr );

	bool point_already_exists = find(p) != end();

	AdvancedPoint &ap = (*this)[p];
	ap.w = b.p1[1];
	ap.pp0 = b.pp1;
	ap.pp1 = Vector(p, 0);
	if (!point_already_exists)
		ap.side0 = WidthPoint::TYPE_INTERPOLATE;
	ap.side1 = side;

	i0->second.pp1 = b.pp0;
}

void
synfig::AdvancedLine::cut(Real p0, Real p1, WidthPoint::SideType side0, WidthPoint::SideType side1)
{
	if (!approximate_less(p0, p1)) return;

	iterator i0 = lower_bound(p0);
	iterator i1 = upper_bound(p1);
	if (i0 == begin())
	{ trunc_left(p1, side1); return; }
	if (i1 == end())
	{ trunc_right(p0, side0); return; }

	iterator i00 = i0, i01 = i00--;
	Bezier b0(
		Vector(i00->first, i00->second.y1()),
		Vector(i01->first, i01->second.y0()),
		i00->second.pp1,
		i01->second.pp0 );

	iterator i10 = i1, i11 = i10--;
	Bezier b1(
		Vector(i10->first, i10->second.y1()),
		Vector(i11->first, i11->second.y0()),
		i10->second.pp1,
		i11->second.pp0 );

	erase(i0, i1);

	bool add0 = b0.p0[1] || b0.p1[1];
	bool add1 = b1.p0[1] || b1.p1[1];
	const Real kx = Real(1)/3;

	if (add0) {
		Real l = b0.p1[0] - b0.p0[0];
		l = approximate_zero_hp(l) ? 0 : 1/l;
		l = (p0 - b0.p0[0])*l;
		b0.split( l, &b0, nullptr );

		// fix x-coord of spline to avoid accumulaeted error
		// x-coord should be linear
		b0.p1[0] = p0;
		Real dx = (b0.p1[0] - b0.p0[0])*kx;
		b0.pp0[0] = b0.p0[0] + dx;
		b0.pp1[0] = b0.p1[0] - dx;
		Real pp = p0 + ((add1 ? p1 : i11->first) - p0)*kx;

		AdvancedPoint &ap = (*this)[p0];
		ap.w = b0.p1[1];
		ap.pp0 = b0.pp1;
		ap.pp1 = Vector(pp, 0);
		ap.side0 = WidthPoint::TYPE_INTERPOLATE;
		ap.side1 = side0;

		i00->second.pp1 = b0.pp0;
	}

	if (add1) {
		Real l = b1.p1[0] - b1.p0[0];
		l = approximate_zero_hp(l) ? 0 : 1/l;
		l = (p1 - b1.p0[0])*l;
		b1.split( l, nullptr, &b1 );

		// fix x-coord of spline to avoid accumulaeted error
		// x-coord should be linear
		b1.p0[0] = p1;
		Real dx = (b1.p1[0] - b1.p0[0])*kx;
		b1.pp0[0] = b1.p0[0] + dx;
		b1.pp1[0] = b1.p1[0] - dx;
		Real pp = p1 - (p1 - (add0 ? p0 : i00->first))*kx;

		AdvancedPoint &ap = (*this)[p1];
		ap.w = b1.p0[1];
		ap.pp0 = Vector(pp, 0);
		ap.pp1 = b1.pp0;
		ap.side0 = side1;
		ap.side1 = WidthPoint::TYPE_INTERPOLATE;

		i11->second.pp0 = b1.pp1;
	}
}

void
synfig::AdvancedLine::build_contour(synfig::rendering::Contour& dst) const
{
	const Real round_k0 = 0.5*sqrt(2);
	const Real round_k1 = sqrt(2) - 1;
	dst.close();
	for(const_iterator i = begin(); i != end(); ++i) {
		if (i->second.side0 != WidthPoint::TYPE_INTERPOLATE && !dst.closed())
			dst.close_mirrored_vert();
		Real s = 1;
		switch(i->second.side0) {
		case WidthPoint::TYPE_INTERPOLATE:
			if (i == begin()) {
				dst.move_to( Vector(i->first, 0) );
				dst.line_to( Vector(i->first, i->second.w) );
			} else {
				const_iterator i0 = i; --i0;
				if (dst.closed()) {
					dst.move_to( Vector(i0->first, i0->second.y1()) );
					if (approximate_equal(i0->second.y1(), i->second.y0())) {
						dst.line_to(Vector(i->first, i->second.y0()));
					} else {
						dst.cubic_to(
							Vector(i->first, i->second.y0()),
							i0->second.pp1,
							i->second.pp0 );
					}
				}
			}
			break;
		case WidthPoint::TYPE_SQUARED:
			dst.move_to( Vector(i->first - i->second.w, 0) );
			dst.line_to( Vector(i->first - i->second.w, i->second.w) );
			dst.line_to( Vector(i->first, i->second.w) );
			break;
		case WidthPoint::TYPE_INNER_PEAK:
			s = -1;
			fallthrough__;
		case WidthPoint::TYPE_PEAK:
			dst.move_to( Vector(i->first - s*i->second.w, 0) );
			dst.line_to( Vector(i->first, i->second.w ) );
			break;
		case WidthPoint::TYPE_INNER_ROUNDED:
			s = -1;
			fallthrough__;
		case WidthPoint::TYPE_ROUNDED:
			dst.move_to( Vector(i->first - s*i->second.w, 0) );
			dst.conic_to(
				Vector(i->first - s*i->second.w*round_k0, i->second.w*round_k0),
				Vector(i->first - s*i->second.w, i->second.w*round_k1) );
			dst.conic_to(
				Vector(i->first, i->second.w),
				Vector(i->first - s*i->second.w*round_k1, i->second.w) );
			break;
		default: // flat
			dst.move_to( Vector(i->first, 0) );
			dst.line_to( Vector(i->first, i->second.w) );
			break;
		}

		s = 1;
		switch(i->second.side1) {
		case WidthPoint::TYPE_INTERPOLATE:
		{
			const_iterator i1 = i; ++i1;
			if (i1 != end()) {
				dst.line_to( Vector(i->first, i->second.y1()) );
				if (approximate_equal(i->second.y1(), i1->second.y0())) {
					dst.line_to(Vector(i1->first, i1->second.y0()));
				} else {
					dst.cubic_to(
						Vector(i1->first, i1->second.y0()),
						i->second.pp1,
						i1->second.pp0 );
				}
			} else {
				dst.line_to( Vector(i->first, 0) );
				dst.close_mirrored_vert();
			}
		}
		break;
		case WidthPoint::TYPE_SQUARED:
			dst.line_to( Vector(i->first + i->second.w, i->second.w) );
			dst.line_to( Vector(i->first + i->second.w, 0) );
			dst.close_mirrored_vert();
			break;
		case WidthPoint::TYPE_INNER_PEAK:
			s = -1;
			fallthrough__;
		case WidthPoint::TYPE_PEAK:
			dst.line_to( Vector(i->first + s*i->second.w, 0) );
			dst.close_mirrored_vert();
			break;
		case WidthPoint::TYPE_INNER_ROUNDED:
			s = -1;
			fallthrough__;
		case WidthPoint::TYPE_ROUNDED:
			dst.conic_to(
				Vector(i->first + s*i->second.w*round_k0, i->second.w*round_k0),
				Vector(i->first + s*i->second.w*round_k1, i->second.w) );
			dst.conic_to(
				Vector(i->first + s*i->second.w, 0),
				Vector(i->first + s*i->second.w, i->second.w*round_k1) );
			dst.close_mirrored_vert();
			break;
		default: // flat
			dst.line_to( Vector(i->first, 0) );
			dst.close_mirrored_vert();
			break;
		}
	}
}
