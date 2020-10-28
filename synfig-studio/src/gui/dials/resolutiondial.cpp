/* === S Y N F I G ========================================================= */
/*!	\file resolutiondial.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Gerco Ballintijn
**	Copyright (c) 2009 Carlos Lopez
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

#include <synfig/general.h>

#include <gui/localization.h>
#include "resolutiondial.h"
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ResolutionDial::ResolutionDial(const Gtk::IconSize &size)
{
	init_button(increase_resolution, size, Gtk::StockID("synfig-increase_resolution"), _("Increase Resolution"), _("Increase Display Resolution"));
	init_button(decrease_resolution, size, Gtk::StockID("synfig-decrease_resolution"), _("Decrease Resolution"), _("Decrease Display Resolution"));
	init_toggle_button(use_low_resolution, _("Low Res"), _("Use Low Resolution when enabled"));
}

void
ResolutionDial::insert_to_toolbar(Gtk::Toolbar &toolbar, int index)
{
	if (index < 0) index = toolbar.get_n_items();

	// reverse order
	toolbar.insert(increase_resolution, index);
	toolbar.insert(use_low_resolution,  index);
	toolbar.insert(decrease_resolution, index);
}

void
ResolutionDial::remove_from_toolbar(Gtk::Toolbar &toolbar)
{
	toolbar.remove(decrease_resolution);
	toolbar.remove(use_low_resolution);
	toolbar.remove(increase_resolution);
}

void
ResolutionDial::init_button(Gtk::ToolButton &button, Gtk::IconSize size, const Gtk::StockID & stockid, const char *label, const char *tooltip)
{
	Gtk::Image *icon = manage(new Gtk::Image(stockid, size));
	icon->set_padding(0, 0);
	icon->show();

	button.set_icon_widget(*icon);
	button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.show();
}

void
ResolutionDial::init_toggle_button(Gtk::ToggleToolButton &button, const char *label, const char *tooltip)
{
	// For label left/right padding
	button.set_name("low-resolution");

	button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.set_is_important(true);
	button.show();
}

void
ResolutionDial::update_lowres(bool flag)
{
	use_low_resolution.set_active(flag);
}
