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

#include <ETL/stringf>

#include <synfig/general.h>

#include "zoomdial.h"
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#include <gui/localization.h>

#include <gui/exception_guard.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ZoomDial::ZoomDial(Gtk::IconSize & size):
	Table(5, 1, false)
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
	current_zoom->add_events(Gdk::SCROLL_MASK);
	current_zoom->signal_leave_notify_event().connect(
		sigc::mem_fun(*this, &ZoomDial::on_current_zoom_leave_event));
	current_zoom->signal_event().connect(
		sigc::mem_fun(*this, &ZoomDial::current_zoom_event) );
	current_zoom->show();
	// select everything except for % sign after user clicks widget
	// using release event here instead of grab_focus because the latter
	// is emitted before gtk sets cursor selection gets nullified
	current_zoom->signal_event_after().connect(
		sigc::mem_fun(*this, &ZoomDial::after_event) );

	attach(*zoom_out, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*current_zoom, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*zoom_in, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_norm, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*zoom_fit, 4, 5, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
}

bool
ZoomDial::on_current_zoom_leave_event(GdkEventCrossing *event)
{
	if (event->type == GDK_LEAVE_NOTIFY) {
		zoom_fit->grab_focus();
		return true;
	}
	return false;
}

void
ZoomDial::after_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (event->type == GDK_BUTTON_RELEASE)
		current_zoom->select_region(0, current_zoom->get_text_length()-1);
	SYNFIG_EXCEPTION_GUARD_END()
}

bool
ZoomDial::current_zoom_event(GdkEvent* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (event->type == GDK_SCROLL)
	{
		if(event->scroll.direction==GDK_SCROLL_DOWN || event->scroll.direction==GDK_SCROLL_LEFT)
			zoom_out->clicked();
		else
		if(event->scroll.direction==GDK_SCROLL_UP || event->scroll.direction==GDK_SCROLL_RIGHT)
			zoom_in->clicked();
		return true;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
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
ZoomDial::set_zoom(synfig::Real value)
{
	current_zoom->set_text(etl::strprintf("%.1lf%%", value*100.0));
}

synfig::Real
ZoomDial::get_zoom(synfig::Real default_value)
{
	std::string s = current_zoom->get_text();
	char buffer[10] = "";
	synfig::Real value = 0.0;
	sscanf(s.c_str(), "%lf%9s", &value, buffer);

	if (std::string(buffer) == "%") value *= 0.01;
	return approximate_greater(value, 0.0) ? value : default_value;
}

