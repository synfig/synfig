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
#include "general.h"


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
			
			cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0-alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_IN);
			cairo_paint(cr);
			
			cairo_restore(cr);
			
			cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
			cairo_paint_with_alpha(cr, alpha);		
			break;
		}
		case Color::BLEND_BRIGHTEN:
		{

			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_HSL_LUMINOSITY);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
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

void cairo_copy_surface(cairo_surface_t* source, cairo_surface_t* dest, float alpha)
{
	cairo_t* cr=cairo_create(dest);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_surface(cr, source, 0, 0);
	cairo_paint_with_alpha(cr, alpha);
	cairo_destroy(cr);
}

cairo_surface_t* cairo_copy_target_image(cairo_surface_t* target, float alpha)
{
	cairo_surface_t* targetimage=cairo_surface_map_to_image(target, NULL);
	cairo_surface_flush(targetimage);
	int w=cairo_image_surface_get_width(targetimage);
	int h=cairo_image_surface_get_height(targetimage);
	cairo_surface_t* image=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_copy_surface(targetimage, image, alpha);
	cairo_surface_mark_dirty(targetimage);
	cairo_surface_unmap_image(target, targetimage);
	return image;
}

void cairo_surface_mask_alpha(cairo_surface_t* image, float alpha)
{
	cairo_t* cr=cairo_create(image);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
	cairo_paint_with_alpha(cr, alpha);
	cairo_destroy(cr);
	
}


/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
