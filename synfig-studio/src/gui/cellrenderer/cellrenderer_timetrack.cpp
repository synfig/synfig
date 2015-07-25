/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_timetrack.cpp
**	\brief Cell renderer for the timetrack. Render all time points (waypoints / keyframes and current time line ...)
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

#include <gtkmm/label.h>
#include "cellrenderer_timetrack.h"
#include <gtk/gtk.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/combobox.h>
#include <ETL/stringf>
#include "widgets/widget_value.h"
#include "app.h"
#include <gtkmm/menu.h>
#include "widgets/widget_time.h"
#include "widgets/widget_timeslider.h"

#include <synfigapp/canvasinterface.h>
#include "instance.h"

#include "general.h"

#include <synfig/layers/layer_pastecanvas.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

//mode for modifier keys
enum MODMODE
{
	NONE = 0,
	SELECT_MASK = Gdk::CONTROL_MASK,
	COPY_MASK = Gdk::SHIFT_MASK,
	DELETE_MASK = Gdk::MOD1_MASK
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CellRenderer_TimeTrack::CellRenderer_TimeTrack():
	Glib::ObjectBase	(typeid(CellRenderer_TimeTrack)),
	Gtk::CellRenderer	(),
	adjustment_			(Gtk::Adjustment::create(10,10,20,0,0,0)),
	mode				(),
	selection			(false),
	dragging			(false),
	property_valuedesc_	(*this,"value_desc",synfigapp::ValueDesc()),
	property_canvas_	(*this,"canvas",synfig::Canvas::Handle()),
	property_adjustment_(*this,"adjustment",adjustment_),
	property_enable_timing_info_(*this,"enable-timing-info", false)
{ }

CellRenderer_TimeTrack::~CellRenderer_TimeTrack()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("CellRenderer_TimeTrack::~CellRenderer_TimeTrack(): Deleted");
}

void
CellRenderer_TimeTrack::set_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x)
{
	property_adjustment_=x;
//	x.signal_value_changed().connect(sigc::mem_fun(*this,&Gtk::Widget::queue_draw));
}

synfig::Canvas::Handle
CellRenderer_TimeTrack::get_canvas()const
{
	return const_cast<CellRenderer_TimeTrack*>(this)->property_canvas().get_value();
}

Glib::RefPtr<Gtk::Adjustment>
CellRenderer_TimeTrack::get_adjustment() const
{
	return (Glib::RefPtr<Gtk::Adjustment>)property_adjustment_;
}

bool
CellRenderer_TimeTrack::is_selected(const Waypoint& waypoint)const
{
	return selected==waypoint;
}

const synfig::Time get_time_dilation_from_vdesc(const synfigapp::ValueDesc &v)
{
#ifdef ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	if(getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS") ||
	   v.get_value_type() != synfig::type_canvas)
		return synfig::Time(1.0);

	synfig::Canvas::Handle canvasparam = v.get_value().get(Canvas::Handle());
	if(!canvasparam)
		return synfig::Time(1.0);

	if (!v.parent_is_layer())
		return synfig::Time(1.0);

	synfig::Layer::Handle layer = v.get_layer();

	if (!etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
		return synfig::Time(1.0);

	return layer->get_param("time_dilation").get(Time());
#else // ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	return synfig::Time::zero();
#endif
}

const synfig::Time get_time_offset_from_vdesc(const synfigapp::ValueDesc &v)
{
#ifdef ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	if(getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS") ||
	   v.get_value_type() != synfig::type_canvas)
		return synfig::Time::zero();

	synfig::Canvas::Handle canvasparam = v.get_value().get(Canvas::Handle());
	if(!canvasparam)
		return synfig::Time::zero();

	if (!v.parent_is_layer())
		return synfig::Time::zero();

	synfig::Layer::Handle layer = v.get_layer();

	if (!etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
		return synfig::Time::zero();

	return layer->get_param("time_offset").get(Time());
#else // ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	return synfig::Time::zero();
#endif
}

