/*! ========================================================================
** Sinfg
** Template Header File
** $Id: trgt_dv.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_TRGT_DV_H
#define __SINFG_TRGT_DV_H

/* === H E A D E R S ======================================================= */

#include <sinfg/target_scanline.h>
#include <sinfg/string.h>
#include <cstdio>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */


class dv_trgt : public sinfg::Target_Scanline
{
	SINFG_TARGET_MODULE_EXT
private:
	int imagecount;
	bool wide_aspect;
	FILE *file;
	sinfg::String filename;
	unsigned char *buffer;
	sinfg::Color *color_buffer;
public:
	dv_trgt(const char *filename);
	virtual ~dv_trgt();
	

	virtual bool set_rend_desc(sinfg::RendDesc *desc);
	virtual bool init();
	virtual bool start_frame(sinfg::ProgressCallback *cb);
	virtual void end_frame();

	virtual sinfg::Color * start_scanline(int scanline);
	virtual bool end_scanline();
};

/* === E N D =============================================================== */

#endif
