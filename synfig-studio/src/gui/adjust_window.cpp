/* === S Y N F I G ========================================================= */
/*!	\file adjust_window.cpp
**	\brief Adjustment Window Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#include "adjust_window.h"
#include "app.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
//using namespace synfig;

using studio::Adjust_Window;

/* === M A C R O S ========================================================= */
const double EPSILON = 1.0e-6;

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Adjust_Window::Adjust_Window(double value, double lower, double upper,
							 double stepinc, double pageinc, double pagesize,
							 const Glib::RefPtr<Gtk::Adjustment> &adj)
: Adjustment(value,lower,upper,stepinc,pageinc,pagesize),
	adj_child(0)
{
	if(adj) set_child_adjustment(adj);
}

Adjust_Window::~Adjust_Window()
{
	//connections should automatically be killed etc.
}

//child interface functions
Glib::RefPtr<Gtk::Adjustment> Adjust_Window::get_child_adjustment()
{
	return adj_child;
}

Glib::RefPtr<const Gtk::Adjustment> Adjust_Window::get_child_adjustment() const
{
	return adj_child;
}

void Adjust_Window::set_child_adjustment(const Glib::RefPtr<Gtk::Adjustment> &child)
{
	childchanged.disconnect();

	adj_child = child;

	// synfig::info("Adjust: connecting to child signals");

	if(child)
	{
		childchanged = child->signal_changed().connect(sigc::mem_fun(*this,&Adjust_Window::update_fromchild));

		update_child();
	}
}

void Adjust_Window::on_changed()
{
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
		printf("%s:%d Adjust_Window::on_changed()\n", __FILE__, __LINE__);

	update_child();
}

void Adjust_Window::on_value_changed()
{
	update_child();
}

//SUB TIME FUNCTIONS
double Adjust_Window::get_sub_lower() const
{
	return get_value();
}

double Adjust_Window::get_sub_upper() const
{
	return get_value() + get_page_size();
}

//---- REFRESH FUNCTIONS -----
void Adjust_Window::update_child()
{
	if(adj_child)
	{
		bool childchanged = false;

		double v = get_value();
		double ve = v + get_page_size();

		//reset child's values if they need to be...
		if(abs(v - adj_child->get_lower()) > EPSILON)
		{
			adj_child->set_lower(v);
			childchanged = true;
		}

		if(abs(ve - adj_child->get_upper()) > EPSILON)
		{
			adj_child->set_upper(ve);
			childchanged = true;
		}

		if(childchanged)
		{
			adj_child->changed();
		}
	}
}

void Adjust_Window::update_fromchild()
{
	if(adj_child)
	{
		double b = adj_child->get_lower();
		double dist = adj_child->get_upper() - b;

		//reset our values if they need to be...
		if(abs(get_value() - b) > EPSILON)
		{
			set_value(b);
			value_changed();
		}

		if(abs(get_page_size() - dist) > EPSILON)
		{
			set_page_size(dist);
			changed();
		}
	}
}
