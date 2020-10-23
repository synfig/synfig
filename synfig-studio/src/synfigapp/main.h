/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/main.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIGAPP_MAIN_H
#define __SYNFIGAPP_MAIN_H

/* === H E A D E R S ======================================================= */

#include <ETL/ref_count>
#include <synfig/string.h>
#include <synfig/main.h>
#include <synfig/distance.h>
#include <synfig/real.h>
#include <synfig/waypoint.h>
#include <sigc++/sigc++.h>
#include "inputdevice.h"
#include "settings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	class Color;
	class Gradient;
};

namespace synfigapp {

/*!	\class synfigapp::Main
**	\brief \writeme
**
**	\writeme
*/
class Main : public synfig::Main
{
	etl::reference_counter ref_count_;
public:
	Main(const synfig::String &basepath,synfig::ProgressCallback *cb=0);
	~Main();

	const etl::reference_counter& ref_count()const { return ref_count_; }

	static const synfig::Color& get_outline_color();
	static const synfig::Color& get_fill_color();
	static const synfig::Gradient& get_gradient();
	static const synfig::Distance& get_bline_width();
	static synfig::Waypoint::Interpolation get_interpolation();


	static void set_interpolation(synfig::Waypoint::Interpolation x);
	static void set_bline_width(synfig::Distance x);
	static void set_outline_color(synfig::Color color);
	static void set_fill_color(synfig::Color color);
	static void set_gradient(synfig::Gradient gradient);
	static void set_gradient_default_colors();
	static void color_swap();

	static synfig::Color::BlendMethod get_blend_method();
	//static const synfig::Real& get_opacity();
	//static void set_blend_method(synfig::Color::BlendMethod);
	//static void set_opacity(synfig::Real);
	//static sigc::signal<void>& signal_blend_method_changed();
	//static sigc::signal<void>& signal_opacity_changed();
	static sigc::signal<void>& signal_interpolation_changed();

	// Input Device stuff
	static InputDevice::Handle add_input_device(const synfig::String id_, InputDevice::Type type_=InputDevice::TYPE_MOUSE);
	static InputDevice::Handle find_input_device(const synfig::String id_);
	static InputDevice::Handle select_input_device(const synfig::String id_);
	static bool select_input_device(InputDevice::Handle input_device);
	static InputDevice::Handle get_selected_input_device();
	static void set_state(synfig::String state);

	static Settings& settings();

	// Signal interfaces
	static sigc::signal<void>& signal_outline_color_changed();
	static sigc::signal<void>& signal_fill_color_changed();
	static sigc::signal<void>& signal_gradient_changed();
	static sigc::signal<void>& signal_bline_width_changed();
	
	static synfig::String get_user_app_directory();

}; // END of class Main

}; // END if namespace synfigapp
/* === E N D =============================================================== */

#endif
