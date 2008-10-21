/* === S Y N F I G ========================================================= */
/*!	\file smartfile.h
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SMARTFILE_H
#define __SYNFIG_SMARTFILE_H

/* === H E A D E R S ======================================================= */

#include <cstdio>
#include <ETL/smart_ptr>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct _FILE_deleter
{
	void operator()(FILE* x)const { if(x!=stdout && x!=stdin) fclose(x); }
};

typedef etl::smart_ptr<FILE,_FILE_deleter> SmartFILE;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
