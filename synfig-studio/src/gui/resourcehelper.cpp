/* === S Y N F I G ========================================================= */
/*!	\file resourcehelper.h
**	\brief Helper to retrieve the app resource paths, such as icons and plugins
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2019 Rodolfo R Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "resourcehelper.h"

#include "app.h"

#endif

synfig::String studio::ResourceHelper::get_image_path()
{

#ifndef IMAGE_DIR
#	define IMAGE_DIR "/usr/local/share/pixmaps/synfigstudio"
#endif

	std::string imagepath;
	char* synfig_root=getenv("SYNFIG_ROOT");
	if(synfig_root) {
		//  Only class About didn't use the synfigstudio directory when using
		//  SYNFIG_ROOT env variable. However, if it weren't set, it would use
		//  IMAGE_DIR builtin variable that includes "synfigstudio". It means
		//  that that About icon is in both folders. Therefore, it is safe to
		//  choose this path.
		imagepath=std::string(synfig_root)+"/share/pixmaps/synfigstudio";
	}
	else {
#if defined CMAKE_BUILD || defined _WIN32
        imagepath=App::get_base_path()+"/share/pixmaps/synfigstudio";
#else
        imagepath=IMAGE_DIR;
#endif
	}

	return imagepath;
}

synfig::String studio::ResourceHelper::get_image_path(const synfig::String& image_filename)
{
	return get_image_path() + '/' + image_filename;
}

#ifndef SYNFIG_DATADIR
#	define SYNFIG_DATADIR "/usr/local/share/synfig/"
#endif

synfig::String studio::ResourceHelper::get_synfig_data_path()
{
#if defined CMAKE_BUILD || defined _WIN32
	std::string synfig_datadir = App::get_base_path();
#else
	std::string synfig_datadir = SYNFIG_DATADIR;
#endif

	if (char* synfig_root = getenv("SYNFIG_ROOT")) {
		synfig_datadir = std::string(synfig_root)
			+ ETL_DIRECTORY_SEPARATOR + "share/synfig";
	}

	return synfig_datadir;
}

synfig::String studio::ResourceHelper::get_plugin_path()
{
	std::string uipath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "plugins";
	return uipath;
}

synfig::String studio::ResourceHelper::get_plugin_path(const synfig::String& plugin_filename)
{
	return get_plugin_path() + '/' + plugin_filename;
}

synfig::String studio::ResourceHelper::get_sound_path()
{
	std::string uipath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "sounds";
	return uipath;
}

synfig::String studio::ResourceHelper::get_sound_path(const synfig::String& sound_filename)
{
	return get_sound_path() + '/' + sound_filename;
}

synfig::String studio::ResourceHelper::get_ui_path()
{
	std::string uipath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "ui";
	return uipath;
}

synfig::String studio::ResourceHelper::get_ui_path(const synfig::String& ui_filename)
{
	return get_ui_path() + '/' + ui_filename;
}
