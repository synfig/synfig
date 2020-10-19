/*! ========================================================================
** Synfig
**
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
**
** === N O T E S ===========================================================
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

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class mng_trgt : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT

private:

	FILE *file;
	int w,h;
	mng_handle mng;

	bool multi_image,ready;
	int imagecount;
	synfig::String filename;
	unsigned char *buffer;
	synfig::Color *color_buffer;

	z_stream zstream;
	unsigned char* zbuffer;
	unsigned int zbuffer_len;

public:

	mng_trgt(const char *filename, const synfig::TargetParam& /* params */);
	virtual ~mng_trgt();

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool init(synfig::ProgressCallback *cb);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual synfig::Color * start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
