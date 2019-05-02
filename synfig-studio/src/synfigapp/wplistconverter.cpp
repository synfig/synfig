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

#include <synfig/general.h>

#include "wplistconverter.h"
#include <synfig/valuenodes/valuenode_wplist.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

WPListConverter::WPListConverter():
	n(), se(), err2max(0.01)
{ }

void
WPListConverter::operator()(std::list<synfig::WidthPoint> &wp_out, const std::list<synfig::Point> &p, const std::list<synfig::Real> &w)
{
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
	clear();
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
	// Calculate the normalized cumulative distances
	for(i=0;i<n;i++)
		norm_distances.push_back(distances[i]/distances[n-1]);
	// Prepare the output
	work_out.resize(n);
	// Prepare the errors
	ek.resize(n);
	ek2.resize(n);
	// Initially I insert all widthpoints with a dash set to true
	// Why?: dash=true means that the widthpoint has to be discarded later
	// Only setting dash to false will validate the widthpoint based on the
	// error rules.
	for(i=0; i<n; i++)
		work_out[i]=WidthPoint(norm_distances[i], widths[i], WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE, true);
	// Now let's insert the first two widthpoints:
	k1=0;
	k2=n-1;
	work_out[k1].set_dash(false);
	work_out[k2].set_dash(false);
	// Calculate kem, ek, ek2 for the first time.
	kem=calculate_ek2(k1, k2, true);
	while(se>err2max)
	{
		if(k1==k2 || k1+1==k2)
			break;
		// Insert a width point at kem
		work_out[kem].set_dash(false);
		// Calculate kem, ek, & ek2 again
		kem=calculate_ek2(k1, k2);
		// calculate new k1 and k2 (boundaries of kem)
		k1=find_prev(kem);
		k2=find_next(kem);
	}
	wp_out.clear();
	// Let's return the non dashed widthpoints only
	for(i=0;i<n;i++)
		if(!work_out[i].get_dash())
			wp_out.push_back(work_out[i]);
}

unsigned int
WPListConverter::calculate_ek2(unsigned int k1, unsigned int k2, bool first_time)
{
	// remember: k2 is at the interval end
	unsigned int i;
	WidthPoint wp_prev, wp_next;

	if(!first_time)
		se=se*n;
	else
		se=0.0;
	Real g, gg;
	if(k2 <= k1+1)
		return k1; // Maybe throw something?
	for(i=k1;i<=k2;i++)
	{
		if(work_out[i].get_dash())
		{
			wp_prev=work_out[find_prev(i)];
			wp_next=work_out[find_next(i)];
			g=widths[i]-widthpoint_interpolate(wp_prev, wp_next, norm_distances[i], 0.0);
		}
		else
			g=widths[i]-work_out[i].get_width(); // should be zero.
		gg=g*g;
		if(!first_time)
		{
			se-=ek2[i];
			se+=gg;
		}
		else
		{
			se+=gg;
		}
		ek[i]=g;
		ek2[i]=gg;
	}
	se=se/n;
	
	unsigned int ret_kem = 0;
	Real curr_max_err2(-1);

	for(i=0;i<n;i++)
	{
		if(curr_max_err2 < ek2[i])
		{
			ret_kem=i;
			curr_max_err2=ek2[i];
		}
	}
	return ret_kem;
}

unsigned int
WPListConverter::find_next(unsigned int k)
{
	if(k>=n-1) return n-1;
	unsigned int i;
	for (i=k+1; i<n; i++)
		if(!work_out[i].get_dash())
			break;
	return i;
}

unsigned int
WPListConverter::find_prev(unsigned int k)
{
	if(k<1) return 0;
	unsigned int i;
	for (i=k-1; i>0; i--)
		if(!work_out[i].get_dash())
			break;
	return i;
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
	n=0;
}

void
WPListConverter::EnforceMinWidth(std::list<synfig::WidthPoint> &wplist, synfig::Real min_pressure)
{
	std::list<synfig::WidthPoint>::iterator	i(wplist.begin()), end(wplist.end());
	for(; i!= end; ++i)
		if(i->get_width() < min_pressure)
			i->set_width(min_pressure);
}


/* === E N T R Y P O I N T ================================================= */


