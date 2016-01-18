/* === S Y N F I G ========================================================= */
/*!	\file synfig/main.h
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

#ifndef __SYNFIG_MAIN_H
#define __SYNFIG_MAIN_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include <ETL/ref_count>

#include "string.h"
#include "progresscallback.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class synfig::Main
**	\brief \writeme
**
**	\writeme
*/
class Main
{
private:
	static Main *instance;
	etl::reference_counter ref_count_;

public:
	synfig::String root_path;
	synfig::String bin_path;
	synfig::String share_path;
	synfig::String locale_path;
	synfig::String lib_path;
	synfig::String lib_synfig_path;

	Main(const synfig::String& basepath,ProgressCallback *cb=0);
	~Main();

	const etl::reference_counter& ref_count()const { return ref_count_; }
	static const Main& get_instance() { assert(instance); return *instance; }
}; // END of class Main

}; // END if namespace synfig

/* === E N D =============================================================== */

#endif
