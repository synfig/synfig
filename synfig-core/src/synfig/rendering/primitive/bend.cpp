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

		Vector tn0 = (flip ? point.t1 : point.t0).norm();
		Vector tn1 = (flip ? point.t0 : point.t1).norm();
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
		
		if (point.mode != Bend::FLAT && !codirected && (d*radius < 0) == flip) {
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
		point.p = last.p;
		point.length = last.length;
		Real l0 = last.l, l = step;
		for(int i = 2; i < segments; ++i, l += step) {
			Vector pp = h.p(l);
			point.l = l0 + l;
			point.length = calc_length ? point.length + (pp - point.p).mag() : point.l;
			point.p = pp;
			point.t0 = point.t1 = h.t(l);
			points.push_back(point);
		}
		point.l = l0 + 1;
		point.length = calc_length ? point.length + (p - point.p).mag() : point.l;
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
	
	first.t0 = last.t0;
	last.t1  = first.t1;
	first.e0 = last.e1 = false;
}

void
Bend::tails()
{
	if (points.size() < 2u) return;

	{
		PointList::iterator i1 = points.begin(), i0 = i1++;
		Real dl = i1->l - i0->l;
		Hermite h(i0->p, i1->p, i0->t1*dl, i1->t0*dl);
		i0->t0 = h.d(0);
		i0->mode = NONE;
	}

	{
		PointList::iterator i0 = points.end(), i1 = (--i0)--;
		Real dl = i1->l - i0->l;
		Hermite h(i0->p, i1->p, i0->t1*dl, i1->t0*dl);
		i1->t1 = h.d(1);
		i1->mode = NONE;
	}
}


Bend::PointList::const_iterator
Bend::find_by_l(Real l) const
{
	if (points.empty())
		return points.end();
	PointList::const_iterator a = points.begin(), b = points.end() - 1, c;
	if (!approximate_less(a->l, l)) return a;
	if (!approximate_less(l, b->l)) return b;
	while( (c = a + (b - a)/2) != a ) {
		if (approximate_equal(l, c->l)) return c;
		(l < c->l ? b : a) = c;
	}
	return c;
}

Bend::PointList::const_iterator
Bend::find(Real length) const
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

Real Bend::length_by_l(Real l) const {
	PointList::const_iterator i = find_by_l(l);
	if (i == points.end())
		return 0;
	
	if (approximate_equal(l, i->l)) return i->length;
	if (l < i->l)
		return i->length + (l - i->l);
	
	PointList::const_iterator j = i + 1;
	if (j == points.end())
		return i->length + (l - i->l);
	
	Real dl = j->l - i->l;
	return approximate_zero(dl) ? i->length : i->length + (j->length - i->length)*(l - i->l)/l;
}

Bend::Point
Bend::interpolate(Real length) const
{
	PointList::const_iterator i = find(length);
	if (i == points.end()) return Point();
	
	if (approximate_equal(length, i->length)) return *i;
	
	if (length < i->length) {
		// extarpolation before
		Real dlen = length - i->length;
		Point s;
		s.p = i->p + i->t0*dlen;
		s.t0 = s.t1 = i->t0;
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
		s.p = i->p + i->t1*dlen;
		s.t0 = s.t1 = i->t1;
		s.l = i->l + dlen;
		s.length = length;
		s.e0 = s.e1 = i->e1;
		return s;
	}
	
	// interpolation
	Real l = j->length - i->length;
	l = approximate_zero(l) ? 0 : (length - i->length)/l;
	Real dl = j->l - i->l;
	Hermite h(i->p, j->p, i->t1*dl, j->t0*dl);
	
	Point s;
	s.p = h.p(l);
	s.t0 = s.t1 = h.t(l);
	s.l = i->l + l*(j->l - i->l);
	s.length = length;
	s.e0 = s.e1 = (i->e1 && j->e0);
	
	return s;
}

