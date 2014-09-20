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

#include <gtkmm/stock.h>
#include "widgets/widget_link.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */



namespace studio {

Widget_Link::Widget_Link() {

	Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon");
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-utils_chain_link_off"), iconsize));
//	toggle_wh_ratio->set_tooltip_text(_("Link width and height"));
	icon->set_padding(0,0);
	icon->show();
	add(*icon);
	set_relief(Gtk::RELIEF_NONE);
}

Widget_Link::~Widget_Link() {
}

}


void Widget_Link::on_toggled()
{
	Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon");
	Gtk::Image *icon;

	if(get_active())
	{
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-utils_chain_link_on"),iconsize));
//		toggle_wh_ratio->set_tooltip_text(_("Unlink width and height"));

//		rend_desc_.set_pixel_ratio(adjustment_width->get_value(), adjustment_height->get_value());
//		rend_desc_.set_flags(rend_desc_.get_flags()|RendDesc::LINK_IM_ASPECT);
	}
	else
	{
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-utils_chain_link_off"),iconsize));
//		toggle_wh_ratio->set_tooltip_text(_("Link width and height"));

//		rend_desc_.set_flags(rend_desc_.get_flags()&~RendDesc::LINK_IM_ASPECT);
	}

	remove();
	add(*icon);
	icon->set_padding(0,0);
	icon->show();

}
