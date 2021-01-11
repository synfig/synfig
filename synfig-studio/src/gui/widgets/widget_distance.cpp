/* === S Y N F I G ========================================================= */
/*!	\file widget_distance.cpp
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

#include <gui/widgets/widget_distance.h>

#include <gui/exception_guard.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Distance::Widget_Distance():
	Gtk::SpinButton(0.05,5),
	distance_(1, Distance::SYSTEM_POINTS),
	adjustment(Gtk::Adjustment::create(0,-100000000,100000000,1,1,0))
{
	set_adjustment(adjustment);
	set_numeric(false);
}

Widget_Distance::~Widget_Distance()
{
}

int
Widget_Distance::on_input(double* new_value)
{
	distance_=synfig::String(get_text());
	*new_value=distance_.get();
	return 1;
}

bool
Widget_Distance::on_output()
{
	try{
	distance_=get_adjustment()->get_value();
	set_text(distance_.get_string(get_digits()));
	} catch (...) { /* synfig::error("Widget_Distance::on_output(): Caught something..."); */ }
	return true;
}

bool
Widget_Distance::on_key_press_event(GdkEventKey* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	return SpinButton::on_key_press_event(event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
Widget_Distance::on_key_release_event(GdkEventKey* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	return SpinButton::on_key_release_event(event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}


void
Widget_Distance::set_value(const synfig::Distance &data)
{
	distance_=data;
	get_adjustment()->set_value(distance_.get());
}

synfig::Distance
Widget_Distance::get_value() const
{
	distance_=get_adjustment()->get_value();
	return distance_;
}
