/*! ========================================================================
** Synfig
** Template Header File
** $Id: trgt_openexr.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_TRGT_OPENEXR_H
#define __SYNFIG_TRGT_OPENEXR_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <cstdio>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <exception>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class exr_trgt : public synfig::Target_Scanline
{
public:
private:
	bool multi_image;
	int imagecount,scanline;
	synfig::String filename;
	Imf::RgbaOutputFile *exr_file;
	Imf::Rgba *buffer;
	etl::surface<Imf::Rgba> out_surface;
#ifndef USE_HALF_TYPE
	synfig::Color *buffer_color;
#endif

	bool ready();
public:
	exr_trgt(const char *filename);
	virtual ~exr_trgt();

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual synfig::Color * start_scanline(int scanline);
	virtual bool end_scanline(void);


	SYNFIG_TARGET_MODULE_EXT
};

/* === E N D =============================================================== */

#endif
