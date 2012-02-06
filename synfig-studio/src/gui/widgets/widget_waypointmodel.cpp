/* === S Y N F I G ========================================================= */
/*!	\file widget_waypointmodel.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Paul Wise
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

#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <ETL/stringf>
#include "widgets/widget_value.h"
#include "app.h"
#include <gtkmm/menu.h>
#include <gtkmm/optionmenu.h>
#include "widgets/widget_time.h"
#include "widgets/widget_waypointmodel.h"
#include "general.h"

#endif

using namespace synfig;
using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_WaypointModel::Widget_WaypointModel():
	Gtk::Table(4,3,false),
	adj_tension(0.0,-20,20,0.1,1),
	adj_continuity(0.0,-20,20,0.1,1),
	adj_bias(0.0,-20,20,0.1,1),
	adj_temporal_tension(0.0,-20,20,0.1,1),
	checkbutton_after(_("Out:")),
	checkbutton_before(_("In:")),
	checkbutton_tension(_("Tension:")),
	checkbutton_continuity(_("Continuity:")),
	checkbutton_bias(_("Bias:")),
	checkbutton_temporal_tension(_("Temporal Tension:"))
{
	before_options=manage(new Widget_Enum());
	before_options->show();
	before_options->set_param_desc(
		ParamDesc("interpolation")
			.set_hint("enum")
			.add_enum_value(INTERPOLATION_CLAMPED,"clamped",_("Clamped"))
			.add_enum_value(INTERPOLATION_TCB,"auto",_("TCB"))
			.add_enum_value(INTERPOLATION_CONSTANT,"constant",_("Constant"))
			.add_enum_value(INTERPOLATION_HALT,"ease",_("Ease In/Out"))
			.add_enum_value(INTERPOLATION_LINEAR,"linear",_("Linear"))
	);
	before_options->set_icon(0, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_clamped"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(1, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_tcb"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(2, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_const"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(3, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_ease"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(4, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_linear"),Gtk::ICON_SIZE_MENU));
	before_options->set_active(0);

	after_options=manage(new Widget_Enum());
	after_options->show();
	after_options->set_param_desc(
		ParamDesc("interpolation")
			.set_hint("enum")
			.add_enum_value(INTERPOLATION_CLAMPED,"clamped",_("Clamped"))
			.add_enum_value(INTERPOLATION_TCB,"auto",_("TCB"))
			.add_enum_value(INTERPOLATION_CONSTANT,"constant",_("Constant"))
			.add_enum_value(INTERPOLATION_HALT,"ease",_("Ease In/Out"))
			.add_enum_value(INTERPOLATION_LINEAR,"linear",_("Linear"))
	);
	after_options->set_icon(0, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_clamped"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(1, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_tcb"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(2, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_const"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(3, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_ease"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(4, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_linear"),Gtk::ICON_SIZE_MENU));
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

	adj_tension.signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	adj_continuity.signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	adj_bias.signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	adj_temporal_tension.signal_value_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));

	before_options->signal_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));
	after_options->signal_changed().connect(sigc::mem_fun(*this,&Widget_WaypointModel::on_change));

	attach(checkbutton_before, 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*before_options, 1, 2, 0,1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(checkbutton_after, 2, 3, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*after_options, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	attach(checkbutton_tension, 0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*spin_tension, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(checkbutton_continuity, 2, 3, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*spin_continuity, 3, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(checkbutton_bias, 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*spin_bias, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(checkbutton_temporal_tension, 2, 3, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	attach(*spin_temporal_tension, 3, 4, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

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

	waypoint_model.set_tension(adj_tension.get_value());
	waypoint_model.set_continuity(adj_continuity.get_value());
	waypoint_model.set_bias(adj_bias.get_value());
	waypoint_model.set_temporal_tension(adj_temporal_tension.get_value());

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

