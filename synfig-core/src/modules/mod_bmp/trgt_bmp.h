/* === S Y N F I G ========================================================= */
/*!	\file trgt_bmp.h
**	\brief Header for BMP Exporter (bmp)
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_BMP_H
#define __SYNFIG_TRGT_BMP_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/smartfile.h>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class bmp : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT

private:

	int rowspan;
	int imagecount;
	bool multi_image;
	synfig::SmartFILE file;
	synfig::filesystem::Path filename;
	std::vector<unsigned char> buffer;
	std::vector<synfig::Color> color_buffer;
	synfig::PixelFormat pf;
	synfig::String sequence_separator;

public:
	bmp(const synfig::filesystem::Path& filename, const synfig::TargetParam& /* params */);
	virtual ~bmp();

	bool set_rend_desc(synfig::RendDesc* desc) override;

	bool start_frame(synfig::ProgressCallback* cb) override;
	void end_frame() override;

	synfig::Color* start_scanline(int scanline) override;
	bool end_scanline() override;
};

/* === E N D =============================================================== */

#endif
