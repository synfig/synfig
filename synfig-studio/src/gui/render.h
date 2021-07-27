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

#include <gtkmm/adjustment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>

#include <gui/dialogs/dialog_targetparam.h>
#include <gui/renddesc.h>

#include <synfig/string.h>
#include <synfig/target.h>

#include <synfigapp/canvasinterface.h>

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{
class AsyncRenderer;

class ProgressLogger;

class RenderSettings : public Gtk::Dialog
{
	std::shared_ptr<synfigapp::CanvasInterface> canvas_interface_;
	Widget_RendDesc widget_rend_desc;

	Gtk::Entry entry_filename;

	Glib::RefPtr<Gtk::Adjustment> adjustment_quality;
	Gtk::SpinButton entry_quality;

	Glib::RefPtr<Gtk::Adjustment> adjustment_antialias;
	Gtk::SpinButton entry_antialias;

	Gtk::CheckButton toggle_single_frame;
	Gtk::CheckButton toggle_extract_alpha;

	Gtk::ComboBoxText comboboxtext_target;
	Gtk::Button *tparam_button;

	std::vector<synfig::String> target_names;
	synfig::String target_name;
	synfig::String calculated_target_name;
	std::vector< std::pair<synfig::TargetAlphaMode,synfig::String> > render_passes;

	void set_target(synfig::String name);

	std::shared_ptr<AsyncRenderer> async_renderer;
	std::unique_ptr<ProgressLogger> progress_logger;

	synfig::TargetParam tparam;

	static std::map<synfig::String, Dialog_TargetParam *> dialog_book;

public:
	RenderSettings(Gtk::Window& parent,std::shared_ptr<synfigapp::CanvasInterface> canvas_interface);
	~RenderSettings();
	void set_entry_filename();

	//Use to add new param dialog.
	static void register_dialog(synfig::String target_name, Dialog_TargetParam * dialog) 
	{dialog_book[target_name] = dialog;}

private:
	void on_rend_desc_changed();
	void on_single_frame_toggle();
	void on_choose_pressed();
	void on_render_pressed();
	bool check_target_destination();
	void on_cancel_pressed();
	void on_targetparam_pressed();
	void submit_next_render_pass();
	void on_comboboxtext_target_changed();
	void on_finished(std::string error_message);
}; // END of class RenderSettings

}; // END of namespace studio


/* === E N D =============================================================== */

#endif
