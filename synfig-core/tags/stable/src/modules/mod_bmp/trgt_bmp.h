/* === S I N F G =========================================================== */
/*!	\file trgt_bmp.h
**	\brief Template Header
**
**	$Id: trgt_bmp.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_TRGT_BMP_H
#define __SINFG_TRGT_BMP_H

/* === H E A D E R S ======================================================= */

#include <sinfg/target_scanline.h>
#include <sinfg/string.h>
#include <cstdio>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class bmp : public sinfg::Target_Scanline
{
	SINFG_TARGET_MODULE_EXT
private:
	int rowspan;
	int imagecount;
	bool multi_image;
	FILE *file;
	sinfg::String filename;
	unsigned char *buffer;
	sinfg::Color *color_buffer;
	sinfg::PixelFormat pf;

public:
	bmp(const char *filename);
	virtual ~bmp();

	virtual bool set_rend_desc(sinfg::RendDesc *desc);
	virtual bool start_frame(sinfg::ProgressCallback *cb);
	virtual void end_frame();
	virtual sinfg::Color * start_scanline(int scanline);
	virtual bool end_scanline();

};

/* === E N D =============================================================== */

#endif
