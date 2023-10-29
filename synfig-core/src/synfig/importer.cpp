/* === S Y N F I G ========================================================= */
/*!	\file importer.cpp
**	\brief It is the base class for all the importers.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cctype>
#include <cstdlib>

#include <algorithm>
#include <functional>
#include <map>

#include <glibmm.h>

#include "general.h"
#include <synfig/localization.h>

#include "importer.h"
#include "string.h"
#include "surface.h"

#include <synfig/rendering/software/surfacesw.h>
#include <synfig/rendering/software/surfaceswpacked.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

using namespace synfig;

Importer::Book* synfig::Importer::book_;

static std::map<FileSystem::Identifier,Importer::LooseHandle> *__open_importers;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
Importer::subsys_init()
{
	book_=new Book();
	__open_importers=new std::map<FileSystem::Identifier,Importer::LooseHandle>();
	return true;
}

bool
Importer::subsys_stop()
{
	delete book_;
	delete __open_importers;
	return true;
}

Importer::Book&
Importer::book()
{
	return *book_;
}

Importer::Handle
Importer::open(const FileSystem::Identifier &identifier, bool force)
{
	if (force) forget(identifier); // force reload

	if(identifier.filename.empty())
	{
		synfig::error(_("Importer::open(): Cannot open empty filename"));
		return nullptr;
	}

	// If we already have an importer open under that filename,
	// then use it instead.
	if(__open_importers->count(identifier))
	{
		//synfig::info("Found importer already open, using it...");
		return (*__open_importers)[identifier];
	}

	String ext(identifier.filename.extension().u8string());
	if (ext.empty()) {
		synfig::error(_("Importer::open(): Couldn't find extension"));
		return nullptr;
	}

	ext = ext.substr(1); // skip initial '.'
	strtolower(ext);


	if(!Importer::book().count(ext))
	{
		synfig::error(_("Importer::open(): Unknown file type -- ")+ext);
		return nullptr;
	}

	try {
		Importer::Handle importer;
		importer=Importer::book()[ext].factory(identifier);
		(*__open_importers)[identifier]=importer;
		return importer;
	}
	catch (const String& str)
	{
		synfig::error(str);
	}
	return nullptr;
}

void Importer::forget(const FileSystem::Identifier &identifier)
{
	__open_importers->erase(identifier);
}

Importer::Importer(const FileSystem::Identifier &identifier):
	identifier(identifier)
{
}


Importer::~Importer()
{
	// Remove ourselves from the open importer list
	std::map<FileSystem::Identifier,Importer::LooseHandle>::iterator iter;
	for(iter=__open_importers->begin();iter!=__open_importers->end();)
		if(iter->second==this)
			__open_importers->erase(iter++); else ++iter;
}

rendering::Surface::Handle
Importer::get_frame(const RendDesc & /* renddesc */, const Time &time)
{
	if (last_surface_ && last_surface_->is_exists() && !is_animated())
		return last_surface_;

	Surface surface;
	if(!get_frame(surface, RendDesc(), time)) {
		warning(strprintf(_("Unable to get frame from \"%s\" [%s]"), identifier.filename.c_str(), time.get_string().c_str()));
		return nullptr;
	}

	const char *s = getenv("SYNFIG_PACK_IMAGES");
	if (s == nullptr || atoi(s) != 0)
		last_surface_ = new rendering::SurfaceSWPacked();
	else
		last_surface_ = new rendering::SurfaceSW();

	if (surface.is_valid())
		last_surface_->assign(surface[0], surface.get_w(), surface.get_h());

	return last_surface_;
}
