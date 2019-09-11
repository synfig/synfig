/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/contour.cpp
**	\brief Contour
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#include <algorithm>

#include <synfig/general.h>

#include "intersector.h"

#include "contour.h"


#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class Contour::Helper {
public:
	template<typename T>
	struct SplitParams {
		typedef T ContourType;

		ContourType &out_contour;
		Rect &out_bounds;
		const Rect bounds;
		const Vector min_size;
		const Vector min_line_size;

		Rect line_bounds;
		int level;

		ChunkType prev_type;
		Vector *prev_point;
		Vector zero;

		SplitParams(
			ContourType &out_contour,
			Rect &ref_bounds,
			const Vector &min_size
		):
			out_contour(out_contour),
			out_bounds(ref_bounds),
			bounds(ref_bounds),
			min_size(min_size),
			min_line_size(min_size*0.5),
			level(46),
			prev_type(),
			prev_point(),
			zero()
		{ out_bounds = Rect(ref_bounds.minx, ref_bounds.maxx); }

		const Vector& get_p0() const { return prev_point ? *prev_point : zero; }
	};

	static Vector& move_to(Contour &out_contour, const Vector &p1)
		{ out_contour.chunks.push_back(Chunk(MOVE, p1)); return out_contour.chunks.back().p1; }
	static Vector& line_to(Contour &out_contour, const Vector &p1)
		{ out_contour.chunks.push_back(Chunk(LINE, p1)); return out_contour.chunks.back().p1; }
	static Vector& close(Contour &out_contour, const Vector &p1)
		{ out_contour.chunks.push_back(Chunk(CLOSE, p1)); return out_contour.chunks.back().p1; }

	static Vector& move_to(std::vector<Vector> &out_contour, const Vector &p1)
		{ out_contour.push_back(p1); return out_contour.back(); }
	static Vector& line_to(std::vector<Vector> &out_contour, const Vector &p1)
		{ out_contour.push_back(p1); return out_contour.back(); }
	static Vector& close(std::vector<Vector> &out_contour, const Vector &p1)
	{
		out_contour.push_back(p1);
		out_contour.push_back(out_contour.front());
		return *(out_contour.rbegin() + 1);
	}

	template<typename T>
	static void move_to(SplitParams<T> &params, const Vector &p1)
	{
		if (params.prev_point)
			params.out_bounds.expand(p1);
		else
			params.out_bounds = Rect(p1);

		if (params.prev_point && params.prev_type == MOVE)
		{
			*params.prev_point = p1;
			return;
		}
		params.prev_type = MOVE;
		params.prev_point = &move_to(params.out_contour, p1);
	}

	template<typename T>
	static void line_to(SplitParams<T> &params, const Vector &p1)
	{
		if (!params.prev_point) {
			move_to(params, params.zero);
		} else
		if (params.prev_type == CLOSE) {
			move_to(params, *params.prev_point);
		} else
		if (params.prev_type == LINE) {
			params.line_bounds = params.line_bounds.expand(p1);
			if ( !etl::intersect(params.bounds, params.line_bounds)
			  || ( params.line_bounds.maxx - params.line_bounds.minx <= params.min_line_size[0]
				&& params.line_bounds.maxy - params.line_bounds.miny <= params.min_line_size[0] ))
			{
				*params.prev_point = p1;
				params.out_bounds.expand(p1);
				return;
			}
		}

		params.prev_type = LINE;
		params.line_bounds = Rect(*params.prev_point, p1);
		params.prev_point = &line_to(params.out_contour, p1);
		params.out_bounds.expand(p1);
	}

	template<typename T>
	static void close(SplitParams<T> &params, const Vector &p1)
	{
		if (!params.prev_point || params.prev_type != LINE)
			return;
		params.prev_type = CLOSE;
		params.prev_point = &close(params.out_contour, p1);
	}

	template<typename T>
	static void conic_split(
		SplitParams<T> &params,
		const Vector &p1,
		const Vector &pp0 )
	{
		assert(params.level > 0);
		const Vector &p0 = params.get_p0();
		Rect bounds = conic_bounds(p0, p1, pp0);
		if ( !etl::intersect(params.bounds, bounds)
		  || ( bounds.maxx - bounds.minx <= params.min_size[0]
			&& bounds.maxy - bounds.miny <= params.min_size[0] ))
		{
			line_to(params, p1);
			return;
		}
		Vector a0 = (p0 + pp0)*0.5;
		Vector a1 = (pp0 + p1)*0.5;
		Vector b = (a0 + a1)*0.5;
		--params.level;
		conic_split(params, b, a0);
		conic_split(params, p1, a1);
		++params.level;
	}

	template<typename T>
	static void cubic_split(
		SplitParams<T> &params,
		const Vector &p1,
		const Vector &pp0,
		const Vector &pp1 )
	{
		assert(params.level > 0);
		const Vector &p0 = params.get_p0();
		Rect b = cubic_bounds(p0, p1, pp0, pp1);
		if ( !etl::intersect(params.bounds, b)
		  || ( b.maxx - b.minx <= params.min_size[0]
			&& b.maxy - b.miny <= params.min_size[0] ))
		{
			line_to(params, p1);
			return;
		}
		Vector a0 = (p0 + pp0)*0.5;
		Vector a1 = (pp0 + pp1)*0.5;
		Vector a2 = (pp1 + p1)*0.5;
		Vector b0 = (a0 + a1)*0.5;
		Vector b1 = (a1 + a2)*0.5;
		Vector c = (b0 + b1)*0.5;
		--params.level;
		cubic_split(params, c, a0, b0);
		cubic_split(params, p1, b1, a1);
		++params.level;
	}

	template<typename T>
	static void contour_split(SplitParams<T> &params, const Contour &contour)
	{
		for(ChunkList::const_iterator i = contour.get_chunks().begin(); i != contour.get_chunks().end(); ++i) {
			switch(i->type) {
			case CLOSE:
				Helper::close(params, i->p1);
				break;
			case MOVE:
				Helper::move_to(params, i->p1);
				break;
			case LINE:
				Helper::line_to(params, i->p1);
				break;
			case CONIC:
				Helper::conic_split(params, i->p1, i->pp0);
				break;
			case CUBIC:
				Helper::cubic_split(params, i->p1, i->pp0, i->pp1);
				break;
			default:
				break;
			}
		}
	}

	template<typename T>
	static void contour_split(SplitParams<T> &params, const Contour &contour, const Matrix &transform_matrix)
	{
		Vector p1, pp0, pp1;
		for(ChunkList::const_iterator i = contour.get_chunks().begin(); i != contour.get_chunks().end(); ++i) {
			switch(i->type) {
			case CLOSE:
				p1 = transform_matrix.get_transformed(i->p1);
				Helper::close(params, p1);
				break;
			case MOVE:
				p1 = transform_matrix.get_transformed(i->p1);
				Helper::move_to(params, p1);
				break;
			case LINE:
				p1 = transform_matrix.get_transformed(i->p1);
				Helper::line_to(params, p1);
				break;
			case CONIC:
				p1 = transform_matrix.get_transformed(i->p1);
				pp0 = transform_matrix.get_transformed(i->pp0);
				Helper::conic_split(params, p1, pp0);
				break;
			case CUBIC:
				p1 = transform_matrix.get_transformed(i->p1);
				pp0 = transform_matrix.get_transformed(i->pp0);
				pp1 = transform_matrix.get_transformed(i->pp1);
				Helper::cubic_split(params, p1, pp0, pp1);
				break;
			default:
				break;
			}
		}
	}
};


