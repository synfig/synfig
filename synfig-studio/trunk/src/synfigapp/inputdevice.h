/* === S Y N F I G ========================================================= */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: inputdevice.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

	typedef etl::handle<InputDevice> Handle;
	
private:
	synfig::String id_;
	Type type_;
	synfig::String state_;
	synfig::Color foreground_color_;
	synfig::Color background_color_;
	synfig::Distance	bline_width_;	
	synfig::Real opacity_;
	synfig::Color::BlendMethod blend_method_;

	DeviceSettings* device_settings;

public:
	InputDevice(const synfig::String id_, Type type_=TYPE_MOUSE);
	~InputDevice();

	const synfig::String& get_id()const { return id_; }
	const synfig::String& get_state()const { return state_; }
	const synfig::Color& get_foreground_color()const { return foreground_color_; }
	const synfig::Color& get_background_color()const { return background_color_; }
	const synfig::Distance& get_bline_width()const { return bline_width_; }
	const synfig::Real& get_opacity()const { return opacity_; }
	const synfig::Color::BlendMethod& get_blend_method()const { return blend_method_; }
	Type get_type()const { return type_; }
	
	void set_state(const synfig::String& x) { state_=x; }
	void set_foreground_color(const synfig::Color& x) { foreground_color_=x; }
	void set_background_color(const synfig::Color& x) { background_color_=x; }
	void set_bline_width(const synfig::Distance& x) { bline_width_=x; }
	void set_blend_method(const synfig::Color::BlendMethod& x) { blend_method_=x; }
	void set_opacity(const synfig::Real& x) { opacity_=x; }
	void set_type(Type x) { type_=x; }
	
	Settings& settings();
	const Settings& settings()const;
}; // END of class InputDevice

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
