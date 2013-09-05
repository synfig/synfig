/* === S Y N F I G ========================================================= */
/*!	\file widget_gradient.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "widgets/widget_gradient.h"
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

#define ARROW_NEGATIVE_THRESHOLD 0.4

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

void
studio::render_gradient_to_window(const Glib::RefPtr<Gdk::Drawable>& window,const Gdk::Rectangle& ca,const synfig::Gradient &gradient)
{
	int	height = ca.get_height();
	int	width = ca.get_width()-4;

	float sample_width(1.0f/(float)width);
	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));
	const Color bg1(0.25, 0.25, 0.25);
	const Color bg2(0.5, 0.5, 0.5);
	Gdk::Color gdk_c;
	int i;
	for(i=0;i<width;i++)
	{
		const Color c(gradient(float(i)/float(width),sample_width));
		const Color c1(Color::blend(c,bg1,1.0).clamped());
		const Color c2(Color::blend(c,bg2,1.0).clamped());
		gushort r1(256*App::gamma.r_F32_to_U8(c1.get_r()));
		gushort g1(256*App::gamma.g_F32_to_U8(c1.get_g()));
		gushort b1(256*App::gamma.b_F32_to_U8(c1.get_b()));
		gushort r2(256*App::gamma.r_F32_to_U8(c2.get_r()));
		gushort g2(256*App::gamma.g_F32_to_U8(c2.get_g()));
		gushort b2(256*App::gamma.b_F32_to_U8(c2.get_b()));

		if((i*2/height)&1)
		{
			gdk_c.set_rgb(r1,g1,b1);
			gc->set_rgb_fg_color(gdk_c);
			window->draw_rectangle(gc, true, ca.get_x()+i+2, ca.get_y(), 1, height/2);

			gdk_c.set_rgb(r2,g2,b2);
			gc->set_rgb_fg_color(gdk_c);
			window->draw_rectangle(gc, true, ca.get_x()+i+2, ca.get_y()+height/2, 1, height/2);
		}
		else
		{
			gdk_c.set_rgb(r2,g2,b2);
			gc->set_rgb_fg_color(gdk_c);
			window->draw_rectangle(gc, true, ca.get_x()+i+2, ca.get_y(), 1, height/2);

			gdk_c.set_rgb(r1,g1,b1);
			gc->set_rgb_fg_color(gdk_c);
			window->draw_rectangle(gc, true, ca.get_x()+i+2, ca.get_y()+height/2, 1, height/2);
		}
	}
	gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
	window->draw_rectangle(gc, false, ca.get_x()+1, ca.get_y()+1, ca.get_width()-3, height-3);
	gc->set_rgb_fg_color(Gdk::Color("#000000"));
	window->draw_rectangle(gc, false, ca.get_x(), ca.get_y(), ca.get_width()-1, height-1);
}

/* === M E T H O D S ======================================================= */

Widget_Gradient::Widget_Gradient():
	editable_(false)
{
	set_size_request(-1,64);
	signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Gradient::redraw));
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);

}

Widget_Gradient::~Widget_Gradient()
{
}

