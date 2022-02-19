/* === S Y N F I G ========================================================= */
/*!	\file widget_waypointmodel.cpp
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Paul Wise
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

#include <gui/widgets/widget_interpolation.h>
#include <gui/widgets/widget_waypointmodel.h>

#include <gui/localization.h>

#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_WaypointModel::Widget_WaypointModel():
	Gtk::Grid(),
	adj_tension(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	adj_continuity(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	adj_bias(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	adj_temporal_tension(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	checkbutton_after(_("Out:")),
	checkbutton_before(_("In:")),
	checkbutton_tension(_("Tension:")),
	checkbutton_continuity(_("Continuity:")),
	checkbutton_bias(_("Bias:")),
	checkbutton_temporal_tension(_("Temporal Tension:"))
{
	before_options=manage(new Widget_Interpolation(Widget_Interpolation::SIDE_BEFORE));
	before_options->show();
	before_options->set_active(0);

	after_options=manage(new Widget_Interpolation(Widget_Interpolation::SIDE_AFTER));
	after_options->show();
	after_options->set_active(0);

	spin_tension=manage(new class Gtk::SpinButton(adj_tension,0.1,3));
	spin_tension->show();
	spin_continuity=manage(new class Gtk::SpinButton(adj_continuity,0.1,3));
	spin_continuity->show();
	spin_bias=manage(new class Gtk::SpinButton(adj_bias,0.1,3));
	spin_bias->show();
	spin_temporal_tension=manage(new class Gtk::SpinButton(adj_temporal_tension,0.1,3));
	spin_temporal_tension->show();

	checkbutton_before.signal_toggled().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	checkbutton_after.signal_toggled().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	checkbutton_tension.signal_toggled().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	checkbutton_continuity.signal_toggled().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	checkbutton_bias.signal_toggled().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	checkbutton_temporal_tension.signal_toggled().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));

	adj_tension->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	adj_continuity->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	adj_bias->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	adj_temporal_tension->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));

	before_options->signal_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	after_options->signal_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));

	set_row_spacing(6);
	set_column_spacing(12);
	set_border_width(18);

	// interpolation in
	attach(checkbutton_before, 0, 0, 1, 1);
	attach(*before_options, 1, 0, 1,1);
	before_options->set_hexpand(true);
	// interpolation out
	attach(checkbutton_after, 2, 0, 1, 1);
	attach(*after_options, 3, 0, 1, 1);
	after_options->set_hexpand(true);

	// tcb options - tension
	attach(checkbutton_tension, 0, 1, 1, 1);
	attach(*spin_tension, 1, 1, 1, 1);
	spin_tension->set_hexpand(true);
	// tcb options - continuity
	attach(checkbutton_continuity, 2, 1, 1, 1);
	attach(*spin_continuity, 3, 1, 1, 1);
	spin_continuity->set_hexpand(true);
	// tcb options - bias
	attach(checkbutton_bias, 0, 2, 1, 1);
	attach(*spin_bias, 1, 2, 1, 1);
	spin_bias->set_hexpand(true);
	// tcb options - temporal tension
	attach(checkbutton_temporal_tension, 2, 2, 1, 1);
	attach(*spin_temporal_tension, 3, 2, 1, 1);
	spin_temporal_tension->set_hexpand(true);

	show_all();
	hide();
	updating=false;
	on_change();
}

void
Widget_WaypointModel::on_change()
{
	if(updating)
		return;

	waypoint_model.set_before((Waypoint::Interpolation)before_options->get_value());
	waypoint_model.set_after((Waypoint::Interpolation)after_options->get_value());

	waypoint_model.set_tension(adj_tension->get_value());
	waypoint_model.set_continuity(adj_continuity->get_value());
	waypoint_model.set_bias(adj_bias->get_value());
	waypoint_model.set_temporal_tension(adj_temporal_tension->get_value());

	waypoint_model.set_before_flag(checkbutton_before.get_active());
	waypoint_model.set_after_flag(checkbutton_after.get_active());
	waypoint_model.set_tension_flag(checkbutton_tension.get_active());
	waypoint_model.set_continuity_flag(checkbutton_continuity.get_active());
	waypoint_model.set_bias_flag(checkbutton_bias.get_active());
	waypoint_model.set_temporal_tension_flag(checkbutton_temporal_tension.get_active());

	before_options->set_sensitive(checkbutton_before.get_active());
	after_options->set_sensitive(checkbutton_after.get_active());
	spin_tension->set_sensitive(checkbutton_tension.get_active());
	spin_continuity->set_sensitive(checkbutton_continuity.get_active());
	spin_bias->set_sensitive(checkbutton_bias.get_active());
	spin_temporal_tension->set_sensitive(checkbutton_temporal_tension.get_active());
}


void
Widget_WaypointModel::set_waypoint_model (const synfig::Waypoint::Model &x)
{
    updating=true;
    before_options->set_value((int)x.get_before());
    after_options->set_value((int)x.get_after());

    adj_tension->set_value(x.get_tension());
    adj_continuity->set_value(x.get_continuity());
    adj_bias->set_value(x.get_bias());
    adj_temporal_tension->set_value(x.get_temporal_tension());

    checkbutton_before.set_active(x.get_before_flag());
    checkbutton_after.set_active(x.get_after_flag());
    checkbutton_tension.set_active(x.get_tension_flag());
    checkbutton_continuity.set_active(x.get_continuity_flag());
    checkbutton_bias.set_active(x.get_bias_flag());
    checkbutton_temporal_tension.set_active(x.get_temporal_tension_flag());
    updating=false;
    on_change();
}


void
Widget_WaypointModel::reset_waypoint_model()
{
    updating=true;
    waypoint_model.reset();

    before_options->set_value(0);
    before_options->set_active(0);
    after_options->set_value(0);
    after_options->set_active(0);

    adj_tension->set_value(0);
    adj_continuity->set_value(0);
    adj_bias->set_value(0);
    adj_temporal_tension->set_value(0);

    checkbutton_before.set_active(false);
    checkbutton_after.set_active(false);
    checkbutton_tension.set_active(false);
    checkbutton_continuity.set_active(false);
    checkbutton_bias.set_active(false);
    checkbutton_temporal_tension.set_active(false);
    updating=false;
    on_change();
}
