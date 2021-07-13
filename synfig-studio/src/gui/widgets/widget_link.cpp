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

#include "widget_link.h"

#include <gtkmm/stock.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Widget_Link::Widget_Link()
	: ObjectBase    ("widget_link")
	, value_active  (*this, "tooltip_active"  , "Unlink")
	, value_inactive(*this, "tooltip_inactive", "Link")
{
	init();
}

Widget_Link::Widget_Link(const std::string &tlt_inactive, const std::string &tlt_active)
	: ObjectBase    ("widget_link")
	, value_active  (*this, "tooltip_active"  , tlt_active)
	, value_inactive(*this, "tooltip_inactive", tlt_inactive)
{
	init();
}

Widget_Link::Widget_Link(BaseObjectType *cobject)
	: ObjectBase    ("widget_link")
	, ToggleButton  (cobject)
	, value_active  (*this, "tooltip_active"  , "Unlink")
	, value_inactive(*this, "tooltip_inactive", "Link")
{
	init();
}

Widget_Link::Widget_Link(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder)
	: ObjectBase    ("widget_link")
	, ToggleButton  (cobject)
	, builder       (builder)
	, value_active  (*this, "tooltip_active"  , "Unlink")
	, value_inactive(*this, "tooltip_inactive", "Link")
{
	init();
}

void
Widget_Link::init()
{
	set_up_icons();

	refresh_tooltip_text();
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
	// set icon margins
	icon_off_->set_margin_start(0);
	icon_off_->set_margin_end(0);
	icon_off_->set_margin_top(0);
	icon_off_->set_margin_bottom(0);
	icon_on_->set_margin_start(0);
	icon_on_->set_margin_end(0);
	icon_on_->set_margin_top(0);
	icon_on_->set_margin_bottom(0);

	icon_off_->show();
	add(*icon_off_);
}

void
Widget_Link::on_toggled()
{
	Gtk::Image *icon;

	// refresh icon
	get_active() ? icon = icon_on_ : icon = icon_off_;

	refresh_tooltip_text();

	remove();
	add(*icon);
	icon->show();
}

void
Widget_Link::refresh_tooltip_text()
{
	if (get_active())
		set_tooltip_text(property_tooltip_active().get_value());
	else
		set_tooltip_text(property_tooltip_inactive().get_value());
}

GType Widget_Link::gtype = 0;

Glib::ObjectBase
*Widget_Link::wrap_new(GObject *o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new Widget_Link(GTK_TOGGLE_BUTTON(o));
	else
		return manage(new Widget_Link(GTK_TOGGLE_BUTTON(o)));
}

void
Widget_Link::register_type()
{
	if(gtype)
		return;

	Widget_Link dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	wrap_register(gtype, Widget_Link::wrap_new);
}

Widget_Link::~Widget_Link() {
	delete icon_on_;
	delete icon_off_;
}
