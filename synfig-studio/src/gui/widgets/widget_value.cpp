/* === S Y N F I G ========================================================= */
/*!	\file widget_value.cpp
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

#include <gtkmm/label.h>
#include "widgets/widget_value.h"
#include <ETL/stringf>
#include <gtkmm/celleditable.h>
#include <gtkmm/editable.h>
#include <gtkmm/entry.h>
#include <gtkmm/eventbox.h>
#include <gtk/gtkentry.h> /* see XXX below */
#include "app.h"


#include "widgets/widget_vector.h"
#include "widgets/widget_filename.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_coloredit.h"
#include "widgets/widget_bonechooser.h"
#include "widgets/widget_canvaschooser.h"
#include "widgets/widget_time.h"

#include "app.h"
#include "widgets/widget_distance.h"

#include "general.h"

#endif

using namespace synfig;
using namespace etl;
using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DIGITS		15

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_ValueBase::Widget_ValueBase():
	Glib::ObjectBase	(typeid(Widget_ValueBase)),
	Gtk::HBox(),
	real_adjustment(0,-2000000000,2000000000,0.05,0.05,0),
	integer_adjustment(0,-2000000000,2000000000,1,1,0),
	angle_adjustment(0,-2000000000,2000000000,1,1,0)
{
	set_no_show_all();

	label=manage(new class Gtk::Label("Unknown Datatype"));
	pack_start(*label);
	label->show();

	vector_widget=manage(new class Widget_Vector());
	pack_start(*vector_widget);

	color_widget=manage(new class Widget_ColorEdit());
	pack_start(*color_widget);

	enum_widget=manage(new class Widget_Enum());
	pack_start(*enum_widget);

	real_widget=manage(new class Gtk::SpinButton(real_adjustment,0.05,DIGITS));
	pack_start(*real_widget);

	integer_widget=manage(new class Gtk::SpinButton(integer_adjustment,1,0));
	pack_start(*integer_widget);

	angle_widget=manage(new class Gtk::SpinButton(angle_adjustment,15,2));
	pack_start(*angle_widget);

	bool_widget=manage(new class Gtk::CheckButton());
	pack_start(*bool_widget);

	//color_widget=manage(new class Gtk::ColorSelection());
	//pack_start(*color_widget);

	string_widget=manage(new class Gtk::Entry());
	pack_start(*string_widget);

	bone_widget=manage(new class Widget_BoneChooser());
	pack_start(*bone_widget);

	canvas_widget=manage(new class Widget_CanvasChooser());
	pack_start(*canvas_widget);

	filename_widget=manage(new class Widget_Filename());
	pack_start(*filename_widget);

	time_widget=manage(new class Widget_Time());
	pack_start(*time_widget);

	distance_widget=manage(new class Widget_Distance());
	pack_start(*distance_widget);


	vector_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	color_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	enum_widget->signal_changed().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	real_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	integer_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	angle_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	string_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	bone_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	canvas_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	filename_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	time_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	distance_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));

	/*signal_focus_in_event().connect(
		sigc::bind_return(
		sigc::hide(
			sigc::mem_fun(*this,&Widget_ValueBase::grab_focus)
		),false
		)
	);*/
}

Widget_ValueBase::~Widget_ValueBase()
{
}

void
Widget_ValueBase::activate()
{
	signal_activate()();
}

void
Widget_ValueBase::inside_cellrenderer()
{
	string_widget->set_has_frame(false);
	string_widget->gobj()->is_cell_renderer = true; // XXX

	real_widget->set_has_frame(false);
	//static_cast<Gtk::Entry*>(real_widget)->gobj()->is_cell_renderer = true; // XXX

	distance_widget->set_has_frame(false);
	//static_cast<Gtk::Entry*>(distance_widget)->gobj()->is_cell_renderer = true; // XXX

	integer_widget->set_has_frame(false);
	//static_cast<Gtk::Entry*>(integer_widget)->gobj()->is_cell_renderer = true; // XXX
	vector_widget->set_has_frame(false);
    //vector_widget->set_digits(10);

	color_widget->set_has_frame(false);
    //color_widget->set_digits(10);
	filename_widget->set_has_frame(false);
	time_widget->set_has_frame(false);
}

