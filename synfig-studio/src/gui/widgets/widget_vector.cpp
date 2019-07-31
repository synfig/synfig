/* === S Y N F I G ========================================================= */
/*!	\file widget_vector.cpp
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

#include <synfig/general.h>
#include <synfig/distance.h>

#include <gtkmm/spinbutton.h>
#include "widgets/widget_vector.h"
#include "widgets/widget_distance.h"
#include "app.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DIGITS		10

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static void init();

Widget_Vector::Widget_Vector():
	Glib::ObjectBase("widget_vector")
{
	init();
}

Widget_Vector::Widget_Vector(BaseObjectType* cobject) :
	Glib::ObjectBase("widget_vector"),
	Gtk::Box(cobject)
{
	init();
}

void Widget_Vector::init() {
	set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	set_homogeneous(false);
	set_spacing(5);

	x_adjustment = Gtk::Adjustment::create(0,-100000000,100000000,0.05,0.05,0);
	y_adjustment = Gtk::Adjustment::create(0,-100000000,100000000,0.05,0.05,0);

	int width_chars = 5;

	entry_x=manage(new class Gtk::Entry());
	entry_x->set_width_chars(width_chars);
	entry_x->signal_changed().connect(sigc::mem_fun(*this,&studio::Widget_Vector::on_entry_x_changed));
	pack_start(*entry_x, Gtk::PACK_EXPAND_WIDGET);
	entry_x->show();

	spinbutton_x=manage(new class Gtk::SpinButton(x_adjustment,0.05,DIGITS));
	spinbutton_x->set_alignment(1);
	spinbutton_x->set_update_policy(Gtk::UPDATE_ALWAYS);
	spinbutton_x->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Vector::on_value_changed));
	pack_start(*spinbutton_x, Gtk::PACK_EXPAND_WIDGET);

	distance_x=manage(new Widget_Distance());
	distance_x->set_digits(4);
	distance_x->set_update_policy(Gtk::UPDATE_ALWAYS);
	distance_x->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Vector::on_value_changed));
	pack_start(*distance_x, Gtk::PACK_EXPAND_WIDGET);

	entry_y=manage(new class Gtk::Entry());
	entry_y->set_width_chars(width_chars);
	entry_y->signal_changed().connect(sigc::mem_fun(*this,&studio::Widget_Vector::on_entry_y_changed));
	pack_start(*entry_y, Gtk::PACK_EXPAND_WIDGET);
	entry_y->show();

	spinbutton_y=manage(new class Gtk::SpinButton(y_adjustment,0.05,DIGITS));
	spinbutton_y->set_alignment(1);
	spinbutton_y->set_update_policy(Gtk::UPDATE_ALWAYS);
	spinbutton_y->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Vector::on_value_changed));
	spinbutton_y->signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_Vector::activate));
	pack_start(*spinbutton_y, Gtk::PACK_EXPAND_WIDGET);

	distance_y=manage(new Widget_Distance());
	distance_y->set_digits(4);
	distance_y->set_update_policy(Gtk::UPDATE_ALWAYS);
	distance_y->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Vector::on_value_changed));
	//distance_y->signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_Vector::activate));
	pack_start(*distance_y, Gtk::PACK_EXPAND_WIDGET);

	//spinbutton_x->show();
	//spinbutton_y->show();

	//spinbutton_x->signal_activate().connect(sigc::mem_fun(*spinbutton_y,&Gtk::SpinButton::grab_focus));
	//distance_x->signal_activate().connect(sigc::mem_fun(*distance_y,&Gtk::SpinButton::grab_focus));
	entry_x->signal_activate().connect(sigc::mem_fun(*entry_y,&Gtk::SpinButton::grab_focus));
	entry_y->signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_Vector::activate));
}

Widget_Vector::~Widget_Vector()
{
}

void
Widget_Vector::on_grab_focus()
{
	//if(canvas_)
	//	distance_x->grab_focus();
	//else
	//	spinbutton_x->grab_focus();
	entry_x->grab_focus();
}

void
Widget_Vector::set_has_frame(bool x)
{
	if(spinbutton_x)
	{
		spinbutton_x->set_has_frame(x);
		spinbutton_y->set_has_frame(x);
		spinbutton_x->set_size_request(48,-1);
		spinbutton_y->set_size_request(48,-1);
	}

	distance_x->set_has_frame(x);
	distance_y->set_has_frame(x);
	distance_x->set_size_request(48,-1);
	distance_y->set_size_request(48,-1);
	
	entry_x->set_has_frame(x);
	entry_y->set_has_frame(x);
}

void
Widget_Vector::set_digits(int x)
{
	if(spinbutton_x)
	{
		spinbutton_x->set_digits(x);
		spinbutton_y->set_digits(x);
		spinbutton_x->set_size_request(48,-1);
		spinbutton_y->set_size_request(48,-1);
	}

	distance_x->set_digits(x);
	distance_y->set_digits(x);
	distance_x->set_size_request(48,-1);
	distance_y->set_size_request(48,-1);
}

void
Widget_Vector::set_value(const synfig::Vector &data)
{
	vector=data;

	if(canvas_){try
	{
		Distance distx(vector[0],Distance::SYSTEM_UNITS),disty(vector[1],Distance::SYSTEM_UNITS);
		distx.convert(App::distance_system,canvas_->rend_desc());
		disty.convert(App::distance_system,canvas_->rend_desc());
		distance_x->set_value(distx);
		distance_y->set_value(disty);
		entry_x->set_text(distance_x->get_value().get_string(4));
		entry_y->set_text(distance_y->get_value().get_string(4));
		//spinbutton_x->hide();
		//spinbutton_y->hide();
	}catch(...) { synfig::error("Widget_Vector::set_value(): Caught something that was thrown"); }}
	else
	{
		spinbutton_x->set_value(vector[0]);
		spinbutton_y->set_value(vector[1]);
		
		String str;
		std::ostringstream sstream_x;
		sstream_x << spinbutton_x->get_value();
		str=sstream_x.str();
		while (*str.rbegin() == '0' && str.length() > 1)
			str=str.substr(0, str.size()-1);
		entry_x->set_text(str);
		std::ostringstream sstream_y;
		sstream_y << spinbutton_y->get_value();
		str=sstream_y.str();
		while (*str.rbegin() == '0' && str.length() > 1)
			str=str.substr(0, str.size()-1);
		entry_y->set_text(str);
		
		//distance_x->hide();
		//distance_y->hide();
	}
}

const synfig::Vector &
Widget_Vector::get_value()
{
	if(!canvas_ && spinbutton_x)
	{
		vector[0]=spinbutton_x->get_value();
		vector[1]=spinbutton_y->get_value();
		//distance_x->hide();
		//distance_y->hide();
	}
	else try
	{
		vector[0]=distance_x->get_value().units(canvas_->rend_desc());
		vector[1]=distance_y->get_value().units(canvas_->rend_desc());
		//spinbutton_x->hide();
		//spinbutton_y->hide();
	}catch(...) { synfig::error("Widget_Vector::set_value(): Caught something that was thrown"); }
	return vector;
}

void
Widget_Vector::on_entry_x_changed()
{
	if(canvas_) {
		Distance distx(0,Distance::SYSTEM_UNITS);
		distx.convert(App::distance_system,canvas_->rend_desc());
		distx = synfig::String(entry_x->get_text());
		distance_x->set_value(distx);
	}
	else try 
	{
		spinbutton_x->set_value(atof(entry_x->get_text().c_str()));
	}catch(...) { synfig::error("Widget_Vector::set_value(): Caught something that was thrown"); }
}

void
Widget_Vector::on_entry_y_changed()
{
	if(canvas_) {
		Distance disty(0,Distance::SYSTEM_UNITS);
		disty.convert(App::distance_system,canvas_->rend_desc());
		disty = synfig::String(entry_y->get_text());
		distance_y->set_value(disty);
	}
	else try 
	{
		spinbutton_y->set_value(atof(entry_y->get_text().c_str()));
	}catch(...) { synfig::error("Widget_Vector::set_value(): Caught something that was thrown"); }
}

void
Widget_Vector::on_value_changed()
{	
	signal_value_changed()();
}

void
Widget_Vector::set_canvas(synfig::Canvas::LooseHandle x)
{
	canvas_=x;
	/*
	if(x)
	{
		if(spinbutton_x)
		{
			spinbutton_x->hide();
			spinbutton_y->hide();
		}
		distance_x->show();
		distance_y->show();
	}
	else
	{
		if(spinbutton_x)
		{
			spinbutton_x->show();
			spinbutton_y->show();
		}
		distance_x->hide();
		distance_y->hide();
	}
	*/
}

void
Widget_Vector::show_all_vfunc()
{
	/*
	if(canvas_)
	{
		distance_x->show();
		distance_y->show();
	}
	else
	{
		spinbutton_x->show();
		spinbutton_y->show();
	}
	*/
	entry_x->show();
	entry_y->show();
	show();
}

GType Widget_Vector::gtype = 0;

Glib::ObjectBase *
Widget_Vector::wrap_new (GObject *o)
{
	if (gtk_widget_is_toplevel (GTK_WIDGET (o)))
		return new Widget_Vector (GTK_BOX(o));
	else
		return Gtk::manage(new Widget_Vector(GTK_BOX(o)));
}

void
Widget_Vector::register_type ()
{
	if (gtype)
		return;

	Widget_Vector dummy;

	gtype = G_OBJECT_TYPE (dummy.gobj());

	Glib::wrap_register (gtype, Widget_Vector::wrap_new);
}
