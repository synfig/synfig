/* === S Y N F I G ========================================================= */
/*!	\file mptr_ppm.h
**	\brief Template Header
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

#ifndef __SYNFIG_MPTR_PPM_H
#define __SYNFIG_MPTR_PPM_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class ppm_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
private:
	synfig::String filename;
public:
	ppm_mptr(const char *filename);
	~ppm_mptr();

	virtual bool get_frame(synfig::Surface &,synfig::Time, synfig::ProgressCallback *);
}; // END of class ppm_mptr

/* === E N D =============================================================== */

#endif
