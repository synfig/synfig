/* === S Y N F I G ========================================================= */
/*!	\file resourcehelper.cpp
**	\brief Helper to retrieve the app resource paths, such as icons and plugins
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2019 Rodolfo R Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <resourcehelper.h>

#include <gui/app.h>

#endif

synfig::String studio::ResourceHelper::get_image_path()
{
	std::string imagepath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "images";
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
	std::string synfig_datadir;
	if (char* synfig_root = getenv("SYNFIG_ROOT")) {
		synfig_datadir = std::string(synfig_root)
			+ ETL_DIRECTORY_SEPARATOR + "share/synfig";
	} else {
#if defined CMAKE_BUILD || defined _WIN32
		synfig_datadir = App::get_base_path()
             + ETL_DIRECTORY_SEPARATOR + "share/synfig";
#else
		synfig_datadir = SYNFIG_DATADIR;
#endif
	}

	return synfig_datadir;
}

synfig::String studio::ResourceHelper::get_icon_path()
{
	std::string iconpath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "icons";
	return iconpath;
}

synfig::String studio::ResourceHelper::get_plugin_path()
{
	std::string pluginpath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "plugins";
	return pluginpath;
}

synfig::String studio::ResourceHelper::get_plugin_path(const synfig::String& plugin_filename)
{
	return get_plugin_path() + '/' + plugin_filename;
}

synfig::String studio::ResourceHelper::get_sound_path()
{
	std::string soundpath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "sounds";
	return soundpath;
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

synfig::String studio::ResourceHelper::get_brush_path()
{
	std::string brushpath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "brushes";
	return brushpath;
}

synfig::String studio::ResourceHelper::get_brush_path(const synfig::String& brush_filename)
{
	return get_brush_path() + '/' + brush_filename;
}

synfig::String studio::ResourceHelper::get_css_path()
{
	std::string csspath = get_synfig_data_path() + ETL_DIRECTORY_SEPARATOR + "css";
	return csspath;
}

synfig::String studio::ResourceHelper::get_css_path(const synfig::String& css_filename)
{
	return get_css_path() + '/' + css_filename;
}
