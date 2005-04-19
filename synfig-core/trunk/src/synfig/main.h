/* === S I N F G =========================================================== */
/*!	\file main.h
**	\brief Template Header
**
**	$Id: main.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_MAIN_H
#define __SINFG_MAIN_H

/* === H E A D E R S ======================================================= */

#include <ETL/ref_count>
#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
	
/*!	\class sinfg::Main
**	\brief \writeme
**
**	\writeme
*/
class Main
{
	etl::reference_counter ref_count_;
public:
	Main(const sinfg::String& basepath,ProgressCallback *cb=0);
	~Main();

	static void load_modules(ProgressCallback *cb=0);

	const etl::reference_counter& ref_count()const { return ref_count_; }
}; // END of class Main

}; // END if namespace sinfg

/* === E N D =============================================================== */

#endif
