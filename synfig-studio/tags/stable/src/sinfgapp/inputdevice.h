/* === S I N F G =========================================================== */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: inputdevice.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_INPUTDEVICE_H
#define __SINFG_INPUTDEVICE_H

/* === H E A D E R S ======================================================= */

#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/distance.h>
#include <sinfg/string.h>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class DeviceSettings;

namespace sinfgapp {
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
	sinfg::String id_;
	Type type_;
	sinfg::String state_;
	sinfg::Color foreground_color_;
	sinfg::Color background_color_;
	sinfg::Distance	bline_width_;	
	sinfg::Real opacity_;
	sinfg::Color::BlendMethod blend_method_;

	DeviceSettings* device_settings;

public:
	InputDevice(const sinfg::String id_, Type type_=TYPE_MOUSE);
	~InputDevice();

	const sinfg::String& get_id()const { return id_; }
	const sinfg::String& get_state()const { return state_; }
	const sinfg::Color& get_foreground_color()const { return foreground_color_; }
	const sinfg::Color& get_background_color()const { return background_color_; }
	const sinfg::Distance& get_bline_width()const { return bline_width_; }
	const sinfg::Real& get_opacity()const { return opacity_; }
	const sinfg::Color::BlendMethod& get_blend_method()const { return blend_method_; }
	Type get_type()const { return type_; }
	
	void set_state(const sinfg::String& x) { state_=x; }
	void set_foreground_color(const sinfg::Color& x) { foreground_color_=x; }
	void set_background_color(const sinfg::Color& x) { background_color_=x; }
	void set_bline_width(const sinfg::Distance& x) { bline_width_=x; }
	void set_blend_method(const sinfg::Color::BlendMethod& x) { blend_method_=x; }
	void set_opacity(const sinfg::Real& x) { opacity_=x; }
	void set_type(Type x) { type_=x; }
	
	Settings& settings();
	const Settings& settings()const;
}; // END of class InputDevice

}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
