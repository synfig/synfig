/* === S I N F G =========================================================== */
/*!	\file main.h
**	\brief Template Header
**
**	$Id: main.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFGAPP_MAIN_H
#define __SINFGAPP_MAIN_H

/* === H E A D E R S ======================================================= */

#include <ETL/ref_count>
#include <sinfg/string.h>
#include <sinfg/general.h>
#include <sinfg/main.h>
#include <sinfg/distance.h>
#include <sinfg/real.h>
#include <sinfg/waypoint.h>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include "inputdevice.h"
#include "settings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
	class Color;
	class Gradient;
};

namespace sinfgapp {
	
/*!	\class sinfgapp::Main
**	\brief \writeme
**
**	\writeme
*/
class Main : public sinfg::Main
{
	etl::reference_counter ref_count_;
public:
	Main(const sinfg::String &basepath,sinfg::ProgressCallback *cb=0);
	~Main();

	const etl::reference_counter& ref_count()const { return ref_count_; }
		
	static const sinfg::Color& get_foreground_color();
	static const sinfg::Color& get_background_color();
	static const sinfg::Gradient& get_gradient();
	static const sinfg::Distance& get_bline_width();
	static sinfg::Waypoint::Interpolation get_interpolation();


	static void set_interpolation(sinfg::Waypoint::Interpolation x);
	static void set_bline_width(sinfg::Distance x);	
	static void set_foreground_color(sinfg::Color color);
	static void set_background_color(sinfg::Color color);
	static void set_gradient(sinfg::Gradient gradient);
	static void set_gradient_default_colors();
	static void color_swap();	

	static sinfg::Color::BlendMethod get_blend_method();
	static const sinfg::Real& get_opacity();
	static void set_blend_method(sinfg::Color::BlendMethod);
	static void set_opacity(sinfg::Real);
	static sigc::signal<void>& signal_blend_method_changed();
	static sigc::signal<void>& signal_opacity_changed();
	static sigc::signal<void>& signal_interpolation_changed();
	
	// Input Device stuff
	static InputDevice::Handle add_input_device(const sinfg::String id_, InputDevice::Type type_=InputDevice::TYPE_MOUSE);
	static InputDevice::Handle find_input_device(const sinfg::String id_);
	static InputDevice::Handle select_input_device(const sinfg::String id_);
	static bool select_input_device(InputDevice::Handle input_device);
	static InputDevice::Handle get_selected_input_device();
	static void set_state(sinfg::String state);

	static Settings& settings();
	
	// Signal interfaces	
	static sigc::signal<void>& signal_foreground_color_changed();
	static sigc::signal<void>& signal_background_color_changed();
	static sigc::signal<void>& signal_gradient_changed();
	static sigc::signal<void>& signal_bline_width_changed();

}; // END of class Main

}; // END if namespace sinfgapp
/* === E N D =============================================================== */

#endif
