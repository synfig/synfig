/* === S Y N F I G ========================================================= */
/*!	\file trgt_magickpp.h
**	\brief Magick++ Target Module
**
**	\legal
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
** \endlegal
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_MAGICKPP_H
#define __SYNFIG_TRGT_MAGICKPP_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/string.h>

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
	std::vector<unsigned char> buffer1, buffer2;
	unsigned char* buffer_pointer;
	unsigned char* current_row_buffer_pointer;
	unsigned char* previous_row_buffer_pointer;
	bool transparent;
	bool is_gif;
	std::vector<synfig::Color> color_buffer;
	std::vector<Magick::Image> images;
	synfig::String sequence_separator;

public:

	magickpp_trgt(const char *filename, const synfig::TargetParam &params):
		width(),
		height(),
		filename(filename),
		buffer_pointer(nullptr),
		current_row_buffer_pointer(nullptr),
		previous_row_buffer_pointer(nullptr),
		transparent(),
		is_gif(false),
		sequence_separator(params.sequence_separator)
	{
		// todo(ice0): fix portable build
		Magick::InitializeMagick(nullptr);
	}
	virtual ~magickpp_trgt();

	bool set_rend_desc(synfig::RendDesc* desc) override;
	bool init(synfig::ProgressCallback* cb) override;

	bool start_frame(synfig::ProgressCallback* cb) override;
	void end_frame() override;

	synfig::Color* start_scanline(int scanline) override;
	bool end_scanline() override;
};

/* === E N D =============================================================== */

#endif
