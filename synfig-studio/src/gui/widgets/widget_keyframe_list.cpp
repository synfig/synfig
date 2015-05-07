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
	adj_default(Gtk::Adjustment::create(0,0,2,1/WIDGET_KEYFRAME_LIST_DEFAULT_FPS,10/WIDGET_KEYFRAME_LIST_DEFAULT_FPS)),
	kf_list_(&default_kf_list_),
	time_ratio("4f", WIDGET_KEYFRAME_LIST_DEFAULT_FPS)
{
	editable_=true;
	fps=WIDGET_KEYFRAME_LIST_DEFAULT_FPS;
	set_size_request(-1,64);
	//! The widget respond to mouse button press and release and to
	//! left button motion
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK /*| Gdk::BUTTON3_MOTION_MASK*/);
	add_events(Gdk::POINTER_MOTION_MASK);
	set_time_adjustment(adj_default);
	queue_draw();

	//! Create the window of the moving tooltip
	moving_tooltip_ = Gtk::manage(new Gtk::Window(Gtk::WINDOW_POPUP));
	moving_tooltip_->set_resizable(false);
	moving_tooltip_->set_name("gtk-tooltips");
	moving_tooltip_->set_border_width (4);
	moving_tooltip_->set_default_size(10, 10);
	moving_tooltip_->set_type_hint(Gdk::WINDOW_TYPE_HINT_TOOLTIP);

	moving_tooltip_label_ = Gtk::manage(new Gtk::Label());
	moving_tooltip_label_->set_alignment(0.5, 0.5);
	moving_tooltip_label_->show();

	moving_tooltip_->add(*moving_tooltip_label_);
}

Widget_Keyframe_List::~Widget_Keyframe_List()
{
}

void
Widget_Keyframe_List::draw_arrow(
	const Cairo::RefPtr<Cairo::Context> &cr,
	double x, double y,
	double width, double height,
	bool fill,
	const synfig::Color &color )
{
	cr->set_source_rgba(color.get_r(), color.get_g(), color.get_b(), color.get_a());
	cr->set_line_width(1.0);
	cr->move_to(x, y);
	cr->line_to(x - 0.5*width, y - height);
	cr->line_to(x + 0.5*width, y - height);
	cr->close_path();
	if (fill) cr->fill(); else cr->stroke();
}

