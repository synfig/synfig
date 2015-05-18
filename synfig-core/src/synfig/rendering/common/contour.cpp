/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/contour.cpp
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

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// ContourBase

void rendering::ContourBase::set_invert(bool x)
	{ if (get_invert() != x) { invert = x; changed_common_data(); } }

void rendering::ContourBase::set_antialias(bool x)
	{ if (get_antialias() != x) { antialias = x; changed_common_data(); } }

void rendering::ContourBase::set_winding_style(Polyspan::WindingStyle x)
	{ if (get_winding_style() != x) { winding_style = x; changed_common_data(); } }

void rendering::ContourBase::set_color(const Color &x)
	{ if (get_color() != x) { color = x; changed_common_data(); } }

void rendering::ContourBase::apply_common_data(const ContourBase &data)
{
	set_invert(data.get_invert());
	set_antialias(data.get_antialias());
	set_winding_style(data.get_winding_style());
	set_color(data.get_color());
}

void rendering::ContourBase::changed_common_data()
{
	bool repeat = true;
	while(repeat)
	{
		repeat = false;
		for(DependentObject::AlternativeList::const_iterator i = get_alternatives().begin(); i != get_alternatives().end(); ++i)
			if (!Handle::cast_dynamic(*i))
				{ remove_alternative(*i); repeat = true; break; }
	}

	for(DependentObject::AlternativeList::const_iterator i = get_alternatives().begin(); i != get_alternatives().end(); ++i)
		Handle::cast_dynamic(*i)->apply_common_data(*this);
}

// Contour

void
rendering::Contour::clear()
{
	if (!chunks.empty()) {
		chunks.clear();
		first = 0;
		changed();
	}
}

void
rendering::Contour::move_to(const Vector &v)
{
	if (chunks.empty()) {
		if (v.is_equal_to(Vector::zero())) {
			chunks.push_back(Chunk(MOVE, v));
			first = chunks.size();
			changed();
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
				chunks.push_back(Chunk(CLOSE, first));
				chunks.push_back(Chunk(MOVE, v));
			}
			first = chunks.size();
			changed();
		}
	}
}

void
rendering::Contour::line_to(const Vector &v)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
	{
		chunks.push_back(Chunk(LINE, v));
		changed();
	}
}

void
rendering::Contour::conic_to(const Vector &v, const Vector &t)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
	{
		chunks.push_back(Chunk(CONIC, v, t));
		changed();
	}
}

void
rendering::Contour::cubic_to(const Vector &v, const Vector &t0, const Vector &t1)
{
	if (!v.is_equal_to(chunks.empty() ? Vector::zero() : chunks.back().p1))
	{
		chunks.push_back(Chunk(CUBIC, v, t0, t1));
		changed();
	}
}

void
rendering::Contour::close()
{
	if (chunks.size() > first) {
		chunks.push_back(Chunk(CLOSE, chunks[first].p1));
		first = chunks.size();
		changed();
	}
}

/* === E N T R Y P O I N T ================================================= */
