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
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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
#define WIDGET_KEYFRAME_LIST_DEFAULT_FPS 24.0
/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Keyframe_List::Widget_Keyframe_List():
	adj_default(0,0,2,1/WIDGET_KEYFRAME_LIST_DEFAULT_FPS,10/WIDGET_KEYFRAME_LIST_DEFAULT_FPS),
	kf_list_(&default_kf_list_),
	time_ratio("4f", WIDGET_KEYFRAME_LIST_DEFAULT_FPS)
{
	adj_timescale=0;
	editable_=true;
	fps=WIDGET_KEYFRAME_LIST_DEFAULT_FPS;
	set_size_request(-1,64);
	//!This signal is called when the widget need to be redrawn
	signal_expose_event().connect(sigc::hide(sigc::mem_fun(*this, &studio::Widget_Keyframe_List::redraw)));
	//! The widget respond to mouse button press and release and to
	//! left button motion
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK /*| Gdk::BUTTON3_MOTION_MASK*/);
	add_events(Gdk::POINTER_MOTION_MASK);
	set_time_adjustment(&adj_default);
	queue_draw();
}

Widget_Keyframe_List::~Widget_Keyframe_List()
{
}

/*! \fn Widget_Keyframe_List::redraw()
**	\brief Redraw event. Should draw all the keyframes +  the selected + the dragged + disabled
**  connected on signal_expose_event()
*/
bool
Widget_Keyframe_List::redraw()
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return true;

	const int h(get_height());
	const int w(get_width());

	//!Boundaries of the drawing area in time units.
	synfig::Time top(adj_timescale->get_upper());
	synfig::Time bottom(adj_timescale->get_lower());

	//! The graphic context
	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));
	//! A rectangle that defines the drawing area.
	Gdk::Rectangle area(0,0,w,h);

	//! draw a background
	gc->set_rgb_fg_color(Gdk::Color("#9d9d9d"));
	window->draw_rectangle(gc, true, 0, 0, w, h);

	if(!editable_)
	{
		return true; //needs fixing!
	}
	//!Returns if there are not keyframes to draw.
	if (kf_list_->empty()) return false;

	//!Loop all the keyframes
	synfig::KeyframeList::iterator iter,selected_iter;
	bool show_selected(false);

	for(iter=kf_list_->begin();iter!=kf_list_->end();iter++)
	{
		//!do not draw keyframes out of the widget boundaries
		if (iter->get_time()>top || iter->get_time()<bottom)
			continue;
		//! If the keyframe is not the selected one
		if(*iter!=selected_kf)
		{
			const int x((int)((float)(iter->get_time()-bottom) * (w/(top-bottom)) ) );
			// Change shape for disabled keyframe
			if (iter->active())
				get_style()->paint_arrow(window, Gtk::STATE_NORMAL,
				Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_DOWN, 1,
				x-h/2+1, 0, h, h );
			else
				get_style()->paint_arrow(window, Gtk::STATE_INSENSITIVE,
				Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_RIGHT, 1,
				x-h/2+1, 0, h, h );
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
		// If not dragging just show the selected keyframe
		if (!dragging_)
		{
			int x((int)((float)(selected_iter->get_time()-bottom) * (w/(top-bottom)) ) );
			// Change shape for disabled keyframe
			if (selected_iter->active())
				get_style()->paint_arrow(window, Gtk::STATE_NORMAL,
				Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_DOWN, 1,
				x-h/2+1, 0, h, h );
			else
				get_style()->paint_arrow(window, Gtk::STATE_NORMAL,
				Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_RIGHT, 1,
				x-h/2+1, 0, h, h );
		}
		// If dragging then show the selected as insensitive and the
		// dragged as selected
		else
		{
			int x((int)((float)(selected_iter->get_time()-bottom) * (w/(top-bottom)) ) );
			get_style()->paint_arrow(window, Gtk::STATE_INSENSITIVE,
			Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_DOWN, 1,
			x-h/2, 0, h, h );
			x=(int)((float)(dragging_kf_time-bottom) * (w/(top-bottom)) ) ;
			get_style()->paint_arrow(window, Gtk::STATE_SELECTED,
			Gtk::SHADOW_OUT, area, *this, " ", Gtk::ARROW_DOWN, 1,
			x-h/2+1, 0, h, h );
		}
	}
	return true;
}


