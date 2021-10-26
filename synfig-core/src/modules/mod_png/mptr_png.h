/* === S Y N F I G ========================================================= */
/*!	\file mptr_png.h
**	\brief Header for PNG Importer (png_mptr)
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MPTR_PNG_H
#define __SYNFIG_MPTR_PNG_H

/* === H E A D E R S ======================================================= */

#include <png.h>
#include <synfig/importer.h>
#include <synfig/string.h>
#include <synfig/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class png_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
private:
	static void png_out_error(png_struct *png_data,const char *msg);
	static void png_out_warning(png_struct *png_data,const char *msg);
	static void read_callback(png_structp png_ptr, png_bytep out_bytes, png_size_t bytes_count_to_read);

public:
	png_mptr(const synfig::FileSystem::Identifier &identifier);
	~png_mptr();

	virtual bool get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, synfig::Time time, synfig::ProgressCallback *callback);
};

/* === E N D =============================================================== */

#endif
