/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.cpp
**	\brief Zoom widget
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2016 caryoscelus
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

#include "zoomdial.h"
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ZoomDial::ZoomDial(Gtk::IconSize & size): Table(5, 1, false)
{
	zoom_in = create_icon(size, Gtk::Stock::ZOOM_IN, _("Zoom In"));
	zoom_out = create_icon(size, Gtk::Stock::ZOOM_OUT, _("Zoom Out"));
	zoom_fit = create_icon(size, Gtk::Stock::ZOOM_FIT, _("Zoom to Fit"));
	zoom_norm = create_icon(size, Gtk::Stock::ZOOM_100, _("Zoom to 100%"));

	current_zoom = manage(new Gtk::Entry());
	set_zoom(1.0);
	current_zoom->set_max_length(10);
	current_zoom->set_editable(true);
	current_zoom->set_width_chars(6);
	current_zoom->show();
	// select everything except for % sign after user clicks widget
	// using release event here instead of grab_focus because the latter
	// is emitted before gtk sets cursor selection gets nullified
	current_zoom->signal_event_after().connect([this](auto event) {
		if (event->type == Gdk::BUTTON_RELEASE)
			current_zoom->select_region(0, current_zoom->get_text_length()-1);
	});

	attach(*zoom_out, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*current_zoom, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_in, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_norm, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_fit, 4, 5, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

Gtk::Button *
ZoomDial::create_icon(Gtk::IconSize size, const Gtk::BuiltinStockID & stockid,
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

void
ZoomDial::set_zoom(synfig::Real zoom)
{
	current_zoom->set_text((boost::format{"%.1f%%"} % (zoom*100)).str());
}

boost::optional<Real>
ZoomDial::get_zoom()
{
	std::istringstream input { current_zoom->get_text() };
	Real zoom;
	if (input >> zoom) {
		String suffix;
		if (input >> suffix) {
			boost::trim(suffix);
			if (suffix == "%") {
				zoom /= 100.0;
			} else if (suffix != "") {
				return boost::none;
			}
		}
	} else {
		return boost::none;
	}
	return boost::make_optional(zoom);
}
