/* === S Y N F I G ========================================================= */
/*!	\file dialog_gradient.cpp
**	\brief Template File
**
**	$Id: dialog_color.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "dialog_color.h"
#include "widget_color.h"
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <synfig/general.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/value_desc.h>
#include "widget_color.h"
#include <gtkmm/spinbutton.h>
#include <synfigapp/main.h>
#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
#include <sigc++/hide.h>
#include "app.h"

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
	Dialog(_("Colors"),false,true),
	dialog_settings(this,"color"),
	busy_(false)
//	adjustment_pos(0,0.0,1.0,0.001,0.001,0.001)
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	// Setup the buttons
	//Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	//ok_button->show();
	//add_action_widget(*ok_button,2);
	//ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Color::on_ok_pressed));

	//Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
	//apply_button->show();
	//add_action_widget(*apply_button,1);
	//apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Color::on_apply_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::hide_return(sigc::mem_fun(*this, &Dialog_Color::on_close_pressed)));
	signal_delete_event().connect(sigc::hide(sigc::mem_fun(*this, &Dialog_Color::on_close_pressed)));

	Gtk::Table* table(manage(new Gtk::Table(2,2,false)));
	get_vbox()->pack_start(*table);

	widget_color=manage(new Widget_ColorEdit());
	widget_color->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Color::on_color_changed));
	//widget_color->signal_activate().connect(sigc::mem_fun(*this,&studio::Dialog_Color::on_color_changed));
	table->attach(*widget_color, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	add_accel_group(App::ui_manager()->get_accel_group());

	show_all_children();
}

Dialog_Color::~Dialog_Color()
{
}

void
Dialog_Color::reset()
{
	signal_edited_.clear();
}

bool
Dialog_Color::on_close_pressed()
{
//	signal_edited_(get_color());
	busy_=false;
	grab_focus();
	reset();
	hide();
	return true;
}

void
Dialog_Color::on_apply_pressed()
{
	busy_=true;
	signal_edited_(get_color());
	busy_=false;
}

void
Dialog_Color::on_color_changed()
{
	busy_=true;
	signal_edited_(get_color());
	busy_=false;
}
