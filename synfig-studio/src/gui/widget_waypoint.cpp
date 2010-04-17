/* === S Y N F I G ========================================================= */
/*!	\file widget_waypoint.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include "dialogs/dialog_waypoint.h"
#include <gtk/gtk.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/combo.h>
#include <ETL/stringf>
#include "widget_value.h"
#include "app.h"
#include <gtkmm/menu.h>
#include <gtkmm/optionmenu.h>
#include "widget_time.h"
#include "widget_waypoint.h"
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

Widget_Waypoint::Widget_Waypoint(etl::handle<synfig::Canvas> canvas):
	Gtk::Alignment(0, 0, 1, 1),
	waypoint(synfig::ValueBase(),0),
	adj_tension(0.0,-20,20,0.1,1),
	adj_continuity(0.0,-20,20,0.1,1),
	adj_bias(0.0,-20,20,0.1,1),
	adj_temporal_tension(0.0,-20,20,0.1,1)
{
	value_widget=manage(new Widget_ValueBase());
	value_widget->set_canvas(canvas);
	value_widget->show();

	value_node_label=manage(new Gtk::Label(_("(Non-static value)")));


	time_widget=manage(new Widget_Time());
	time_widget->set_fps(canvas->rend_desc().get_frame_rate());
	//spinbutton=manage(new Gtk::SpinButton(time_adjustment,0.05,3));
	//spinbutton->set_update_policy(Gtk::UPDATE_ALWAYS);
	//spinbutton->show();

	before_options=manage(new class Gtk::Menu());
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("TCB Smooth")));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Constant")));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Linear")));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Ease In")));
	// before_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Manual")));

	after_options=manage(new class Gtk::Menu());
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("TCB Smooth")));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Constant")));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Linear")));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Ease Out")));
	// after_options->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Manual")));

	before=manage(new class Gtk::OptionMenu());
	before->show();
	before->set_menu(*before_options);

	after=manage(new class Gtk::OptionMenu());
	after->show();
	after->set_menu(*after_options);

	spin_tension=manage(new class Gtk::SpinButton(adj_tension,0.1,3));
	spin_tension->show();
	spin_continuity=manage(new class Gtk::SpinButton(adj_continuity,0.1,3));
	spin_continuity->show();
	spin_bias=manage(new class Gtk::SpinButton(adj_bias,0.1,3));
	spin_bias->show();
	spin_temporal_tension=manage(new class Gtk::SpinButton(adj_temporal_tension,0.1,3));
	spin_temporal_tension->show();

	set_padding(12, 12, 12, 12);

	Gtk::VBox *widgetBox = manage(new Gtk::VBox(false, 12));
	add(*widgetBox);

	Gtk::Frame *waypointFrame = manage(new Gtk::Frame(_("Waypoint")));
	waypointFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) waypointFrame->get_label_widget())->set_markup(_("<b>Waypoint</b>"));
	widgetBox->pack_start(*waypointFrame, false, false, 0);

	Gtk::Alignment *waypointPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	waypointPadding->set_padding(6, 0, 24, 0);
	waypointFrame->add(*waypointPadding);

	Gtk::Table *waypointTable = manage(new Gtk::Table(2, 2, false));
	waypointTable->set_row_spacings(6);
	waypointTable->set_col_spacings(12);
	waypointPadding->add(*waypointTable);

	Gtk::Label *waypointValueLabel = manage(new Gtk::Label(_("_Value"), true));
	waypointValueLabel->set_alignment(0, 0.5);
	waypointValueLabel->set_mnemonic_widget(*value_widget);
	waypointTable->attach(*waypointValueLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	waypointTable->attach(*value_widget, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	waypointTable->attach(*value_node_label, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Label *waypointTimeLabel = manage(new Gtk::Label(_("_Time"), true));
	waypointTimeLabel->set_alignment(0, 0.5);
	waypointTimeLabel->set_mnemonic_widget(*time_widget);
	waypointTable->attach(*waypointTimeLabel, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	waypointTable->attach(*time_widget, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Frame *interpolationFrame = manage(new Gtk::Frame(_("Interpolation")));
	interpolationFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) interpolationFrame->get_label_widget())->set_markup(_("<b>Interpolation</b>"));
	widgetBox->pack_start(*interpolationFrame, false, false, 0);

	Gtk::Alignment *interpolationPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	interpolationPadding->set_padding(6, 0, 24, 0);
	interpolationFrame->add(*interpolationPadding);

	Gtk::Table *interpolationTable = manage(new Gtk::Table(2, 2, false));
	interpolationTable->set_row_spacings(6);
	interpolationTable->set_col_spacings(12);
	interpolationPadding->add(*interpolationTable);

	Gtk::Label *interpolationInLabel = manage(new Gtk::Label(_("_In Interpolation"), true));
	interpolationInLabel->set_alignment(0, 0.5);
	interpolationInLabel->set_mnemonic_widget(*before);
	interpolationTable->attach(*interpolationInLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	interpolationTable->attach(*before, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Label *interpolationOutLabel = manage(new Gtk::Label(_("_Out Interpolation"), true));
	interpolationOutLabel->set_alignment(0, 0.5);
	interpolationOutLabel->set_mnemonic_widget(*after);
	interpolationTable->attach(*interpolationOutLabel, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	interpolationTable->attach(*after, 1, 2, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Frame *tcbFrame = manage(new Gtk::Frame(_("TCB Parameters")));
	tcbFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) tcbFrame->get_label_widget())->set_markup(_("<b>TCB Parameter</b>"));
	widgetBox->pack_start(*tcbFrame, false, false, 0);

	Gtk::Alignment *tcbPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	tcbPadding->set_padding(6, 0, 24, 0);
	tcbFrame->add(*tcbPadding);

	Gtk::Table *tcbTable = manage(new Gtk::Table(4, 2, false));
	tcbTable->set_row_spacings(6);
	tcbTable->set_col_spacings(12);
	tcbPadding->add(*tcbTable);

	Gtk::Label *tensionLabel = manage(new Gtk::Label(_("T_ension"), true));
	tensionLabel->set_alignment(0, 0.5);
	tensionLabel->set_mnemonic_widget(*spin_tension);
	spin_tension->set_alignment(1);
	tcbTable->attach(*tensionLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	tcbTable->attach(*spin_tension, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Label *continuityLabel = manage(new Gtk::Label(_("_Continuity"), true));
	continuityLabel->set_alignment(0, 0.5);
	continuityLabel->set_mnemonic_widget(*spin_continuity);
	spin_continuity->set_alignment(1);
	tcbTable->attach(*continuityLabel, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	tcbTable->attach(*spin_continuity, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Label *biasLabel = manage(new Gtk::Label(_("_Bias"), true));
	biasLabel->set_alignment(0, 0.5);
	biasLabel->set_mnemonic_widget(*spin_bias);
	spin_bias->set_alignment(1);
	tcbTable->attach(*biasLabel, 0, 1, 2, 3, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	tcbTable->attach(*spin_bias, 1, 2, 2, 3, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Label *temporalTensionLabel = manage(new Gtk::Label(_("Te_mporal Tension"), true));
	temporalTensionLabel->set_alignment(0, 0.5);
	temporalTensionLabel->set_mnemonic_widget(*spin_temporal_tension);
	spin_temporal_tension->set_alignment(1);
	tcbTable->attach(*temporalTensionLabel, 0, 1, 3, 4, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	tcbTable->attach(*spin_temporal_tension, 1, 2, 3, 4, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	show_all();
	hide();
	//attach(*hbox, 1, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	set_canvas(canvas);
}

void
Widget_Waypoint::set_canvas(synfig::Canvas::Handle x)
{
	canvas=x;
	assert(canvas);

	time_widget->set_fps(canvas->rend_desc().get_frame_rate());
	value_widget->set_canvas(canvas);
}

void
Widget_Waypoint::set_waypoint(synfig::Waypoint &x)
{
	time_widget->set_fps(canvas->rend_desc().get_frame_rate());

	waypoint=x;

	//! \todo This really needs to be fixed to support value node waypoints!
	if(waypoint.is_static())
	{
		value_widget->set_value(waypoint.get_value());
		value_widget->show();
		value_node_label->hide();
	}
	else
	{
		value_widget->hide();
		value_node_label->show();
	}

	time_widget->set_value(waypoint.get_time());

	before->set_history((int)waypoint.get_before());
	after->set_history((int)waypoint.get_after());

	adj_tension.set_value(waypoint.get_tension());
	adj_continuity.set_value(waypoint.get_continuity());
	adj_bias.set_value(waypoint.get_bias());
	adj_temporal_tension.set_value(waypoint.get_temporal_tension());

}
const synfig::Waypoint &
Widget_Waypoint::get_waypoint()const
{
	//! \todo This too!
	waypoint.set_time(time_widget->get_value());
	if(waypoint.is_static())
		waypoint.set_value(value_widget->get_value());
	//int i;

	waypoint.set_before((synfig::Waypoint::Interpolation)before->get_history());
	waypoint.set_after((synfig::Waypoint::Interpolation)after->get_history());

	waypoint.set_tension(adj_tension.get_value());
	waypoint.set_continuity(adj_continuity.get_value());
	waypoint.set_bias(adj_bias.get_value());
	waypoint.set_temporal_tension(adj_temporal_tension.get_value());
	return waypoint;
}
