/* === S Y N F I G ========================================================= */
/*!	\file trgt_mng.h
**	\brief MNG Target Module
**
**	\legal
**	Copyright (c) 2007 Paul Wise
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

#ifndef __SYNFIG_TRGT_MNG_H
#define __SYNFIG_TRGT_MNG_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <cstdio> // FILE

#if !defined(MNG_SUPPORT_FULL)
#define MNG_SUPPORT_FULL 1
#endif

#if !defined(MNG_SUPPORT_READ)
#define MNG_SUPPORT_READ 1
#endif

#if !defined(MNG_SUPPORT_WRITE)
#define MNG_SUPPORT_WRITE 1
#endif

#if !defined(MNG_SUPPORT_DISPLAY)
#define MNG_SUPPORT_DISPLAY 1
#endif

#if !defined(MNG_ACCESS_CHUNKS)
#define MNG_ACCESS_CHUNKS 1
#endif

#include <libmng.h>

#include <synfig/smartfile.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class mng_trgt : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT

private:

	synfig::SmartFILE file;
	int w,h;
	mng_handle mng;

	bool multi_image,ready;
	int imagecount;
	synfig::filesystem::Path filename;
	std::vector<unsigned char> buffer;
	std::vector<synfig::Color> color_buffer;

	z_stream zstream;
	unsigned char* zbuffer;
	unsigned int zbuffer_len;

public:

	mng_trgt(const synfig::filesystem::Path& filename, const synfig::TargetParam& /* params */);
	virtual ~mng_trgt();

	bool set_rend_desc(synfig::RendDesc* desc) override;
	bool init(synfig::ProgressCallback* cb) override;

	bool start_frame(synfig::ProgressCallback* cb) override;
	void end_frame() override;

	synfig::Color* start_scanline(int scanline) override;
	bool end_scanline() override;
};

/* === E N D =============================================================== */

#endif