Contour::Contour():
	first(0),
	autocurve_begin(false),
	autocurve_end(false),
	bounds_calculated(false),
	invert(false),
	antialias(false),
	winding_style(WINDING_NON_ZERO)
	{ }

Contour::~Contour()
	{ }

void
Contour::touch_chunks()
{
	autocurve_end = false;
	bounds_calculated = false;
	intersector.reset();
}

void
Contour::clear()
{
	if (!chunks.empty()) {
		chunks.clear();
		first = 0;
		touch_chunks();
	}
}

void
Contour::move_to(const Vector &v)
{
	autocurve_end = false;
	if (closed()) {
		chunks.push_back(Chunk(MOVE, v));
		touch_chunks();
	} else
	if (!v.is_equal_to(chunks.back().p1)) {
		if (chunks.back().type == MOVE) {
			chunks.back().p1 = v;
			touch_chunks();
		} else {
			close();
			move_to(v);
		}
	}
}

void
Contour::line_to(const Vector &v)
{
	autocurve_end = false;
	Vector prev = chunks.empty() ? Vector::zero() : chunks.back().p1;
	if (closed()) move_to(prev);
	if (!v.is_equal_to(prev)) {
		chunks.push_back(Chunk(LINE, v));
		touch_chunks();
	}
}

void
Contour::conic_to(const Vector &v, const Vector &pp0)
{
	autocurve_end = false;
	Vector prev = chunks.empty() ? Vector::zero() : chunks.back().p1;
	if (closed()) move_to(prev);
	if (!v.is_equal_to(prev)) {
		chunks.push_back(Chunk(CONIC, v, pp0));
		touch_chunks();
	}
}

