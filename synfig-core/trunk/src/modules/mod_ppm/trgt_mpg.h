/*! ========================================================================
** Synfig
** Template Header File
** $Id: trgt_mpg.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_TRGT_MPG_H
#define __SYNFIG_TRGT_MPG_H

/* === H E A D E R S ======================================================= */

#include <synfig/synfig.h>
#include <stdio.h>
#include "trgt_ppm.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */


class bsd_mpeg1 : public synfig::Target
{
public:
private:
// 	synfig::RendDesc desc;
 	synfig::Target *passthru;
 	String filename;
	FILE *paramfile;
public:
	bsd_mpeg1(const char *filename);

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual ~bsd_mpeg1();
	

	virtual unsigned char * start_scanline(int scanline);
	virtual bool end_scanline(void);

	static synfig::Target *New(const char *filename);

	static const char Name[];
	static const char Ext[];

};

/* === E N D =============================================================== */

#endif
