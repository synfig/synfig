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

#ifndef SYNFIG_STUDIO_RESOURCEHELPER_H
#define SYNFIG_STUDIO_RESOURCEHELPER_H

#include "synfig/string.h"

namespace studio {

class ResourceHelper
{
public:
	static synfig::String get_image_path();
	static synfig::String get_image_path(const synfig::String& image_filename);

	static synfig::String get_ui_path();
	static synfig::String get_ui_path(const synfig::String& ui_filename);
};

};

#endif // SYNFIG_STUDIO_RESOURCEHELPER_H