void
Contour::cubic_to(const Vector &v, const Vector &pp0, const Vector &pp1)
{
	autocurve_end = false;
	Vector prev = chunks.empty() ? Vector::zero() : chunks.back().p1;
	if (closed()) move_to(prev);
	if ( !v.is_equal_to(prev)
	  || (!pp0.is_equal_to(pp1) && !pp0.is_equal_to(prev) && !pp1.is_equal_to(prev)) )
	{
		chunks.push_back(Chunk(CUBIC, v, pp0, pp1));
		touch_chunks();
	}
}

void
Contour::autocurve_corner()
	{ autocurve_end = false; }

void
Contour::autocurve_to(const Vector &v, bool corner)
{
	bool curve = autocurve_end;
	Vector prev = chunks.empty() ? Vector::zero() : chunks.back().p1;
	if (closed()) move_to(prev);
	if (!v.is_equal_to(prev)) {
		const Real kline = Real(1)/3;
		const Real ksmooth = Real(0.5)/3;
		
		if (!chunks.empty() && chunks.back().type == MOVE)
			autocurve_begin = true;
		
		Vector d = (v - prev)*kline;
		Vector pp0 = prev + d;
		Vector pp1 = v - d ;
		if (curve && !chunks.empty()) {
			ChunkList::iterator i0 = chunks.end(); --i0;
			if (i0 != chunks.begin() && i0->type == CUBIC) {
				ChunkList::iterator i1 = i0--;
				if (!prev.is_equal_to(i0->p1)) {
					Vector d0 = prev - i0->p1;
					Vector d1 = v - prev;
					Real l0 = d0.mag();
					Real l1 = d1.mag();
					i1->pp1 = i1->p1 - (d0 + d1*(l0/l1))*ksmooth;
					pp0 = i1->p1 + (d0*(l1/l0) + d1)*ksmooth;
				}
			}
		}
		chunks.push_back(Chunk(CUBIC, v, pp0, pp1));
		touch_chunks();
	}
	autocurve_end = !corner;
}

void
Contour::close()
{
	if (!closed()) {
		assert(chunks.back().type != CLOSE);
		Vector v = chunks[first].p1;
		if (autocurve_end) autocurve_to(v); else autocurve_begin = false;
		if (autocurve_begin && first + 1 < (int)chunks.size()) {
			reserve(1);
			Chunk &chunk = chunks[first + 1];
			if (!v.is_equal_to( chunk.p1 )) {
				autocurve_to(chunk.p1);
				chunk.pp0 = chunks.back().pp0;
				chunks.back() = Chunk(CLOSE, v);
			} else chunks.push_back(Chunk(CLOSE, v));
		} else chunks.push_back(Chunk(CLOSE, v));
		first = (int)chunks.size();
		touch_chunks();
		autocurve_begin = false;
	}
}

void
Contour::remove_collapsed_tail()
{
	if ((int)chunks.size() > 2) {
		ChunkList::iterator i0 = chunks.end(), i2 = --i0, i1 = (--i0)--;
		if ( i2->type == i1->type
		  && i2->p1.is_equal_to( i0->p1 )
		  && i2->pp0.is_equal_to( i1->pp1 )
		  && i2->pp1.is_equal_to( i1->pp0 ) )
			chunks.pop_back();
	}
}

