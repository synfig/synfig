/* === S Y N F I G ========================================================= */
/*!	\file dialog_keyframe.cpp
**	\brief Keyframe properties dialog implementation
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <gui/dialogs/dialog_keyframe.h>

#include <gtkmm/button.h>

#include <gui/localization.h>
#include <gui/widgets/widget_waypointmodel.h>

#include <synfigapp/action.h>
#include <synfigapp/instance.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Keyframe::Dialog_Keyframe(Gtk::Window& parent, etl::handle<synfigapp::CanvasInterface> canvas_interface):
	Gtk::Dialog(_("Keyframe Dialog"),parent),
	canvas_interface(canvas_interface)
{
	// Set up the buttons
	{
		Gtk::Button *delete_button(manage(new class Gtk::Button(Gtk::StockID("gtk-delete"))));
		delete_button->show();
		add_action_widget(*delete_button,3);
		delete_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::on_delete_pressed));

		Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
		apply_button->show();
		add_action_widget(*apply_button,1);
		apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::on_apply_pressed));

		Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
		cancel_button->show();
		add_action_widget(*cancel_button,0);
		cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::hide));

		Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
		ok_button->show();
		add_action_widget(*ok_button,2);
		ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Keyframe::on_ok_pressed));
	}

	Gtk::Grid *grid=manage(new Gtk::Grid());
	grid->set_row_spacing(6);
	grid->set_column_spacing(12);

	get_content_area()->add(*grid);

	// Allow setting descriptions for keyframes
	entry_description.set_text("");
	grid->attach(*manage(new Gtk::Label(_("Description: "))),   0, 0, 1, 1);
	grid->attach(entry_description,                             1, 0, 3, 1);
	entry_description.set_hexpand(true);

	// Allow toggling active status for keyframes
	grid->attach(*manage(new Gtk::Label(_("Active: "))),        4, 0, 1, 1);
	grid->attach(entry_toogle,                                  5, 0, 1, 1);

	widget_waypoint_model=Gtk::manage(new Widget_WaypointModel());
	widget_waypoint_model->show();
	grid->attach(*widget_waypoint_model,                        0, 1, 6, 2);

	grid->show_all();
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
	entry_description.set_text(keyframe_.get_description());
	entry_toogle.set_active(keyframe_.active());

	widget_waypoint_model->reset_waypoint_model();

	if (keyframe_.has_model())
	{
	    // TODO operator = for wp::model ?
	    widget_waypoint_model->set_waypoint_model(keyframe_.get_waypoint_model());
	}
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
	//! Set the new description if needed
	if(entry_description.get_text() != keyframe_.get_description())
	{
		keyframe_.set_description(entry_description.get_text());

		synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeSet"));
		assert(action);

		action->set_param("canvas",canvas_interface->get_canvas());
		action->set_param("canvas_interface",canvas_interface);
		action->set_param("keyframe",keyframe_);

		if(!canvas_interface->get_instance()->perform_action(action))
		{
		}
	}

	//! Update the active status if needed
	if(entry_toogle.get_active() != keyframe_.active())
	{
		keyframe_.set_active(entry_toogle.get_active());

		synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeToggl"));
		assert(action);

		action->set_param("canvas",canvas_interface->get_canvas());
		action->set_param("canvas_interface",canvas_interface);
		action->set_param("keyframe",keyframe_);
		action->set_param("new_status",keyframe_.active ());

		if(!canvas_interface->get_instance()->perform_action(action))
		{
		}
	}

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
