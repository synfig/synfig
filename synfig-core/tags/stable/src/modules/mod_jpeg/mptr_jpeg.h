/*! ========================================================================
** Sinfg
** Template Header File
** $Id: mptr_jpeg.h,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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

#ifndef __SINFG_MPTR_JPEG_H
#define __SINFG_MPTR_JPEG_H

/* === H E A D E R S ======================================================= */

#define NOMINMAX
#include <sinfg/importer.h>
#include <sinfg/string.h>
#include <sinfg/surface.h>
_ETL_BEGIN_CDECLS
#include <jpeglib.h>
_ETL_END_CDECLS
#include <setjmp.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class jpeg_mptr : public sinfg::Importer
{
	SINFG_IMPORTER_MODULE_EXT
private:
	sinfg::String filename;
	sinfg::Surface surface_buffer;

	struct jpeg_decompress_struct cinfo;

	static void my_error_exit (j_common_ptr cinfo);

public:
	jpeg_mptr(const char *filename);
	~jpeg_mptr();

	virtual bool get_frame(sinfg::Surface &,sinfg::Time, sinfg::ProgressCallback *);
};

/* === E N D =============================================================== */

#endif
