/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/bend.cpp
**	\brief Bend
**
**	$Id$
**
**	\legal
**	......... ... 2019 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
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


#include <ETL/hermite>

#include <synfig/curve.h>

#include "bend.h"


#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

namespace {

	class Intersection {
	public:
		Real l;
		Bend::Point point;
		explicit Intersection(const Real &l = Real(), const Bend::Point &point = Bend::Point()):
			l(l), point(point) { }
		bool operator<(const Intersection &other) const { return l < other.l; }
	};
	
	typedef std::vector<Intersection> IntersectionList;

	
	void touch(Contour &dst, bool &dst_move_flag, const Vector &p) {
		if (dst_move_flag) {
			dst.move_to(p);
			dst_move_flag = false;
		} else {
			dst.line_to(p);
		}
	}

	void half_corner(Contour &dst, bool &dst_move_flag, const Bend::Point &point, Real radius, bool flip, bool out) {
		if (point.mode == Bend::NONE || approximate_zero(radius))
			return;

		const Vector &tn0 = flip ? point.tn1 : point.tn0;
		const Vector &tn1 = flip ? point.tn0 : point.tn1;
		Real d = tn0 * tn1.perp();
		Real dd = tn0 * tn1;
		bool codirected = approximate_zero(d);
		if (codirected && !approximate_less(dd, Real(0)))
			return;
		
		const Vector p0 = tn0.perp() * radius;
		const Vector p1 = tn1.perp() * radius;
		const Vector &p = out ? p1 : p0;
		const Vector &center = point.p;
		const Real rmod = fabs(radius);
		
		if (!codirected && (d*radius < 0) == flip) {
			if (point.mode == Bend::ROUND) {
				const Vector pp = (p0 + p1).norm()*rmod;
				const Vector ppp = (p + pp).norm()*rmod;
				Real k = (ppp - p).mag()/(3*rmod);
				if (p * ppp.perp() < 0) k = -k;
				if (out) {
					touch(dst, dst_move_flag, center + pp);
					dst.cubic_to(
						center + ppp,
						center + pp  +  pp.perp()*k,
						center + ppp - ppp.perp()*k );
					dst.cubic_to(
						center + p,
						center + ppp + ppp.perp()*k,
						center + p   -   p.perp()*k );
				} else {
					k = -k;
					touch(dst, dst_move_flag, center + p);
					dst.cubic_to(
						center + ppp,
						center + p   +   p.perp()*k,
						center + ppp - ppp.perp()*k );
					dst.cubic_to(
						center + pp,
						center + ppp + ppp.perp()*k,
						center + pp  -  pp.perp()*k);
				}
			} else
			if (point.mode == Bend::CORNER) {
				Real dd = (p1 - p0) * tn1.perp();
				Vector pp = p0 + tn0*(dd/d);
				if (out) {
					touch(dst, dst_move_flag, center + pp);
					dst.line_to(center + p);
				} else {
					touch(dst, dst_move_flag, center + p);
					dst.line_to(center + pp);
				}
			}
		} else {
			Vector pp = (p0 + p1)*0.5;
			if (out) {
				touch(dst, dst_move_flag, center + pp);
				dst.line_to(center + p);
			} else {
				touch(dst, dst_move_flag, center + p);
				dst.line_to(center + pp);
			}
		}
	}
}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
Bend::add(const Vector &p, const Vector &t0, const Vector &t1, Mode mode, bool calc_length, int segments)
{
	Point point;
	point.e0 = point.e1 = true;
	
	if (!points.empty()) {
		Point &last = points.back();
		if ( last.p.is_equal_to(p)
		  && last.t1.is_equal_to(Vector())
		  && t0.is_equal_to(Vector()) )
		{
			// skip zero length segment
			last.t1 = t1;
			last.mode = mode;
			return;
		}
		
		const Real step = Real(1)/segments;
		const Hermite h(last.p, p, last.t1, t0);
		last.tn1 = h.d0();
		point.p = last.p;
		point.length = last.length;
		Real l0 = last.l, l = step;
		for(int i = 2; i < segments; ++i, l += step) {
			Vector pp = h.p(l);
			point.l = l0 + l;
			point.length += calc_length ? (pp - point.p).mag() : point.l;
			point.p = pp;
			point.t0 = point.t1 = h.t(l);
			point.tn0 = point.tn1 = h.d(l);
			points.push_back(point);
		}
		point.l = l0 + 1;
		point.length += calc_length ? (p - point.p).mag() : point.l;
		point.tn0 = h.d1();
	}

	point.p = p;
	point.t0 = t0;
	point.t1 = t1;
	point.mode = mode;
	points.push_back(point);
}

