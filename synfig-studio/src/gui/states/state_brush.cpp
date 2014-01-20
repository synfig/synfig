/* === S Y N F I G ========================================================= */
/*!	\file state_brush.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <glibmm/timeval.h>

#include "state_brush.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"

#include <synfigapp/blineconvert.h>
#include <synfigapp/wplistconverter.h>
#include <synfigapp/main.h>

#include <ETL/gaussian>
#include "docks/dialog_tooloptions.h"

#include "general.h"

#include "brush.h"

#include <synfig/layer_bitmap.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateBrush studio::state_brush;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBrush_Context : public sigc::trackable
{
	typedef etl::smart_ptr<std::list<synfig::Point> > StrokeData;
	typedef etl::smart_ptr<std::list<synfig::Real> > WidthData;
	typedef Layer_Bitmap TargetLayer;
	typedef etl::handle<TargetLayer> TargetLayerHandle;

	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	WorkArea::PushState push_state;

	bool prev_table_status;

	Gtk::Menu menu;

	Glib::TimeVal time;
	brush::Brush br;
	brush::SurfaceWrapper wrapper;
	TargetLayerHandle layer;

	void refresh_ducks();

	synfigapp::Settings& settings;

public:
	void load_settings();
	void save_settings();

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_down_handler(const Smach::event& x);
	Smach::event_result event_mouse_up_handler(const Smach::event& x);
	Smach::event_result event_mouse_draw_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();
	Smach::event_result process_stroke(TargetLayerHandle layer, StrokeData stroke_data, WidthData width_data, bool region_flag=false);

	StateBrush_Context(CanvasView* canvas_view);
	~StateBrush_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Time get_time()const { return get_canvas_interface()->get_time(); }
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}
};	// END of class StateBrush_Context


/* === M E T H O D S ======================================================= */

StateBrush::StateBrush():
	Smach::state<StateBrush_Context>("brush")
{
	insert(event_def(EVENT_STOP,&StateBrush_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateBrush_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateBrush_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateBrush_Context::event_mouse_up_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateBrush_Context::event_mouse_draw_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateBrush_Context::event_refresh_tool_options));
}

StateBrush::~StateBrush()
{
}


void
StateBrush_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		String value;

		// TODO: load something
	}
	catch(...)
	{
		synfig::warning("State Brush: Caught exception when attempting to load settings.");
	}
}

void
StateBrush_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");

		// TODO: save something
	}
	catch(...)
	{
		synfig::warning("State Brush: Caught exception when attempting to save settings.");
	}
}

StateBrush_Context::StateBrush_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	push_state(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings())
{
	load_settings();

	// configure brush
	br.set_base_value(BRUSH_OPAQUE, 					 0.85f );
	br.set_mapping_n(BRUSH_OPAQUE_MULTIPLY, INPUT_PRESSURE, 4);
	br.set_mapping_point(BRUSH_OPAQUE_MULTIPLY, INPUT_PRESSURE, 0, 0.f, 0.f);
	br.set_mapping_point(BRUSH_OPAQUE_MULTIPLY, INPUT_PRESSURE, 1, 0.222222f, 0.f);
	br.set_mapping_point(BRUSH_OPAQUE_MULTIPLY, INPUT_PRESSURE, 2, 0.324074f, 1.f);
	br.set_mapping_point(BRUSH_OPAQUE_MULTIPLY, INPUT_PRESSURE, 2, 1.f, 1.f);
	br.set_base_value(BRUSH_OPAQUE_LINEARIZE,			 0.9f  );
	br.set_base_value(BRUSH_RADIUS_LOGARITHMIC,			 2.6f  );
	br.set_base_value(BRUSH_HARDNESS,					 0.69f );
	br.set_base_value(BRUSH_DABS_PER_ACTUAL_RADIUS,		 6.0f  );
	br.set_base_value(BRUSH_DABS_PER_SECOND, 			54.45f );
	br.set_base_value(BRUSH_SPEED1_SLOWNESS, 			 0.04f );
	br.set_base_value(BRUSH_SPEED2_SLOWNESS, 			 0.08f );
	br.set_base_value(BRUSH_SPEED1_GAMMA, 				 4.0f  );
	br.set_base_value(BRUSH_SPEED2_GAMMA, 				 4.0f  );
	br.set_base_value(BRUSH_OFFSET_BY_SPEED_SLOWNESS,	 1.0f  );
	br.set_base_value(BRUSH_SMUDGE,						 0.9f  );
	br.set_base_value(BRUSH_SMUDGE_LENGTH,				 0.12f );
	br.set_base_value(BRUSH_STROKE_DURATION_LOGARITHMIC, 4.0f  );

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Hide all tangent and width ducks
	get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Turn off duck clicking
	get_work_area()->set_allow_duck_clicks(false);

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);
	get_work_area()->set_cursor(Gdk::PENCIL);

	App::dock_toolbox->refresh();
	refresh_ducks();
}

void
StateBrush_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	// TODO: add some widget
	App::dialog_tool_options->set_local_name(_("Brush Tool"));
	App::dialog_tool_options->set_name("brush");
}

Smach::event_result
StateBrush_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateBrush_Context::~StateBrush_Context()
{
	save_settings();

	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateBrush_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateBrush_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBrush_Context::event_mouse_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			// Enter the stroke state to get the stroke
			layer = TargetLayerHandle::cast_dynamic( canvas_view_->get_selection_manager()->get_selected_layer() );
			if (layer)
			{
				time.assign_current_time();
				wrapper.surface = &layer->surface;
				br.reset();
				return Smach::RESULT_ACCEPT;
			}
			break;
		}

	default:
		break;
	}
	return Smach::RESULT_OK;
}

Smach::event_result
StateBrush_Context::event_mouse_up_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			if (layer)
			{
				wrapper.surface = NULL;
				layer = NULL;
				return Smach::RESULT_ACCEPT;
			}
			break;
		}

	default:
		break;
	}

	return Smach::RESULT_OK;
}

Smach::event_result
StateBrush_Context::event_mouse_draw_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			if (layer)
			{
				Glib::TimeVal prev_time = time;
				time.assign_current_time();
				double delta_time = (time - prev_time).as_double();
				Rect r = layer->get_bounding_rect();
				br.stroke_to(
					&wrapper,
					(event.pos[0] - r.minx)/(r.maxx - r.minx)*wrapper.surface->get_w(),
					(1.0 - (event.pos[1] - r.miny)/(r.maxy - r.miny))*wrapper.surface->get_h(),
					event.pressure,
					0.f, 0.f,
					delta_time );
				layer->changed();
				return Smach::RESULT_ACCEPT;
			}
			break;
		}

	default:
		break;
	}

	return Smach::RESULT_OK;
}

void
StateBrush_Context::refresh_ducks()
{
	get_canvas_view()->queue_rebuild_ducks();
}
