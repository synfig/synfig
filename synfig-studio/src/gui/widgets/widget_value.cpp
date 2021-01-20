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

#include <gui/widgets/widget_value.h>

#include <gui/app.h>
#include <gui/widgets/widget_bonechooser.h>
#include <gui/widgets/widget_canvaschooser.h>
#include <gui/widgets/widget_coloredit.h>
#include <gui/widgets/widget_distance.h>
#include <gui/widgets/widget_enum.h>
#include <gui/widgets/widget_filename.h>
#include <gui/widgets/widget_fontfamily.h>
#include <gui/widgets/widget_sublayer.h>
#include <gui/widgets/widget_time.h>
#include <gui/widgets/widget_vector.h>

#include <synfig/general.h>

#endif

using namespace std;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DIGITS		15

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_ValueBase::Widget_ValueBase():
	Glib::ObjectBase	(typeid(Widget_ValueBase)),
	Gtk::HBox(),
	real_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,0.05,0.05,0)),
	integer_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	angle_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0))
{
	set_hexpand();

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

	sublayer_widget=manage(new class Widget_Sublayer());
	pack_start(*sublayer_widget);

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

	fontfamily_widget=manage(new class Widget_FontFamily());
	pack_start(*fontfamily_widget);

	vector_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	color_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	enum_widget->signal_changed().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	sublayer_widget->signal_changed().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	real_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	integer_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	angle_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	string_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	bone_widget->signal_changed().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	canvas_widget->signal_changed().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	filename_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	time_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	distance_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));
	fontfamily_widget->signal_activate().connect(sigc::mem_fun(*this,&Widget_ValueBase::activate));

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
	// TODO: what is it?
	//string_widget->gobj()->is_cell_renderer = true; // XXX

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
	sublayer_widget->set_sensitive(x);
	angle_widget->set_sensitive(x);
	filename_widget->set_sensitive(x);
	time_widget->set_sensitive(x);
	distance_widget->set_sensitive(x);
	fontfamily_widget->set_sensitive(x);
}

void Widget_ValueBase::popup_combobox()
{
	Gtk::ComboBox * combobox = nullptr;
	Type &type(get_value().get_type());
	if (type == type_integer) {
		string param_hint = get_param_desc().get_hint();
		string child_param_hint = get_child_param_desc().get_hint();
		if ( param_hint == "enum" || child_param_hint == "enum" )
			combobox = enum_widget;
	} else if (type == type_canvas) {
		combobox = canvas_widget;
	} else if (type == type_bone_valuenode) {
		combobox = bone_widget;
	} else if (type == type_string) {
		string param_hint = get_param_desc().get_hint();
		string child_param_hint = get_child_param_desc().get_hint();
		if( param_hint == "sublayer_name" || child_param_hint == "sublayer_name")
			combobox = sublayer_widget;
		else if( param_hint == "font_family" || child_param_hint == "font_family")
			combobox = fontfamily_widget;
	}

	if (combobox)
		combobox->popup();
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
	sublayer_widget->hide();
	angle_widget->hide();
	filename_widget->hide();
	time_widget->hide();
	distance_widget->hide();
	fontfamily_widget->hide();

	value=data;
	try{
		Type &type(value.get_type());
		if (type == type_vector)
		{
			if (child_param_desc.get_is_distance() || param_desc.get_is_distance())
				vector_widget->set_canvas(canvas);
			vector_widget->set_value(value.get(Vector()));
			vector_widget->show();
		}
		else
		if (type == type_real)
		{
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
		}
		else
		if (type == type_time)
		{
			if(canvas)time_widget->set_fps(canvas->rend_desc().get_frame_rate());
			time_widget->set_value(value.get(Time()));
			time_widget->show();
		}
		else
		if (type == type_angle)
		{
			angle_widget->set_value(Angle::deg(value.get(Angle())).get());
			angle_widget->show();
		}
		else
		if (type == type_integer)
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
		else
		if (type == type_bone_valuenode)
		{
			assert(canvas);
			bone_widget->set_parent_canvas(canvas);
			bone_widget->set_value_desc(get_value_desc());
			bone_widget->set_value(value.get(etl::loose_handle<synfig::ValueNode_Bone>()));
			bone_widget->show();
		}
		else
		if (type == type_canvas)
		{
			assert(canvas);
			canvas_widget->set_parent_canvas(canvas);
			canvas_widget->set_value(value.get(etl::loose_handle<synfig::Canvas>()));
			canvas_widget->show();
		}
		else
		if (type == type_bool)
		{
			bool_widget->set_active(value.get(bool()));
			bool_widget->show();
		}
		else
		if (type == type_string)
		{
			if(child_param_desc.get_hint()=="filename" || param_desc.get_hint()=="filename")
			{
				filename_widget->set_value(value.get(string()));
				filename_widget->show();
			}
			else if(child_param_desc.get_hint()=="sublayer_name" || param_desc.get_hint()=="sublayer_name")
			{
				sublayer_widget->set_value_desc(value_desc);
				sublayer_widget->set_value(value.get(string()));
				sublayer_widget->show();
			}
			else if(child_param_desc.get_hint()=="font_family" || param_desc.get_hint()=="font_family")
			{
				fontfamily_widget->set_value(value.get(string()));
				fontfamily_widget->show();
			}
			else
			{
				string_widget->set_text(value.get(string()));
				string_widget->show();
			}
		}
		else
		if (type == type_color)
		{
			color_widget->set_value(value.get(synfig::Color()));
			color_widget->show();
		}
		else
		{
			label->show();
		}
	}catch(...) { synfig::error(__FILE__":%d: Caught something that was thrown",__LINE__); }
}

