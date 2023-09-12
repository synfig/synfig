/* === S Y N F I G ========================================================= */
/*!	\file mptr_imagemagick.cpp
**	\brief ImageMagick Importer Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2015 Jérôme Blanchi
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_imagemagick.h"

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/filesystemnative.h>
#include <synfig/filesystemtemporary.h>
#include <synfig/os.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(imagemagick_mptr);
SYNFIG_IMPORTER_SET_NAME(imagemagick_mptr,"imagemagick");
SYNFIG_IMPORTER_SET_EXT(imagemagick_mptr,"miff");
SYNFIG_IMPORTER_SET_VERSION(imagemagick_mptr,"0.1");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(imagemagick_mptr, false);

/* === M E T H O D S ======================================================= */


imagemagick_mptr::imagemagick_mptr(const synfig::FileSystem::Identifier& identifier)
	: synfig::Importer(identifier)
{ }

imagemagick_mptr::~imagemagick_mptr()
{
}

bool
imagemagick_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, Time /*time*/, synfig::ProgressCallback *cb)
{
	if(identifier.filename.empty() || !identifier.file_system)
	{
		if(cb)cb->error(_("No file to load"));
		else synfig::error(_("No file to load"));
		return false;
	}

	bool is_temporary_file = false;
	std::string filename = identifier.file_system->get_real_filename(identifier.filename.u8string());
	std::string target_filename=FileSystemTemporary::generate_system_temporary_filename("imagemagick", ".png");

	std::string filename_extension = identifier.filename.extension().u8string();

	if (filename.empty()) {
		is_temporary_file = true;
		filename = FileSystemTemporary::generate_system_temporary_filename("imagemagick", filename_extension);

		// try to copy file to a temp file
		if (!FileSystem::copy(identifier.file_system, identifier.filename.u8string(), identifier.file_system, filename))
		{
			std::string msg = strprintf(_("Cannot create temporary file of %s"), identifier.filename.u8_str());
			if (cb)
				cb->error(msg);
			else
				synfig::error(msg);
			return false;
		}
	}

	OS::RunArgs args;
	args.push_back(filesystem::Path(filename));

	if (filename_extension == ".psd" || filename_extension == ".xcf")
		args.push_back("-flatten");

	args.push_back(strprintf("png32:%s", target_filename.c_str()));

	bool success = OS::run_sync({"convert"}, args);
	if (!success) {
		synfig::error(_("Unable to open convert [ImageMagick]"));
		return false;
	}

	if(is_temporary_file)
		identifier.file_system->file_remove(filename);

	Importer::Handle importer(Importer::open(synfig::FileSystem::Identifier(synfig::FileSystemNative::instance(), target_filename)));

	if(!importer)
	{
		if(cb)cb->error(_("Unable to open ")+target_filename);
		else synfig::error(_("Unable to open ")+target_filename);
		return false;
	}

	if(!importer->get_frame(surface,renddesc,0,cb))
	{
		if(cb)cb->error(_("Unable to get frame from ")+target_filename);
		else synfig::error(_("Unable to get frame from ")+target_filename);
		return false;
	}

	if(!surface)
	{
		if(cb)cb->error(_("Bad surface from ")+target_filename);
		else synfig::error(_("Bad surface from ")+target_filename);
		return false;
	}

	if(1)
	{
		// remove odd premultiplication
		for(int i=0;i<surface.get_w()*surface.get_h();i++)
		{
			Color c(surface[0][i]);

			if(c.get_a())
			{
				surface[0][i].set_r(c.get_r()/c.get_a()/c.get_a());
				surface[0][i].set_g(c.get_g()/c.get_a()/c.get_a());
				surface[0][i].set_b(c.get_b()/c.get_a()/c.get_a());
			}
			else
			{
				surface[0][i].set_r(0);
				surface[0][i].set_g(0);
				surface[0][i].set_b(0);
			}
			surface[0][i].set_a(c.get_a());
		}
	}

	Surface bleh(surface);
	surface=bleh;

	remove(target_filename.c_str());
	return true;
}
