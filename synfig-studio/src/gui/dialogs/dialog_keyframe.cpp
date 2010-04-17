/* === S Y N F I G ========================================================= */
/*!	\file dialog_keyframe.cpp
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

#include "dialogs/dialog_keyframe.h"
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include "widgets/widget_waypointmodel.h"
#include <synfigapp/action.h>
#include <synfigapp/instance.h>

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

Dialog_Keyframe::Dialog_Keyframe(Gtk::Window& parent, etl::handle<synfigapp::CanvasInterface> canvas_interface):
	Gtk::Dialog(_("Keyframe Dialog"),parent,false,true),
	canvas_interface(canvas_interface)
{
	// Set up the buttons
	{
		Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
		ok_button->show();
		add_action_widget(*ok_button,2);
		ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::on_ok_pressed));

		Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
		apply_button->show();
		add_action_widget(*apply_button,1);
		apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::on_apply_pressed));

		Gtk::Button *delete_button(manage(new class Gtk::Button(Gtk::StockID("gtk-delete"))));
		delete_button->show();
		add_action_widget(*delete_button,3);
		delete_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::on_delete_pressed));

		Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
		cancel_button->show();
		add_action_widget(*cancel_button,0);
		cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::hide));
	}

	Gtk::Table *table=manage(new Gtk::Table(2,2,false));

	get_vbox()->pack_start(*table);

/*  // \todo Allow setting descriptions for keyframes

	entry_description.set_text(_("Not yet implemented"));

	table->attach(*manage(new Gtk::Label(_("Description"))), 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	table->attach(entry_description, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
*/

	table->show_all();

	widget_waypoint_model=Gtk::manage(new Widget_WaypointModel());
	widget_waypoint_model->show();
	table->attach(*widget_waypoint_model, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

}

Dialog_Keyframe::~Dialog_Keyframe()
{
}

const synfig::Keyframe&
Dialog_Keyframe::get_keyframe()const
{
	return keyframe_;
}

void
Dialog_Keyframe::set_keyframe(const synfig::Keyframe& x)
{
	keyframe_=x;
}

void
Dialog_Keyframe::on_ok_pressed()
{
	on_apply_pressed();
	hide();
}


void
Dialog_Keyframe::on_delete_pressed()
{
	synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeRemove"));

	assert(action);

	action->set_param("canvas",canvas_interface->get_canvas());
	action->set_param("canvas_interface",canvas_interface);
	action->set_param("keyframe",keyframe_);
	action->set_param("model",widget_waypoint_model->get_waypoint_model());

	if(canvas_interface->get_instance()->perform_action(action))
	{
		hide();
	}
}


void
Dialog_Keyframe::on_apply_pressed()
{
	if(widget_waypoint_model->get_waypoint_model().is_trivial())
		return;

	synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeWaypointSet"));

	assert(action);

	action->set_param("canvas",canvas_interface->get_canvas());
	action->set_param("canvas_interface",canvas_interface);
	action->set_param("keyframe",keyframe_);
	action->set_param("model",widget_waypoint_model->get_waypoint_model());

	if(!canvas_interface->get_instance()->perform_action(action))
	{
	}
}
