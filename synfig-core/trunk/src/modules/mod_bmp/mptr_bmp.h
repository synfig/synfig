/*! ========================================================================
** Synfig
** Template Header File
** $Id: mptr_bmp.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SYNFIG_MPTR_PPM_H
#define __SYNFIG_MPTR_PPM_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <cstdio>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class bmp_mptr : public synfig::Importer
{
SYNFIG_IMPORTER_MODULE_EXT

private:
	synfig::String filename;

public:
	bmp_mptr(const char *filename);
	~bmp_mptr();

	virtual bool get_frame(synfig::Surface &,synfig::Time, synfig::ProgressCallback *);
};

/* === E N D =============================================================== */

#endif
