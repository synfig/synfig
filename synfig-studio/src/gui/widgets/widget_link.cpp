/* === S Y N F I G ========================================================= */
/*!	\file widget_vector.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2014 Jérôme Blanchi
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

#include <gtkmm/stock.h>
#include "widgets/widget_link.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

const string Widget_Link::tooltip_wh_on   = "Link width and height";
const string Widget_Link::tooltip_wh_off  = "Unlink width and height";
const string Widget_Link::tooltip_res_on  = "Link x and y resolution";
const string Widget_Link::tooltip_res_off = "Unlink x and y resolution";

Widget_Link::Widget_Link():
	Glib::ObjectBase          ("widget_link"),
	is_custom_widget_called   (true),
	property_tooltip_active_  (*this, "tooltip_active", ""),
	property_tooltip_inactive_(*this, "tooltip_inactive", "")
{
	init();
}

Widget_Link::Widget_Link(const std::string &tlt_inactive, const std::string &tlt_active):
	Glib::ObjectBase          ("widget_link"),
	is_custom_widget_called   (false),
	property_tooltip_active_  (*this, "tooltip_active", ""),
	property_tooltip_inactive_(*this, "tooltip_inactive", "")
{
	init(tlt_inactive, tlt_active);
}

Widget_Link::Widget_Link(BaseObjectType *cobject):
	Glib::ObjectBase          ("widget_link"),
	Gtk::ToggleButton         (cobject),
	is_custom_widget_called   (true),
	property_tooltip_active_  (*this, "tooltip_active", ""),
	property_tooltip_inactive_(*this, "tooltip_inactive", "")
{
	init();
}

// This constructor is meant to be called by builder->get_widget_derived()
Widget_Link::Widget_Link(BaseObjectType *cobject, const std::string &tlt_inactive, const std::string &tlt_active):
	Glib::ObjectBase          ("widget_link"),
	Gtk::ToggleButton         (cobject),
	is_custom_widget_called   (true),
	property_tooltip_active_  (*this, "tooltip_active", ""),
	property_tooltip_inactive_(*this, "tooltip_inactive", "")
{
	init(tlt_inactive, tlt_active);
}

Widget_Link::~Widget_Link() {
	delete icon_on_;
	delete icon_off_;
}

void
Widget_Link::init()
{
	set_up_icons();
	refresh_tooltip_texts();
}

void
Widget_Link::init(const string &tlt_inactive, const string &tlt_active)
{
	set_up_icons();

	tooltip_inactive_ = tlt_inactive;
	tooltip_active_   = tlt_active;

	set_tooltip_text(tooltip_inactive_);
}

void
Widget_Link::set_up_icons()
{
	const Glib::RefPtr<Gtk::StyleContext> context = get_style_context();

	// hardfixed icon size. chain icon is not a square but a rectangle.
	Glib::RefPtr<Gtk::IconSet> chain_icon = Gtk::IconSet::lookup_default(Gtk::StockID("synfig-utils_chain_link_off"));
	Glib::RefPtr<Gdk::Pixbuf> chain_icon_pixbuff = chain_icon->render_icon_pixbuf(context, (Gtk::IconSize)-1);
	Glib::RefPtr<Gdk::Pixbuf> chain_icon_pixbuff_scaled = chain_icon_pixbuff->scale_simple(16, 32, Gdk::INTERP_BILINEAR);
	// not use manage() otherwise the not-shown icon at exit wouldn't be deleted...
	icon_off_ = new Gtk::Image(chain_icon_pixbuff_scaled);

	chain_icon = Gtk::IconSet::lookup_default(Gtk::StockID("synfig-utils_chain_link_on"));
	chain_icon_pixbuff_scaled = chain_icon->render_icon_pixbuf(context, (Gtk::IconSize)-1)->scale_simple(16, 32, Gdk::INTERP_BILINEAR);
	// not use manage() otherwise the not-shown icon at exit wouldn't be deleted...
	icon_on_ = new Gtk::Image(chain_icon_pixbuff_scaled);

	icon_off_->set_padding(0,0);
	icon_on_->set_padding(0,0);

	icon_off_->show();
	add(*icon_off_);
}

void
Widget_Link::on_toggled()
{
	Gtk::Image *icon;

	// refresh icon
	get_active() ? icon = icon_on_ : icon = icon_off_;

	// refresh tooltip text
	if (!is_custom_widget_called)
		get_active() ? set_tooltip_text(tooltip_active_) : set_tooltip_text(tooltip_inactive_);
	else
		refresh_tooltip_texts();

	remove();
	add(*icon);
	icon->show();
}

void
Widget_Link::refresh_tooltip_texts()
{
	string widget = get_tooltip_text();

	if (get_active()) {

		if (widget.find("width") != string::npos)
			set_tooltip_text(tooltip_wh_off);

		if (widget.find("resolution") != string::npos)
			set_tooltip_text(tooltip_res_off);

	} else {

		if (widget.find("width") != string::npos)
			set_tooltip_text(tooltip_wh_on);

		if (widget.find("resolution") != string::npos)
			set_tooltip_text(tooltip_res_on);

	}
}

GType Widget_Link::gtype = 0;

Glib::ObjectBase
*Widget_Link::wrap_new(GObject *o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new Widget_Link(GTK_TOGGLE_BUTTON(o));
	else
		return Gtk::manage(new Widget_Link(GTK_TOGGLE_BUTTON(o)));
}

void
Widget_Link::register_type()
{
	if(gtype)
		return;

	Widget_Link dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	Glib::wrap_register(gtype, Widget_Link::wrap_new);
}
