/* === S Y N F I G ========================================================= */
/*!	\file trgt_cairo_png.cpp
**	\brief png_cairo_trgt Target Png Cairo Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include "trgt_cairo_png.h"

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(cairo_png_trgt);
SYNFIG_TARGET_SET_NAME(cairo_png_trgt,"cairo_png");
SYNFIG_TARGET_SET_EXT(cairo_png_trgt,"png");
SYNFIG_TARGET_SET_VERSION(cairo_png_trgt,"0.2");
SYNFIG_TARGET_SET_CVS_ID(cairo_png_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

//Target *cairo_png_trgt::New(const char *filename){	return new cairo_png_trgt(filename);}

cairo_png_trgt::cairo_png_trgt(const char *Filename, const synfig::TargetParam &params):
	multi_image(false),
	imagecount(0),
	filename(Filename),
	base_filename(Filename),
	sequence_separator(params.sequence_separator)
{ }

cairo_png_trgt::~cairo_png_trgt()
{ }

bool
cairo_png_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PixelFormat((int)PF_RGB|(int)PF_A));
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

bool
cairo_png_trgt::obtain_surface(cairo_surface_t *&surface)
{
	if(filename=="-")
	{
		synfig::error("Cairo PNG surface does not support writing to stdout");
	}
	else if(multi_image)
	{
		filename = (filename_sans_extension(base_filename) +
						   sequence_separator +
						   etl::strprintf("%04d",imagecount) +
						   filename_extension(base_filename));
	}
	else
	{
		filename = base_filename;
	}

	int w=desc.get_w(), h=desc.get_h();
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	return true;
}

bool
cairo_png_trgt::put_surface(cairo_surface_t *surface, synfig::ProgressCallback *cb)
{
	if(cairo_surface_status(surface))
	{
		if(cb) cb->error(_("Cairo Surface bad status"));
		return false;
	}

	cairo_status_t status;
	if (get_alpha_mode()==TARGET_ALPHA_MODE_EXTRACT)
	{
		cairo_t *cr = cairo_create(surface);
		cairo_push_group_with_content(cr, CAIRO_CONTENT_COLOR_ALPHA);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_paint(cr);
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_mask_surface(cr, cairo_get_target(cr), 0, 0);

		status = cairo_surface_write_to_png(cairo_get_group_target(cr), filename.c_str());

		cairo_destroy(cr);
	}
	else
		status = cairo_surface_write_to_png(surface, filename.c_str());
	if(status!=CAIRO_STATUS_SUCCESS) synfig::warning(cairo_status_to_string(status));

	imagecount++;

	cairo_surface_destroy(surface);
	return true;
}

