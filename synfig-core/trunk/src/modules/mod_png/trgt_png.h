/*! ========================================================================
** Sinfg
** Template Header File
** $Id: trgt_png.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_TRGT_PNG_H
#define __SINFG_TRGT_PNG_H

/* === H E A D E R S ======================================================= */

#include <sinfg/target_scanline.h>
#include <sinfg/string.h>
#include <cstdio>
#include <png.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class png_trgt : public sinfg::Target_Scanline
{
	SINFG_TARGET_MODULE_EXT
private:
	FILE *file;
	int w,h;
	png_structp png_ptr;
	png_infop info_ptr;

	static void png_out_error(png_struct *png,const char *msg);
	static void png_out_warning(png_struct *png,const char *msg);
	bool multi_image,ready;
	int imagecount;
	sinfg::String filename;
	unsigned char *buffer;
	sinfg::Color *color_buffer;
public:
	png_trgt(const char *filename);
	virtual ~png_trgt();

	virtual bool set_rend_desc(sinfg::RendDesc *desc);
	virtual bool start_frame(sinfg::ProgressCallback *cb);
	virtual void end_frame();

	virtual sinfg::Color * start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
