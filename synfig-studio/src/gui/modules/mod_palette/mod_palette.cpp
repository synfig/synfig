/* === S Y N F I G ========================================================= */
/*!	\file mod_palette.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "mod_palette.h"
#include "dock_paledit.h"
#include "dock_palbrowse.h"

#include <gui/app.h>
#include <gui/docks/dockmanager.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
studio::ModPalette::start_vfunc()
{
	dock_pal_edit=new Dock_PalEdit();
	App::get_dock_manager()->register_dockable(*dock_pal_edit);

	//dock_pal_browse=new Dock_PalBrowse();
	//App::get_dock_manager()->register_dockable(*dock_pal_browse);

	return true;
}

bool
studio::ModPalette::stop_vfunc()
{
	//App::get_dock_manager()->unregister_dockable(*dock_pal_browse);
	App::get_dock_manager()->unregister_dockable(*dock_pal_edit);

	delete dock_pal_edit;
	//delete dock_pal_browse;

	return true;
}
