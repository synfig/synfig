/* === S Y N F I G ========================================================= */
/*!	\file state.cpp
**	\brief Generic tool state
**
**	$Id$
**
**	\legal
**	Copyright (c) 2016 caryoscelus
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

#include "state.h"
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

void
State_Context::load_settings()
{
	try
	{
		do_load_settings();
	}
	catch(...)
	{
		synfig::warning("State "+get_name()+": Caught exception when attempting to load settings.");
	}
}

void
State_Context::save_settings()
{
	try
	{
		do_save_settings();
	}
	catch(...)
	{
		synfig::warning("State "+get_name()+": Caught exception when attempting to save settings.");
	}
}

void
State_Context::do_load_settings()
{
    synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
}

void
State_Context::do_save_settings()
{
    synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
}

String
State_Context::get_setting(String name) const
{
	return settings.get_value(get_name_lower()+"."+name);
}

void
State_Context::set_setting(String name, String value)
{
	settings.set_value(get_name_lower()+"."+name, value);
}

State_Context::State_Context(CanvasView* canvas_view) :
    canvas_view_(canvas_view),
    settings(synfigapp::Main::get_selected_input_device()->settings())
{
}
