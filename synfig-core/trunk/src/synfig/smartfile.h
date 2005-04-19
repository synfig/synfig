/* === S Y N F I G ========================================================= */
/*!	\file smartfile.h
**	\brief Template Header
**
**	$Id: smartfile.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
