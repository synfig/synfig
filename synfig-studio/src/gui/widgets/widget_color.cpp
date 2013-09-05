/* === S Y N F I G ========================================================= */
/*!	\file widget_color.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "widgets/widget_color.h"
#include <cmath>
#include "app.h"
#include <gtkmm/drawingarea.h>

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

Gdk::Color
studio::colorconv_synfig2gdk(const synfig::Color &c_)
{
	const synfig::Color c(c_.clamped());
	Gdk::Color ret;
	ret.set_rgb(
			256*App::gamma.r_F32_to_U8(c.get_r()),
			256*App::gamma.g_F32_to_U8(c.get_g()),
			256*App::gamma.b_F32_to_U8(c.get_b())
		);
	return ret;
}

void
studio::render_color_to_window(const Glib::RefPtr<Gdk::Drawable>& window,const Gdk::Rectangle& ca,const synfig::Color &color)
{
	const int height(ca.get_height());
	const int width(ca.get_width());

	const int square_size(height/2);

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));

	if(color.get_alpha()!=1.0)
	{
		// In this case we need to render the alpha squares

		const Color bg1(Color::blend(color,Color(0.75, 0.75, 0.75),1.0).clamped());
		const Color bg2(Color::blend(color,Color(0.5, 0.5, 0.5),1.0).clamped());

		Gdk::Color gdk_c1(colorconv_synfig2gdk(bg1));
		Gdk::Color gdk_c2(colorconv_synfig2gdk(bg2));

		bool toggle(false);
		for(int i=0;i<width;i+=square_size)
		{
			const int square_width(min(square_size,width-i));

			if(toggle)
			{
				gc->set_rgb_fg_color(gdk_c1);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y(), square_width, square_size);

				gc->set_rgb_fg_color(gdk_c2);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
				toggle=false;
			}
			else
			{
				gc->set_rgb_fg_color(gdk_c2);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y(), square_width, square_size);

				gc->set_rgb_fg_color(gdk_c1);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
				toggle=true;
			}
		}
	}
	else
	{
		// In this case we have a solid color to use
		Gdk::Color gdk_c1(colorconv_synfig2gdk(color));

		gc->set_rgb_fg_color(gdk_c1);
		window->draw_rectangle(gc, true, ca.get_x(), ca.get_y(), width-1, height-1);
	}
	gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
	window->draw_rectangle(gc, false, ca.get_x()+1, ca.get_y()+1, width-3, height-3);
	gc->set_rgb_fg_color(Gdk::Color("#000000"));
	window->draw_rectangle(gc, false, ca.get_x(), ca.get_y(), width-1, height-1);
}

/* === C L A S S E S ======================================================= */


/* === M E T H O D S ======================================================= */

Widget_Color::Widget_Color()
{
	color=Color(0,0,0,0);
	set_size_request(-1,16);

	signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Color::redraw));
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

}

Widget_Color::~Widget_Color()
{
}

void
Widget_Color::set_value(const synfig::Color &data)
{
	assert(data.is_valid());
	color=data;
	queue_draw();
}

const synfig::Color &
Widget_Color::get_value()
{
	assert(color.is_valid());
	return color;
}

bool
Widget_Color::on_event(GdkEvent *event)
{
	switch(event->type)
	{
	case GDK_BUTTON_PRESS:
		if(event->button.button==1)
		{
			signal_activate_();
			return true;
		}
		if(event->button.button==2)
		{
			signal_middle_click_();
			return true;
		}
		if(event->button.button==3)
		{
			signal_right_click_();
			return true;
		}
		break;

	default:
		break;
	}
	return false;
}

bool
Widget_Color::redraw(GdkEventExpose */*bleh*/)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return true;

	const int h(get_height());
	const int w(get_width());

	render_color_to_window(window,Gdk::Rectangle(0,0,w,h),color);

	return true;
}
