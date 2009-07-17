/* === S Y N F I G ========================================================= */
/*!	\file widget_keyframe_list.cpp
**	\brief A custom widget to manage keyframes in the timeline.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2009 Carlos López
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

#include "widget_keyframe_list.h"
#include "app.h"
#include <gtkmm/menu.h>
#include <synfig/exception.h>
#include <ETL/misc>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Keyframe_List::Widget_Keyframe_List():
	editable_(true),
	adj_default(0,0,2,1/24,10/24),
	adj_timescale(0),
	fps(24)
{
	set_size_request(-1,64);
	//!This signal is called when the widget need to be redrawn
	signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Keyframe_List::redraw));
	//! The widget respond to mouse button press and release and to
	//! left button motion
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
	set_time_adjustment(&adj_default);

}

Widget_Keyframe_List::~Widget_Keyframe_List()
{
}

bool
Widget_Keyframe_List::redraw(GdkEventExpose */*bleh*/)
{
	const int h(get_height());
	const int w(get_width());

	//!Boundaries of the drawing area in time units.
	synfig::Time top(adj_timescale->get_upper());
	synfig::Time bottom(adj_timescale->get_lower());

	//! The graphic context
	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(get_window()));
	//! A rectangle that defines the drawing area.
	Gdk::Rectangle area(0,0,w,h);

	if(!editable_)
	{
		return true;
	}

	//! draw a background
	gc->set_rgb_fg_color(Gdk::Color("#7f7f7f"));
	get_window()->draw_rectangle(gc, false, 0, 0, w, h);

	//!Loop all the keyframes
	synfig::KeyframeList::iterator iter,selected_iter;
	bool show_selected(false);
	for(iter=kf_list_.begin();iter!=kf_list_.end();iter++)
	{
		//!do not draw keyframes out of the widget boundaries
		if (iter->get_time()>top || iter->get_time()<bottom)
			continue;
		//! If the keyframe is not the selected one
		if(*iter!=selected_kf)
		{
			const int x((int)((float)(iter->get_time()-bottom) * (w/(top-bottom)) ) );
			get_style()->paint_arrow(get_window(), Gtk::STATE_NORMAL,
			Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_DOWN, 1,
			x-h/2, 0, h, h );
		}
		else
		{
			selected_iter=iter;
			show_selected=true;
		}
	}

	// we do this so that we can be sure that
	// the selected keyframe is shown on top
	if(show_selected)
	{
			const int x((int)((float)(selected_iter->get_time()-bottom) * (w/(top-bottom)) ) );
			get_style()->paint_arrow(get_window(), Gtk::STATE_SELECTED,
			Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_DOWN, 1,
			x-h/2, 0, h, h );
	}

	return true;
}


void
Widget_Keyframe_List::set_kf_list(const synfig::KeyframeList& x)
{
	kf_list_=x;
	if(kf_list_.size())
		set_selected_keyframe(*kf_list_.find_next(synfig::Time::zero()));
}

void
Widget_Keyframe_List::set_selected_keyframe(const synfig::Keyframe &x)
{
	selected_kf=x;
	//signal_keyframe_selected_(selected_kf);
	queue_draw();
}

bool
Widget_Keyframe_List::perform_move_kf()
{
	return false;
}

bool
Widget_Keyframe_List::on_event(GdkEvent *event)
{
	const int x(static_cast<int>(event->button.x));
	const int y(static_cast<int>(event->button.y));
		//!Boundaries of the drawing area in time units.
	synfig::Time top(adj_timescale->get_upper());
	synfig::Time bottom(adj_timescale->get_lower());
		//!pos is the [0,1] relative horizontal place on the widget
	float pos((float)x/(float)get_width());
	if(pos<0.0f)pos=0.0f;
	if(pos>1.0f)pos=1.0f;
		//! The time where the event x is
	synfig::Time t((float)(pos*(top-bottom)));
		//! here the guts of the event
	switch(event->type)
	{
	case GDK_MOTION_NOTIFY:
		if(editable_)
		{
			if(!kf_list_.size()) return true;
			// stick to integer frames.
			if(fps)
				{
					t = floor(t*fps + 0.5)/fps;
				}
			dragging_kf_time=t;
			queue_draw();
			return true;
		}
		break;
	case GDK_BUTTON_PRESS:
		changed_=false;
		if(event->button.button==1)
		{
			if(editable_)
			{
				synfig::KeyframeList::iterator selected;
				selected = kf_list_.find_next(t);
				set_selected_keyframe(*selected);
				queue_draw();
				return true;
			}
			else
			{
				return true;
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
		if(editable_ && event->button.button==1)
		{
		// stick to integer frames.
		if(fps)
			{
				t = floor(t*fps + 0.5)/fps;
			}
		bool stat=perform_move_kf();
		synfig::info("Dropping keyframe at: %s", t.get_string().c_str());
		return stat;
		}
	default:
		break;
	}


	return false;
}


void Widget_Keyframe_List::set_time_adjustment(Gtk::Adjustment *x)
{
	//disconnect old connections
	time_value_change.disconnect();
	time_other_change.disconnect();

	//connect update function to new adjustment
	adj_timescale = x;

	if(x)
	{
		time_value_change = x->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_Keyframe_List::queue_draw));
		time_other_change = x->signal_changed().connect(sigc::mem_fun(*this,&Widget_Keyframe_List::queue_draw));
	}
}

void
Widget_Keyframe_List::set_fps(float d)
{
	if(fps != d)
	{
		fps = d;
		//update everything since we need to redraw already
		queue_draw();
	}
}
