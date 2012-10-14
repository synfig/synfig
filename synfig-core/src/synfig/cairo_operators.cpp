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
#include "surface.h"
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
			cairo_set_operator(cr, CAIRO_OPERATOR_LIGHTEN);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_DARKEN:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_DARKEN);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_MULTIPLY:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_HUE:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_HSL_HUE);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_SATURATION:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_HSL_SATURATION);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_LUMINANCE:
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
		case Color::BLEND_SCREEN:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_SCREEN);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_HARD_LIGHT:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr), alpha);
			cairo_set_operator(cr, CAIRO_OPERATOR_HARD_LIGHT);
			cairo_identity_matrix(cr);
			cairo_mask_surface(cr, dest, 0,0);
			cairo_surface_destroy(dest);
			break;
		}
		case Color::BLEND_ALPHA_OVER:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OUT);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		case Color::BLEND_STRAIGHT_ONTO:
		{
			cairo_surface_t* dest=cairo_copy_target_image(cairo_get_target(cr));
			cairo_set_operator(cr, CAIRO_OPERATOR_IN);
			cairo_paint(cr);
			cairo_surface_t* source=cairo_copy_target_image(cairo_get_target(cr));
			cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
			cairo_paint(cr);
			cairo_set_source_surface(cr, dest, 0, 0);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint(cr);
			cairo_set_source_surface(cr, source, 0, 0);
			cairo_paint_with_alpha_operator(cr, alpha, Color::BLEND_STRAIGHT);
			cairo_surface_destroy(dest);
			cairo_surface_destroy(source);
			break;
		}
		case Color::BLEND_ADD:
		case Color::BLEND_SUBTRACT:
		case Color::BLEND_DIVIDE:
		case Color::BLEND_ALPHA_BRIGHTEN:
		case Color::BLEND_ALPHA_DARKEN:
		case Color::BLEND_OVERLAY:
		default:
		{
			cairo_surface_t* target=cairo_get_target(cr);
			cairo_surface_t* dest=cairo_copy_target_image(target);
			
			cairo_save(cr);
			cairo_reset_clip(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
			cairo_paint(cr);
			cairo_restore(cr);
			
			cairo_paint(cr);
			
			CairoSurface cresult(target);
			CairoSurface cdest(dest);
			assert(cdest.map_cairo_image());
			assert(cresult.map_cairo_image());

			int w=cresult.get_w();
			int h=cresult.get_h();
			
			for(int y=0;y<h;y++)
				for(int x=0;x<w;x++)
				{
					Color src=Color(cresult[y][x].demult_alpha());
					Color dst=Color(cdest[y][x].demult_alpha());
					cresult[y][x]=CairoColor(Color::blend(src, dst, alpha,	method).clamped()).premult_alpha();
				}
			
			cresult.unmap_cairo_image();
			cdest.unmap_cairo_image();

			cairo_surface_destroy(dest);
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
