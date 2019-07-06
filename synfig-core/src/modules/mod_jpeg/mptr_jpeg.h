/* === S Y N F I G ========================================================= */
/*!	\file mptr_jpeg.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_MPTR_JPEG_H
#define __SYNFIG_MPTR_JPEG_H

/* === H E A D E R S ======================================================= */

#define NOMINMAX
#include <synfig/importer.h>
#include <synfig/string.h>
#include <synfig/surface.h>

extern "C" {
#include <jpeglib.h>
}

#include <setjmp.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class jpeg_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
private:
	static void my_error_exit (j_common_ptr cinfo);

public:
	jpeg_mptr(const synfig::FileSystem::Identifier &identifier);
	~jpeg_mptr();

	virtual bool get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, synfig::Time time, synfig::ProgressCallback *callback);
};

/* === E N D =============================================================== */

#endif
