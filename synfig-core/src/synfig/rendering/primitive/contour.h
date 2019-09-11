/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/contour.h
**	\brief Contour Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_CONTOUR_H
#define __SYNFIG_RENDERING_CONTOUR_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <ETL/handle>

#include <synfig/vector.h>
#include <synfig/rect.h>
#include <synfig/matrix.h>
#include <synfig/color.h>
#include <synfig/mutex.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Intersector;

class Contour: public etl::shared_object
{
public:
	typedef etl::handle<Contour> Handle;

	enum WindingStyle
	{
	    WINDING_NON_ZERO = 0,  //!< less than -1 --> 1;  -1 --> 1;   0 --> 0;   1 --> 1;  greater than 1 --> 1
	    WINDING_EVEN_ODD = 1,  //!< add or subtract multiples of 2 to get into range -1:1, then as above
	};

	enum {
	    WINDING_END = 2        //!< \internal
	};

	enum ChunkType {
		CLOSE,
		MOVE,
		LINE,
		CONIC,
		CUBIC
	};

	struct Chunk {
		ChunkType type;
		Vector p1;
		Vector pp0, pp1; // intermediate points for CONIC and CUBIC
		Chunk(): type() { }
		Chunk(ChunkType type, const Vector &p1):
			type(type), p1(p1), pp0(p1), pp1(p1) { }
		Chunk(ChunkType type, const Vector &p1, const Vector &pp0):
			type(type), p1(p1), pp0(pp0), pp1(pp0) { }
		Chunk(ChunkType type, const Vector &p1, const Vector &pp0, const Vector &pp1):
			type(type), p1(p1), pp0(pp0), pp1(pp1) { }

		explicit Chunk(const Vector &p1):
			type(LINE), p1(p1), pp0(p1), pp1(p1) { }
		Chunk(const Vector &p1, const Vector &pp0):
			type(CONIC), p1(p1), pp0(pp0), pp1(pp0) { }
		Chunk(const Vector &p1, const Vector &pp0, const Vector &pp1):
			type(CUBIC), p1(p1), pp0(pp0), pp1(pp1) { }
	};

	typedef std::vector<Chunk> ChunkList;

private:
	class Helper;

	ChunkList chunks;
	int first;
	bool autocurve_begin, autocurve_end;
	
	mutable Mutex bounds_read_mutex;
	mutable bool bounds_calculated;
	mutable Rect bounds;
	
	mutable Mutex intersector_read_mutex;
	mutable etl::handle<Intersector> intersector;

	//! call this when 'chunks' or 'first' was changed
	void touch_chunks();

public:
	// TODO: is it valid place for these fields
	bool invert;
	bool antialias;
	WindingStyle winding_style;
	Color color;

	Contour();
	~Contour();

	inline bool closed() const
		{ return (int)chunks.size() <= first; }
	inline int beginning_of_unclosed() const
		{ return first; }
	
	void reserve(size_t size)
		{ chunks.reserve(chunks.size() + size); }
	
	void clear();
	void move_to(const Vector &v);
	void line_to(const Vector &v);
	void conic_to(const Vector &v, const Vector &pp0);
	void cubic_to(const Vector &v, const Vector &pp0, const Vector &pp1);
	void autocurve_to(const Vector &v, bool corner = false);
	void autocurve_corner();
	void close();

	void remove_collapsed_tail();
	
	void close_mirrored(const Matrix &transform);
	void close_mirrored_hor()
		{ close_mirrored(Matrix().set_scale(-1, 1)); }
	void close_mirrored_vert()
		{ close_mirrored(Matrix().set_scale(1, -1)); }
	
	void assign(const Contour &other);
	
	void add_chunk(const Chunk &chunk);
	void add_chunks(const Chunk *begin, const Chunk *end);
	void add_chunks(const ChunkList &chunks)
		{ if (!chunks.empty()) add_chunks(&chunks.front(), &chunks.back() + 1); }

	//! attach list from (end-1) to begin
	//! list will be attached as line
	//! curve information for first segment (begin) of incoming list will be ignored, only p1 will be used
	void add_chunks_reverse(const Chunk *begin, const Chunk *end);
	void add_chunks_reverse(const ChunkList &chunks)
		{ if (!chunks.empty()) add_chunks_reverse(&chunks.front(), &chunks.back() + 1); }
		
	void arc(const Vector &center, Real radius, Real angle0, Real angle1, bool connect = false);

	const ChunkList& get_chunks() const { return chunks; }

	Rect calc_bounds() const;
	Rect calc_bounds(const Matrix &transform_matrix) const;

	//! actualize internal value of bounds (if needed) and return it
	//! method is thread-safe for constant contours - you must not modify a contour while this call
	Rect get_bounds() const;
	
	void to_intersector(Intersector &intersector) const;
	etl::handle<Intersector> crerate_intersector() const;

	//! actualize internal copy of intersector (if needed) and return it
	//! method is thread-safe for constant contours - you must not modify a contour while this call
	const Intersector& get_intersector() const;

	void split(
		Contour &out_contour,
		Rect &ref_bounds,
		const Vector &min_size ) const;
	void split(
		std::vector<Vector> &out_contour,
		Rect &ref_bounds,
		const Vector &min_size ) const;

	void split(
		Contour &out_contour,
		Rect &ref_bounds,
		const Matrix &transform_matrix,
		const Vector &min_size ) const;
	void split(
		std::vector<Vector> &out_contour,
		Rect &ref_bounds,
		const Matrix &transform_matrix,
		const Vector &min_size ) const;

	static Rect line_bounds(
		const Vector &p0,
		const Vector &p1 )
	{ return Rect(p0).expand(p1); }

	static Rect conic_bounds(
		const Vector &p0,
		const Vector &p1,
		const Vector &pp0 )
	{ return Rect(p0).expand(p1).expand(pp0); }

	static Rect cubic_bounds(
		const Vector &p0,
		const Vector &p1,
		const Vector &pp0,
		const Vector &pp1 )
	{ return Rect(p0).expand(p1).expand(pp0).expand(pp1); }
	
	static bool check_is_inside(int intersections, WindingStyle winding_style = WINDING_NON_ZERO, bool invert = false);
	bool is_inside(int intersections) const
		{ return check_is_inside(intersections, winding_style, invert); }
	bool is_inside(const Point &p, WindingStyle winding_style, bool invert) const;
	bool is_inside(const Point &p) const
		{ return is_inside(p, winding_style, invert); }

	static void reverse(Chunk *begin, Chunk *end, Vector &first);
	static void reverse(ChunkList &chunks, Vector &first)
		{ if (!chunks.empty()) reverse(&chunks.front(), &chunks.back() + 1, first); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
