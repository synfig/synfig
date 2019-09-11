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

#include <synfig/general.h>

#include "widgets/widget_color.h"
#include <cmath>
#include "app.h"
#include <gtkmm/drawingarea.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

synfig::Color
studio::colorconv_apply_gamma(const synfig::Color &c_)
{
	const synfig::Color c(c_.clamped());
	return synfig::Color(
		App::gamma.r_F32_to_F32(c.get_r()),
		App::gamma.g_F32_to_F32(c.get_g()),
		App::gamma.b_F32_to_F32(c.get_b()),
		App::gamma.b_F32_to_F32(c.get_a()) );
}

void
studio::render_color_to_window(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &ca, const synfig::Color &color)
{
	const int height(ca.get_height());
	const int width(ca.get_width());

	const int square_size(height/2);

	if(color.get_alpha()!=1.0)
	{
		// In this case we need to render the alpha squares

		const Color bg1(
			colorconv_apply_gamma(
				Color::blend(color,Color(0.75, 0.75, 0.75),1.0).clamped() ));
		const Color bg2(
			colorconv_apply_gamma(
				Color::blend(color,Color(0.5, 0.5, 0.5),1.0).clamped() ));

		bool toggle(false);
		for(int i=0;i<width;i+=square_size)
		{
			const int square_width(min(square_size,width-i));

			if(toggle)
			{
		        cr->set_source_rgb(bg1.get_r(), bg1.get_g(), bg1.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y(), square_width, square_size);
		        cr->fill();

		        cr->set_source_rgb(bg2.get_r(), bg2.get_g(), bg2.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
		        cr->fill();
				toggle=false;
			}
			else
			{
		        cr->set_source_rgb(bg2.get_r(), bg2.get_g(), bg2.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y(), square_width, square_size);
		        cr->fill();

		        cr->set_source_rgb(bg1.get_r(), bg1.get_g(), bg1.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
		        cr->fill();
				toggle=true;
			}
		}
	}
	else
	{
		synfig::Color c = colorconv_apply_gamma(color);
        cr->set_source_rgb(c.get_r(), c.get_g(), c.get_b());
        cr->rectangle(ca.get_x(), ca.get_y(), width-1, height-1);
        cr->fill();
	}

	cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->rectangle(ca.get_x()+1, ca.get_y()+1, width-3, height-3);
    cr->stroke();

    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->rectangle(ca.get_x(), ca.get_y(), width-1, height-1);
    cr->stroke();
}

/* === C L A S S E S ======================================================= */


/* === M E T H O D S ======================================================= */

Widget_Color::Widget_Color()
{
	color=Color(0,0,0,0);
	set_size_request(-1,16);
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
Widget_Color::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	render_color_to_window(cr, Gdk::Rectangle(0,0,get_width(),get_height()), color);
	return true;
}
