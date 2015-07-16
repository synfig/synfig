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
#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Contour: public etl::shared_object
{
public:
	typedef etl::handle<Contour> Handle;

	enum WindingStyle
	{
	    WINDING_NON_ZERO=0,			//!< less than -1 --> 1;  -1 --> 1;   0 --> 0;   1 --> 1;  greater than 1 --> 1
	    WINDING_EVEN_ODD=1,			//!< add or subtract multiples of 2 to get into range -1:1, then as above
	};

	enum {
	    WINDING_END=2				//!< \internal
	};

	enum ChunkType {
		CLOSE,
		MOVE,
		LINE,
		CUBIC,
		CONIC
	};

	struct Chunk {
		ChunkType type;
		Vector p1, t0, t1;
		Chunk(): type() { }
		Chunk(ChunkType type, const Vector &p1, const Vector &t0 = Vector(), const Vector &t1 = Vector()):
			type(type), p1(p1), t0(t0), t1(t1) { }
	};

	typedef std::vector<Chunk> ChunkList;

private:
	ChunkList chunks;
	size_t first;

public:
	// TODO: is it valid place for these fields
	bool invert;
	bool antialias;
	WindingStyle winding_style;
	Color color;

	Contour():
		first(0), invert(false), antialias(false),
		winding_style(WINDING_NON_ZERO)
		{ }

	void clear();
	void move_to(const Vector &v);
	void line_to(const Vector &v);
	void cubic_to(const Vector &v, const Vector &t0, const Vector &t1);
	void conic_to(const Vector &v, const Vector &t);
	void close();

	void assign(const Contour &other);

	const ChunkList& get_chunks() const { return chunks; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
