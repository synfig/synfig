/* === S Y N F I G ========================================================= */
/*!	\file dialog_waypoint.cpp
**	\brief Template Header
**
**	$Id: widget_waypoint.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
#include "dialog_waypoint.h"
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
	Gtk::Table(4,3,false),
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
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem("TCB Smooth"));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Constant"));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Linear"));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Ease In"));
	before_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Manual"));

	after_options=manage(new class Gtk::Menu());
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem("TCB Smooth"));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Constant"));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Linear"));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Ease Out"));
	after_options->items().push_back(Gtk::Menu_Helpers::MenuElem("Manual"));

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
	
	
	Gtk::HBox *hbox(manage(new Gtk::HBox()));
	hbox->show();
	hbox->pack_start(*value_widget);
	hbox->pack_start(*value_node_label);
	
	attach(*manage(new Gtk::Label(_("ValueBase:"))), 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	//attach(*value_widget, 1, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//attach(*value_node_label, 0, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*manage(new Gtk::Label(_("Time:"))), 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*time_widget, 1, 4, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*manage(new Gtk::Label(_("In:"))), 0, 1, 3, 4, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*before, 1, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*manage(new Gtk::Label(_("Out:"))), 2, 3, 3, 4, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*after, 3, 4, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	attach(*manage(new Gtk::Label(_("Tension:"))), 0, 1, 4, 5, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*spin_tension, 1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*manage(new Gtk::Label(_("Continuity:"))), 2, 3, 4, 5, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*spin_continuity, 3, 4, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*manage(new Gtk::Label(_("Bias:"))), 0, 1, 5, 6, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*spin_bias, 1, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*manage(new Gtk::Label(_("Temporal Tension:"))), 2, 3, 5, 6, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	attach(*spin_temporal_tension, 3, 4, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	show_all();
	hide();
	attach(*hbox, 1, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
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
			
#warning This really needs to be fixed to support value node waypoints!
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
#warning This too!
	waypoint.set_time(time_widget->get_value());
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
