/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/transformation.cpp
**	\brief Transformation
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

#include "transformation.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

rendering::Transformation::TransformedPoint
rendering::Transformation::transform_vfunc(const Point &x) const
{
	return TransformedPoint(x);
}

Point
rendering::Transformation::get_derivation_vfunc(int level, const Point &x, Real epsilon = 1e-6) const
{
	if (level <= 0) return transform_vfunc(x).p;
	return Point(
		( get_derivation_vfunc(level-1, Point(x[0] + epsilon, x[1]), epsilon)[0]
		- get_derivation_vfunc(level-1, Point(x[0] - epsilon, x[1]), epsilon)[0] ) / (2.0*epsilon),
		( get_derivation_vfunc(level-1, Point(x[0], x[1] + epsilon), epsilon)[1]
		- get_derivation_vfunc(level-1, Point(x[0], x[1] - epsilon), epsilon)[1] ) / (2.0*epsilon) );
}

rendering::Transformation::TransformedPoint
rendering::Transformation::transform(const Point &x) const
	{ return transform_vfunc(x); }

Point
rendering::Transformation::get_derivation(int level, const Point &x, Real epsilon = 1e-6) const
	{ return get_derivation_vfunc(level, x, epsilon); }

/* === E N T R Y P O I N T ================================================= */
