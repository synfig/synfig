/* === S Y N F I G ========================================================= */
/*!	\file listimporter.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
#include <vector>
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
	SYNFIG_IMPORTER_MODULE_EXT
private:
	float fps;
	std::vector<String> filename_list;
	std::list<Importer::Handle> frame_cache;

	Importer::Handle get_sub_importer(const RendDesc &renddesc, Time time, ProgressCallback *cb);

public:
	ListImporter(const FileSystem::Identifier &identifier);

	~ListImporter();

	virtual bool get_frame(Surface &surface, const RendDesc &renddesc, Time time, ProgressCallback *cb=NULL);
	virtual rendering::Surface::Handle get_frame(const RendDesc &renddesc, const Time &time);
	virtual bool is_animated();

};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
