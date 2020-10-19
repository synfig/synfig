/* === S Y N F I G ========================================================= */
/*!	\file synfig/render.cpp
**	\brief Renderer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <cassert>

#include "render.h"

#include "general.h"
#include <synfig/localization.h>

#include "canvas.h"
#include "context.h"
#include "surface.h"
#include "target.h"


#endif

/* === M A C R O S ========================================================= */

/* === P R O C E D U R E S ================================================= */

bool
synfig::render(
	Context context,
	Target_Scanline::Handle target,
	const RendDesc &desc,
	ProgressCallback *callback)
{
	Point::value_type
		u,v,		// Current location in image
		su,sv,		// Starting locations
		du, dv,		// Distance between pixels
		dsu,dsv;	// Distance between subpixels

	bool
		no_clamp=!desc.get_clamp();

	int
		w(desc.get_w()),
		h(desc.get_h()),
		a(desc.get_antialias());

	Point
		tl(desc.get_tl()),
		br(desc.get_br());

	//Gamma
	//	gamma(desc.get_gamma());

	int
		x,y,		// Current location on output bitmap
		x2,y2;		// Subpixel counters

	Color::value_type
		pool;		// Alpha pool (for correct alpha antialiasing)

	assert(target);

	// If we do not have a target then bail
	if(!target)
		return false;

	// Calculate the number of channels
	//chan=pixel_size(desc.get_pixel_format());

	// Calculate the distance between pixels
	du=(br[0]-tl[0])/(Point::value_type)w;
	dv=(br[1]-tl[1])/(Point::value_type)h;

	// Calculate the distance between sub pixels
	dsu=du/(Point::value_type)a;
	dsv=dv/(Point::value_type)a;

	// Calculate the starting points
	su=tl[0]+(du-dsu)/(Point::value_type)2.0;
	sv=tl[1]-(dv-dsv)/(Point::value_type)2.0;

	// Mark the start of a new frame.
	if(!target->start_frame(callback))
		return false;

	// Loop through all horizontal lines
	for(y=0,v=sv;y<h;y++,v+=dv)
	{
		// Set the current pixel pointer
		// to the start of the line
		Color *colordata=target->start_scanline(y);

		if(!colordata)
		{
			if(callback)callback->error(_("Target panic"));
			else throw(std::string(_("Target panic")));
			return false;
		}

		// If we have a callback that we need
		// to report to, do so now.
		if(callback)
			if( callback->amount_complete(y,h) == false )
			{
				// If the callback returns false,
				// then the render has been aborted.
				// Exit gracefully.

				target->end_scanline();
				target->end_frame();
				return false;
			}

		// Loop through every pixel in row
		for(x=0,u=su;x<w;x++,u+=du)
		{
			Color &c(*(colordata++));
			c=Color::alpha();

			// Loop through all subpixels
			for(y2=0,pool=0;y2<a;y2++)
				for(x2=0;x2<a;x2++)
				{
					Color color=context.get_color(
						Point(
							u+(Point::value_type)(x2)*dsu,
							v+(Point::value_type)(y2)*dsv
							)
						);
					if(!no_clamp)
					{
						color=color.clamped();
						c+=color*color.get_a();
						pool+=color.get_a();
					}
					else
					{
						c+=color*color.get_a();
						pool+=color.get_a();
					}
				}
			if(pool)
				c/=pool;
		}

		// Send the buffer to the render target.
		// If anything goes wrong, cleanup and bail.
		if(!target->end_scanline())
		{
			if(callback)callback->error(_("Target panic"));
			else throw(std::string(_("Target panic")));
			return false;
		}
	}

	// Finish up the target's frame
	target->end_frame();

	// Give the callback one more last call,
	// this time with the full height as the
	// current line
	if(callback)
		callback->amount_complete(h,h);

	// Report our success
	return(true);
}


