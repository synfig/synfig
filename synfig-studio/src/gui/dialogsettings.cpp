/* === S Y N F I G ========================================================= */
/*!	\file dialogsettings.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "dialogsettings.h"
#include <synfigapp/main.h>
#include <gdkmm/general.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DialogSettings::DialogSettings(Gtk::Window* window,const synfig::String& name):
	window(window),
	name(name)
{
	synfigapp::Main::settings().add_domain(this,"window."+name);
}

DialogSettings::~DialogSettings()
{
	synfigapp::Main::settings().remove_domain("window."+name);
}

bool
DialogSettings::get_value(const synfig::String& key, synfig::String& value)const
{
	if(key=="pos")
	{
		// if(!window->is_visible())return false;
		int x,y; window->get_position(x,y);
		value=strprintf("%d %d",x,y);
		return true;
	}
	if(key=="size")
	{
		// if(!window->is_visible())return false;
		int x,y; window->get_size(x,y);
		value=strprintf("%d %d",x,y);
		return true;
	}
	if(key=="x")
	{
		int x,y; window->get_position(x,y);
		value=strprintf("%d",x);
		return true;
	}
	if(key=="y")
	{
		int x,y; window->get_position(x,y);
		value=strprintf("%d",y);
		return true;
	}
	if(key=="w")
	{
		int x,y; window->get_size(x,y);
		value=strprintf("%d",x);
		return true;
	}
	if(key=="h")
	{
		int x,y; window->get_size(x,y);
		value=strprintf("%d",y);
		return true;
	}
	if(key=="visible")
	{
		value=window->get_visible()?"1":"0";
		return true;
	}

	return synfigapp::Settings::get_value(key,value);
}

bool
DialogSettings::set_value(const synfig::String& key,const synfig::String& value)
{
	int screen_w(Gdk::screen_width());
	int screen_h(Gdk::screen_height());

	if(value.empty())
		return false;

	if(key=="pos")
	{
		int x,y;
		if(!strscanf(value,"%d %d",&x, &y))
			return false;

		if (x > screen_w) x = screen_w - 150;
		if (x < 0) x = 0;
		
		if (y > screen_h) y = screen_h - 150;
		if (y < 0) y = 0;

		window->move(x,y);
		return true;
	}
	if(key=="size")
	{
		int x,y;
		if(!strscanf(value,"%d %d",&x, &y))
			return false;

		if (x > screen_w) x = 150;
		if (x < 0) x = 0;
		
		if (y > screen_h) y = 150;
		if (y < 0) y = 0;

		window->set_default_size(x,y);
		return true;
	}
	if(key=="x")
	{
		int x,y; window->get_position(x,y);
		x=atoi(value.c_str());
		window->move(x,y);
		return true;
	}
	if(key=="y")
	{
		int x,y; window->get_position(x,y);
		y=atoi(value.c_str());
		window->move(x,y);
		return true;
	}
	if(key=="w")
	{
		int x,y; window->get_size(x,y);
		x=atoi(value.c_str());
		window->set_default_size(x,y);
		return true;
	}
	if(key=="h")
	{
		int x,y; window->get_size(x,y);
		y=atoi(value.c_str());
		window->set_default_size(x,y);
		return true;
	}
	if(key=="visible")
	{
		if(value=="0")
			window->hide();
		else
			window->present();
		return true;
	}

	return synfigapp::Settings::set_value(key,value);
}

synfigapp::Settings::KeyList
DialogSettings::get_key_list()const
{
	synfigapp::Settings::KeyList ret(synfigapp::Settings::get_key_list());

	ret.push_back("size");
	ret.push_back("pos");
	ret.push_back("visible");

	return ret;
}
