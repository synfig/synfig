/* === S Y N F I G ========================================================= */
/*!	\file dialog_gradient.cpp
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

#include <gui/dialogs/dialog_gradient.h>

#include <gtkmm/spinbutton.h>
#include <gtkmm/table.h>

#include <gui/localization.h>
#include <gui/widgets/widget_coloredit.h>
#include <gui/widgets/widget_gradient.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>
#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Gradient::Dialog_Gradient():
	Dialog(_("Gradient Editor")),
	dialog_settings(this,"gradient"),
	adjustment_pos(Gtk::Adjustment::create(0,0.0,1.0,0.001,0.001,0))
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
	set_keep_above(false);

	set_role("gradient_editor");

	// Setup the buttons
	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Gradient::hide));

	set_default_button = manage(new class Gtk::Button(Gtk::StockID(_("Set as Default"))));
	set_default_button->show();
	add_action_widget(*set_default_button,2);
	set_default_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Gradient::on_set_default_pressed));

	Gtk::Table* table(manage(new Gtk::Table(2,2,false)));
	get_content_area()->pack_start(*table);

	widget_gradient=manage(new Widget_Gradient());
	widget_gradient->set_editable();
	widget_gradient->signal_cpoint_selected().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_cpoint_selected));
	widget_gradient->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_changed));
	//table->attach(*manage(new Gtk::Label(_("Not yet fully implemented"))), 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	table->attach(*widget_gradient, 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	widget_color=manage(new Widget_ColorEdit());
	widget_color->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_values_adjusted));
	widget_color->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_changed));
	widget_color->signal_activated().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_values_adjusted));
	table->attach(*widget_color, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);


	spinbutton_pos=manage(new class Gtk::SpinButton(adjustment_pos,0.0001,4));
	spinbutton_pos->set_update_policy(Gtk::UPDATE_ALWAYS);
	adjustment_pos->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_values_adjusted));
	adjustment_pos->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_changed));
	table->attach(*spinbutton_pos, 0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);


	show_all_children();
}

Dialog_Gradient::~Dialog_Gradient()
{
}

void
Dialog_Gradient::set_gradient(const synfig::Gradient& x)
{
	widget_gradient->set_value(x);
}

const Gradient&
Dialog_Gradient::get_gradient() const
{
	return widget_gradient->get_value();
}

void
Dialog_Gradient::reset()
{
	value_changed_connection.disconnect();
	signal_edited_.clear();
}

void
Dialog_Gradient::on_set_default_pressed()
{
	synfigapp::Main::set_gradient(get_gradient());
}

void
Dialog_Gradient::on_changed()
{
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
		printf("%s:%d Dialog_Gradient::on_changed()\n", __FILE__, __LINE__);

	signal_edited_(get_gradient());
}

void
Dialog_Gradient::on_cpoint_selected(synfig::Gradient::CPoint x)
{
	widget_color->set_value(x.color);
	adjustment_pos->set_value(x.pos);
}

void
Dialog_Gradient::on_values_adjusted()
{
	synfig::Gradient::CPoint x(widget_gradient->get_selected_cpoint());
	x.color=widget_color->get_value();
	x.pos=adjustment_pos->get_value();
	widget_gradient->update_cpoint(x);
}

static void
dialog_gradient_value_desc_edit(synfig::Gradient /*g*/,synfigapp::ValueDesc /*x*/,handle<synfigapp::CanvasInterface> /*canvas_interface*/)
{
//	canvas_interface->connect_value(x,ValueBase(g));
}

void
Dialog_Gradient::edit(const synfigapp::ValueDesc &x, std::shared_ptr<synfigapp::CanvasInterface> canvas_interface, synfig::Time time)
{
	reset();
	if(x.is_const())
		set_gradient(x.get_value().get(Gradient()));
	else if(x.is_value_node())
		set_gradient((*x.get_value_node())(time).get(Gradient()));

	signal_edited().connect(
		sigc::bind(
			sigc::bind(
				sigc::ptr_fun(dialog_gradient_value_desc_edit),
				canvas_interface
			),
			x
		)
	);

	present();
}