void
Widget_Keyframe_List::set_kf_list(synfig::KeyframeList* x)
{
	kf_list_=x;
	set_selected_keyframe(selected_none);
	selected_=false;
	dragging_=false;
}

void
Widget_Keyframe_List::set_selected_keyframe(const synfig::Keyframe &x)
{
	selected_kf=x;
	selected_=true;
	dragging_kf_time=selected_kf.get_time();
	//signal_keyframe_selected_(selected_kf);
	dragging_=false;
	queue_draw();
}

bool
Widget_Keyframe_List::perform_move_kf(bool delta=false)
{
	if(!selected_)
		return false;
	if(dragging_kf_time == selected_kf.get_time())
		return false; // change this checking if not sticked to integer frames
	Time selected_kf_time(selected_kf.get_time());
	Time prev, next;
	kf_list_->find_prev_next(selected_kf_time, prev, next);
	// Not possible to set delta to the first keyframe
	// perform normal movement
	// As suggested by Zelgadis it is better to not perform anything.
	if (prev==Time::begin() && delta==true)
	{
		synfig::info(_("Not possible to ALT-drag the first keyframe"));
		return false;
	}
	if(!delta)
		{
			synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeSet"));
			if(!action)
			return false;
			selected_kf.set_time(dragging_kf_time);
			action->set_param("canvas",canvas_interface_->get_canvas());
			action->set_param("canvas_interface",canvas_interface_);
			action->set_param("keyframe",selected_kf);
			try
			{
				canvas_interface_->get_instance()->perform_action(action);
			}
			catch(...)
			{
				return false;
			}
		}
	else
		{
			Keyframe prev_kf(*kf_list_->find_prev(selected_kf_time));
			Time prev_kf_time(prev_kf.get_time());
			if (prev_kf_time >= dragging_kf_time) //Not allowed
			{
				synfig::warning(_("Delta set not allowed"));
				synfig::info("Widget_Keyframe_List::perform_move_kf(%i)::prev_kf_time=%s", delta, prev_kf_time.get_string().c_str());
				synfig::info("Widget_Keyframe_List::perform_move_kf(%i)::dragging_kf_time=%s", delta, dragging_kf_time.get_string().c_str());
				return false;
			}
			else
			{
				Time old_delta_time(selected_kf_time-prev_kf_time);
				Time new_delta_time(dragging_kf_time-prev_kf_time);
				Time change_delta(new_delta_time-old_delta_time);
				synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeSetDelta"));
				if(!action)
					return false;
				action->set_param("canvas",canvas_interface_->get_canvas());
				action->set_param("canvas_interface",canvas_interface_);
				action->set_param("keyframe",prev_kf);
				action->set_param("delta",change_delta);
				canvas_interface_->get_instance()->perform_action(action);
			}
		}
	queue_draw();
	return true;
}

