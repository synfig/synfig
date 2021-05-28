/* === S Y N F I G ========================================================= */
/*!	\file resourcehelper.h
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

#ifndef SYNFIG_STUDIO_RESOURCEHELPER_H
#define SYNFIG_STUDIO_RESOURCEHELPER_H

#include <synfig/string.h>

namespace studio {

class ResourceHelper
{
public:
	static synfig::String get_image_path();
	static synfig::String get_image_path(const synfig::String& image_filename);

	static synfig::String get_synfig_data_path();

	static synfig::String get_icon_path();
	static synfig::String get_icon_path(const synfig::String& icon_filename);

	static synfig::String get_plugin_path();
	static synfig::String get_plugin_path(const synfig::String& plugin_filename);

	static synfig::String get_sound_path();
	static synfig::String get_sound_path(const synfig::String& sound_filename);

	static synfig::String get_ui_path();
	static synfig::String get_ui_path(const synfig::String& ui_filename);

	static synfig::String get_brush_path();
	static synfig::String get_brush_path(const synfig::String& brush_filename);

	static synfig::String get_css_path();
	static synfig::String get_css_path(const synfig::String& css_filename);
};

};

#endif // SYNFIG_STUDIO_RESOURCEHELPER_H
