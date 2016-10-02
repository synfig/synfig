/* === S Y N F I G ========================================================= */
/*!	\file state.h
**	\brief Generic tool state (header)
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_STATE_H
#define __SYNFIG_STUDIO_STATE_H

/* === H E A D E R S ======================================================= */

#include "state_minimal.h"
#include "canvasview.h"
#include <synfig/general.h>

/* === M A C R O S ========================================================= */

// indentation for options layout
#ifndef SPACING
#define SPACING(name, px) \
	Gtk::Alignment *name = Gtk::manage(new Gtk::Alignment()); \
	name->set_size_request(px)
#endif

#define GAP	(3)
#define INDENTATION (6)

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class State_Context : public StateMinimal_Context
{
protected:
	etl::handle<CanvasView> canvas_view_;

	//Toolbox settings
	synfigapp::Settings& settings;

protected:
	virtual const synfig::String get_name_lower() const = 0;
	virtual const synfig::String get_name() const = 0;
	virtual const synfig::String get_local_name() const = 0;

	synfig::String get_setting(synfig::String name) const;
	template <typename T>
	T get_setting(synfig::String name, T default_value) const
	{
		return settings.tget_value(get_name_lower()+"."+name, default_value);
	}
	template <typename T>
	void set_setting(synfig::String name, T value)
	{
		settings.tset_value(get_name_lower()+"."+name, value);
	}

	//! Load settings unsafe implementation
	virtual void do_load_settings();
	//! Save settings unsafe implementation
	virtual void do_save_settings();

public:
	//! Load settings safe wrapper
	//! \see do_load_settings
	void load_settings();
	//! Save settings safe wrapper
	//! \see do_save_settings
	void save_settings();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	void layer_creation(Gtk::ToggleButton &button, const synfig::String& stockid, const synfig::String& tooltip);
	virtual void toggle_layer_creation() = 0;

public:
	State_Context(CanvasView* canvas_view);
	virtual ~State_Context() = default;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
