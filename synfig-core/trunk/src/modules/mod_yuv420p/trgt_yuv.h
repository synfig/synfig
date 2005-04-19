/* === S Y N F I G ========================================================= */
/*!	\file trgt_yuv.h
**	\brief Template File
**
**	$Id: trgt_yuv.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_PPM_H
#define __SYNFIG_TRGT_PPM_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/smartfile.h>
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

	yuv(const char *filename);
	virtual ~yuv();

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual synfig::Color* start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
