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
	editable_(false)
{
	set_size_request(-1,64);
	signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Keyframe_List::redraw));
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);

}

Widget_Keyframe_List::~Widget_Keyframe_List()
{
}

bool
Widget_Gradient::redraw(GdkEventExpose */*bleh*/)
{
	const int h(get_height());
	const int w(get_width());

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(get_window()));
	Gdk::Rectangle area(0,0,w,h);
	if(!editable_)
	{
		return true;
	}

	gc->set_rgb_fg_color(Gdk::Color("#7f7f7f"));
	get_window()->draw_rectangle(gc, false, 0, 0, w, h);

	KeyframeList::iterator iter,selected_iter;
	bool show_selected(false);
	for(iter=kf_list_.begin();iter!=kf_list_.end();iter++)
	{
		if(*iter!=selected_kf)
		get_style()->paint_arrow(
			get_window(),
			(*iter==selected_kf)?Gtk::STATE_SELECTED:Gtk::STATE_ACTIVE,
			Gtk::SHADOW_OUT,
			area,
			*this,
			" ",
			Gtk::ARROW_DOWN,
			1,
			int(iter->get_time()*w)-h/2+1, /// to be fixed
			0,
			h,
			h
		);
		else
		{
			selected_iter=iter;
			show_selected=true;
		}
	}

	// we do this so that we can be sure that
	// the selected marker is shown on top
	if(show_selected)
	{
		get_style()->paint_arrow(
			get_window(),
			Gtk::STATE_SELECTED,
			Gtk::SHADOW_OUT,
			area,
			*this,
			" ",
			Gtk::ARROW_DOWN,
			1,
			round_to_int(selected_iter->get_time()*w)-h/2+1,
			0,
			h,
			h
		);
	}

	return true;
}


void
Widget_Keyframe_List::popup_menu(Time /*t*/)
{
/*	Gtk::Menu* menu(manage(new Gtk::Menu()));
	menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));

	menu->items().clear();

	menu->items().push_back(
		Gtk::Menu_Helpers::MenuElem(
			_("Insert CPoint"),
			sigc::bind(
				sigc::mem_fun(*this,&studio::Widget_Gradient::insert_cpoint),
				x
			)
		)
	);

	if(!gradient_.empty())
	{
		menu->items().push_back(
			Gtk::Menu_Helpers::MenuElem(
				_("Remove CPoint"),
				sigc::bind(
					sigc::mem_fun(*this,&studio::Widget_Gradient::remove_cpoint),
					x
				)
			)
		);
	}

	menu->popup(0,0);
*/}

void
Widget_Keyframe_List::set_value(const synfig::KeyframeList& x)
{
	kf_list_=x;
	if(kf_list_.size())
		set_selected_keyframe(*kf_list_.find_next(synfig::Time::zero()));
	queue_draw();
}

void
Widget_Keyframe_List::set_selected_keyframe(const synfig::Keyframe &x)
{
	selected_kf=x;
	signal_keyframe_selected_(selected_kf);
	queue_draw();
}

void
Widget_Keyframe_List::update_keyframe(const synfig::Keyframe &x)
{
	try
	{
		KeyframeList::iterator iter(keyframe_list_.find(x));
		iter->pos=x.pos;///////7
		iter->color=x.color;/////////
		gradient_.sort();/////////
		queue_draw();
	}
	catch(synfig::Exception::NotFound)
	{
		// Yotta...
	}
}

bool
Widget_Keyframe_List::on_event(GdkEvent *event)
{
	//if(editable_)
	{
		const int x(static_cast<int>(event->button.x));
		const int y(static_cast<int>(event->button.y));

		float pos((float)x/(float)get_width());
		if(pos<0.0f)pos=0.0f;////////
		if(pos>1.0f)pos=1.0f;////////

		switch(event->type)
		{
		case GDK_MOTION_NOTIFY:
			if(editable_ && y>get_height()-h)
			{
				if(!keyframe_list_.size()) return true;
				synfig::KeyframeList::iterator iter(keyframe_list_.find(selected_kf.get_guid()));
				iter->pos=pos;/////
				gradient_.sort();

//				signal_value_changed_();
				changed_=true;
				queue_draw();
				return true;
			}
			break;
		case GDK_BUTTON_PRESS:
			changed_=false;
			if(event->button.button==1)
			{
				if(editable_ && y>get_height()-CONTROL_HEIGHT)
				{
					set_selected_cpoint(*gradient_.proximity(pos));
					queue_draw();
					return true;
				}
				else
				{
					signal_clicked_();
					return true;
				}
			}
			else if(editable_ && event->button.button==3)
			{
				popup_menu(pos);
				return true;
			}
			break;
		case GDK_BUTTON_RELEASE:
			if(editable_ && event->button.button==1 && y>get_height()-CONTROL_HEIGHT)
			{
				set_selected_cpoint(*gradient_.proximity(pos));
				if(changed_)signal_value_changed_();
				return true;
			}
		default:
			break;
		}
	}

	return false;
}