bool
Widget_Keyframe_List::on_event(GdkEvent *event)
{
	const int x(static_cast<int>(event->button.x));
	//const int y(static_cast<int>(event->button.y));
	//!Boundaries of the drawing area in time units.
	synfig::Time top(adj_timescale->get_upper());
	synfig::Time bottom(adj_timescale->get_lower());
	//!pos is the [0,1] relative horizontal place on the widget
	float pos((float)x/(get_width()));
	if(pos<0.0f)pos=0.0f;
	if(pos>1.0f)pos=1.0f;
	//! The time where the event x is
	synfig::Time t((float)(bottom+pos*(top-bottom)));
	//Do not respond mouse events if the list is empty
	if(!kf_list_->size())
		return true;

	//! here the guts of the event
	switch(event->type)
	{
	case GDK_MOTION_NOTIFY:
		if(editable_)
		{
			// here is captured mouse motion
			// AND left or right mouse button pressed
			if (event->motion.state & (GDK_BUTTON1_MASK /*| GDK_BUTTON3_MASK*/))
			{
				// stick to integer frames. It can be optional in the future
				if(fps) t = floor(t*fps + 0.5)/fps;
				dragging_kf_time=t;
				dragging_=true;
				queue_draw();
				return true;
			}
			// here is captured mouse motion
			// AND NOT left or right mouse button pressed
			else
			{
				Glib::ustring ttip="";
				synfig::Time p_t,n_t;
				kf_list_->find_prev_next(t, p_t, n_t);
				if( (p_t==Time::begin() 	&& 	n_t==Time::end())
				||
				((t-p_t)>time_ratio 	&& (n_t-t)>time_ratio)
				)
				{
					ttip = _("Click and drag keyframes");
				}
				else if ((t-p_t)<(n_t-t))
				{
					synfig::Keyframe kf(*kf_list_->find_prev(t));
					synfig::String kf_name(kf.get_description().c_str());
					ttip = kf_name.c_str();
				}
				else
				{
					synfig::Keyframe kf(*kf_list_->find_next(t));
					synfig::String kf_name(kf.get_description().c_str());
					ttip = kf_name.c_str();
				}
				set_tooltip_text(ttip);
				dragging_=false;
				queue_draw();
				return true;
			}
		}
		break;
	case GDK_BUTTON_PRESS:
		changed_=false;
		dragging_=false;
		if(event->button.button==1 /*|| event->button.button==3*/)
		{
			if(editable_)
			{
				synfig::Time prev_t,next_t;
				kf_list_->find_prev_next(t, prev_t, next_t, false);
				if( (prev_t==Time::begin() 	&& 	next_t==Time::end())
				||
				((t-prev_t)>time_ratio 	&& (next_t-t)>time_ratio)
				)
				{
					set_selected_keyframe(selected_none);
					selected_=false;
					queue_draw();
				}
				else if ((t-prev_t)<(next_t-t))
				{
					set_selected_keyframe(*(kf_list_->find_prev(t, false)));
					queue_draw();
					selected_=true;
				}
				else
				{
					set_selected_keyframe(*(kf_list_->find_next(t, false)));
					queue_draw();
					selected_=true;
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		break;
	case GDK_BUTTON_RELEASE:
		if(editable_ && (event->button.button==1 /*|| event->button.button==3*/))
		{
			// stick to integer frames.
			if(fps)	t = floor(t*fps + 0.5)/fps;
			bool stat=false;
			if(dragging_)
				{
					//if (event->button.button==3)
					if(event->button.state & GDK_MOD1_MASK)
					{
						stat=perform_move_kf(true);
					}
					else
					{
						stat=perform_move_kf(false);
					}
				}
			dragging_=false;
			return stat;
		}
		break;
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

void
Widget_Keyframe_List::set_canvas_interface(etl::loose_handle<synfigapp::CanvasInterface> h)
{
	canvas_interface_=h;
	// Store the values used from the canvas interface.
	if (canvas_interface_)
	{
		set_fps(canvas_interface_->get_canvas()->rend_desc().get_frame_rate());
		set_kf_list(&canvas_interface_->get_canvas()->keyframe_list());
		canvas_interface_->signal_keyframe_added().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&studio::Widget_Keyframe_List::redraw)
				)
			)
		);
		canvas_interface_->signal_keyframe_changed().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&studio::Widget_Keyframe_List::redraw)
				)
			)
		);
		canvas_interface_->signal_keyframe_removed().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&studio::Widget_Keyframe_List::redraw)
				)
			)
		);
	}
}