void
Bend::loop(bool calc_length, int segments)
{
	if (points.empty()) return;
	
	Point &point = points.front();
	add(point.p, point.t0, point.t1, point.mode, calc_length, segments);
	
	Point &first = points.front();
	Point &last = points.back();
	
	first.t0  = last.t0;
	first.tn0 = last.tn0;
	last.t1   = first.t1;
	last.tn1  = first.tn1;
	first.e0 = last.e1 = false;
}

void
Bend::tails()
{
	if (points.size() < 2u) return;

	Point &first = points.front();
	Point &last = points.back();

	first.t0  = first.t1;
	first.tn0 = first.tn1;
	last.t1   = last.t0;
	last.tn1  = last.tn0;
	first.mode = last.mode = NONE;
}


Bend::PointList::const_iterator
Bend::find(const Real &length) const
{
	if (points.empty())
		return points.end();
	PointList::const_iterator a = points.begin(), b = points.end() - 1, c;
	if (!approximate_less(a->length, length)) return a;
	if (!approximate_less(length, b->length)) return b;
	while( (c = a + (b - a)/2) != a ) {
		if (approximate_equal(length, c->length)) return c;
		(length < c->length ? b : a) = c;
	}
	return c;
}

const Bend::Point*
Bend::find_exact(const Real &length) const
{
	PointList::const_iterator i = find(length);
	return i == points.end() || !approximate_equal(length, i->length) ? nullptr : &*i;
}

Bend::Point
Bend::interpolate(const Real &length) const
{
	PointList::const_iterator i = find(length);
	if (i == points.end()) return Point();
	
	if (approximate_equal(length, i->length)) return *i;
	
	if (length < i->length) {
		// extarpolation before
		Real dlen = length - i->length;
		Point s;
		s.p = i->p + i->tn0*dlen;
		s.t0 = s.t1 = s.tn0 = s.tn1 = i->tn0;
		s.l = i->l + dlen;
		s.length = length;
		s.e0 = s.e1 = i->e0;
		return s;
	}
	
	PointList::const_iterator j = i + 1;
	if (j == points.end()) {
		// extarpolation after
		Real dlen = length - i->length;
		Point s;
		s.p = i->p + i->tn1*dlen;
		s.t0 = s.t1 = s.tn0 = s.tn1 = i->tn1;
		s.l = i->l + dlen;
		s.length = length;
		s.e0 = s.e1 = i->e1;
		return s;
	}
	
	// interpolation
	Real l = (length - i->length)/(j->length - i->length);
	Real dl = j->l - i->l;
	Hermite h(i->p, j->p, i->t1*dl, j->t0*dl);
	
	Point s;
	s.p = h.p(l);
	s.t0 = s.t1 = h.t(l);
	s.tn0 = s.tn1 = h.d(l);
	s.l = i->l + l*(j->l - i->l);
	s.length = length;
	s.e0 = s.e1 = (i->e1 && j->e0);
	
	return s;
}