void
Widget_ValueBase::set_sensitive(bool x)
{
	Gtk::HBox::set_sensitive(x);
	label->set_sensitive(x);
	vector_widget->set_sensitive(x);
	real_widget->set_sensitive(x);
	integer_widget->set_sensitive(x);
	bool_widget->set_sensitive(x);
    color_widget->set_sensitive(x);
	string_widget->set_sensitive(x);
	bone_widget->set_sensitive(x);
	canvas_widget->set_sensitive(x);
	enum_widget->set_sensitive(x);
	angle_widget->set_sensitive(x);
	filename_widget->set_sensitive(x);
	time_widget->set_sensitive(x);
	distance_widget->set_sensitive(x);
}

void
Widget_ValueBase::set_value(const synfig::ValueBase &data)
{
	label->hide();
	vector_widget->hide();
	real_widget->hide();
	integer_widget->hide();
	bool_widget->hide();
    color_widget->hide();
	string_widget->hide();
	bone_widget->hide();
	canvas_widget->hide();
	enum_widget->hide();
	angle_widget->hide();
	filename_widget->hide();
	time_widget->hide();
	distance_widget->hide();

	value=data;
	try{
	switch(value.get_type())
	{
	case ValueBase::TYPE_VECTOR:
		vector_widget->set_canvas(canvas);
		vector_widget->set_value(value.get(Vector()));
		vector_widget->show();
		break;
	case ValueBase::TYPE_REAL:
		if(( child_param_desc.get_is_distance() || param_desc.get_is_distance() )&& canvas)
		{
			Distance dist(value.get(Real()),Distance::SYSTEM_UNITS);
			dist.convert(App::distance_system,canvas->rend_desc());
			distance_widget->set_value(dist);
			distance_widget->show();
		}
		else
		{
			real_widget->set_value(value.get(Real()));
			real_widget->show();
		}
		break;
	case ValueBase::TYPE_TIME:
		if(canvas)time_widget->set_fps(canvas->rend_desc().get_frame_rate());
		time_widget->set_value(value.get(Time()));
		time_widget->show();
		break;
	case ValueBase::TYPE_ANGLE:
		angle_widget->set_value(Angle::deg(value.get(Angle())).get());
		angle_widget->show();
		break;
	case ValueBase::TYPE_INTEGER:
		{
			String child_param_hint(child_param_desc.get_hint());
			String param_hint(param_desc.get_hint());
			if(child_param_hint!="enum" && param_hint!="enum")
			{
				integer_widget->set_value(value.get(int()));
				integer_widget->show();
			}
			else
			{
				if(child_param_hint=="enum")
					enum_widget->set_param_desc(child_param_desc);
				else
					enum_widget->set_param_desc(param_desc);
				enum_widget->set_value(value.get(int()));
				enum_widget->show();
			}
		}
		break;
	case ValueBase::TYPE_VALUENODE_BONE:
		assert(canvas);
		bone_widget->set_parent_canvas(canvas);
		bone_widget->set_value_desc(get_value_desc());
		bone_widget->set_value(value.get(etl::loose_handle<synfig::ValueNode_Bone>()));
		bone_widget->show();
		break;
	case ValueBase::TYPE_CANVAS:
		assert(canvas);
		canvas_widget->set_parent_canvas(canvas);
		canvas_widget->set_value(value.get(etl::loose_handle<synfig::Canvas>()));
		canvas_widget->show();
		break;
	case ValueBase::TYPE_BOOL:
		bool_widget->set_active(value.get(bool()));
		bool_widget->show();
		break;
	case ValueBase::TYPE_STRING:
		if(child_param_desc.get_hint()!="filename" && param_desc.get_hint()!="filename")
		{
			string_widget->set_text(value.get(string()));
			string_widget->show();
		}
		else
		{
			filename_widget->set_value(value.get(string()));
			filename_widget->show();
		}
		break;
	case ValueBase::TYPE_COLOR:
        {
		color_widget->set_value(value.get(synfig::Color()));
		color_widget->show();
/*
			Gdk::Color gdkcolor;
			synfig::Color color=value.get(synfig::Color());
			gdkcolor.set_rgb_p(color.get_r(),color.get_g(),color.get_b());
			color_widget->set_current_color(gdkcolor);
			color_widget->set_has_opacity_control(true);
			color_widget->set_current_alpha((unsigned short)(color.get_a()*65535.0));
			color_widget->show();
*/
		}
		break;
	default:
		label->show();
		break;
	}
	}catch(...) { synfig::error(__FILE__":%d: Caught something that was thrown",__LINE__); }
}