bool
Widget_Keyframe_List::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return true;

	const double h(get_height());
	const double w(get_width());
	const double y(h - 2);
	const double ah(h - 4);
	const double aw(2*ah);

	//!Boundaries of the drawing area in time units.
	synfig::Time top(adj_timescale->get_upper());
	synfig::Time bottom(adj_timescale->get_lower());

	//! The style of the widget.
	Glib::RefPtr<Gtk::StyleContext> style = get_style_context();
	Gtk::StateFlags state = style->get_state();

	//TODO hardcoded colors
	//! Colors
	Color background(0.46, 0.55, 0.70, 1.0);
	Color normal(0.0, 0.0, 0.0, 1.0);
	Color selected(1.0, 1.0, 1.0, 1.0);
	Color drag_old_position(1.0, 1.0, 1.0, 0.6);
	Color drag_new_position(1.0, 1.0, 1.0, 1.0);

	//! Draw a background
	cr->set_source_rgba(background.get_r(), background.get_g(), background.get_b(), background.get_a());
	cr->rectangle(0.0, 0.0, w, h);
	cr->fill();
	cr->restore();

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
		if (iter->get_time()>top || iter->get_time()<bottom)
			continue;
		if(*iter!=selected_kf)
		{
			const double x = (double)(iter->get_time()-bottom) * (w/(top-bottom));
			draw_arrow(cr, x, y, aw, ah, iter->active(), normal);
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
		if (!dragging_)
		{
			const double x = (double)(selected_iter->get_time()-bottom) * (w/(top-bottom));
			draw_arrow(cr, x, y, aw, ah, selected_iter->active(), selected);
		}
		else
		{
			const double prev_x = (double)(selected_iter->get_time()-bottom) * (w/(top-bottom));
			const double new_x = (double)(dragging_kf_time-bottom) * (w/(top-bottom));
			draw_arrow(cr, prev_x, y, aw, ah, selected_iter->active(), drag_old_position);
			draw_arrow(cr, new_x, y, aw, ah, selected_iter->active(), drag_new_position);
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
	if (x == selected_none) return;

	if (x == selected_kf)
	{
		// synfig::Keyframe::operator== only on uniqueid::operator==
		// \see synfig::UniqueID::operator==
		// In all case, refresh keyframe description to do not loose it
		selected_kf.set_description(x.get_description());
		// refresh keyframe time also.
		selected_kf.set_time(x.get_time());
		// and activation status
		selected_kf.set_active(x.active());
		return;
	}

	selected_kf=x;
	selected_=true;
	dragging_kf_time=selected_kf.get_time();

	if(canvas_interface_)
		canvas_interface_->signal_keyframe_selected()(selected_kf, (void*)this);

	dragging_=false;
	queue_draw();
}

void
Widget_Keyframe_List::on_keyframe_selected(synfig::Keyframe keyframe, void* emitter)
{
	if((void*)this == emitter)	return;

	if (keyframe == selected_kf)	return;

	selected_kf=keyframe;
	selected_=true;

	dragging_kf_time=selected_kf.get_time();
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
				canvas_interface_->signal_keyframe_selected()(selected_kf, (void*)this);
			}
			catch(...)
			{
				return false;
			}

		}
	else
		{
			// find prev from selected kf time including deactivated kf
			Keyframe prev_kf(*kf_list_->find_prev(selected_kf_time, false));
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
	//Do not respond mouse events if the list is empty
	if(!kf_list_->size())
		return true;

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
				if (!selected_) return true;
				// stick to integer frames. It can be optional in the future
				if(fps) t = floor(t*fps + 0.5)/fps;
				dragging_kf_time=t;
				dragging_=true;
				queue_draw();

				//! Moving tooltip displaying the dragging time
				{
					int x_root = static_cast<int>(event->button.x_root);
					int x_origin; int y_origin;
					get_window()->get_origin (x_origin, y_origin);

					Glib::ustring tooltip_label (_("Time : "));
					tooltip_label.append( dragging_kf_time.get_string(fps,App::get_time_format()) );
					tooltip_label.append("\n");
					tooltip_label.append( _("Old Time : ") );
					tooltip_label.append(selected_kf.get_time().get_string(fps,App::get_time_format()));
					moving_tooltip_label_->set_text (tooltip_label);

					if(!moving_tooltip_->get_visible ())
					{
						//! Show the tooltip and fix his y coordinate (up to the widget)
						moving_tooltip_->show();
						moving_tooltip_y_ = y_origin - moving_tooltip_->get_height();
					}
					//! Move the tooltip to a nice position
					moving_tooltip_->move(x_root, moving_tooltip_y_);
				}

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
					ttip = (kf_name.length() == 0)? _("No name") : kf_name.c_str();
				}
				else
				{
					synfig::Keyframe kf(*kf_list_->find_next(t));
					synfig::String kf_name(kf.get_description().c_str());
					ttip = (kf_name.length() == 0)? _("No name") : kf_name.c_str();
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
		if(event->button.button==1 || event->button.button==3)
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
					switch(event->button.button)
					{
					case 1:
						set_selected_keyframe(selected_none);
						selected_=false;
						queue_draw();
					break;
					case 3:
						Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(App::ui_manager()->get_widget("/menu-keyframe"));
						if(menu)
						{
							menu->popup(event->button.button,gtk_get_current_event_time());
						}
					break;
					}
				}
				else if ((t-prev_t)<(next_t-t))
				{
					switch(event->button.button)
					{
					case 1:
						set_selected_keyframe(*(kf_list_->find_prev(t, false)));
						queue_draw();
						selected_=true;
					break;
					case 3:
						set_selected_keyframe(*(kf_list_->find_prev(t, false)));
						queue_draw();
						selected_=true;

						Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(App::ui_manager()->get_widget("/menu-keyframe"));
						if(menu)
						{
							menu->popup(event->button.button,gtk_get_current_event_time());
						}
					break;
					}
				}
				else
				{
					switch(event->button.button)
					{
					case 1:
						set_selected_keyframe(*(kf_list_->find_next(t, false)));
						queue_draw();
						selected_=true;
						break;
					case 3:
						set_selected_keyframe(*(kf_list_->find_next(t, false)));
						queue_draw();
						selected_=true;

						Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(App::ui_manager()->get_widget("/menu-keyframe"));
						if(menu)
						{
							menu->popup(event->button.button,gtk_get_current_event_time());
						}
						break;
					}
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		break;
	case GDK_2BUTTON_PRESS:
		if(event->button.button==1)
		{
			if(selected_ && editable_ && canvas_interface_)
			{
				canvas_interface_->signal_keyframe_properties()();
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
					moving_tooltip_->hide();
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


void Widget_Keyframe_List::set_time_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x)
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
					sigc::mem_fun(*this,&studio::Widget_Keyframe_List::queue_draw)
				)
			)
		);
		canvas_interface_->signal_keyframe_changed().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&studio::Widget_Keyframe_List::queue_draw)
				)
			)
		);
		canvas_interface_->signal_keyframe_removed().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&studio::Widget_Keyframe_List::queue_draw)
				)
			)
		);
		canvas_interface_->signal_keyframe_selected().connect(
				sigc::mem_fun(*this,&studio::Widget_Keyframe_List::on_keyframe_selected)
		);
	}
}
