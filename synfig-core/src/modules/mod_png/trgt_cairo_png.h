/* === S Y N F I G ========================================================= */
/*!	\file trgt_cairo_png.h
**	\brief Template Header Target Png Cairo Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_CAIRO_PNG_H
#define __SYNFIG_TRGT_CAIRO_PNG_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_cairo.h>
#include <synfig/surface.h>
#include <synfig/string.h>
#include <synfig/targetparam.h>
#include <cairo.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class cairo_png_trgt : public synfig::Target_Cairo
{
	SYNFIG_TARGET_MODULE_EXT
private:
	//int w,h;

	bool multi_image;
	int imagecount;
	synfig::String filename;
	synfig::String base_filename;

	synfig::String sequence_separator;
public:
	cairo_png_trgt(const char *filename, const synfig::TargetParam& /* params */);
	virtual ~cairo_png_trgt();

	virtual bool set_rend_desc(synfig::RendDesc *desc);

	virtual bool obtain_surface(cairo_surface_t *&surface);
	virtual bool put_surface(cairo_surface_t *surface, synfig::ProgressCallback *cb=NULL);
};

/* === E N D =============================================================== */

#endif
