/* === S Y N F I G ========================================================= */
/*!	\file dialog_waypoint.cpp
**	\brief Template Header
**
**	$Id: dialog_waypoint.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

Dialog_Waypoint::Dialog_Waypoint(Gtk::Window& parent,etl::handle<synfig::Canvas> canvas):
	Dialog(_("Waypoint Editor"),parent,false,true),
	canvas(canvas)
{
	assert(canvas);
    waypointwidget=manage(new class Widget_Waypoint(canvas));
	get_vbox()->pack_start(*waypointwidget);

	Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Waypoint::on_ok_pressed));

	Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
	apply_button->show();
	add_action_widget(*apply_button,1);
	apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Waypoint::on_apply_pressed));

	Gtk::Button *delete_button(manage(new class Gtk::Button(Gtk::StockID("gtk-delete"))));
	delete_button->show();
	add_action_widget(*delete_button,3);
	delete_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Waypoint::on_delete_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Waypoint::hide));

	
	waypointwidget->show_all();
}

Dialog_Waypoint::~Dialog_Waypoint()
{
}

void
Dialog_Waypoint::on_ok_pressed()
{
	hide();
    signal_changed_();
}

void
Dialog_Waypoint::on_apply_pressed()
{
    signal_changed_();	
}

void
Dialog_Waypoint::on_delete_pressed()
{
	hide();
    signal_delete_();	
}

void
Dialog_Waypoint::set_waypoint(synfig::ValueNode_Animated::Waypoint x)
{
	waypointwidget->set_waypoint(x);
}

const synfig::ValueNode_Animated::Waypoint &
Dialog_Waypoint::get_waypoint()const
{
	return waypointwidget->get_waypoint();
}

void
Dialog_Waypoint::set_value_desc(synfigapp::ValueDesc value_desc)
{
	value_desc_=value_desc;
	if(value_desc.get_value_node() && value_desc.get_value_node()->get_parent_canvas())
		waypointwidget->set_canvas(value_desc.get_value_node()->get_parent_canvas());
}

void
Dialog_Waypoint::reset()
{
}
