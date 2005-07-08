/* === S Y N F I G ========================================================= */
/*!	\file widget_time.cpp
**	\brief Template File
**
**	$Id: widget_distance.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include "widget_distance.h"
#include "app.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#if ! defined(_)
#define _(x)	(x)
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Distance::Widget_Distance():
	Gtk::SpinButton(0.05,5),
	adjustment(0,-100000000,100000000,1,1,1)
//	adjustment(0,-100000000,100000000,1,2,0)
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
