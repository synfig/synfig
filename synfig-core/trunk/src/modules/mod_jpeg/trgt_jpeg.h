/*! ========================================================================
** Sinfg
** Template Header File
** $Id: trgt_jpeg.h,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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

#ifndef __SINFG_TRGT_JPEG_H
#define __SINFG_TRGT_JPEG_H

/* === H E A D E R S ======================================================= */

#define NOMINMAX
#include <sinfg/target_scanline.h>
#include <sinfg/string.h>
#include <cstdio>
_ETL_BEGIN_CDECLS
#include <jpeglib.h>
_ETL_END_CDECLS
#include <setjmp.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class jpeg_trgt : public sinfg::Target_Scanline
{
	SINFG_TARGET_MODULE_EXT
private:
	FILE *file;
	int w,h,quality;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	
	bool multi_image,ready;
	int imagecount;
	sinfg::String filename;
	unsigned char *buffer;
	sinfg::Color *color_buffer;
public:
	jpeg_trgt(const char *filename);
	virtual ~jpeg_trgt();

	virtual bool set_rend_desc(sinfg::RendDesc *desc);
	virtual bool start_frame(sinfg::ProgressCallback *cb);
	virtual void end_frame();

	virtual sinfg::Color * start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
