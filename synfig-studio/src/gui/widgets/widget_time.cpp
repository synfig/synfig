/* === S Y N F I G ========================================================= */
/*!	\file widget_time.cpp
**	\brief Template File
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

#include "widgets/widget_time.h"
#include "app.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Time::Widget_Time() :
	Glib::ObjectBase("widget_time")
{
	init();
}

Widget_Time::Widget_Time(BaseObjectType* cobject) :
	Glib::ObjectBase("widget_time"),
	Gtk::Entry(cobject)
{
	init();
}

void
Widget_Time::init()
{
	fps_ = 0;
	time_ = 0;

	signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_Time::refresh_value));
	signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_Time::refresh_text));

	add_events(Gdk::SCROLL_MASK);
}

Widget_Time::~Widget_Time()
{
}

void
Widget_Time::refresh_text()
{
	set_text(time_.get_string(fps_,App::get_time_format()));
}


void
Widget_Time::set_value(const synfig::Time &data)
{
	time_=data;
	refresh_text();
}

synfig::Time
Widget_Time::get_value() const
{
	return time_;
}

void
Widget_Time::set_fps(float x)
{
	fps_=Time(x);
	refresh_text();
}

void
Widget_Time::refresh_value()
{
	try
	{
		Time newtime(get_text(),fps_);
		if(abs(newtime-time_)>=0.001)
		{
			time_=newtime;
			refresh_text();
			signal_value_changed()();
		}
	}
	catch(...)
	{
		throw string("Caught unknown exception");
	}
}

bool
Widget_Time::on_event(GdkEvent* event)
{
	const Time scroll_amount(0.25);

	switch(event->type)
	{
	case GDK_SCROLL:
		if(event->scroll.direction==GDK_SCROLL_DOWN || event->scroll.direction==GDK_SCROLL_LEFT)
		{
			time_-=scroll_amount;
			refresh_text();
			signal_value_changed()();
		}
		else if(event->scroll.direction==GDK_SCROLL_UP || event->scroll.direction==GDK_SCROLL_RIGHT)
		{
			time_+=scroll_amount;
			refresh_text();
			signal_value_changed()();
		}
		return true;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
		if (!has_focus())
			grab_focus_without_selecting();
		break;
	default:
		break;
	}

	return Gtk::Entry::on_event(event);
}

bool
Widget_Time::on_focus_out_event(GdkEventFocus* event)
{
	refresh_value();
	refresh_text();
	return Gtk::Entry::on_focus_out_event(event);
}

bool
Widget_Time::on_focus_in_event(GdkEventFocus* event)
{
	// if defined, show the full time format "0h 0m 5s 0f" when the time widget gets focus
	if (getenv("SYNFIG_SHOW_FULL_TIME_ON_FOCUS"))
		set_text(time_.get_string(fps_,App::get_time_format()|Time::FORMAT_FULL));

	return Gtk::Entry::on_focus_in_event(event);
}

GType Widget_Time::gtype = 0;

Glib::ObjectBase *
Widget_Time::wrap_new (GObject *o)
{
	if (gtk_widget_is_toplevel (GTK_WIDGET (o)))
		return new Widget_Time (GTK_ENTRY(o));
	else
		return Gtk::manage(new Widget_Time(GTK_ENTRY(o)));
}

void
Widget_Time::register_type ()
{
	if (gtype)
		return;

	Widget_Time dummy;

	gtype = G_OBJECT_TYPE (dummy.gobj());

	Glib::wrap_register (gtype, Widget_Time::wrap_new);
}
