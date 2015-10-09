/* === S Y N F I G ========================================================= */
/*!	\file importer.cpp
**	\brief It is the base class for all the importers.
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cctype>

#include <algorithm>
#include <functional>
#include <map>

#include <glibmm.h>

#include "general.h"
#include <synfig/localization.h>

#include "canvas.h"
#include "importer.h"
#include "string.h"
#include "surface.h"

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

using namespace etl;
using namespace std;
using namespace synfig;

Importer::Book* synfig::Importer::book_;

map<FileSystem::Identifier,Importer::LooseHandle> *__open_importers;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
Importer::subsys_init()
{
	book_=new Book();
	__open_importers=new map<FileSystem::Identifier,Importer::LooseHandle>();
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
Importer::open(const FileSystem::Identifier &identifier)
{
	if(identifier.filename.empty())
	{
		synfig::error(_("Importer::open(): Cannot open empty filename"));
		return 0;
	}

	// If we already have an importer open under that filename,
	// then use it instead.
	if(__open_importers->count(identifier))
	{
		//synfig::info("Found importer already open, using it...");
		return (*__open_importers)[identifier];
	}

	if(filename_extension(identifier.filename) == "")
	{
		synfig::error(_("Importer::open(): Couldn't find extension"));
		return 0;
	}

	String ext(filename_extension(identifier.filename));
	if (ext.size()) ext = ext.substr(1); // skip initial '.'
	std::transform(ext.begin(),ext.end(),ext.begin(),&::tolower);


	if(!Importer::book().count(ext))
	{
		synfig::error(_("Importer::open(): Unknown file type -- ")+ext);
		return 0;
	}

	try {
		Importer::Handle importer;
		importer=Importer::book()[ext].factory(identifier);
		(*__open_importers)[identifier]=importer;
		return importer;
	}
	catch (String str)
	{
		synfig::error(str);
	}
	return 0;
}

Importer::Importer(const FileSystem::Identifier &identifier):
	gamma_(2.2),
	identifier(identifier)
{
}


Importer::~Importer()
{
	// Remove ourselves from the open importer list
	map<FileSystem::Identifier,Importer::LooseHandle>::iterator iter;
	for(iter=__open_importers->begin();iter!=__open_importers->end();++iter)
		if(iter->second==this)
		{
			__open_importers->erase(iter);
		}
}