#define CONTROL_HEIGHT		16
bool
Widget_Gradient::redraw(GdkEventExpose */*bleh*/)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return false;

	const int h(get_height());
	const int w(get_width());

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));
	Gdk::Rectangle area(0,0,w,h);
	if(!editable_)
	{
		render_gradient_to_window(window,area,gradient_);
		return true;
	}

	render_gradient_to_window(window,Gdk::Rectangle(0,0,w,h),gradient_);

	gc->set_rgb_fg_color(Gdk::Color("#7f7f7f"));
	Gradient::iterator iter,selected_iter;
	bool show_selected(false);
	for(iter=gradient_.begin();iter!=gradient_.end();iter++)
	{
		if(*iter!=selected_cpoint)
		get_style()->paint_arrow(
			window,
		  (iter->color.get_y()<ARROW_NEGATIVE_THRESHOLD)?Gtk::STATE_SELECTED:Gtk::STATE_ACTIVE, //use light arrow on dark color, and dark arrow on light color , todo detect from style which is darkest from SELECTED or ACTIVE, here SELECTED is lighter.
			Gtk::SHADOW_OUT,
			area,
			*this,
			" ",
			Gtk::ARROW_UP,
			1,
			int(iter->pos*w)-CONTROL_HEIGHT/2+1,
			h-CONTROL_HEIGHT,
			CONTROL_HEIGHT,
			CONTROL_HEIGHT
		);
		else
		{
			selected_iter=iter;
			show_selected=true;
		}
	}

	// we do this so that we can be sure that
	// the selected marker is shown on top
  // show 2 arrows for selected, to compensate for lack of contrast in some color schemes, such as ubuntu default theme
  // Gtk::STATE_SELECTED was used, but resulted in a barely visible arrow in some color scheme, hence double arrow with contrasting color

	if(show_selected)
	{
		get_style()->paint_arrow(
			window,
			(selected_iter->color.get_y()<ARROW_NEGATIVE_THRESHOLD)?Gtk::STATE_SELECTED:Gtk::STATE_ACTIVE, //use light arrow on dark color, and dark arrow on light color , todo detect from style which is darkest from SELECTED or ACTIVE
			Gtk::SHADOW_OUT,
			area,
			*this,
			" ",
			Gtk::ARROW_UP,
			1,
			round_to_int(selected_iter->pos*w)-CONTROL_HEIGHT/2+1,
			h-CONTROL_HEIGHT,
			CONTROL_HEIGHT,
			CONTROL_HEIGHT
		); // paint_arrow(window, state_type, shadow_type, area, widget, detail, arrow_type, fill, x, y, width, height)
		get_style()->paint_arrow(
			window,
						(selected_iter->color.get_y()<ARROW_NEGATIVE_THRESHOLD)?Gtk::STATE_SELECTED:Gtk::STATE_ACTIVE, //use light arrow on dark color, and dark arrow on light color , todo detect from style which is darkest from SELECTED or ACTIVE
			Gtk::SHADOW_OUT,
			area,
			*this,
			" ",
			Gtk::ARROW_UP,
			1,
			round_to_int(selected_iter->pos*w)-CONTROL_HEIGHT/2+1,
			h-CONTROL_HEIGHT*1.3,
			CONTROL_HEIGHT,
			CONTROL_HEIGHT
		);
	}

	return true;
}

void
Widget_Gradient::insert_cpoint(float x)
{
	Gradient::CPoint new_cpoint;
	new_cpoint.pos=x;
	new_cpoint.color=gradient_(x);
	gradient_.push_back(new_cpoint);
	gradient_.sort();
	gradient_.sort();
	set_selected_cpoint(new_cpoint);
	queue_draw();
}

void
Widget_Gradient::remove_cpoint(float x)
{
	gradient_.erase(gradient_.proximity(x));
	signal_value_changed_();
	queue_draw();
}

void
Widget_Gradient::popup_menu(float x)
{
	Gtk::Menu* menu(manage(new Gtk::Menu()));
	menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));

	menu->items().clear();

	menu->items().push_back(
		Gtk::Menu_Helpers::MenuElem(
			_("Insert Color Stop"),
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
				_("Remove Color Stop"),
				sigc::bind(
					sigc::mem_fun(*this,&studio::Widget_Gradient::remove_cpoint),
					x
				)
			)
		);
	}

	menu->popup(0,0);
}

void
Widget_Gradient::set_value(const synfig::Gradient& x)
{
	gradient_=x;
	if(gradient_.size())
		set_selected_cpoint(*gradient_.proximity(0.0f));
	queue_draw();
}

void
Widget_Gradient::set_selected_cpoint(const synfig::Gradient::CPoint &x)
{
	selected_cpoint=x;
	signal_cpoint_selected_(selected_cpoint);
	queue_draw();
}

void
Widget_Gradient::update_cpoint(const synfig::Gradient::CPoint &x)
{
	try
	{
		Gradient::iterator iter(gradient_.find(x));
		iter->pos=x.pos;
		iter->color=x.color;
		gradient_.sort();
		queue_draw();
	}
	catch(synfig::Exception::NotFound)
	{
		// Yotta...
	}
}

bool
Widget_Gradient::on_event(GdkEvent *event)
{
	//if(editable_)
	{
		const int x(static_cast<int>(event->button.x));
		const int y(static_cast<int>(event->button.y));

		float pos((float)x/(float)get_width());
		if(pos<0.0f)pos=0.0f;
		if(pos>1.0f)pos=1.0f;

		switch(event->type)
		{
		case GDK_MOTION_NOTIFY:
			if(editable_ && y>get_height()-CONTROL_HEIGHT)
			{
				if(!gradient_.size()) return true;
				Gradient::iterator iter(gradient_.find(selected_cpoint));
				//! Use SHIFT to stack two CPoints together.
				if(event->button.state&GDK_SHIFT_MASK)
				{
					float begin(-100000000),end(100000000);
					Gradient::iterator before(iter),after(iter);
					after++;
					if(iter!=gradient_.begin())
					{
						before--;
						begin=before->pos;
					}
					if(after!=gradient_.end())
					{
						end=after->pos;
					}

					if(pos>end)
						pos=end;
					if(pos<begin)
						pos=begin;

					iter->pos=pos;
				}
				else
				{
					iter->pos=pos;
					gradient_.sort();
				}

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
