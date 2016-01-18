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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

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



void
Contour::clear()
{
	if (!chunks.empty()) {
		chunks.clear();
		first = 0;
	}
}

void
Contour::move_to(const Vector &v)
{
	if (chunks.empty()) {
		first = chunks.size();
		chunks.push_back(Chunk(MOVE, v));
	} else {
		if (!v.is_equal_to(chunks.back().p1)) {
			if (chunks.back().type == MOVE)
			{
				chunks.back().p1 = v;
			}
			else
			if (chunks.back().type == CLOSE)
			{
				chunks.push_back(Chunk(MOVE, v));
			}
			else
			{
				chunks.push_back(Chunk(CLOSE, chunks[first].p1));
				chunks.push_back(Chunk(MOVE, v));
			}
			first = chunks.size();
		}
	}
}

void
Contour::line_to(const Vector &v)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
		chunks.push_back(Chunk(LINE, v));
}

void
Contour::conic_to(const Vector &v, const Vector &pp0)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
		chunks.push_back(Chunk(CONIC, v, pp0));
}

void
Contour::cubic_to(const Vector &v, const Vector &pp0, const Vector &pp1)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
		chunks.push_back(Chunk(CUBIC, v, pp0, pp1));
}

void
Contour::close()
{
	if (chunks.size() > first)
	{
		chunks.push_back(Chunk(CLOSE, chunks[first].p1));
		first = chunks.size();
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
}

Rect
Contour::calc_bounds(const Matrix &transform_matrix) const
{
	if (chunks.empty()) return Rect::zero();
	Rect bounds(chunks.front().p1);
	for(ChunkList::const_iterator i = chunks.begin(); i != chunks.end(); ++i) {
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
	}
	return bounds;
}

void
Contour::split(
	Contour &out_contour,
	Rect &ref_bounds,
	const Vector &min_size ) const
{
	Helper::SplitParams<Contour> params(out_contour, ref_bounds, min_size);
	Helper::contour_split(params, *this);
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

/* === E N T R Y P O I N T ================================================= */
