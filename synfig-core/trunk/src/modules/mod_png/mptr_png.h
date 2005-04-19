/*! ========================================================================
** Synfig
** Template Header File
** $Id: mptr_png.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_MPTR_PNG_H
#define __SYNFIG_MPTR_PNG_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <png.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class png_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
private:
	synfig::String filename;
	synfig::Surface surface_buffer;

	png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

	static void png_out_error(png_struct *png_data,const char *msg);
	static void png_out_warning(png_struct *png_data,const char *msg);
	static int read_chunk_callback(png_struct *png_data, png_unknown_chunkp chunk);

public:
	png_mptr(const char *filename);
	~png_mptr();

	virtual bool get_frame(synfig::Surface &,synfig::Time, synfig::ProgressCallback *);
};

/* === E N D =============================================================== */

#endif
