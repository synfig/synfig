/* === S I N F G =========================================================== */
/*!	\file mod_palette.cpp
**	\brief Template File
**
**	$Id: mod_palette.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "../app.h"
#include "../dockmanager.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
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

	dock_pal_browse=new Dock_PalBrowse();
	App::get_dock_manager()->register_dockable(*dock_pal_browse);
	
	return true;
}

bool
studio::ModPalette::stop_vfunc()
{
	App::get_dock_manager()->unregister_dockable(*dock_pal_browse);
	App::get_dock_manager()->unregister_dockable(*dock_pal_edit);

	delete dock_pal_edit;
	delete dock_pal_browse;

	return true;
}
