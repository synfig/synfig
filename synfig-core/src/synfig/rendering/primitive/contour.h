/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/contour.h
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

#include <synfig/rendering/task.h>
#include <cstring>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class ContourBase: public etl::shared_object
{
public:
	typedef etl::handle<ContourBase> Handle;

private:
	bool invert;
	bool antialias;
	Polyspan::WindingStyle winding_style;
	Color color;

public:
	ContourBase(): invert(), antialias(), winding_style(Polyspan::WINDING_NON_ZERO) { }

	bool get_invert() const { return invert; }
	void set_invert(bool x);

	bool get_antialias() const { return antialias; }
	void set_antialias(bool x);

	Polyspan::WindingStyle get_winding_style() const { return winding_style; }
	void set_winding_style(Polyspan::WindingStyle x);

	const Color& get_color() const { return color; }
	void set_color(const Color &x);

	void apply_common_data(const ContourBase &data);
	void changed_common_data();
};

class Contour: public ContourBase
{
public:
	typedef etl::handle<Contour> Handle;

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
	Contour(): first(0) { }

	void clear();
	void move_to(const Vector &v);
	void line_to(const Vector &v);
	void cubic_to(const Vector &v, const Vector &t0, const Vector &t1);
	void conic_to(const Vector &v, const Vector &t);
	void close();

	const ChunkList& get_chunks() const { return chunks; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
