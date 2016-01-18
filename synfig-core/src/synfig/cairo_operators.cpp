/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
 **	\brief Template File
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2012-2013 Carlos López
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
#include <synfig/localization.h>


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
		case Color::BLEND_MULTIPLY:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, alpha);
			cairo_pop_group_to_source(cr);

			cairo_matrix_t matrix;
			cairo_get_matrix(cr, &matrix);
			cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_mask(cr, pattern);
			
			cairo_pattern_destroy(pattern);
			break;
		}
		case Color::BLEND_HUE:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, alpha);
			cairo_pop_group_to_source(cr);

			cairo_matrix_t matrix;
			cairo_get_matrix(cr, &matrix);
			cairo_set_operator(cr, CAIRO_OPERATOR_HSL_HUE);
			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_mask(cr, pattern);
			
			cairo_pattern_destroy(pattern);
			break;
		}
		case Color::BLEND_SATURATION:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, alpha);
			cairo_pop_group_to_source(cr);

			cairo_matrix_t matrix;
			cairo_get_matrix(cr, &matrix);
			cairo_set_operator(cr, CAIRO_OPERATOR_HSL_SATURATION);
			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_mask(cr, pattern);
			
			cairo_pattern_destroy(pattern);
			break;
		}
		case Color::BLEND_LUMINANCE:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, alpha);
			cairo_pop_group_to_source(cr);

			cairo_matrix_t matrix;
			cairo_get_matrix(cr, &matrix);
			cairo_set_operator(cr, CAIRO_OPERATOR_HSL_LUMINOSITY);
			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_mask(cr, pattern);
			
			cairo_pattern_destroy(pattern);
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
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, alpha);
			cairo_pop_group_to_source(cr);
			
			cairo_matrix_t matrix;
			cairo_get_matrix(cr, &matrix);
			cairo_set_operator(cr, CAIRO_OPERATOR_SCREEN);
			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_mask(cr, pattern);
			
			cairo_pattern_destroy(pattern);
			break;
		}
		case Color::BLEND_HARD_LIGHT:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint_with_alpha(cr, alpha);
			cairo_pop_group_to_source(cr);

			cairo_matrix_t matrix;
			cairo_get_matrix(cr, &matrix);
			cairo_set_operator(cr, CAIRO_OPERATOR_HARD_LIGHT);
			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_mask(cr, pattern);
			
			cairo_pattern_destroy(pattern);
			break;
		}
		case Color::BLEND_ALPHA_OVER:
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OUT);
			cairo_paint_with_alpha(cr, alpha);
			break;
		}
		case Color::BLEND_STRAIGHT_ONTO: // I don't find a suitable way to produce the
		// render method so I fall back to pixel operations.