//kind of a hack... pointer is ugly
const synfig::Node::time_set *get_times_from_vdesc(const synfigapp::ValueDesc &v)
{
	if(!getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS") &&
	   v.get_value_type() == synfig::type_canvas)
	{
		synfig::Canvas::Handle canvasparam = v.get_value().get(Canvas::Handle());

		if(canvasparam)
			return &canvasparam->get_times();
	}

	ValueNode *base_value = v.get_value_node().get();

	ValueNode_DynamicList *parent_value_node =
			v.parent_is_value_node() ?
				dynamic_cast<ValueNode_DynamicList *>(v.get_parent_value_node().get()) :
				0;

	//we want a dynamic list entry to override the normal...
	if(parent_value_node)
	{
		return &parent_value_node->list[v.get_index()].get_times();
	}else if(base_value) //don't render stuff if it's just animated...
	{
		return &base_value->get_times();
	}
	return 0;
}

bool get_closest_time(const synfig::Node::time_set &tset, const Time &t, const Time &range, Time &out)
{
	Node::time_set::const_iterator	i,j,end = tset.end();

	// stop the crash mentioned in bug #1689282
	// doesn't solve the underlying problem though, I don't think
	if (tset.size() == 0)
	{
		synfig::error(__FILE__":%d: tset.size() == 0",__LINE__);
		return false;
	}

	//TODO add in RangeGet so it's not so damn hard to click on points

	i = tset.upper_bound(t); //where t is the lower bound, t < [first,i)
	j = i; --j;

	double dist = Time::end();
	double closest = 0;

	if(i != end)
	{
		closest = i->get_time();
		dist = abs(i->get_time() - t);
	}

	if(j != end && (abs(j->get_time() - t) < dist) )
	{
		closest = j->get_time();
		dist = abs(j->get_time() - t);
	}

	if( dist <= range/2 )
	{
		out = closest;
		return true;
	}

	return false;
}

