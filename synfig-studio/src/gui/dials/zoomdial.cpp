/* === S Y N F I G ========================================================= */
/*!	\file zoomdial.cpp
**	\brief Zoom widget
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2016 caryoscelus
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

#include "zoomdial.h"

#include <gtkmm/image.h>

#include <gui/actiondatabase.h>
#include <gui/actionwidgethelper.h>
#include <gui/app.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ZoomDial::ZoomDial(const std::string& action_prefix)
	: Gtk::Grid()
{
	const std::string action_zoom_in  = action_prefix.empty() ? "" : action_prefix + "." + "canvas-zoom-in";
	const std::string action_zoom_out = action_prefix.empty() ? "" : action_prefix + "." + "canvas-zoom-out";
	const std::string action_zoom_fit = action_prefix.empty() ? "" : action_prefix + "." + "canvas-zoom-fit";
	const std::string action_zoom_100 = action_prefix.empty() ? "" : action_prefix + "." + "canvas-zoom-100";

	if (App::get_action_database()->has(action_zoom_in)) {
		zoom_in = ActionWidgetHelper::create_action_button(action_zoom_in);
		zoom_out = ActionWidgetHelper::create_action_button(action_zoom_out);
		zoom_fit = ActionWidgetHelper::create_action_button(action_zoom_fit);
		zoom_norm = ActionWidgetHelper::create_action_button(action_zoom_100);
	} else {
		zoom_in = ActionWidgetHelper::create_action_button(action_zoom_in, "zoom-in", "", _("Zoom In"));
		zoom_out = ActionWidgetHelper::create_action_button(action_zoom_out, "zoom-out", "", _("Zoom Out"));
		zoom_fit = ActionWidgetHelper::create_action_button(action_zoom_fit, "zoom-fit-best", "", _("Zoom to Fit"));
		zoom_norm = ActionWidgetHelper::create_action_button(action_zoom_100, "zoom-original", "", _("Zoom to 100%"));
	}

	zoom_in->set_relief(Gtk::RELIEF_NONE);
	zoom_out->set_relief(Gtk::RELIEF_NONE);
	zoom_fit->set_relief(Gtk::RELIEF_NONE);
	zoom_norm->set_relief(Gtk::RELIEF_NONE);

	current_zoom = manage(new Gtk::Entry());
	set_zoom(1.0);
	current_zoom->set_max_length(10);
	current_zoom->set_editable(true);
	current_zoom->set_width_chars(6);
	current_zoom->add_events(Gdk::SCROLL_MASK);
	current_zoom->signal_event().connect(
		sigc::mem_fun(*this, &ZoomDial::current_zoom_event) );
	current_zoom->show();
	// select everything except for % sign after user clicks widget
	// using release event here instead of grab_focus because the latter
	// is emitted before gtk sets cursor selection gets nullified
	current_zoom->signal_event_after().connect(
		sigc::mem_fun(*this, &ZoomDial::after_event) );

	attach(*zoom_out, 0, 0, 1, 1);
	attach(*current_zoom, 1, 0, 1, 1);
	attach(*zoom_in, 2, 0, 1, 1);
	attach(*zoom_norm, 3, 0, 1, 1);
	attach(*zoom_fit, 4, 0, 1, 1);
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

void
ZoomDial::set_zoom(synfig::Real value)
{
	current_zoom->set_text(synfig::strprintf("%.1lf%%", value*100.0));
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