//		{
//			cairo_push_group(cr);
//			cairo_save(cr);
//			
//			cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0-alpha);
//			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_IN);
//			cairo_paint(cr);
//			
//			cairo_restore(cr);
//			
//			cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
//			cairo_paint_with_alpha(cr, alpha);
//			cairo_pop_group_to_source(cr);
//						
//			cairo_matrix_t matrix;
//			cairo_get_matrix(cr, &matrix);
//			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
//			cairo_pattern_t* pattern=cairo_pattern_create_for_surface(cairo_get_target(cr));
//			cairo_pattern_set_matrix(pattern, &matrix);
//			cairo_mask(cr, pattern);
//
//			cairo_pattern_destroy(pattern);
//			break;
//		}
		case Color::BLEND_OVERLAY:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint(cr);
			cairo_pattern_t* pattern=cairo_pop_group(cr);
			
			cairo_surface_t* source;
			cairo_status_t status;
			status=cairo_pattern_get_surface(pattern, &source);
			if(status)
			{
				// return gracefully
				synfig::error("%s", cairo_status_to_string(status));
				cairo_pattern_destroy(pattern);
				return;
			}
			CairoSurface csource(source);
			CairoSurface cdest(cairo_get_target(cr));

			if(!cdest.map_cairo_image())
			{
					// return gracefully
					cairo_pattern_destroy(pattern);
					return;
			}
			if(!csource.map_cairo_image())
			   {
				   // return gracefully
				   cairo_pattern_destroy(pattern);
				   cdest.unmap_cairo_image();
				   return;
			   }
			
			double x1, y1, x2, y2, x0, y0;
			cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
			cairo_user_to_device(cr, &x1, &y1);
			cairo_user_to_device(cr, &x2, &y2);
			x0=x2<x1?x2:x1;
			y0=y2<y1?y2:y1;

			int w=csource.get_w();
			int h=csource.get_h();
			int h0=(int)y0;
			int w0=(int)x0;
			for(int y=0;y<h;y++)
				for(int x=0;x<w;x++)
				{
					CairoColor ret=CairoColor::blend(csource[y][x].demult_alpha(), cdest[h0+y][w0+x].demult_alpha(), alpha,	method);
					csource[y][x]=ret.premult_alpha();
				}
			csource.unmap_cairo_image();
			cdest.unmap_cairo_image();
			
			cairo_save(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
			cairo_reset_clip(cr);
			cairo_identity_matrix(cr);
			cairo_set_source_surface(cr, source, 0, 0);
			cairo_paint(cr);
			cairo_restore(cr);
			
			cairo_pattern_destroy(pattern);
			break;
			
		}
		case Color::BLEND_BRIGHTEN:
		case Color::BLEND_DARKEN:
		case Color::BLEND_ADD:
		case Color::BLEND_SUBTRACT:
		case Color::BLEND_DIFFERENCE:
		case Color::BLEND_DIVIDE:
		case Color::BLEND_ALPHA_DARKEN:
		case Color::BLEND_ALPHA_BRIGHTEN:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint(cr);
			cairo_pattern_t* pattern=cairo_pop_group(cr);
			
			cairo_surface_t* source;
			cairo_status_t status;
			status=cairo_pattern_get_surface(pattern, &source);
			if(status)
			{
				// return gracefully
				synfig::error("%s", cairo_status_to_string(status));
				cairo_pattern_destroy(pattern);
				return;
			}
			CairoSurface csource(source);
			CairoSurface cdest(cairo_get_target(cr));
			
			if(!cdest.map_cairo_image())
			{
				// return gracefully
				cairo_pattern_destroy(pattern);
				return;
			}
			if(!csource.map_cairo_image())
			{
				// return gracefully
				cairo_pattern_destroy(pattern);
				cdest.unmap_cairo_image();
				return;
			}
			double x1, y1, x2, y2, x0, y0;
			cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
			cairo_user_to_device(cr, &x1, &y1);
			cairo_user_to_device(cr, &x2, &y2);
			x0=x2<x1?x2:x1;
			y0=y2<y1?y2:y1;
			
			int w=csource.get_w();
			int h=csource.get_h();
			int h0=(int)y0;
			int w0=(int)x0;
			for(int y=0;y<h;y++)
				for(int x=0;x<w;x++)
				{
					CairoColor src=csource[y][x].demult_alpha();
					CairoColor dest=cdest[h0+y][w0+x].demult_alpha();
					CairoColor ret=CairoColor::blend(src, dest, alpha, method);
					ret=ret.premult_alpha();
					cdest[h0+y][w0+x]=ret;
				}
			csource.unmap_cairo_image();
			cdest.unmap_cairo_image();

			cairo_pattern_destroy(pattern);
			break;
		}
		case Color::BLEND_COLOR:
		default:
		{
			cairo_push_group(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint(cr);
			cairo_pattern_t* pattern=cairo_pop_group(cr);
			
			cairo_surface_t* source;
			cairo_status_t status;
			status=cairo_pattern_get_surface(pattern, &source);
			if(status)
			{
				// return gracefully
				synfig::error("%s", cairo_status_to_string(status));
				cairo_pattern_destroy(pattern);
				return;
			}
			CairoSurface csource(source);
			CairoSurface cdest(cairo_get_target(cr));
			
			if(!cdest.map_cairo_image())
			{
				// return gracefully
				cairo_pattern_destroy(pattern);
				return;
			}
			if(!csource.map_cairo_image())
			{
				// return gracefully
				cairo_pattern_destroy(pattern);
				cdest.unmap_cairo_image();
				return;
			}
			
			double x1, y1, x2, y2, x0, y0;
			cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
			cairo_user_to_device(cr, &x1, &y1);
			cairo_user_to_device(cr, &x2, &y2);
			x0=x2<x1?x2:x1;
			y0=y2<y1?y2:y1;
			
			int w=csource.get_w();
			int h=csource.get_h();
			int h0=(int)y0;
			int w0=(int)x0;
			
			for(int y=0;y<h;y++)
				for(int x=0;x<w;x++)
				{
					Color src=Color(csource[y][x].demult_alpha());
					Color dst=Color(cdest[h0+y][w0+x].demult_alpha());
					cdest[h0+y][w0+x]=CairoColor(Color::blend(src, dst, alpha, method).clamped().premult_alpha());
				}
			csource.unmap_cairo_image();
			cdest.unmap_cairo_image();

			cairo_pattern_destroy(pattern);
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
