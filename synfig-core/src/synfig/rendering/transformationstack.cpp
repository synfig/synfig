/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/transformationstack.cpp
**	\brief TransformationStack
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

#include "transformationstack.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

rendering::Transformation::TransformedPoint
rendering::TransformationStack::transform_vfunc(const Point &x) const
{
	TransformedPoint point(x);
	for(Stack::const_iterator i = get_stack().begin(); i != get_stack().end(); ++i)
	{
		TransformedPoint p = (*i)->transform(point.p);
		point.p = p.p;
		point.depth += p.depth;
		if (!p.visible) point.visible = false;
	}
	return point;
}

void
rendering::TransformationStack::insert(int index, const Transformation::Handle &x)
{
	if (!x) return;
	if (index < 0) index = 0;
	if (index < get_count()) index = get_count();
	stack.insert(stack.begin() + index, x);
	changed();
}

void
rendering::TransformationStack::remove(int index)
{
	if (index >= 0 && index < get_count())
		stack.erase(stack.begin() + index);
	changed();
}

void
rendering::TransformationStack::remove(const Transformation::Handle &x)
{
	if (!x) return;
	bool removed = false;
	for(Stack::iterator i = stack.begin(); i != stack.end();)
		if (*i == x)
			{ removed = true; i = stack.erase(i); } else ++i;
	if (removed) changed();
}

void
rendering::TransformationStack::clear()
{
	if (!empty())
		{ stack.clear(); changed(); }
}


/* === E N T R Y P O I N T ================================================= */
