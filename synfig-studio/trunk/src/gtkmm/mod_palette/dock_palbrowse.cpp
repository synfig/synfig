/* === S I N F G =========================================================== */
/*!	\file dialog_palette.cpp
**	\brief Template File
**
**	$Id: dock_palbrowse.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "dock_palbrowse.h"
#include "dock_paledit.h"
#include "mod_palette.h"

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

Dock_PalBrowse::Dock_PalBrowse():
	Dockable("pal_browse",_("Palette Browser")/*,Gtk::StockID("gtk-select-color")*/)
{	
}

Dock_PalBrowse::~Dock_PalBrowse()
{
}
