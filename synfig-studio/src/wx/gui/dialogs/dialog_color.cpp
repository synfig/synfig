/* === S Y N F I G ========================================================= */
/*!	\file dialog_color.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "dialogs/dialog_color.h"
#include "widgets/widget_color.h"
#include <synfig/general.h>
#include <synfigapp/main.h>
#include <gtkmm/button.h>
#include "app.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Color::Dialog_Color():
	Dialog(_("Colors")),
	dialog_settings(this, "color"),
	busy_(false)
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);

	create_color_edit_widget();
	create_set_color_button("synfig-set_outline_color", _("Set as Outline"), 0,
			sigc::mem_fun(*this, &Dialog_Color::on_set_oc_pressed));
	create_set_color_button("synfig-set_fill_color", _("Set as Fill"), 1,
			sigc::mem_fun(*this, &Dialog_Color::on_set_fc_pressed));
	create_close_button();

	add_accel_group(App::ui_manager()->get_accel_group());
	show_all_children();
}

Dialog_Color::~Dialog_Color()
{
}

void
Dialog_Color::create_color_edit_widget()
{
	color_edit_widget = manage(new Widget_ColorEdit());
	color_edit_widget->signal_value_changed().connect(sigc::mem_fun(*this,
			&studio::Dialog_Color::on_color_changed));
	get_vbox()->pack_start(*color_edit_widget);
}

void
Dialog_Color::create_set_color_button(const char *stock_id,
		const Glib::ustring& tip_text, int index,
		const sigc::slot0<void>& callback)
{
	Gtk::Button *set_color_button = manage(new Gtk::Button());
	Gtk::Image *set_color_icon = manage(new Gtk::Image(Gtk::StockID(stock_id),
			Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)));
	set_color_button->add(*set_color_icon);
	set_color_icon->show();
	set_color_button->set_tooltip_text(tip_text);
	set_color_button->show();
	add_action_widget(*set_color_button, index);
	set_color_button->signal_clicked().connect(callback);
}

void
Dialog_Color::create_close_button()
{
	Gtk::Button *close_button(manage(new Gtk::Button(Gtk::StockID("gtk-close"))));
	close_button->show();
	add_action_widget(*close_button, 2);
	close_button->signal_clicked().connect(sigc::hide_return(sigc::mem_fun(*this,
			&Dialog_Color::on_close_pressed)));
	signal_delete_event().connect(sigc::hide(sigc::mem_fun(*this,
			&Dialog_Color::on_close_pressed)));
}

void
Dialog_Color::on_color_changed()
{
	busy_ = true;
	signal_edited_(get_color());
	busy_ = false;
}

void
Dialog_Color::on_set_oc_pressed()
{
	busy_ = true;
	synfigapp::Main::set_outline_color(get_color());
	busy_ = false;
}

void
Dialog_Color::on_set_fc_pressed()
{
	busy_ = true;
	synfigapp::Main::set_fill_color(get_color());
	busy_ = false;
}

bool
Dialog_Color::on_close_pressed()
{
	busy_ = false;
	grab_focus();
	reset();
	hide();
	return true;
}

void
Dialog_Color::reset()
{
	signal_edited_.clear();
}
