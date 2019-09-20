/* === S Y N F I G ========================================================= */
/*!	\file inputdevice.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2010 Carlos López
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

#include "inputdevice.h"
#include "settings.h"
#include <cstdio>
#include <ETL/stringf>
#include "main.h"

#include <synfigapp/localization.h>

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
		try
		{
			synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
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
			if(key=="outline_color")
			{
				Color c(input_device->get_outline_color());
				value=strprintf("%f %f %f %f",(float)c.get_r(),(float)c.get_g(),(float)c.get_b(),(float)c.get_a());

				return true;
			}
			if(key=="fill_color")
			{
				Color c(input_device->get_fill_color());
				value=strprintf("%f %f %f %f",(float)c.get_r(),(float)c.get_g(),(float)c.get_b(),(float)c.get_a());

				return true;
			}
			if(key=="mode")
			{
				get_mode_value(value);
				return true;
			}
			if(key=="axes")
			{
				get_axes_value(value);
				return true;
			}
			if(key=="keys")
			{
				get_keys_value(value);
				return true;
			}
		}
		catch(...)
		{
			synfig::warning("DeviceSettings: Caught exception when attempting to get value.");
		}
		return Settings::get_value(key, value);
	}

	void get_mode_value(synfig::String & value) const
	{
		if (input_device->get_mode() == InputDevice::MODE_SCREEN)
			value = "screen";
		else if (input_device->get_mode() == InputDevice::MODE_WINDOW)
			value = "window";
		else
			value = "disabled";
	}

	void get_axes_value(synfig::String & value) const
	{
		vector<InputDevice::AxisUse> axes = input_device->get_axes();
		value = strprintf("%zu", axes.size());
		vector<InputDevice::AxisUse>::const_iterator itr;
		for (itr = axes.begin(); itr != axes.end(); itr++)
			value += strprintf(" %u", (unsigned int) *itr);
	}

	void get_keys_value(synfig::String & value) const
	{
		vector<InputDevice::DeviceKey> keys = input_device->get_keys();
		value = strprintf("%zu", keys.size());
		vector<InputDevice::DeviceKey>::const_iterator itr;
		for (itr = keys.begin(); itr != keys.end(); itr++)
			value += strprintf(" %u %u", itr->keyval, itr->modifiers);
	}

	virtual bool set_value(const synfig::String& key,const synfig::String& value)
	{
		try
		{
			synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
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
			if(key=="outline_color")
			{
				float r=0,g=0,b=0,a=1;
				if(!strscanf(value,"%f %f %f %f",&r,&g,&b,&a))
					return false;
				input_device->set_outline_color(synfig::Color(r,g,b,a));
				return true;
			}
			if(key=="fill_color")
			{
				float r=0,g=0,b=0,a=1;
				if(!strscanf(value,"%f %f %f %f",&r,&g,&b,&a))
					return false;
				input_device->set_fill_color(synfig::Color(r,g,b,a));
				return true;
			}
			if(key=="mode")
			{
				set_mode_value(value);
				return true;
			}
			if(key=="axes")
			{
				set_axes_value(value);
				return true;
			}
			if(key=="keys")
			{
				set_keys_value(value);
				return true;
			}
		}
		catch(...)
		{
			synfig::warning("DeviceSettings: Caught exception when attempting to set value.");
		}
		return Settings::set_value(key, value);
	}

	void set_mode_value(const synfig::String & value)
	{
		InputDevice::Mode mode;
		if (value == "screen")
			mode = InputDevice::MODE_SCREEN;
		else if (value == "window")
			mode = InputDevice::MODE_WINDOW;
		else
			mode = InputDevice::MODE_DISABLED;

		input_device->set_mode(mode);
	}

	void set_axes_value(const synfig::String & value)
	{
		std::vector<InputDevice::AxisUse> axes;

		unsigned pos = value.find(' ', 0);
		if (pos < value.size()) {
			int num_axes = atoi(value.substr(0, pos).c_str());
			axes.resize(num_axes);

			for (int axis = 0; axis < num_axes; axis++) {
				int last = pos;
				pos = value.find(' ', pos + 1);
				axes[axis] = InputDevice::AxisUse(atoi(value.substr(last, pos).c_str()));
			}
		}

		input_device->set_axes(axes);
	}

	void set_keys_value(const synfig::String & value)
	{
		std::vector<InputDevice::DeviceKey> keys;

		unsigned pos = value.find(' ', 0);
		if (pos < value.size()) {
			int num_keys = atoi(value.substr(0, pos).c_str());
			keys.resize(num_keys);

			for (int key = 0; key < num_keys; key++) {
				int last = pos;
				pos = value.find(' ', pos + 1);
				keys[key].keyval = (unsigned int) atol(value.substr(last, pos).c_str());
				last = pos;
				pos = value.find(' ', pos + 1);
				keys[key].modifiers = (unsigned int) atol(value.substr(last, pos).c_str());
			}
		}

		input_device->set_keys(keys);
	}

	virtual KeyList get_key_list()const
	{
		KeyList ret(Settings::get_key_list());
		ret.push_back("outline_color");
		ret.push_back("fill_color");
		ret.push_back("state");
		ret.push_back("bline_width");
		ret.push_back("opacity");
		ret.push_back("mode");
		ret.push_back("axes");
		ret.push_back("keys");
		return ret;
	}
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

InputDevice::InputDevice(const synfig::String id_, Type type_):
	id_(id_),
	type_(type_),
	state_((type_==TYPE_PEN)?"draw":"normal"),
	outline_color_(Color::black()),
	fill_color_(Color::white()),
	bline_width_(Distance(1,Distance::SYSTEM_POINTS)),
	opacity_(1.0f),
	blend_method_(Color::BLEND_BY_LAYER),
	mode_(MODE_DISABLED)
{
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
