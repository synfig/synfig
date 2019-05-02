/* === S Y N F I G ========================================================= */
/*!	\file inputdevice.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_INPUTDEVICE_H
#define __SYNFIG_INPUTDEVICE_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/distance.h>
#include <synfig/string.h>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class DeviceSettings;

namespace synfigapp {
class Settings;


/*!	\class  InputDevice  inputdevice.h  "synfigapp/inputdevice.h"
**	\brief  This class provides a device independent representation the state
**	        of an input device.
**  \see  studio::DeviceTracker
**  \see  synfigapp::Settings
**
**   The representation includes both the GDK state (e.g., mode) and synfigstudio
**   state (e.g., outline color). An object of this class can be saved and
**   restored using its Settings object, provided by the settings method.
*/
class InputDevice : public etl::shared_object
{
public:
	enum Type
	{
		TYPE_MOUSE,
		TYPE_PEN,
		TYPE_ERASER,
		TYPE_CURSOR
	};

	enum Mode
	{
		MODE_DISABLED,
		MODE_SCREEN,
		MODE_WINDOW
	};

	enum AxisUse
	{
	  AXIS_IGNORE,
	  AXIS_X,
	  AXIS_Y,
	  AXIS_PRESSURE,
	  AXIS_XTILT,
	  AXIS_YTILT,
	  AXIS_WHEEL,
	  AXIS_LAST
	};

	struct DeviceKey
	{
	  unsigned int keyval;
	  unsigned int modifiers;
	};

	typedef etl::handle<InputDevice> Handle;

private:
	synfig::String id_;
	Type type_;
	synfig::String state_;
	synfig::Color outline_color_;
	synfig::Color fill_color_;
	synfig::Distance	bline_width_;
	synfig::Real opacity_;
	synfig::Color::BlendMethod blend_method_;
	Mode mode_;
	std::vector<AxisUse> axes_;
	std::vector<DeviceKey> keys_;

	DeviceSettings* device_settings;

public:
	InputDevice(const synfig::String id_, Type type_=TYPE_MOUSE);
	~InputDevice();

	const synfig::String& get_id()const { return id_; }
	const synfig::String& get_state()const { return state_; }
	const synfig::Color& get_outline_color()const { return outline_color_; }
	const synfig::Color& get_fill_color()const { return fill_color_; }
	const synfig::Distance& get_bline_width()const { return bline_width_; }
	const synfig::Real& get_opacity()const { return opacity_; }
	const synfig::Color::BlendMethod& get_blend_method()const { return blend_method_; }
	Type get_type()const { return type_; }
	Mode get_mode()const { return mode_; }
	const std::vector<AxisUse> & get_axes()const { return axes_; }
	const std::vector<DeviceKey> & get_keys()const { return keys_; }

	void set_state(const synfig::String& x) { state_=x; }
	void set_outline_color(const synfig::Color& x) { outline_color_=x; }
	void set_fill_color(const synfig::Color& x) { fill_color_=x; }
	void set_bline_width(const synfig::Distance& x) { bline_width_=x; }
	void set_blend_method(const synfig::Color::BlendMethod& x) { blend_method_=x; }
	void set_opacity(const synfig::Real& x) { opacity_=x; }
	void set_type(Type x) { type_=x; }
	void set_mode(Mode x) { mode_=x; }
	void set_axes(const std::vector<AxisUse>& x) { axes_=x; }
	void set_keys(const std::vector<DeviceKey>& x) { keys_=x; }

	Settings& settings();
	const Settings& settings()const;
}; // END of class InputDevice

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