const synfig::ValueBase &
Widget_ValueBase::get_value()
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_VECTOR:
		value=vector_widget->get_value();
		break;
	case ValueBase::TYPE_REAL:
		if((child_param_desc.get_is_distance() || param_desc.get_is_distance()) && canvas)
			value=distance_widget->get_value().units(canvas->rend_desc());
		else
			value=real_widget->get_value();
		break;
	case ValueBase::TYPE_TIME:
		value=time_widget->get_value();
		break;
	case ValueBase::TYPE_ANGLE:
		value=Angle::deg(angle_widget->get_value());
		break;
	case ValueBase::TYPE_VALUENODE_BONE:
		value=bone_widget->get_value();
		break;
	case ValueBase::TYPE_CANVAS:
		value=canvas_widget->get_value();
		break;
	case ValueBase::TYPE_INTEGER:
		if(child_param_desc.get_hint()!="enum" && param_desc.get_hint()!="enum")
		{
			value=integer_widget->get_value_as_int();
		}
		else
		{
			value=enum_widget->get_value();
		}

		break;
	case ValueBase::TYPE_BOOL:
		value=bool_widget->get_active();
		break;
	case ValueBase::TYPE_STRING:
		if(param_desc.get_hint()!="filename")
		{
			value=string(string_widget->get_text());
		}
		else
		{
			value=string(filename_widget->get_value());
		}
		break;
	case ValueBase::TYPE_COLOR:
        {
			value=color_widget->get_value();
/*
			Gdk::Color gdkcolor;
			synfig::Color color;
			gdkcolor=color_widget->get_current_color();
			color.set_r(gdkcolor.get_red_p());
            color.set_g(gdkcolor.get_green_p());
            color.set_b(gdkcolor.get_blue_p());
			color.set_a(color_widget->get_current_alpha()/65535.0);

			value=color;
*/
		}
		break;
	default:
		break;
	}

	return value;
}


void
Widget_ValueBase::on_grab_focus()
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_VECTOR:
		vector_widget->grab_focus();
		break;
	case ValueBase::TYPE_REAL:
		if(param_desc.get_is_distance()&& canvas)
			distance_widget->grab_focus();
		else
			real_widget->grab_focus();
		break;
	case ValueBase::TYPE_TIME:
		time_widget->grab_focus();
		break;
	case ValueBase::TYPE_ANGLE:
		angle_widget->grab_focus();
		break;
	case ValueBase::TYPE_VALUENODE_BONE:
		bone_widget->grab_focus();
		break;
	case ValueBase::TYPE_CANVAS:
		canvas_widget->grab_focus();
		break;
	case ValueBase::TYPE_INTEGER:
		if(child_param_desc.get_hint()!="enum" && param_desc.get_hint()!="enum")
		{
			integer_widget->grab_focus();
		}
		else
		{
			enum_widget->grab_focus();
		}

		break;
	case ValueBase::TYPE_BOOL:
		bool_widget->grab_focus();
		break;
	case ValueBase::TYPE_STRING:
		if(param_desc.get_hint()!="filename")
		{
			string_widget->grab_focus();
		}
		else
		{
			filename_widget->grab_focus();
		}
		break;
	case ValueBase::TYPE_COLOR:
        {
			color_widget->grab_focus();
		}
		break;
	default:
		break;
	}
}

/*
Glib::SignalProxy0<void>
Widget_ValueBase::signal_activate()
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_VECTOR:
		return vector_widget->signal_activate();
		break;
	case ValueBase::TYPE_REAL:
		if(param_desc.get_is_distance()&& canvas)
			return distance_widget->signal_activate();
		else
			return real_widget->signal_activate();

		break;
	case ValueBase::TYPE_TIME:
		return time_widget->signal_activate();
		break;
	case ValueBase::TYPE_ANGLE:
		return angle_widget->signal_activate();
		break;
	case ValueBase::TYPE_VALUENODE_BONE:
		return bone_widget->signal_activate();
		break;
	case ValueBase::TYPE_CANVAS:
		return canvas_widget->signal_activate();
		break;
	case ValueBase::TYPE_INTEGER:
		if(param_desc.get_hint()!="enum")
			return integer_widget->signal_activate();
		else
			return enum_widget->signal_activate();

		break;
	case ValueBase::TYPE_BOOL:
		return string_widget->signal_activate();
		break;
	case ValueBase::TYPE_STRING:
		if(param_desc.get_hint()!="filename")
		{
			return string_widget->signal_activate();
		}
		else
		{
			return filename_widget->signal_activate();
		}
		break;
	case ValueBase::TYPE_COLOR:
        {
			return color_widget->signal_activate();
		}
		break;
	default:
		return string_widget->signal_activate();
		break;
	}
}
*/