void
Contour::assign(const Contour &other)
{
	chunks = other.chunks;
	first = other.first;
	invert = other.invert;
	antialias = other.antialias;
	winding_style = other.winding_style;
	color = other.color;
	
	// other contour is constant, so we need to lock mutexes for read the bounds and intersector
	// see comments for get_bounds() and get_intersector() declarations
	{
		Mutex::Lock lock(other.bounds_read_mutex);
		bounds_calculated = other.bounds_calculated;
		bounds = other.bounds;
	}
	{
		Mutex::Lock lock(other.intersector_read_mutex);
		intersector = other.intersector;
	}
}

void
Contour::add_chunk(const Chunk &chunk) {
	switch(chunk.type) {
	case CLOSE: close(); break;
	case LINE:  line_to (chunk.p1); break;
	case MOVE:  move_to (chunk.p1); break;
	case CONIC: conic_to(chunk.p1, chunk.pp0); break;
	case CUBIC: cubic_to(chunk.p1, chunk.pp0, chunk.pp1); break;
	default: break;
	}
}

void
Contour::add_chunks(const Chunk *begin, const Chunk *end)
{
	assert(begin);
	while(begin < end) add_chunk(*(begin++));
}

void
Contour::add_chunks_reverse(const Chunk *begin, const Chunk *end)
{
	assert(begin);
	if (end <= begin) return;
	line_to((end-1)->p1);
	for(const Chunk *curr = end - 1, *prev = curr - 1; prev >= begin; curr = prev--)
		switch(curr->type) {
		case CLOSE:
		case LINE:  line_to (prev->p1); break;
		case MOVE:  move_to (prev->p1); break;
		case CONIC: conic_to(prev->p1, curr->pp0); break;
		case CUBIC: cubic_to(prev->p1, curr->pp1, curr->pp0); break;
		default: break;
		}
}

void Contour::reverse(Chunk *begin, Chunk *end, Vector &first) {
	// scroll p1 values
	Vector f = first;
	for(Chunk *c = begin; c < end; ++c)
		std::swap(c->p1, f);
	first = f;

	// just flip
	Chunk *b = begin, *e = end - 1;
	while(b < e) {
		std::swap(b->type, e->type);
		std::swap(b->p1, e->p1);
		std::swap(b->pp0, e->pp1);
		std::swap(b->pp1, e->pp0);
		++b; --e;
	}
	if (b == e)
		std::swap(b->pp0, b->pp1);
}


void
Contour::arc(const Vector &center, Real radius, Real angle0, Real angle1, bool connect)
{
	Real angle = angle0;
	Vector p0 = center + Vector(radius*cos(angle), radius*sin(angle));
	if (connect) line_to(p0); else move_to(p0);

	int segments = (int)ceil(4.0*fabs(angle1 - angle0)/PI);
	if (segments <= 0) return;
	Real da = 0.5*(angle1 - angle0)/segments;
	Real radius2 = radius/cos(da);
	for(int i = 0; i < segments; ++i) {
		angle += da;
		Vector pp0 = center + Vector(radius2*cos(angle), radius2*sin(angle));
		angle += da;
		Vector p1 = center + Vector(radius*cos(angle), radius*sin(angle));
		conic_to(p1, pp0);
	}
}

Rect
Contour::calc_bounds() const
{
	if (chunks.empty()) return Rect::zero();
	Rect bounds(chunks.front().p1);
	for(ChunkList::const_iterator i = chunks.begin(); i != chunks.end(); ++i)
		switch(i->type) {
		case CUBIC:
			bounds.expand(i->pp1);
		case CONIC:
			bounds.expand(i->pp0);
		case CLOSE:
		case MOVE:
		case LINE:
			bounds.expand(i->p1);
		default:
			break;
		}
	return bounds;
}

