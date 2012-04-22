/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/render.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_STUDIO_GTKMM_RENDER_H
#define __SYNFIG_STUDIO_GTKMM_RENDER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/optionmenu.h>

#include <synfig/string.h>
#include <synfig/targetparam.h>

#include <synfigapp/canvasinterface.h>

#include "renddesc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{
class AsyncRenderer;

class RenderSettings : public Gtk::Dialog
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface_;
	Widget_RendDesc widget_rend_desc;

	Gtk::Entry entry_filename;

	Gtk::Adjustment adjustment_quality;
	Gtk::SpinButton entry_quality;

	Gtk::Adjustment adjustment_antialias;
	Gtk::SpinButton entry_antialias;

	Gtk::CheckButton toggle_single_frame;

	Gtk::OptionMenu optionmenu_target;
	Gtk::Menu *menu_target;
	Gtk::Button *tparam_button;

	synfig::String target_name;

	void set_target(synfig::String name);

	etl::handle<AsyncRenderer> async_renderer;

	synfig::TargetParam tparam;

public:
	RenderSettings(Gtk::Window& parent,etl::handle<synfigapp::CanvasInterface> canvas_interface);
	~RenderSettings();
	void set_entry_filename();

private:

	void on_rend_desc_changed();
	void on_single_frame_toggle();
	void on_choose_pressed();
	void on_render_pressed();
	void on_cancel_pressed();
	void on_targetparam_pressed();

	void on_finished();
}; // END of class RenderSettings

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
