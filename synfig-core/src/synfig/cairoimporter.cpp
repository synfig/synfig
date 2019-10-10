/* === S Y N F I G ========================================================= */
/*!	\file cairoimporter.cpp
**	\brief It is the base class for all the cairo importers.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#include "cairoimporter.h"

#include "general.h"
#include <synfig/localization.h>

#include "canvas.h"
#include "surface.h"
#include "string.h"

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

using namespace etl;
using namespace std;
using namespace synfig;

CairoImporter::Book* synfig::CairoImporter::book_;

map<FileSystem::Identifier, CairoImporter::LooseHandle> *__open_cairoimporters;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
CairoImporter::subsys_init()
{
	book_=new Book();
	__open_cairoimporters=new map<FileSystem::Identifier, CairoImporter::LooseHandle>();
	return true;
}

bool
CairoImporter::subsys_stop()
{
	delete book_;
	delete __open_cairoimporters;
	return true;
}

CairoImporter::Book&
CairoImporter::book()
{
	return *book_;
}

CairoImporter::Handle
CairoImporter::open(const FileSystem::Identifier &identifier)
{
	if(identifier.filename.empty())
	{
		synfig::error(_("CairoImporter::open(): Cannot open empty filename"));
		return 0;
	}

	// If we already have an importer open under that filename,
	// then use it instead.
	if(__open_cairoimporters->count(identifier))
	{
		//synfig::info("Found cairo importer already open, using it...");
		return (*__open_cairoimporters)[identifier];
	}

	if(filename_extension(identifier.filename) == "")
	{
		synfig::error(_("CairoImporter::open(): Couldn't find extension"));
		return 0;
	}

	String ext(filename_extension(identifier.filename));
	if (ext.size()) ext = ext.substr(1); // skip initial '.'
	std::transform(ext.begin(),ext.end(),ext.begin(),&::tolower);


	if(!CairoImporter::book().count(ext))
	{
		synfig::error(_("CairoImporter::open(): Unknown file type -- ")+ext);
		return 0;
	}

	try {
		CairoImporter::Handle importer;
		importer=CairoImporter::book()[ext].factory(identifier);
		(*__open_cairoimporters)[identifier]=importer;
		return importer;
	}
	catch (const String& str)
	{
		synfig::error(str);
	}
	return 0;
}

CairoImporter::CairoImporter(const FileSystem::Identifier &identifier):
	identifier(identifier)
{
}


CairoImporter::~CairoImporter()
{
	// Remove ourselves from the open importer list
	map<FileSystem::Identifier,CairoImporter::LooseHandle>::iterator iter;
	for(iter=__open_cairoimporters->begin();iter!=__open_cairoimporters->end();++iter)
		if(iter->second==this)
		{
			__open_cairoimporters->erase(iter);
		}
}
