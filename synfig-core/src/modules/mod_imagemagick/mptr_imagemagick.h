/* === S Y N F I G ========================================================= */
/*!	\file mptr_imagemagick.h
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

#ifndef __SYNFIG_MPTR_IMAGEMAGICK_H
#define __SYNFIG_MPTR_IMAGEMAGICK_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <cstdio>
#include "string.h"
#include <synfig/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class imagemagick_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
private:
	FILE *file;
	//int cur_frame;
	synfig::Surface frame;

public:
	imagemagick_mptr(const synfig::FileSystem::Identifier &identifier);

	~imagemagick_mptr();

	virtual bool get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, synfig::Time time, synfig::ProgressCallback *callback);
};

/* === E N D =============================================================== */

#endif
