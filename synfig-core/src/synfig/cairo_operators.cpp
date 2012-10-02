/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
 **	\brief Template File
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2012 Carlos López
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

#include "cairo_operators.h"


#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

void cairo_paint_with_alpha_operator(cairo_t* acr, float alpha, Color::BlendMethod method)
{
	cairo_t* cr=cairo_reference(acr);
	switch (method)
	{
		case Color::BLEND_COMPOSITE:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		case Color::BLEND_STRAIGHT:
		{
			cairo_save(cr);
			
			cairo_surface_t* dest=cairo_get_target(cr);
			cairo_surface_flush(dest);
			cairo_surface_t* destimage=cairo_surface_map_to_image(dest, NULL);
			int w=cairo_image_surface_get_width(destimage);
			int h=cairo_image_surface_get_height(destimage);
			cairo_surface_t* newdest=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
			
			cairo_t* destcr=cairo_create(newdest);
			cairo_set_source_surface(destcr, destimage, 0, 0);
			cairo_paint_with_alpha(destcr, 1.0-alpha);
			cairo_destroy(destcr);
			
			destcr=cairo_create(destimage);
			cairo_set_source_surface(destcr, newdest, 0, 0);
			cairo_set_operator(destcr, CAIRO_OPERATOR_SOURCE);
			cairo_paint(destcr);
			cairo_destroy(destcr);
			
			cairo_surface_unmap_image(dest, destimage);
			cairo_surface_mark_dirty(dest);
			
			cairo_surface_destroy(newdest);
			cairo_restore(cr);

			cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		case Color::BLEND_BEHIND:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		case Color::BLEND_ONTO:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		case Color::BLEND_ALPHA_OVER:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OUT);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		default:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
	}
	cairo_destroy(cr);
}


/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
