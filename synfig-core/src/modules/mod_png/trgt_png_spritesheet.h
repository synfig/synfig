/* === S Y N F I G ========================================================= */
/*!	\file trgt_png_spritesheet.h
**	\brief Sprite sheet render target.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2013		Moritz Grosch (LittleFox) <littlefox@fsfe.org>
**  Copyright (c) 2015		Denis Zdorovtsov (mrtrizer) <mrtrizer@gmail.com>
**
**  Based on trgt_png.h
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
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_PNG_SPRITESHEET_H
#define __SYNFIG_TRGT_PNG_SPRITESHEET_H

/* === H E A D E R S ======================================================= */

#include <png.h>
#include <synfig/target_scanline.h>
#include <synfig/string.h>
#include <synfig/targetparam.h>
#include <cstdio>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class png_trgt_spritesheet : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT
private:
	struct PngImage
	{
		PngImage():
			width(0),
			height(0),
			color_type(0),
			bit_depth(0){}
		unsigned int width;
		unsigned int height;
		png_byte color_type;
		png_byte bit_depth;
		png_structp png_ptr;
		png_infop info_ptr;
	};

	static void png_out_error(png_struct *png,const char *msg);
	static void png_out_warning(png_struct *png,const char *msg);
	bool ready;
	//bool initialized;
	int imagecount;
	int lastimage;
	int numimages;
	unsigned int cur_y;
	unsigned int cur_row;
	unsigned int cur_col;
	synfig::TargetParam params;
	synfig::Color ** color_data;
	unsigned int sheet_width;
	unsigned int sheet_height;
	FILE * in_file_pointer;
	FILE * out_file_pointer;
	unsigned int cur_out_image_row;
	PngImage in_image;
	synfig::String filename;
	synfig::String sequence_separator;
	synfig::Color * overflow_buff;

	bool is_final_image_size_acceptable() const;
	std::string get_image_size_error_message() const;

public:
	png_trgt_spritesheet(const char *filename, const synfig::TargetParam& /* params */);
	virtual ~png_trgt_spritesheet();

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual synfig::Color * start_scanline(int scanline);
	virtual bool end_scanline();
	bool read_png_file();
	bool write_png_file();
	bool load_png_file();
};

/* === E N D =============================================================== */

#endif
