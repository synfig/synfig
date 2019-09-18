/* === S Y N F I G ========================================================= */
/*!	\file trgt_yuv.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_YUV_H
#define __SYNFIG_TRGT_YUV_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/smartfile.h>
#include <synfig/targetparam.h>
#include <cstdio>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class yuv : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT

private:

	synfig::String filename;
	synfig::SmartFILE file;
	synfig::Surface surface;

	bool dithering;

public:

	yuv(const char *filename, const synfig::TargetParam& /* params */);
	virtual ~yuv();

	virtual bool init(synfig::ProgressCallback *cb);
	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual synfig::Color* start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
