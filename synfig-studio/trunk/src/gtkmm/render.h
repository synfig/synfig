/* === S Y N F I G ========================================================= */
/*!	\file render.h
**	\brief Template Header
**
**	$Id: render.h,v 1.2 2005/01/10 08:13:44 darco Exp $
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

#ifndef __SYNFIG_STUDIO_GTKMM_RENDER_H
#define __SYNFIG_STUDIO_GTKMM_RENDER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/optionmenu.h>

#include <synfig/string.h>

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
	Gtk::Tooltips tooltips;

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

	synfig::String target_name;
	
	void set_target(synfig::String name);

	etl::handle<AsyncRenderer> async_renderer;

public:
	RenderSettings(Gtk::Window& parent,etl::handle<synfigapp::CanvasInterface> canvas_interface);
	~RenderSettings();

private:

	void on_rend_desc_changed();
	void on_single_frame_toggle();
	void on_choose_pressed();
	void on_render_pressed();
	void on_cancel_pressed();
	
	void on_finished();
}; // END of class RenderSettings
	
}; // END of namespace studio


/* === E N D =============================================================== */

#endif