void
CellRenderer_TimeTrack::render_vfunc(
	const ::Cairo::RefPtr< ::Cairo::Context>& cr,
	Gtk::Widget& /* widget */,
	const Gdk::Rectangle& /* background_area */,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState /* flags */)
{
	if(!cr)
		return;

	Glib::RefPtr<Gtk::Adjustment> adjustment=get_adjustment();
	// Gtk::StateType state = Gtk::STATE_ACTIVE;
	// Gtk::ShadowType shadow;

	Gdk::Color
		curr_time_color("#0000ff"),
		inactive_color("#000000"),
		keyframe_color("#a07f7f");
	Gdk::Color activepoint_color[2];

	activepoint_color[0]=Gdk::Color("#ff0000");
	activepoint_color[1]=Gdk::Color("#00ff00");

	synfig::Canvas::Handle canvas(property_canvas().get_value());

	synfigapp::ValueDesc value_desc = property_value_desc().get_value();
	synfig::ValueNode *base_value = value_desc.get_value_node().get();
	// synfig::ValueNode_Animated *value_node=dynamic_cast<synfig::ValueNode_Animated*>(base_value);

	synfig::ValueNode_DynamicList *parent_value_node(0);
	if(property_value_desc().get_value().parent_is_value_node())
		parent_value_node=dynamic_cast<synfig::ValueNode_DynamicList*>(property_value_desc().get_value().get_parent_value_node().get());

	// If the canvas is defined, then load up the keyframes
	if(canvas)
	{
		const synfig::KeyframeList& keyframe_list(canvas->keyframe_list());
		synfig::KeyframeList::const_iterator iter;

		for(iter=keyframe_list.begin();iter!=keyframe_list.end();++iter)
		{
			if(!iter->get_time().is_valid())
				continue;

			const int x((int)((float)cell_area.get_width()/(adjustment->get_upper()-adjustment->get_lower())*(iter->get_time()-adjustment->get_lower())));
			if(iter->get_time()>=adjustment->get_lower() && iter->get_time()<adjustment->get_upper())
			{
				cr->set_source_rgb(keyframe_color.get_red_p(), keyframe_color.get_green_p(), keyframe_color.get_blue_p());
				cr->rectangle(cell_area.get_x()+x, cell_area.get_y(), 1, cell_area.get_height()+1);
				cr->fill();
			}
		}
	}

	//render all the time points that exist
	{
		const synfig::Node::time_set *tset = get_times_from_vdesc(value_desc);

		if(tset)
		{
			const synfig::Time time_offset = get_time_offset_from_vdesc(value_desc);
			const synfig::Time time_dilation = get_time_dilation_from_vdesc(value_desc);
			synfig::Node::time_set::const_iterator	i = tset->begin(), end = tset->end();

			float 	lower = adjustment->get_lower(),
					upper = adjustment->get_upper();

			Gdk::Rectangle area(cell_area);

			bool valselected = sel_value.get_value_node() == base_value && !sel_times.empty();

			float cfps = get_canvas()->rend_desc().get_frame_rate();

			vector<Time>	drawredafter;

			Time diff = actual_time - actual_dragtime;//selected_time-drag_time;
			for(; i != end; ++i)
			{
				//find the coordinate in the drawable space...
				Time t_orig = i->get_time();
				if(!t_orig.is_valid()) continue;
				Time t = t_orig - time_offset;
				if (time_dilation!=0)
					t = t / time_dilation;
				if(t<adjustment->get_lower() || t>adjustment->get_upper()) continue;

				//if it found it... (might want to change comparison, and optimize
				//					 sel_times.find to not produce an overall nlogn solution)

				bool selected=false;
				//not dragging... just draw as per normal
				//if move dragging draw offset
				//if copy dragging draw both...

				if(valselected && sel_times.find(t_orig) != sel_times.end())
				{
					if(dragging) //skip if we're dragging because we'll render it later
					{
						if(mode & COPY_MASK) // draw both blue and red moved
						{
							drawredafter.push_back(t + diff.round(cfps));
						}else if(mode & DELETE_MASK) //it's just red...
						{
							selected=true;
						}else //move - draw the red on top of the others...
						{
							drawredafter.push_back(t + diff.round(cfps));
							continue;
						}
					}else
					{
						selected=true;
					}
				}

				//synfig::info("Displaying time: %.3f s",(float)t);
				const int x = (int)((t-lower)*area.get_width()/(upper-lower));

				//should draw me a grey filled circle...
				Gdk::Rectangle area2(
					area.get_x() - area.get_height()/2 + x + 1,
					area.get_y() + 1,
					area.get_height()-2,
					area.get_height()-2
				);
				if (time_dilation!=0)
				{
					TimePoint tp = *i;
					tp.set_time((tp.get_time() - time_offset) / time_dilation);
					render_time_point_to_window(cr,area2,tp,selected);
				}
			}

			{
				vector<Time>::iterator i = drawredafter.begin(), end = drawredafter.end();
				for(; i != end; ++i)
				{
					//find the coordinate in the drawable space...
					Time t = *i;

					if(!t.is_valid())
						continue;

					//synfig::info("Displaying time: %.3f s",(float)t);
					const int x = (int)((t-lower)*area.get_width()/(upper-lower));

					//should draw me a grey filled circle...

					Gdk::Rectangle area2(
						area.get_x() - area.get_height()/2 + x + 1,
						area.get_y() + 1,
						area.get_height()-2,
						area.get_height()-2
					);
					render_time_point_to_window(cr,area2,*i,true);
				}
			}
		}
	}

	Gdk::Rectangle area(cell_area);
	// If the parent of this value node is a dynamic list, then
	// render the on and off times
	if(parent_value_node)
	{
		const int index(property_value_desc().get_value().get_index());
		const synfig::ValueNode_DynamicList::ListEntry& list_entry(parent_value_node->list[index]);
		const synfig::ValueNode_DynamicList::ListEntry::ActivepointList& activepoint_list(list_entry.timing_info);
		synfig::ValueNode_DynamicList::ListEntry::ActivepointList::const_iterator iter,next;

		bool is_off(false);
		if(!activepoint_list.empty())
			is_off=!activepoint_list.front().state;

		int xstart(0);

		int x=0 /*,prevx=0*/;
		for(next=activepoint_list.begin(),iter=next++;iter!=activepoint_list.end();iter=next++)
		{
			x=((int)((float)area.get_width()/(adjustment->get_upper()-adjustment->get_lower())*(iter->time-adjustment->get_lower())));
			if(x<0)x=0;
			if(x>area.get_width())x=area.get_width();

			bool status_at_time=0;
			if(next!=activepoint_list.end())
			{
				status_at_time=!list_entry.status_at_time((iter->time+next->time)/2.0);
			}
			else
				status_at_time=!list_entry.status_at_time(Time::end());

			if(!is_off && status_at_time)
			{
				xstart=x;
				is_off=true;
			}
			else
			if(is_off && !status_at_time)
			{
				// render the off time has a dashed line
				draw_activepoint_off(cr, inactive_color, area.get_height()*2,
														area.get_x()+xstart,
														area.get_y(),
														x-xstart,
														area.get_y());

				is_off=false;
			}

			if(iter->time>=adjustment->get_lower() && iter->time<adjustment->get_upper())
			{
				int w(1);
				if(selected==*iter)
					w=3;
				cr->set_source_rgb( activepoint_color[iter->state].get_red_p(),
									activepoint_color[iter->state].get_green_p(),
									activepoint_color[iter->state].get_red_p() );
				cr->rectangle(area.get_x()+x-w/2, area.get_y(), w, area.get_height());
				cr->fill();
			}
			//prevx=x;
		}
		if(is_off)
		{
			// render the off time has a dashed line
			draw_activepoint_off(cr, inactive_color, area.get_height()*2,
													area.get_x()+xstart,
													area.get_y(),
													area.get_width()-xstart,
													area.get_y());
		}
	}

	// Render a line that defines the current tick in time
	{
		const int x((int)((float)area.get_width()/(adjustment->get_upper()-adjustment->get_lower())*(adjustment->get_value()-adjustment->get_lower())));
		if(adjustment->get_value()>=adjustment->get_lower() && adjustment->get_value()<adjustment->get_upper())
		{
			cr->set_source_rgb( curr_time_color.get_red_p(),
								curr_time_color.get_green_p(),
								curr_time_color.get_red_p() );
			cr->rectangle(area.get_x()+x, area.get_y(), 1, area.get_height());
			cr->fill();
		}
	}
}