void
Bend::bend(Contour &dst, const Contour &src, const Matrix &matrix, int segments) const
{
	if (points.empty() || src.get_chunks().empty())
		return;

	const int last_segment = segments - 1;
	const Real step = Real(1)/segments;
	Contour::ChunkList::const_iterator i = src.get_chunks().begin();
	Vector p0 = matrix.get_transformed(i->p1);
	size_t dst_size = dst.get_chunks().size();
	bool dst_move_flag = true;
	
	IntersectionList intersections;
	while(++i != src.get_chunks().end()) {
		Bezier b;
		switch(i->type) {
			case Contour::CUBIC:
				b = Bezier(
					p0,
					matrix.get_transformed(i->p1),
					matrix.get_transformed(i->pp0),
					matrix.get_transformed(i->pp1) );
				break;
			case Contour::CONIC:
				b = Bezier(
					p0,
					matrix.get_transformed(i->p1),
					matrix.get_transformed(i->pp0) );
				break;
			case Contour::MOVE:
				p0 = matrix.get_transformed(i->p1);
				dst_move_flag = true;
				continue;
			default: // line
				b = Bezier(p0, matrix.get_transformed(i->p1));
				break;
		}
		p0 = b.p1;
		
		Real bends[3];
		int bends_count = b.bends(0, bends);
		Range r(b.p0[0]);
		r.expand(b.p1[0]);
		for(Real *i = bends, *end = i + bends_count; i != end; ++i)
			r.expand( b.p(0, *i) );
		bends_count += b.inflection(0, bends + bends_count);
		
		for(PointList::const_iterator bi = find(r.min); bi != points.end() && !approximate_less(r.max, bi->length); ++bi) {
			Real roots[3];
			int count = b.intersections(0, roots, bi->length);
			for(Real *i = roots, *end = i + count; i != end; ++i)
				intersections.push_back( Intersection(*i, *bi) );
		}
		for(Real *i = bends, *end = i + bends_count; i != end; ++i)
			intersections.push_back( Intersection(*i, interpolate( b.p(0, *i) )) );
		Real bsl = step;
		for(int i = 1; i < last_segment; ++i, bsl += step)
			intersections.push_back( Intersection(bsl, interpolate( b.p(0, bsl) )) );
		sort(intersections.begin(), intersections.end());
		
		Intersection prev(0, interpolate(b.p0[0]));
		intersections.push_back( Intersection(1, interpolate(b.p1[0])) );
		
		for(IntersectionList::const_iterator bi = intersections.begin(); bi != intersections.end(); ++bi) {
			const Intersection &next = *bi;
			if (approximate_equal(prev.l, next.l)) continue;

			bool flip = next.point.l < prev.point.l;
			bool e0 = flip ? prev.point.e0 : prev.point.e1;
			bool e1 = flip ? next.point.e1 : next.point.e0;
			if (e0 && e1) {
				Real dl = next.point.l - prev.point.l;
				const Vector &t0 = flip ? prev.point.t0 : prev.point.t1;
				const Vector &t1 = flip ? next.point.t1 : next.point.t0;
				
				Hermite bend_h( prev.point.p, next.point.p, t0*dl, t1*dl );
				Bezier src_b = b.sub(prev.l, next.l);
				
				Real k = next.point.length - prev.point.length;
				k = approximate_zero(k) ? 0 : 1/k;
				Real ll0 = (src_b.pp0[0] - prev.point.length)*k;
				Real ll1 = (src_b.pp1[0] - next.point.length)*k + 1;
				Real kk = flip ? -1 : 1;
				Bezier dst_b(
					bend_h.p0     + bend_h.d(  0).perp()*src_b.p0[1]*kk,
					bend_h.p1     + bend_h.d(  1).perp()*src_b.p1[1]*kk,
					bend_h.p(ll0) + bend_h.d(ll0).perp()*src_b.pp0[1]*kk,
					bend_h.p(ll1) + bend_h.d(ll1).perp()*src_b.pp1[1]*kk );

				if (next.point.l == prev.point.l) {
					half_corner(dst, dst_move_flag, prev.point, src_b.p0[1], true, true);
					touch(dst, dst_move_flag, dst_b.p0);
					dst.cubic_to(dst_b.p1, dst_b.pp0, dst_b.pp1);
					half_corner(dst, dst_move_flag, next.point, src_b.p1[1], false, false);
				} else {
					half_corner(dst, dst_move_flag, prev.point, src_b.p0[1], flip, true);
					touch(dst, dst_move_flag, dst_b.p0);
					dst.cubic_to(dst_b.p1, dst_b.pp0, dst_b.pp1);
					half_corner(dst, dst_move_flag, next.point, src_b.p1[1], flip, false);
				}
			}
			
			prev = next;
		}
		
		intersections.clear();
	}
	
	if (dst_size < dst.get_chunks().size())
		dst.close();
}

