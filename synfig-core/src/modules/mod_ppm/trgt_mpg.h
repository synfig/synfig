/* === S Y N F I G ========================================================= */
/*!	\file trgt_mpg.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_MPG_H
#define __SYNFIG_TRGT_MPG_H

/* === H E A D E R S ======================================================= */

#include <synfig/synfig.h>
#include <cstdio>
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
	bsd_mpeg1(const char *filename, const synfig::TargetParam& /* params */);

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
