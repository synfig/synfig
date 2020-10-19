/*! ========================================================================
** Synfig
**
**	Copyright (c) 2007 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_MAGICKPP_H
#define __SYNFIG_TRGT_MAGICKPP_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/string.h>
#include <synfig/targetparam.h>
#include <cstdio>

#include <vector>

#ifdef IMAGEMAGICK_CONFIG
#include "config.h"
#endif

#include <Magick++.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class magickpp_trgt : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT

private:

	int width, height;

	synfig::String filename;
	unsigned char *buffer1, *start_pointer, *buffer_pointer;
	unsigned char *buffer2, *previous_buffer_pointer;
	bool transparent;
	synfig::Color *color_buffer;
	std::vector<Magick::Image> images;
	synfig::String sequence_separator;

public:

	magickpp_trgt(const char *filename, const synfig::TargetParam &params):
		width(),
		height(),
		filename(filename),
		buffer1(NULL),
		start_pointer(NULL),
		buffer_pointer(NULL),
		buffer2(NULL),
		previous_buffer_pointer(NULL),
		transparent(),
		color_buffer(NULL),
		sequence_separator(params.sequence_separator)
	{ }
	virtual ~magickpp_trgt();

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool init(synfig::ProgressCallback *cb);

	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual synfig::Color* start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
