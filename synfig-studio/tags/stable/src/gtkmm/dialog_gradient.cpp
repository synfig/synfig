/* === S I N F G =========================================================== */
/*!	\file dialog_gradient.cpp
**	\brief Template File
**
**	$Id: dialog_gradient.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "dialog_gradient.h"
#include "widget_gradient.h"
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <sinfg/general.h>
#include <sinfgapp/canvasinterface.h>
#include <sinfgapp/value_desc.h>
#include <sinfgapp/main.h>
#include "widget_color.h"
#include <gtkmm/spinbutton.h>
#include "app.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Gradient::Dialog_Gradient():
	Dialog(_("Gradient Editor"),false,true),
	dialog_settings(this,"gradient"),
	adjustment_pos(0,0.0,1.0,0.001,0.001,0.001)
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	// Setup the buttons
	Gtk::Button *grab_button(manage(new class Gtk::Button(Gtk::StockID("Grab"))));
	grab_button->show();
	add_action_widget(*grab_button,2);
	grab_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Gradient::on_grab_pressed));

	/*
	Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
	apply_button->show();
	add_action_widget(*apply_button,1);
	apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Gradient::on_apply_pressed));
	*/
	
	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Gradient::hide));	

	Gtk::Table* table(manage(new Gtk::Table(2,2,false)));
	get_vbox()->pack_start(*table);

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
	adjustment_pos.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_values_adjusted));
	adjustment_pos.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Gradient::on_changed));
	table->attach(*spinbutton_pos, 0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	

	
	add_accel_group(App::ui_manager()->get_accel_group());

	show_all_children();
}

Dialog_Gradient::~Dialog_Gradient()
{
}

void
Dialog_Gradient::set_gradient(const sinfg::Gradient& x)
{
	widget_gradient->set_value(x);
}

void
Dialog_Gradient::reset()
{
	value_changed_connection.disconnect();
	signal_edited_.clear();
}

void
Dialog_Gradient::on_grab_pressed()
{
	sinfgapp::Main::set_gradient(get_gradient());
//	signal_edited_(get_gradient());
//	reset();
//	hide();
}

void
Dialog_Gradient::on_apply_pressed()
{
	signal_edited_(get_gradient());
}

void
Dialog_Gradient::on_changed()
{
	signal_edited_(get_gradient());
}

void
Dialog_Gradient::on_cpoint_selected(sinfg::Gradient::CPoint x)
{
	widget_color->set_value(x.color);
	adjustment_pos.set_value(x.pos);
}

void
Dialog_Gradient::on_values_adjusted()
{
	sinfg::Gradient::CPoint x(widget_gradient->get_selected_cpoint());
	x.color=widget_color->get_value();
	x.pos=adjustment_pos.get_value();
	widget_gradient->update_cpoint(x);
}

static void
dialog_gradient_value_desc_edit(sinfg::Gradient g,sinfgapp::ValueDesc x,handle<sinfgapp::CanvasInterface> canvas_interface)
{
//	canvas_interface->connect_value(x,ValueBase(g));
}

void
Dialog_Gradient::edit(const sinfgapp::ValueDesc &x, etl::handle<sinfgapp::CanvasInterface> canvas_interface, sinfg::Time time)
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
