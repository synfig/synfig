/* === S Y N F I G ========================================================= */
/*!	\file state_null.cpp
**	\brief Null State File
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

#include "state_null.h"
#include "workarea.h"
#include "event_mouse.h"
#include "event_layerclick.h"
#include "toolbox.h"
#include "dialog_tooloptions.h"
#include <gtkmm/dialog.h>
#include "widget_waypointmodel.h"
#include <synfig/valuenode_animated.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_const.h>
#include "canvasview.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateNull studio::state_null;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateNull_Context : public sigc::trackable
{
	CanvasView *canvas_view;

	CanvasView* get_canvas_view() { return canvas_view; }
	Canvas::Handle get_canvas() { return canvas_view->get_canvas(); }
	WorkArea* get_work_area() { return canvas_view->get_work_area(); }
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() { return canvas_view->canvas_interface(); }

	Gtk::Table options_table;

public:
	StateNull_Context(CanvasView *canvas_view);
	~StateNull_Context();

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_refresh_ducks_handler(const Smach::event& x);
	Smach::event_result event_undo_handler(const Smach::event& x);
	Smach::event_result event_redo_handler(const Smach::event& x);
	Smach::event_result event_mouse_button_down_handler(const Smach::event& x);
	Smach::event_result event_multiple_ducks_clicked_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_layer_click(const Smach::event& x);

	void refresh_tool_options();
}; // END of class StateNull_Context

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateNull::StateNull():
	Smach::state<StateNull_Context>("null")
{
	insert(event_def(EVENT_STOP,&StateNull_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateNull_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateNull_Context::event_refresh_ducks_handler));
	insert(event_def(EVENT_UNDO,&StateNull_Context::event_undo_handler));
	insert(event_def(EVENT_REDO,&StateNull_Context::event_redo_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateNull_Context::event_mouse_button_down_handler));
	insert(event_def(EVENT_WORKAREA_MULTIPLE_DUCKS_CLICKED,&StateNull_Context::event_multiple_ducks_clicked_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateNull_Context::event_refresh_tool_options));
	insert(event_def(EVENT_WORKAREA_LAYER_CLICKED,&StateNull_Context::event_layer_click));
}

StateNull::~StateNull()
{
}

StateNull_Context::StateNull_Context(CanvasView *canvas_view):
	canvas_view(canvas_view)
{
	// Synfig Studio's default state is initialized in the canvas view constructor
	// As a result, it cannot reference canvas view or workarea when created
	// Other states need to reference the workarea,
	//    so a null state was created to be the default

	options_table.attach(*manage(new Gtk::Label(_("Welcome to Synfig Studio"))),	0, 2,  0,  1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.show_all();
	refresh_tool_options();
}

StateNull_Context::~StateNull_Context()
{
}

void
StateNull_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Welcome to Synfig Studio"));
	App::dialog_tool_options->set_name("null");
}

Smach::event_result
StateNull_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNull_Context::event_stop_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NULL: Received Stop Event");
	canvas_view->stop();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNull_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NULL: Received Refresh Event");
	canvas_view->rebuild_tables();
	canvas_view->work_area->queue_render_preview();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNull_Context::event_refresh_ducks_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NULL: Received Refresh Ducks");
	canvas_view->queue_rebuild_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNull_Context::event_undo_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NULL: Received Undo Event");
	canvas_view->get_instance()->undo();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNull_Context::event_redo_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NULL: Received Redo Event");
	canvas_view->get_instance()->redo();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateNull_Context::event_mouse_button_down_handler(const Smach::event& x)
{
	// synfig::info("STATE NULL: Received mouse button down Event");

	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	switch(event.button)
	{
	case BUTTON_RIGHT:
		canvas_view->popup_main_menu();
		return Smach::RESULT_ACCEPT;
	default:
		return Smach::RESULT_OK;
	}
}

Smach::event_result
StateNull_Context::event_layer_click(const Smach::event& x)
{
	const EventLayerClick& event(*reinterpret_cast<const EventLayerClick*>(&x));

	if(event.layer)
	{
		// synfig::info("STATE NULL: Received layer click Event, \"%s\"",event.layer->get_name().c_str());
	}
	else
	{
		// synfig::info("STATE NULL: Received layer click Event with an empty layer.");
	}

	switch(event.button)
	{
	case BUTTON_LEFT:
		if(!(event.modifier&Gdk::CONTROL_MASK))
			canvas_view->get_selection_manager()->clear_selected_layers();
		if(event.layer)
		{
			std::list<Layer::Handle> layer_list(canvas_view->get_selection_manager()->get_selected_layers());
			std::set<Layer::Handle> layers(layer_list.begin(),layer_list.end());
			if(layers.count(event.layer))
			{
				layers.erase(event.layer);
				layer_list=std::list<Layer::Handle>(layers.begin(),layers.end());
				canvas_view->get_selection_manager()->clear_selected_layers();
				canvas_view->get_selection_manager()->set_selected_layers(layer_list);
			}
			else
			{
				canvas_view->get_selection_manager()->set_selected_layer(event.layer);
			}
		}
		return Smach::RESULT_ACCEPT;
	case BUTTON_RIGHT:
		canvas_view->popup_layer_menu(event.layer);
		return Smach::RESULT_ACCEPT;
	default:
		return Smach::RESULT_OK;
	}
}

/*
void
StateNull_Context::edit_several_waypoints(std::list<synfigapp::ValueDesc> value_desc_list)
{
	Gtk::Dialog dialog(
		"Edit Multiple Waypoints",		// Title
		true,		// Modal
		true		// use_separator
	);

	Widget_WaypointModel widget_waypoint_model;
	widget_waypoint_model.show();

	dialog.get_vbox()->pack_start(widget_waypoint_model);


	dialog.add_button(Gtk::StockID("gtk-apply"),1);
	dialog.add_button(Gtk::StockID("gtk-cancel"),0);
	dialog.show();

	if(dialog.run()==0)
		return;
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Set Waypoints"));

	std::list<synfigapp::ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		synfigapp::ValueDesc value_desc(*iter);

		if(!value_desc.is_valid())
			continue;

		ValueNode_Animated::Handle value_node;

		// If this value isn't a ValueNode_Animated, but
		// it is somewhat constant, then go ahead and convert
		// it to a ValueNode_Animated.
		if(!value_desc.is_value_node() || ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase value;
			if(value_desc.is_value_node())
				value=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node())->get_value();
			else
				value=value_desc.get_value();

			value_node=ValueNode_Animated::create(value,get_canvas()->get_time());

			synfigapp::Action::Handle action;

			if(!value_desc.is_value_node())
			{
				action=synfigapp::Action::create("ValueDescConnect");
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=synfigapp::Action::create("ValueNodeReplace");
				action->set_param("dest",value_desc.get_value_node());
				action->set_param("src",ValueNode::Handle(value_node));
			}

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());


			if(!get_canvas_interface()->get_instance()->perform_action(action))
			{
				get_canvas_view()->get_ui_interface()->error(_("Unable to convert to animated waypoint"));
				group.cancel();
				return;
			}
		}
		else
		{
			if(value_desc.is_value_node())
				value_node=ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node());
		}


		if(value_node)
		{

			synfigapp::Action::Handle action(synfigapp::Action::create("WaypointSetSmart"));

			if(!action)
			{
				get_canvas_view()->get_ui_interface()->error(_("Unable to find WaypointSetSmart action"));
				group.cancel();
				return;
			}


			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("value_node",ValueNode::Handle(value_node));
			action->set_param("time",get_canvas()->get_time());
			action->set_param("model",widget_waypoint_model.get_waypoint_model());

			if(!get_canvas_interface()->get_instance()->perform_action(action))
			{
				get_canvas_view()->get_ui_interface()->error(_("Unable to set a specific waypoint"));
				group.cancel();
				return;
			}
		}
		else
		{
			//get_canvas_view()->get_ui_interface()->error(_("Unable to animate a specific valuedesc"));
			//group.cancel();
			//return;
		}

	}
}
*/

