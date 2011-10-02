/* === S Y N F I G ========================================================= */
/*!	\file wplistconverter.cpp
**	\brief Implementation for a converter between widths and positions
**	form a extended device and a equivalent ValueNode_WPList
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#endif

#include "wplistconverter.h"
#include <synfig/valuenode_wplist.h>


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

WPListConverter::WPListConverter()
{
	err2max=0.01;
}

void
WPListConverter::operator()(std::list<synfig::WidthPoint> &wp_out, const std::list<synfig::Point> &p, const std::list<synfig::Real> &w)
{
	// number of data (points and widths)
	unsigned int n;
	// number of data (float format)
	Real nf;
	// indexes k1, k2 for the interval considered, kem where the error
	// is maximum
	unsigned int k1, k2, kem;
	// return if less than two points
	if (p.size() < 2)
		return;
	// Maybe this happens so for the moment we just bail
	if(p.size()!=w.size())
	{
		synfig::info("sizes don't match Points size = %d , Widths size = %d", p.size(), w.size());
		return;
	}
	// Remove duplicated
	std::list<synfig::Point>::const_iterator p_iter = p.begin(), end = p.end();
	std::list<synfig::Real>::const_iterator	w_iter = w.begin();
	Point c(*p_iter);
	points.push_back(c);
	widths.push_back(*w_iter);
	p_iter++;
	for(;p_iter != end; ++p_iter,++w_iter)
		if (*p_iter != c)
		{
			points.push_back(c = *p_iter);
			widths.push_back(*w_iter);
		}
	// once removed the duplicated then get the sizes of the work vectors
	n=points.size();
	nf=Real(n);
	// Calculate the cumulative distances
	Point p1(points[0]), p2;
	Real d(0);
	unsigned int i;
	for(i=0;i<n;i++)
	{
		p2=points[i];
		d+=(p2-p1).mag();
		distances.push_back(d);
		p1=p2;
	}
	synfig::info("distances size = %d", distances.size());
	// Calculate the normalized cumulative distances
	for(i=0;i<n;i++)
	{
		norm_distances.push_back(distances[i]/distances[n]);
	}
	// Prepare the output
	work_out.resize(n);
	// Prepare the errors

	// Initially I insert all widthpoints with a dash set to true
	// Why?: dash=true means that the widthpoint has to be discarded later
	// Only setting dash to false will validate the widhtpoint based on the
	// error rules.
	for(i=0; i<n; i++)
		work_out[i]=WidthPoint(widths[i], norm_distances[i], WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE, true);
	// Now let's insert the first two widthpoints:
	work_out[0].set_dash(false);
	work_out[n-1].set_dash(false);
}

unsigned int
WPListConverter::calculate_ek2(unsigned int k1, unsigned int k2, Real &e)
{
	// remember: k2 is one more past the interval
	unsigned int i;
	Real g;
	for(i=k1;i<k2;i++)
	{
		WidthPoint wp_prev(work_out[find_prev(i)]);
		WidthPoint wp_next(work_out[find_next(i)]);
		g=ek[i]=widths[i]-widthpoint_interpolate(wp_prev, wp_next, norm_distances[i], false);
		ek2[i]=g*g;
	}
	// work in progress...
	return 0;
}

unsigned int
WPListConverter::find_next(unsigned int k)
{
	// work in progress...
	return k;
}

unsigned int
WPListConverter::find_prev(unsigned int k)
{
	// work in progress...
	return k;
}


void
WPListConverter::clear()
{
	points.clear();
	widths.clear();
	distances.clear();
	norm_distances.clear();
	work_out.clear();
	ek.clear();
	ek2.clear();
}

/* === E N T R Y P O I N T ================================================= */


