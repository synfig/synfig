/* === S Y N F I G ========================================================= */
/*!	\file state_smoothmove.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
**  Copyright (c) 2010 Carlos López
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/states/state_smoothmove.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/duck.h>
#include <gui/localization.h>
#include <gui/onemoment.h>
#include <gui/states/state_normal.h>
#include <gui/workarea.h>

#include <synfig/general.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateSmoothMove studio::state_smooth_move;

/* === C L A S S E S & S T R U C T S ======================================= */

class DuckDrag_SmoothMove : public DuckDrag_Base
{
	float radius;

	synfig::Vector last_translate_;
	synfig::Vector drag_offset_;
	synfig::Vector snap;

	std::vector<synfig::Vector> last_;
	std::vector<synfig::Vector> positions;

public:
	DuckDrag_SmoothMove();
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);

	void set_radius(float x) { radius=x; }
	float get_radius()const { return radius; }
};


class studio::StateSmoothMove_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	//Duckmatic::Push duckmatic_push;

	synfigapp::Settings& settings;

	etl::handle<DuckDrag_SmoothMove> duck_dragger_;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	Glib::RefPtr<Gtk::Adjustment> adj_radius;
	Gtk::SpinButton  spin_radius;
	Gtk::Label spin_label;

	float pressure;

public:
	float get_radius()const { return adj_radius->get_value(); }
	void set_radius(float x) { return adj_radius->set_value(x); }

	void refresh_radius() { duck_dragger_->set_radius(get_radius()*pressure); }

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	void refresh_tool_options();

	StateSmoothMove_Context(CanvasView* canvas_view);

	~StateSmoothMove_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	void load_settings();
	void save_settings();
};	// END of class StateSmoothMove_Context

/* === M E T H O D S ======================================================= */

StateSmoothMove::StateSmoothMove():
	Smach::state<StateSmoothMove_Context>("smooth_move")
{
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateSmoothMove_Context::event_refresh_tool_options));
	insert(event_def(EVENT_STOP,&StateSmoothMove_Context::event_stop_handler));
}

StateSmoothMove::~StateSmoothMove()
{
}

void* StateSmoothMove::enter_state(studio::CanvasView* machine_context) const
{
	return new StateSmoothMove_Context(machine_context);
}

void
StateSmoothMove_Context::load_settings()
{
	try
	{
		set_radius(settings.get_value("smooth_move.radius", 1.0));
	}
	catch(...)
	{
		synfig::warning("State SmothMove: Caught exception when attempting to load settings.");
	}
}

void
StateSmoothMove_Context::save_settings()
{
	try
	{
		settings.set_value("smooth_move.radius",double(get_radius()));
	}
	catch(...)
	{
		synfig::warning("State SmoothMove: Caught exception when attempting to save settings.");
	}
}

StateSmoothMove_Context::StateSmoothMove_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
//	duckmatic_push(get_work_area()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	duck_dragger_(new DuckDrag_SmoothMove()),
	adj_radius(Gtk::Adjustment::create(1,0,100000,0.01,0.1)),
	spin_radius(adj_radius,0.1,3)
{
	pressure=1.0f;

	// Toolbox widgets
	title_label.set_label(_("SmoothMove Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	spin_label.set_label(_("Radius:"));
	spin_label.set_halign(Gtk::ALIGN_START);
	spin_label.set_valign(Gtk::ALIGN_CENTER);

	spin_radius.set_halign(Gtk::ALIGN_END);
	spin_radius.set_valign(Gtk::ALIGN_CENTER);

	load_settings();

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 2, 1);
	options_grid.attach(spin_label,
		0, 1, 1, 1);
	options_grid.attach(spin_radius,
		1, 1, 1, 1);

	spin_radius.signal_value_changed().connect(sigc::mem_fun(*this,&StateSmoothMove_Context::refresh_radius));

	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.set_margin_bottom(0);
	options_grid.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	get_work_area()->set_allow_layer_clicks(true);
	get_work_area()->set_duck_dragger(duck_dragger_);

	App::dock_toolbox->refresh();

	get_work_area()->set_cursor(Gdk::FLEUR);
	//get_work_area()->reset_cursor();
}

void
StateSmoothMove_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Smooth Move"));
	App::dialog_tool_options->set_icon("tool_smooth_move_icon");
}