Smach::event_result
StateNull_Context::event_multiple_ducks_clicked_handler(const Smach::event& /*x*/)
{
	// synfig::info("STATE NULL: Received multiple duck click event");

	//const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	std::list<synfigapp::ValueDesc> value_desc_list;

	// Create a list of value_descs associated with selection
	const DuckList selected_ducks(get_work_area()->get_selected_ducks());
	DuckList::const_iterator iter;
	for(iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		synfigapp::ValueDesc value_desc((*iter)->get_value_desc());

		if(!value_desc.is_valid())
			continue;

		if(value_desc.get_value_type()==ValueBase::TYPE_BLINEPOINT && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			value_desc_list.push_back(
				synfigapp::ValueDesc(
					ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
					,ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
                                                               ->get_link_index_from_name("point")
				)
			);
		}
		else
			value_desc_list.push_back(value_desc);
	}

	Gtk::Menu *menu=manage(new Gtk::Menu());
	menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));

	canvas_view->get_instance()->make_param_menu(menu,canvas_view->get_canvas(),value_desc_list);

	/*
	synfigapp::Action::ParamList param_list;
	param_list=get_canvas_interface()->generate_param_list(value_desc_list);

	canvas_view->add_actions_to_menu(menu, param_list,synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE);

	menu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Edit Waypoints"),
		sigc::bind(
			sigc::mem_fun(
				*this,
				&studio::StateNull_Context::edit_several_waypoints
			),
			value_desc_list
		)
	));
	*/
	menu->popup(3,gtk_get_current_event_time());

	return Smach::RESULT_ACCEPT;
}

