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
#if defined CMAKE_BUILD
        imagepath=App::get_base_path()+"/share/pixmaps/synfigstudio";
#elif defined _WIN32
        imagepath=App::get_base_path()+"/share/pixmaps";
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

synfig::String studio::ResourceHelper::get_ui_path()
{
#ifndef UI_DIR
#   define UI_DIR "/usr/local/share/synfig/ui"
#endif

	std::string uipath;
	char* synfig_root=getenv("SYNFIG_ROOT");
	if(synfig_root) {
		uipath=std::string(synfig_root)+"/share/synfig/ui";
	}
	else {
#if defined CMAKE_BUILD || defined _WIN32
	    uipath=App::get_base_path()+"/share/synfig/ui";
#else
        uipath=UI_DIR;
#endif
	}
	return uipath;
}

synfig::String studio::ResourceHelper::get_ui_path(const synfig::String& ui_filename)
{
	return get_ui_path() + '/' + ui_filename;
}
