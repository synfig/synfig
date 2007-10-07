/* === S Y N F I G ========================================================= */
/*!	\file listimporter.h
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

#ifndef __SYNFIG_LISTIMPORTER_H
#define __SYNFIG_LISTIMPORTER_H

/* === H E A D E R S ======================================================= */

#include "importer.h"
#include "surface.h"
#include <ETL/smart_ptr>
#include <vector>
//#include <deque>
#include <list>
#include <utility>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class ListImporter
**	\todo Write more detailed description
*/
class ListImporter : public Importer
{
	float fps;
	std::vector<String> filename_list;
	std::list<std::pair<int,Surface> > frame_cache;
protected:
	ListImporter(const String &filename);

public:

	virtual ~ListImporter();

	virtual bool get_frame(Surface &surface,Time time, ProgressCallback *callback=NULL);

	virtual bool is_animated();

	static Importer* create(const char *filename);
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
