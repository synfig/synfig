/* === S I N F G =========================================================== */
/*!	\file state_mirror.cpp
**	\brief Template File
**
**	$Id: state_mirror.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <sinfg/valuenode_dynamiclist.h>
#include <sinfgapp/action_system.h>

#include "state_mirror.h"
#include "../canvasview.h"
#include "../workarea.h"
#include "../app.h"

#include <sinfgapp/action.h>
#include "../event_mouse.h"
#include "../event_layerclick.h"
#include "../toolbox.h"
#include "../dialog_tooloptions.h"
#include <gtkmm/optionmenu.h>
#include "../duck.h"
#include <sinfgapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

enum Axis {
	AXIS_X,
	AXIS_Y
} ;

/* === G L O B A L S ======================================================= */

StateMirror studio::state_mirror;

/* === C L A S S E S & S T R U C T S ======================================= */

class DuckDrag_Mirror : public DuckDrag_Base
{
	sinfg::Vector center;

	std::vector<sinfg::Vector> positions;
	
	
public:
	Axis axis;

	DuckDrag_Mirror();
	void begin_duck_drag(Duckmatic* duckmatic, const sinfg::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const sinfg::Vector& vector);
};


class studio::StateMirror_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
		
	sinfgapp::Settings& settings;

	etl::handle<DuckDrag_Mirror> duck_dragger_;

	Gtk::Table options_table;
	
	
	Gtk::CheckButton checkbutton_axis_x;
	Gtk::CheckButton checkbutton_axis_y;
	
public:

	Axis get_axis()const { return checkbutton_axis_x.get_active()?AXIS_X:AXIS_Y; }
	void set_axis(Axis a)
	{
		if(a==AXIS_X)
		{
			checkbutton_axis_x.set_active(true);
			checkbutton_axis_y.set_active(false);
		}
		else
		{
			checkbutton_axis_y.set_active(true);
			checkbutton_axis_x.set_active(false);
		}
			
		duck_dragger_->axis=get_axis();
	}
	
	void update_axis_y()
	{
		checkbutton_axis_x.set_active(!checkbutton_axis_y.get_active());
		duck_dragger_->axis=get_axis();
	}
	void update_axis_x()
	{
		checkbutton_axis_y.set_active(!checkbutton_axis_x.get_active());
		duck_dragger_->axis=get_axis();
	}
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();

	StateMirror_Context(CanvasView* canvas_view);

	~StateMirror_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<sinfgapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	sinfg::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}
	
	void load_settings();
	void save_settings();
};	// END of class StateMirror_Context

/* === M E T H O D S ======================================================= */

StateMirror::StateMirror():
	Smach::state<StateMirror_Context>("mirror")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateMirror_Context::event_refresh_tool_options));
}	

StateMirror::~StateMirror()
{
}

void
StateMirror_Context::load_settings()
{	
	String value;

	settings.get_value("mirror.axis",value);
	set_axis((Axis)atoi(value.c_str()));
}

void
StateMirror_Context::save_settings()
{	
	settings.set_value("mirror.lock_aspect",strprintf("%d",(int)get_axis()));
}

StateMirror_Context::StateMirror_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	settings(sinfgapp::Main::get_selected_input_device()->settings()),
	duck_dragger_(new DuckDrag_Mirror()),
	checkbutton_axis_x(_("Horizontal")),
	checkbutton_axis_y(_("Vertical"))
{	
	// Set up the tool options dialog
	options_table.attach(*manage(new Gtk::Label(_("Mirror Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	options_table.attach(checkbutton_axis_x, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	options_table.attach(checkbutton_axis_y, 0, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	checkbutton_axis_x.signal_toggled().connect(sigc::mem_fun(*this,&StateMirror_Context::update_axis_x));
	checkbutton_axis_y.signal_toggled().connect(sigc::mem_fun(*this,&StateMirror_Context::update_axis_y));
		
	options_table.show_all();
	refresh_tool_options();
	App::dialog_tool_options->present();
	
	get_work_area()->allow_layer_clicks=true;
	get_work_area()->set_duck_dragger(duck_dragger_);

//	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);
	get_canvas_view()->work_area->reset_cursor();

	App::toolbox->refresh();

	set_axis(AXIS_X);
	load_settings();
}

void
StateMirror_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Mirror Tool"));
	App::dialog_tool_options->set_name("mirror");
}

Smach::event_result
StateMirror_Context::event_refresh_tool_options(const Smach::event& x)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateMirror_Context::~StateMirror_Context()
{
	save_settings();

	get_work_area()->clear_duck_dragger();
	get_canvas_view()->work_area->reset_cursor();

	App::dialog_tool_options->clear();

	App::toolbox->refresh();
}




DuckDrag_Mirror::DuckDrag_Mirror():
	axis(AXIS_X)
{
}

#ifndef EPSILON
#define EPSILON	0.0000001
#endif

void
DuckDrag_Mirror::begin_duck_drag(Duckmatic* duckmatic, const sinfg::Vector& offset)
{


	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	positions.clear();
	int i;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		Point p((*iter)->get_trans_point());
		positions.push_back(p);
	}

}


void
DuckDrag_Mirror::duck_drag(Duckmatic* duckmatic, const sinfg::Vector& vector)
{
	center=vector;
	int i;
	
		const DuckList selected_ducks(duckmatic->get_selected_ducks());
		DuckList::const_iterator iter;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if(((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

		Vector p(positions[i]);
		//Point p((*iter)->get_trans_point());
		
		if(axis==AXIS_X)
			p[0]=-(p[0]-center[0])+center[0];
		if(axis==AXIS_Y)
			p[1]=-(p[1]-center[1])+center[1];
		
		(*iter)->set_trans_point(p);
	}
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		if(!((*iter)->get_type()!=Duck::TYPE_VERTEX&&(*iter)->get_type()!=Duck::TYPE_POSITION))continue;

		Vector p(positions[i]);
		//Point p((*iter)->get_trans_point());
		
		if(axis==AXIS_X)
			p[0]=-(p[0]-center[0])+center[0];
		if(axis==AXIS_Y)
			p[1]=-(p[1]-center[1])+center[1];
		
		(*iter)->set_trans_point(p);
	}
}

bool
DuckDrag_Mirror::end_duck_drag(Duckmatic* duckmatic)
{
	duckmatic->signal_edited_selected_ducks();
	return true;
}
