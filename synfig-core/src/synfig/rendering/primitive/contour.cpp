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
		if (v.is_equal_to(Vector::zero())) {
			first = chunks.size();
			chunks.push_back(Chunk(MOVE, v));
		}
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
	{
		chunks.push_back(Chunk(LINE, v));
	}
}

void
Contour::conic_to(const Vector &v, const Vector &t)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
	{
		chunks.push_back(Chunk(CONIC, v, t));
	}
}

void
Contour::cubic_to(const Vector &v, const Vector &t0, const Vector &t1)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
	{
		chunks.push_back(Chunk(CUBIC, v, t0, t1));
	}
}

void
Contour::close()
{
	if (chunks.size() > first) {
		chunks.push_back(Chunk(CLOSE, chunks[first].p1));
		first = chunks.size();
	}
}

/* === E N T R Y P O I N T ================================================= */
