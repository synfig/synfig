/* === S Y N F I G ========================================================= */
/*!	\file inputdevice.cpp
**	\brief Template File
**
**	$Id: inputdevice.cpp,v 1.2 2005/01/12 04:08:32 darco Exp $
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

#include "inputdevice.h"
#include "settings.h"
#include <cstdio>
#include <ETL/stringf>
#include "main.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

class DeviceSettings : public Settings
{
	InputDevice* input_device;
public:
	DeviceSettings(InputDevice* input_device):
		input_device(input_device) { }
		

	virtual bool get_value(const synfig::String& key, synfig::String& value)const
	{
		if(key=="state")
		{
			value=input_device->get_state();
			return true;
		}
		if(key=="bline_width")
		{
			value=strprintf("%s",input_device->get_bline_width().get_string().c_str());
			return true;
		}
		if(key=="opacity")
		{
			value=strprintf("%f",(float)input_device->get_opacity());
			return true;
		}
		if(key=="blend_method")
		{
			value=strprintf("%i",(int)input_device->get_blend_method());
			return true;
		}
		if(key=="color")
		{
			Color c(input_device->get_foreground_color());
			value=strprintf("%f %f %f %f",(float)c.get_r(),(float)c.get_g(),(float)c.get_b(),(float)c.get_a());

			return true;
		}
		
		return Settings::get_value(key, value);
	}
	
	virtual bool set_value(const synfig::String& key,const synfig::String& value)
	{
			DEBUGPOINT();
		if(key=="state")
		{
			input_device->set_state(value);
			return true;
		}
		if(key=="bline_width")
		{
			input_device->set_bline_width(synfig::Distance(value));
			return true;
		}
		if(key=="opacity")
		{
			input_device->set_opacity(atof(value.c_str()));
			return true;
		}
		if(key=="blend_method")
		{
			input_device->set_blend_method(Color::BlendMethod(atoi(value.c_str())));
			return true;
		}
		if(key=="color")
		{
			float r=0,g=0,b=0,a=1;
			if(!strscanf(value,"%f %f %f %f",&r,&g,&b,&a))
				return false;
			input_device->set_foreground_color(synfig::Color(r,g,b,a));
			return true;
		}
		
		return Settings::set_value(key, value);
	}
	
	virtual KeyList get_key_list()const
	{
		KeyList ret(Settings::get_key_list());
		ret.push_back("color");
		ret.push_back("state");
		ret.push_back("bline_width");
		ret.push_back("blend_method");
		ret.push_back("opacity");
		return ret;
	}
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

InputDevice::InputDevice(const synfig::String id_, Type type_):
	id_(id_),
	type_(type_),
	state_((type_==TYPE_PEN)?"sketch":"normal"),
	foreground_color_(Color::black()),
	background_color_(Color::white()),
	bline_width_(Distance(1,Distance::SYSTEM_POINTS)),
	opacity_(1.0f),
	blend_method_(Color::BLEND_COMPOSITE)
{
	switch(type_)
	{
		case TYPE_MOUSE:
			state_="normal";
			break;

		case TYPE_PEN:
			state_="draw";
			break;

		case TYPE_ERASER:
			state_="normal";
			break;

		case TYPE_CURSOR:
			state_="normal";
			break;

		default:
			state_="normal";
			break;
	}
	
	device_settings=new DeviceSettings(this);
	Main::settings().add_domain(device_settings,"input_device."+id_);
}

InputDevice::~InputDevice()
{
	Main::settings().remove_domain("input_device."+id_);	
	delete device_settings;
}

Settings&
InputDevice::settings()
{
	return *device_settings;
}

const Settings&
InputDevice::settings()const
{
	return *device_settings;
}