void CellRenderer_TimeTrack::draw_activepoint_off(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		Gdk::Color inactive_color,
		int line_width,
		int from_x,
		int from_y,
		int to_x,
		int to_y)
{
	std::valarray< double > activepoint_off_dashes(2);
	activepoint_off_dashes[0] = 1.0;
	activepoint_off_dashes[1] = 2.0;

	cr->set_dash (activepoint_off_dashes, 0.0);
	cr->set_source_rgb( inactive_color.get_red_p(),
						inactive_color.get_green_p(),
						inactive_color.get_red_p() );

	cr->set_line_width(line_width);
	cr->move_to(from_x, from_y);
	cr->line_to(to_y, to_y);
	cr->stroke();

}

synfig::ValueNode_Animated::WaypointList::iterator
CellRenderer_TimeTrack::find_editable_waypoint(const synfig::Time& /*t*/,const synfig::Time& scope)
{
	synfig::ValueNode_Animated *value_node=dynamic_cast<synfig::ValueNode_Animated*>(property_value_desc().get_value().get_value_node().get());

    Time nearest(Time::end());

	synfig::ValueNode_Animated::WaypointList::iterator iter,ret;

	if(value_node)
	{
		for(
			iter=value_node->editable_waypoint_list().begin();
			iter!=value_node->editable_waypoint_list().end();
			iter++
			)
		{
			Time val=abs(iter->get_time()-selected_time);
			if(val<nearest)
			{
				nearest=val;
				ret=iter;
			}
		}

		if(nearest!=Time::end() && nearest<scope)
		{
			return ret;
		}
	}
	throw int();
}