void
Bend::bend(Contour &dst, const Contour &src, const Matrix &matrix, int segments, Hints hints) const
{
	if (points.empty() || src.get_chunks().empty())
		return;

	const int last_segment = segments - 1;
	const Real step = Real(1)/segments;
	const Real stepk = 1/step;
	Contour::ChunkList::const_iterator i = src.get_chunks().begin();
	Vector p0 = matrix.get_transformed(i->p1);
	size_t dst_size = dst.get_chunks().size();
	bool dst_move_flag = true;
	
	IntersectionList intersections;
	while(++i != src.get_chunks().end()) {
		Hermite h;
		switch(i->type) {
			case Contour::CUBIC:
				h = Bezier(
					p0,
					matrix.get_transformed(i->p1),
					matrix.get_transformed(i->pp0),
					matrix.get_transformed(i->pp1) ).get_hermite();
				break;
			case Contour::CONIC:
				h = Bezier(
					p0,
					matrix.get_transformed(i->p1),
					matrix.get_transformed(i->pp0) ).get_hermite();
				break;
			case Contour::MOVE:
				p0 = matrix.get_transformed(i->p1);
				continue;
			default:
				h = Hermite(p0, matrix.get_transformed(i->p1));
				break;
		}
		p0 = h.p1;
		
		Real bends[2];
		int bends_count = h.bends(0, bends);
		Range r(h.p0[0]);
		r.expand(h.p1[0]);
		for(Real *i = bends, *end = i + bends_count; i != end; ++i)
			r.expand( h.p(0, *i) );
		bool b0 = bends_count > 0, b1 = bends_count > 1;
		
		bool bs[segments] = {};
		
		for(PointList::const_iterator bi = find(r.min); bi != points.end() && !approximate_less(r.max, bi->length); ++bi) {
			Real roots[3];
			int count = h.intersections(0, roots, bi->length);
			for(Real *i = roots, *end = i + count; i != end; ++i) {
				if (b0 && approximate_equal_lp(*i, bends[0])) b0 = false;
				if (b1 && approximate_equal_lp(*i, bends[1])) b1 = false;
				Real seg = *i * stepk;
				if (approximate_equal_lp(seg, round(seg))) {
					int segi = (int)round(seg);
					if (segi > 0 && segi < last_segment) bs[segi] = true;
				}
				intersections.push_back( Intersection(*i, *bi) );
			}
		}
		if (b0) intersections.push_back( Intersection(bends[0], interpolate( h.p(0, bends[0]) )) );
		if (b1) intersections.push_back( Intersection(bends[1], interpolate( h.p(0, bends[1]) )) );
		Real bsl = step;
		for(int i = 1; i < last_segment; ++i, bsl += step)
			if (bs[i]) intersections.push_back( Intersection(bsl, interpolate( h.p(0, bsl) )) );
		sort(intersections.begin(), intersections.end());
		
		Intersection prev(0, interpolate(h.p0[0]));
		intersections.push_back( Intersection(1, interpolate(h.p1[0])) );
		
		for(IntersectionList::const_iterator bi = intersections.begin(); bi != intersections.end(); ++bi) {
			const Intersection &next = *bi;
			if (approximate_equal(prev.l, next.l)) continue;

			bool flip = next.point.l < prev.point.l;
			bool e0 = flip ? prev.point.e0 : prev.point.e1;
			bool e1 = flip ? next.point.e1 : next.point.e0;
			if (e0 && e1) {
				Vector x0 = flip ? prev.point.tn0 : prev.point.tn1;
				Vector x1 = flip ? next.point.tn1 : next.point.tn0;
				Vector y0 = x0.perp();
				Vector y1 = x1.perp();
				
				Hermite src_h = h.sub(prev.l, next.l);

				Hermite dst_h(
					prev.point.p + y0*src_h.p0[1],
					next.point.p + y1*src_h.p1[1],
					x0*src_h.t0[0] + y0*src_h.t0[1],
					x1*src_h.t1[0] + y1*src_h.t1[1] );
				
				if (hints & HINT_NORM_TANGENTS) {
					Real k = 0.75*(dst_h.p1 - dst_h.p0).mag();
					dst_h.t0 = dst_h.t0.norm()*k;
					dst_h.t1 = dst_h.t1.norm()*k;
				} else
				if (hints & HINT_SCALE_TANGENTS) {
					Real k = (src_h.p1 - src_h.p0).mag_squared();
					if (!approximate_zero(k)) {
						k = sqrt((dst_h.p1 - dst_h.p0).mag_squared()/k);
						dst_h.t0 *= k;
						dst_h.t1 *= k;
					}
				}
				
				Bezier dst_b = dst_h.get_bezier();
				
				half_corner(dst, dst_move_flag, prev.point, src_h.p0[1], flip, true);
				touch(dst, dst_move_flag, dst_b.p0);
				dst.cubic_to(dst_b.p1, dst_b.pp0, dst_b.pp1);
				half_corner(dst, dst_move_flag, next.point, src_h.p1[1], flip, false);
			}
			
			prev = next;
		}
		
		intersections.clear();
	}
	
	if (dst_size < dst.get_chunks().size())
		dst.close();
}

