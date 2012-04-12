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

ResolutionDial::ResolutionDial(Gtk::IconSize & size): Gtk::Table(3, 1, false)
{

	increase_resolution = create_icon(size, Gtk::StockID("synfig-increase_resolution"), _("Increase Display Resolution"));
	decrease_resolution = create_icon(size, Gtk::StockID("synfig-decrease_resolution"), _("Decrease Display Resolution"));
	use_low_resolution = create_check(_("Low Res"), _("Use Low Resolution when enabled"));

	attach(*decrease_resolution, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*use_low_resolution, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*increase_resolution, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

Gtk::Button *
ResolutionDial::create_icon(Gtk::IconSize size, const Gtk::StockID & stockid,
		const char * tooltip)
{
	Gtk::Button *button = manage(new class Gtk::Button());
	Gtk::Image *icon = manage(new Gtk::Image(stockid, size));
	button->add(*icon);
	button->set_tooltip_text(tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}

Gtk::CheckButton *
ResolutionDial::create_check(const char *label, const char * tooltip)
{
	Gtk::CheckButton *cbutton = manage(new class Gtk::CheckButton());
	cbutton->set_label(label);
	cbutton->set_tooltip_text(tooltip);
	cbutton->show();

	return cbutton;
}

void
ResolutionDial::update_lowres(bool flag)
{
	use_low_resolution->set_active(flag);
}
