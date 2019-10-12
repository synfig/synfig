/* === S Y N F I G ========================================================= */
/*!	\file cairolistimporter.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2013 Carlos López
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

#ifndef __SYNFIG_CAIROLISTIMPORTER_H
#define __SYNFIG_CAIROLISTIMPORTER_H

/* === H E A D E R S ======================================================= */

#include "cairoimporter.h"
#include "surface.h"
#include <vector>
#include <list>
#include <utility>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class CairoListImporter
**	\todo Write more detailed description
*/
class CairoListImporter : public CairoImporter
{
	SYNFIG_CAIROIMPORTER_MODULE_EXT
public:
	class CacheElement
	{
	public:
		String frame_name;
		cairo_surface_t* surface;
		CacheElement()
		{
			surface=NULL;
		}
		//Copy constructor
		CacheElement(const CacheElement& other): frame_name(other.frame_name), surface(cairo_surface_reference(other.surface))
		{
		}
		~CacheElement()
		{
			if(surface)
				cairo_surface_destroy(surface);
		}
	};

private:
	float fps;
	std::vector<String> filename_list;
	std::list<CacheElement> frame_cache;

public:

	CairoListImporter(const FileSystem::Identifier &identifier);
	~CairoListImporter();

	virtual bool get_frame(cairo_surface_t *&csurface, const RendDesc &renddesc, Time time, ProgressCallback *callback=NULL);

	virtual bool is_animated();

};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
