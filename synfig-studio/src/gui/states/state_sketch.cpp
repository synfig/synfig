/* === S Y N F I G ========================================================= */
/*!	\file state_sketch.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include "state_sketch.h"
#include "state_normal.h"
#include "state_stroke.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenodes/valuenode_bline.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"

#include <synfigapp/blineconvert.h>
#include <synfigapp/main.h>

#include <ETL/gaussian>

#include "docks/dialog_tooloptions.h"

#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/actiongroup.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateSketch studio::state_sketch;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateSketch_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	WorkArea::PushState push_state;

	bool prev_table_status;

	Gtk::Table options_table;
	Gtk::Button button_clear_sketch;
	Gtk::Button button_undo_stroke;
	Gtk::Button button_save_sketch;
	Gtk::Button button_load_sketch;
	Gtk::CheckButton checkbutton_show_sketch;

	void clear_sketch();
	void save_sketch();
	void load_sketch();
	void undo_stroke();
	void toggle_show_sketch();

public:

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_down_handler(const Smach::event& x);

	Smach::event_result event_stroke(const Smach::event& x);

	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();

	StateSketch_Context(CanvasView* canvas_view);

	~StateSketch_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Time get_time()const { return get_canvas_interface()->get_time(); }
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

};	// END of class StateSketch_Context


/* === M E T H O D S ======================================================= */

StateSketch::StateSketch():
	Smach::state<StateSketch_Context>("sketch")
{
	insert(event_def(EVENT_STOP,&StateSketch_Context::event_stop_handler));
	//insert(event_def(EVENT_REFRESH,&StateSketch_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateSketch_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateSketch_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_STROKE,&StateSketch_Context::event_stroke));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateSketch_Context::event_refresh_tool_options));
}

StateSketch::~StateSketch()
{
}

void
StateSketch_Context::save_sketch()
{
	synfig::String filename(basename(get_canvas()->get_file_name())+".sketch");

	while(App::dialog_save_file_sketch(_("Save Sketch"), filename, SKETCH_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if(find(filename.begin(),filename.end(),'*')!=filename.end())
			continue;

		if(get_work_area()->save_sketch(filename))
			break;

		get_canvas_view()->get_ui_interface()->error(_("Unable to save sketch"));
	}
}

void
StateSketch_Context::load_sketch()
{
	synfig::String filename(basename(get_canvas()->get_file_name())+".sketch");

	while(App::dialog_open_file_sketch(_("Load Sketch"), filename, SKETCH_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if(find(filename.begin(),filename.end(),'*')!=filename.end())
			continue;

		if(get_work_area()->load_sketch(filename))
			break;

		get_canvas_view()->get_ui_interface()->error(_("Unable to load sketch"));
	}
	get_work_area()->queue_draw();
}

void
StateSketch_Context::clear_sketch()
{
	get_work_area()->clear_persistent_strokes();

	// if the sketch is currently shown, make sure it is updated
	//! \todo is there a better way than this of getting Duckmatic to update its stroke_list_?
	if (checkbutton_show_sketch.get_active())
	{
		get_work_area()->set_show_persistent_strokes(false);
		get_work_area()->set_show_persistent_strokes(true);
		get_canvas_view()->get_smach().process_event(EVENT_REFRESH);
	}
}

void
StateSketch_Context::undo_stroke()
{
	if(!get_work_area()->persistent_stroke_list().empty())
	{
		get_work_area()->persistent_stroke_list().pop_back();

		// if the sketch is currently shown, make sure it is updated
		//! \todo is there a better way than this of getting Duckmatic to update its stroke_list_?
		if (checkbutton_show_sketch.get_active())
		{
			get_work_area()->set_show_persistent_strokes(false);
			get_work_area()->set_show_persistent_strokes(true);
			get_canvas_view()->get_smach().process_event(EVENT_REFRESH);
		}
	}
}

void
StateSketch_Context::toggle_show_sketch()
{
	get_work_area()->set_show_persistent_strokes(checkbutton_show_sketch.get_active());
	get_work_area()->queue_draw();
}

StateSketch_Context::StateSketch_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	push_state(*get_work_area()),
	checkbutton_show_sketch(_("Show Sketch"))
{
	checkbutton_show_sketch.set_active(get_work_area()->get_show_persistent_strokes());

	button_clear_sketch.signal_clicked().connect(sigc::mem_fun(*this,&studio::StateSketch_Context::clear_sketch));
	button_undo_stroke.signal_clicked().connect(sigc::mem_fun(*this,&studio::StateSketch_Context::undo_stroke));
	button_save_sketch.signal_clicked().connect(sigc::mem_fun(*this,&studio::StateSketch_Context::save_sketch));
	button_load_sketch.signal_clicked().connect(sigc::mem_fun(*this,&studio::StateSketch_Context::load_sketch));
	checkbutton_show_sketch.signal_clicked().connect(sigc::mem_fun(*this,&studio::StateSketch_Context::toggle_show_sketch));

	options_table.attach(*manage(new Gtk::Label(_("Sketch Tool"))),	0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_show_sketch,					0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(button_undo_stroke, 0, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(button_clear_sketch, 0, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(button_save_sketch, 0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	//options_table.attach(button_load_sketch, 1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	options_table.show_all();
	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	get_work_area()->set_cursor(Gdk::PENCIL);

	// Turn off duck clicking
	get_work_area()->set_allow_duck_clicks(false);

	// clear out the ducks
	//get_work_area()->clear_ducks();

	// Refresh the work area
	//get_work_area()->queue_draw();

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateSketch_Context::on_user_click));

	App::dock_toolbox->refresh();
}

StateSketch_Context::~StateSketch_Context()
{
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	// Enable the time bar
	//get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	//get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

void
StateSketch_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Sketch Tool"));
	App::dialog_tool_options->set_name("sketch");

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-undo"),
		_("Undo Last Stroke")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&studio::StateSketch_Context::undo_stroke
		)
	);
	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-clear"),
		_("Clear Sketch")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&studio::StateSketch_Context::clear_sketch
		)
	);
	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-save-as"),
		_("Save Sketch As...")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&studio::StateSketch_Context::save_sketch
		)
	);

	App::dialog_tool_options->add_button(
		Gtk::StockID("gtk-open"),
		_("Open a Sketch")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&studio::StateSketch_Context::load_sketch
		)
	);
}

Smach::event_result
StateSketch_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSketch_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateSketch_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSketch_Context::event_mouse_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			// Enter the stroke state to get the stroke
			get_canvas_view()->get_smach().push_state(&state_stroke);
			return Smach::RESULT_ACCEPT;
		}

	default:
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateSketch_Context::event_stroke(const Smach::event& x)
{
	const EventStroke& event(*reinterpret_cast<const EventStroke*>(&x));

	assert(event.stroke_data);

	get_work_area()->add_persistent_stroke(event.stroke_data,synfigapp::Main::get_outline_color());

	return Smach::RESULT_ACCEPT;
}