Smach::event_result
StateSmoothMove_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateSmoothMove_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

StateSmoothMove_Context::~StateSmoothMove_Context()
{
	save_settings();

	get_work_area()->clear_duck_dragger();
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	App::dock_toolbox->refresh();
}




DuckDrag_SmoothMove::DuckDrag_SmoothMove():radius(1.0f)
{
}

void
DuckDrag_SmoothMove::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	last_translate_=Vector(0,0);
		drag_offset_=duckmatic->find_duck(offset)->get_trans_point();

		//snap=drag_offset-duckmatic->snap_point_to_grid(drag_offset);
		//snap=offset-drag_offset_;
		snap=Vector(0,0);

	last_.clear();
	positions.clear();
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;
	int i;
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		last_.push_back(Vector(0,0));
		positions.push_back((*iter)->get_trans_point());
	}
}

void
DuckDrag_SmoothMove::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;
	synfig::Vector vect(duckmatic->snap_point_to_grid(vector)-drag_offset_+snap);

	int i;

	Time time(duckmatic->get_time());

	// process vertex and position ducks first
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		// skip this duck if it is NOT a vertex or a position
		if (((*iter)->get_type() != Duck::TYPE_VERTEX &&
			 (*iter)->get_type() != Duck::TYPE_POSITION))
			continue;
		Point p(positions[i]);

		float dist(1.0f-(p-drag_offset_).mag()/get_radius());
		if(dist<0)
			dist=0;

		last_[i]=vect*dist;
		(*iter)->set_trans_point(p+last_[i], time);
	}

	// then process non vertex and non position ducks
	for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
	{
		// skip this duck if it IS a vertex or a position
		if (!((*iter)->get_type() != Duck::TYPE_VERTEX &&
			 (*iter)->get_type() != Duck::TYPE_POSITION))
			continue;
		Point p(positions[i]);

		float dist(1.0f-(p-drag_offset_).mag()/get_radius());
		if(dist<0)
			dist=0;

		last_[i]=vect*dist;
		(*iter)->set_trans_point(p+last_[i], time);
	}

	// then patch up the tangents for the vertices we've moved
	duckmatic->update_ducks();

	last_translate_=vect;
	//snap=Vector(0,0);
}

bool
DuckDrag_SmoothMove::end_duck_drag(Duckmatic* duckmatic)
{
	//synfig::info("end_duck_drag(): Diff= %f",last_translate_.mag());
	if(last_translate_.mag()>0.0001)
	{
		const DuckList selected_ducks(duckmatic->get_selected_ducks());
		DuckList::const_iterator iter;

		int i;

		smart_ptr<OneMoment> wait;if(selected_ducks.size()>20)wait.spawn();

		for(i=0,iter=selected_ducks.begin();iter!=selected_ducks.end();++iter,i++)
		{
			if(last_[i].mag()>0.0001)
				{
				if ((*iter)->get_type() == Duck::TYPE_ANGLE)
					{
						if(!(*iter)->signal_edited()(**iter))
						{
							throw String("Bad edit");
						}
					}
					else if (App::restrict_radius_ducks &&
							 (*iter)->is_radius())
					{
						Point point((*iter)->get_point());
						bool changed = false;

						if (point[0] < 0)
						{
							point[0] = 0;
							changed = true;
						}
						if (point[1] < 0)
						{
							point[1] = 0;
							changed = true;
						}

						if (changed) (*iter)->set_point(point);

						if(!(*iter)->signal_edited()(**iter))
						{
							throw String("Bad edit");
						}
					}
					else
					{
						if(!(*iter)->signal_edited()(**iter))
						{
							throw String("Bad edit");
						}
					}
				}
		}
		//duckmatic->get_selected_ducks()=new_set;
		//duckmatic->refresh_selected_ducks();
		return true;
	}
	else
	{
		duckmatic->signal_user_click_selected_ducks(0);
		return false;
	}
}