bool
CellRenderer_TimeTrack::activate_vfunc(
	GdkEvent* event,
	Gtk::Widget& /*widget*/,
	const Glib::ustring& treepath,
	const Gdk::Rectangle& /*background_area*/,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState /*flags*/)
{
	if (!event)
	{
		// Catch a null event received us a result of a keypress (only?)
		return true; //On tab key press, Focus go to next panel. If return false, focus goes to canvas
	}

	path=treepath;
	synfig::ValueNode_Animated::WaypointList::iterator iter;
	Glib::RefPtr<Gtk::Adjustment> adjustment=get_adjustment();

	// synfig::ValueNode_Animated *value_node=dynamic_cast<synfig::ValueNode_Animated*>(property_value_desc().get_value().get_value_node().get());

	synfig::Canvas::Handle canvas(get_canvas());

	Time deltatime = 0;
	Time curr_time;
	switch(event->type)
	{
	case GDK_MOTION_NOTIFY:
		curr_time=((float)event->motion.x-(float)cell_area.get_x())/(float)cell_area.get_width()*(adjustment->get_upper()-adjustment->get_lower())+adjustment->get_lower();

		mode = NONE;
		{
			Gdk::ModifierType mod;
			Gdk::Event(event).get_state(mod);
			mode = mod;
		}
		break;
	case GDK_BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
	default:
		curr_time=((float)event->button.x-(float)cell_area.get_x())/(float)cell_area.get_width()*(adjustment->get_upper()-adjustment->get_lower())+adjustment->get_lower();
		{
			Gdk::ModifierType mod;
			Gdk::Event(event).get_state(mod);
			mode = mod;
		}
		break;
	}
	actual_time = curr_time;
	if(canvas)
		curr_time=curr_time.round(canvas->rend_desc().get_frame_rate());
	selected_time=curr_time;

    Time pixel_width((adjustment->get_upper()-adjustment->get_lower())/cell_area.get_width());

    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		//selected_time=((float)event->button.x-(float)cell_area.get_x())/(float)cell_area.get_width()*(adjustment->get_upper()-adjustment->get_lower())+adjustment->get_lower();

		//Deal with time point selection, but only if they aren't involved in the insanity...
		if(/*!value_node && */event->button.button == 1)
		{
			Time stime;

			/*!	UI specification:

				When nothing is selected, clicking on a point in either normal mode or
					additive mode will select the time point closest to the click.
					Subtractive click will do nothing

				When things are already selected, clicking on a selected point does
					nothing (in both normal and add mode).  Add mode clicking on an unselected
					point adds it to the set.  Normal clicking on an unselected point will
					select only that one time point.  Subtractive clicking on any point
					will remove it from the the set if it is included.
			*/

			synfigapp::ValueDesc valdesc = property_value_desc().get_value();
			const Node::time_set *tset = get_times_from_vdesc(valdesc);
			const synfig::Time time_offset = get_time_offset_from_vdesc(valdesc);
			const synfig::Time time_dilation = get_time_dilation_from_vdesc(valdesc);

			bool clickfound = tset && get_closest_time(*tset,actual_time * time_dilation+time_offset,pixel_width*cell_area.get_height(),stime);
			bool selectmode = mode & SELECT_MASK;

			//NOTE LATER ON WE SHOULD MAKE IT SO MULTIPLE VALUENODES CAN BE SELECTED AT ONCE
			//we want to jump to the value desc if we're not currently on it
			//	but only if we want to add the point
			if(clickfound && !(sel_value == valdesc))
			{
				sel_value = valdesc;
				sel_times.clear();
			}

			//now that we've made sure we're selecting the correct value, deal with the already selected points
			set<Time>::iterator foundi = clickfound ? sel_times.find(stime) : sel_times.end();
			bool found = foundi != sel_times.end();

			//remove all other points from our list... (only select the one we need)
			if(!selectmode && !found)
			{
				sel_times.clear();
			}

			if(found && selectmode) //remove a single already selected point
			{
				sel_times.erase(foundi);
			}else if(clickfound) //otherwise look at adding it
			{
				//for replace the list was cleared earlier, and for add it wasn't so it works
				sel_times.insert(stime);
			}
		}

		selection=false;
		try
		{
			iter=find_editable_waypoint(selected_time,pixel_width*cell_area.get_height()/2);
			selected_waypoint=iter;
			selected=*iter;

			selection=true;
		}
		catch(int)
		{
			selection=false;
			selected=synfig::UniqueID::nil();
		}

		if((!sel_times.empty() || selection) && event->button.button==1)
		{
			dragging=true;
			drag_time=selected_time;
			actual_dragtime=actual_time;
		}
		//selected_time=iter->time;

		/*
		// Activepoint Selection
		if(parent_value_node)
		{
			const int index(property_value_desc().get_value().get_index());
			const synfig::ValueNode_DynamicList::ListEntry::ActivepointList& activepoint_list(parent_value_node->list[index].timing_info);
			synfig::ValueNode_DynamicList::ListEntry::ActivepointList::const_iterator iter;

			for(iter=activepoint_list.begin();iter!=activepoint_list.end();++iter)
			{
				Time val=abs(iter->time-selected_time);
				if(val<nearest)
				{
					nearest=val;
					selected=*iter;
					selection=true;
				}
			}
			// Perhaps I should signal if we selected this activepoint?
		}*/

			if(event->button.button==3)
			{
				Time stime;
				synfigapp::ValueDesc valdesc = property_value_desc().get_value();
				const Node::time_set *tset = get_times_from_vdesc(valdesc);
				synfig::Time time_offset = get_time_offset_from_vdesc(valdesc);
				const synfig::Time time_dilation = get_time_dilation_from_vdesc(valdesc);

				bool clickfound = tset && get_closest_time(*tset,actual_time * time_dilation +time_offset,pixel_width*cell_area.get_height(),stime);

				etl::handle<synfig::Node> node;
				if(!getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS") &&
				   valdesc.get_value(stime).get_type()==type_canvas)
				{
					node=Canvas::Handle(valdesc.get_value(stime).get(Canvas::Handle()));
				}
				else //if(valdesc.is_value_node())
				{
					node=valdesc.get_value_node();
				}

				if(clickfound && node)
					signal_waypoint_clicked_cellrenderer()(node, stime, time_offset, time_dilation, 2);
			}

		break;
	case GDK_MOTION_NOTIFY:
		//if(selection && dragging)
		//	selected_time=((float)event->motion.x-(float)cell_area.get_x())/(float)cell_area.get_width()*(adjustment->get_upper()-adjustment->get_lower())+adjustment->get_lower();
		return true;

		break;
	case GDK_BUTTON_RELEASE:
		{
			//selected_time=((float)event->button.x-(float)cell_area.get_x())/(float)cell_area.get_width()*(adjustment->get_upper()-adjustment->get_lower())+adjustment->get_lower();
			dragging=false;

			/*if(event->button.button==3 && selection)
			{
				signal_waypoint_clicked_cellrenderer()(path,*selected_waypoint,event->button.button-1);
				return true;
			}
			*/

			//Time point stuff...
			if(event->button.button == 1)
			{
				bool delmode = (mode & DELETE_MASK) && !(mode & COPY_MASK);
				const synfig::Time time_dilation = get_time_dilation_from_vdesc(sel_value);
				deltatime = (actual_time - actual_dragtime) * time_dilation;
				if(sel_times.size() != 0 && (delmode || !deltatime.is_equal(Time(0))))
				{
					synfigapp::Action::ParamList param_list;
					param_list.add("canvas",canvas_interface()->get_canvas());
					param_list.add("canvas_interface",canvas_interface());

					if(!getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS") &&
					   sel_value.get_value_type() == synfig::type_canvas)
					{
						param_list.add("addcanvas",sel_value.get_value().get(Canvas::Handle()));
					}else
					{
						param_list.add("addvaluedesc",sel_value);
					}

					set<Time>	newset;
					std::set<synfig::Time>::iterator i = sel_times.begin(), end = sel_times.end();
					for(; i != end; ++i)
					{
						param_list.add("addtime",*i);

						newset.insert((*i + deltatime).round(get_canvas()->rend_desc().get_frame_rate()));
					}

					if(!delmode)
						param_list.add("deltatime",deltatime);
				//	param_list.add("time",canvas_interface()->get_time());

					if(mode & COPY_MASK) //copy
					{
						etl::handle<studio::Instance>::cast_static(canvas_interface()->get_instance())
							->process_action("TimepointsCopy", param_list);
					}else if(delmode) //DELETE
					{
						etl::handle<studio::Instance>::cast_static(canvas_interface()->get_instance())
							->process_action("TimepointsDelete", param_list);
					}else //MOVE
					{
						etl::handle<studio::Instance>::cast_static(canvas_interface()->get_instance())
							->process_action("TimepointsMove", param_list);
					}

					//now replace all the selected with the new selected
					sel_times = newset;
				}
			}



			/*if(value_node && selection)
			{
				if(selected_time==drag_time && event->button.button!=3)
					signal_waypoint_clicked_cellrenderer()(path,*selected_waypoint,event->button.button-1);
				else
				if(event->button.button==1)
				{
					synfig::Waypoint waypoint(*selected_waypoint);
					Time newtime((waypoint.get_time()+(selected_time-drag_time)).round(canvas->rend_desc().get_frame_rate()));
					if(waypoint.get_time()!=newtime)
					{
						waypoint.set_time(newtime);
						signal_waypoint_changed_(waypoint,value_node);
					}
				}
			}*/

			//if(selection)
			//	selected_time=iter->time;
			//selected_time=iter->get_time();
			return true;
		}
	default:
		//std::cerr<<"unknown event type "<<event->type<<std::endl;
		return false;
		break;
	}



	return false;
}



// The following three functions don't get documented correctly by
// doxygen 1.5.[23] because of a bug with any function whose name
// begins with 'property'.  Fixed in doxygen 1.5.4 apparently.  See
// http://bugzilla.gnome.org/show_bug.cgi?id=471185 .
Glib::PropertyProxy<synfigapp::ValueDesc>
CellRenderer_TimeTrack::property_value_desc()
{
	return Glib::PropertyProxy<synfigapp::ValueDesc>(this,"value_desc");
}

Glib::PropertyProxy<synfig::Canvas::Handle>
CellRenderer_TimeTrack::property_canvas()
{
	return Glib::PropertyProxy<synfig::Canvas::Handle>(this,"canvas");
}

Glib::PropertyProxy< Glib::RefPtr<Gtk::Adjustment> >
CellRenderer_TimeTrack::property_adjustment()
{
	return Glib::PropertyProxy< Glib::RefPtr<Gtk::Adjustment> >(this,"adjustment");
}

void
CellRenderer_TimeTrack::set_canvas_interface(etl::loose_handle<synfigapp::CanvasInterface>	h)
{
	canvas_interface_ = h;
}
