/* === S Y N F I G ========================================================= */
/*!	\file mptr_cairo_png.h
**	\brief Template Header
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

#ifndef __SYNFIG_MPTR_CAIRO_PNG_H
#define __SYNFIG_MPTR_CAIRO_PNG_H

/* === H E A D E R S ======================================================= */

#include "cairo.h"

#include <synfig/cairoimporter.h>
#include <synfig/string.h>
#include <synfig/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class cairo_png_mptr : public synfig::CairoImporter
{
	SYNFIG_CAIROIMPORTER_MODULE_EXT
private:
	cairo_surface_t* csurface_;

	static cairo_status_t read_callback(void *closure, unsigned char *data, unsigned int length);

public:
	cairo_png_mptr(const synfig::FileSystem::Identifier &identifier);
	~cairo_png_mptr();

	virtual bool get_frame(cairo_surface_t *&csurface, const synfig::RendDesc &renddesc, synfig::Time time, synfig::ProgressCallback *callback);
	virtual bool get_frame(cairo_surface_t *&csurface, const synfig::RendDesc &renddesc, synfig::Time time,
						   bool &trimmed, unsigned int &width, unsigned int &height, unsigned int &top, unsigned int &left,
						   synfig::ProgressCallback *callback);
};

/* === E N D =============================================================== */

#endif
