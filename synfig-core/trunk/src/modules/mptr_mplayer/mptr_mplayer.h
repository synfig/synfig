/*! ========================================================================
** Synfig
** Template Header File
** $Id: mptr_mplayer.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_MPTR_MPLAYER_H
#define __SYNFIG_MPTR_MPLAYER_H

/* === H E A D E R S ======================================================= */

#include <synfig/synfig.h>
#include <stdio.h>
#include "string.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class mplayer_mptr : public synfig::Importer
{
public:
private:
	String filename;
	
public:
	mplayer_mptr(const char *filename);
	~mplayer_mptr();


	static const char Name[];
	static const char Ext[];

	virtual bool GetFrame(synfig::Time time, synfig::Surface &, synfig::ProgressCallback *);

	static synfig::Importer *New(const char *filename);
};

/* === E N D =============================================================== */

#endif