void
Widget_ValueBase::set_canvas(etl::handle<synfig::Canvas> x)
{
	assert(x);
	canvas=x;
	filename_widget->set_canvas(canvas);
}

const synfig::ValueBase &
Widget_ValueBase::get_value()
{
	Type &type(value.get_type());
	if (type == type_vector)
		value=vector_widget->get_value();
	else
	if (type == type_real)
	{
		if((child_param_desc.get_is_distance() || param_desc.get_is_distance()) && canvas)
			value=distance_widget->get_value().units(canvas->rend_desc());
		else
			value=real_widget->get_value();
	}
	else
	if (type == type_time)
		value=time_widget->get_value();
	else
	if (type == type_angle)
		value=Angle::deg(angle_widget->get_value());
	else
	if (type == type_bone_valuenode)
		value=bone_widget->get_value();
	else
	if (type == type_canvas)
		value=canvas_widget->get_value();
	else
	if (type == type_integer)
	{
		if(child_param_desc.get_hint()!="enum" && param_desc.get_hint()!="enum")
		{
			value=integer_widget->get_value_as_int();
		}
		else
		{
			value=enum_widget->get_value();
		}
	}
	else
	if (type == type_bool)
		value=bool_widget->get_active();
	else
	if (type == type_string)
	{
		if(child_param_desc.get_hint()=="filename" || param_desc.get_hint()=="filename")
		{
			value=string(filename_widget->get_value());
		}
		else if(child_param_desc.get_hint()=="sublayer_name" || param_desc.get_hint()=="sublayer_name") {
			value=string(sublayer_widget->get_value());
		}
		else if(child_param_desc.get_hint()=="font_family" || param_desc.get_hint()=="font_family") {
			value=string(fontfamily_widget->get_value());
		}
		else
		{
			value=string(string_widget->get_text());
		}
	}
	else
	if (type == type_color)
	{
		value=color_widget->get_value();
	}

	return value;
}


void
Widget_ValueBase::on_grab_focus()
{
	Type &type(value.get_type());
	if (type == type_vector)
		vector_widget->grab_focus();
	else
	if (type == type_real)
	{
		if((param_desc.get_is_distance() || child_param_desc.get_is_distance()) && canvas)
			distance_widget->grab_focus();
		else
			real_widget->grab_focus();
	}
	else
	if (type == type_time)
		time_widget->grab_focus();
	else
	if (type == type_angle)
		angle_widget->grab_focus();
	else
	if (type == type_bone_valuenode)
	{
		bone_widget->grab_focus();
		popup_combobox();
	}
	else
	if (type == type_canvas)
	{
		canvas_widget->grab_focus();
		popup_combobox();
	}
	else
	if (type == type_integer)
	{
		if(child_param_desc.get_hint()!="enum" && param_desc.get_hint()!="enum")
		{
			integer_widget->grab_focus();
		}
		else
		{
			enum_widget->grab_focus();
			popup_combobox();
		}
	}
	else
	if (type == type_bool)
		bool_widget->grab_focus();
	else
	if (type == type_string)
	{
		if(param_desc.get_hint()=="filename")
		{
			filename_widget->grab_focus();
		}
		else if(param_desc.get_hint()=="sublayer_name") {
			sublayer_widget->grab_focus();
			popup_combobox();
		}
		else if(param_desc.get_hint()=="font_family") {
			// don't pop it up, since it has an entry widget too

			// fontfamily_widget->grab_focus();
			// popup_combobox();
		}
		else
		{
			string_widget->grab_focus();
		}
	}
	else
	if (type == type_color)
		color_widget->grab_focus();
}