Rect
Contour::calc_bounds(const Matrix &transform_matrix) const
{
	if (chunks.empty()) return Rect::zero();
	Rect bounds( transform_matrix.get_transformed(chunks.front().p1) );
	for(ChunkList::const_iterator i = chunks.begin(); i != chunks.end(); ++i)
		switch(i->type) {
		case CUBIC:
			bounds.expand( transform_matrix.get_transformed(i->pp1) );
		case CONIC:
			bounds.expand( transform_matrix.get_transformed(i->pp0) );
		case CLOSE:
		case MOVE:
		case LINE:
			bounds.expand( transform_matrix.get_transformed(i->p1) );
		default:
			break;
		}
	return bounds;
}

Rect
Contour::get_bounds() const
{
	Mutex::Lock lock(bounds_read_mutex);
	if (!bounds_calculated) {
		bounds = calc_bounds();
		bounds_calculated = true;
	}
	return bounds;
}

void
Contour::to_intersector(Intersector &intersector) const
{
	if (chunks.empty()) return;
	if (chunks.front().type != MOVE) intersector.move_to(Point());
	for(ChunkList::const_iterator i = chunks.begin(); i != chunks.end(); ++i)
		switch(i->type) {
		case CLOSE:
		case LINE:  intersector.line_to (i->p1); break;
		case MOVE:  intersector.move_to (i->p1); break;
		case CONIC: intersector.conic_to(i->p1, i->pp0); break;
		case CUBIC: intersector.cubic_to(i->p1, i->pp0, i->pp1); break;
		default: break;
		}
}

Intersector::Handle
Contour::crerate_intersector() const
{
	Intersector::Handle intersector(new Intersector());
	to_intersector(*intersector);
	intersector->close();
	return intersector;
}

const Intersector&
Contour::get_intersector() const
{
	Mutex::Lock lock(intersector_read_mutex);
	if (!intersector) {
		intersector = crerate_intersector();
		Mutex::Lock lock(bounds_read_mutex);
		if (!bounds_calculated) {
			bounds = intersector->get_bounds();
			bounds_calculated = true;
		}
	}
	return *intersector;
}

void
Contour::split(
	Contour &out_contour,
	Rect &ref_bounds,
	const Vector &min_size ) const
{
	Helper::SplitParams<Contour> params(out_contour, ref_bounds, min_size);
	Helper::contour_split(params, *this);
	out_contour.touch_chunks();
}

void
Contour::split(
	std::vector<Vector> &out_contour,
	Rect &ref_bounds,
	const Vector &min_size ) const
{
	Helper::SplitParams< std::vector<Vector> > params(out_contour, ref_bounds, min_size);
	Helper::contour_split(params, *this);
}

void
Contour::split(
	Contour &out_contour,
	Rect &ref_bounds,
	const Matrix &transform_matrix,
	const Vector &min_size ) const
{
	Helper::SplitParams<Contour> params(out_contour, ref_bounds, min_size);
	Helper::contour_split(params, *this, transform_matrix);
	out_contour.touch_chunks();
}

void
Contour::split(
	std::vector<Vector> &out_contour,
	Rect &ref_bounds,
	const Matrix &transform_matrix,
	const Vector &min_size ) const
{
	Helper::SplitParams< std::vector<Vector> > params(out_contour, ref_bounds, min_size);
	Helper::contour_split(params, *this, transform_matrix);
}

bool
Contour::check_is_inside(int intersections, WindingStyle winding_style, bool invert)
{
	bool inside = winding_style == WINDING_NON_ZERO ? !!intersections
	            : winding_style == WINDING_EVEN_ODD ? !!(intersections % 2)
				: false;
	return invert ? !inside : inside;
}

bool
Contour::is_inside(const Point &p, WindingStyle winding_style, bool invert) const
	{ return check_is_inside(get_intersector().intersect(p), winding_style, invert); }

void
Contour::close_mirrored(const Matrix &transform)
{
	int count = (int)chunks.size() - first;
	if (first >= 0 && count > 0)
	{
		reserve(count);
		for(ChunkList::const_reverse_iterator ri1 = chunks.rbegin(), rend = ri1 + count, ri0 = ri1++; ri1 != rend; ri0 = ri1++)
			add_chunk( Chunk(
				ri0->type,
				transform.get_transformed(ri1->p1),
				transform.get_transformed(ri0->pp1),
				transform.get_transformed(ri0->pp0) ));
		close();
	}
}
	
/* === E N T R Y P O I N T ================================================= */

