/*!	\file gui/widgets/widget_colorslider.h
**	\brief Widget for choosing the value of a color component
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Rodolfo Ribeiro Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widget_colorslider.h"

#include <gui/app.h>
#include <gui/exception_guard.h>

#endif

using namespace synfig;
using namespace studio;

ColorSlider::ColorSlider(Type x):
	type(x)
{
	set_size_request(-1,16);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

void
ColorSlider::set_type(Type x) { type=x; queue_draw(); }

void
ColorSlider::set_color(synfig::Color x) { color_=x; queue_draw(); }

void
ColorSlider::slider_color_TYPE_R(synfig::Color &color, float amount) { color.set_r(amount); }
void
ColorSlider::slider_color_TYPE_G(synfig::Color &color, float amount) { color.set_g(amount); }
void
ColorSlider::slider_color_TYPE_B(synfig::Color &color, float amount) { color.set_b(amount); }
void
ColorSlider::slider_color_TYPE_Y(synfig::Color &color, float amount) { color.set_y(amount); }
void
ColorSlider::slider_color_TYPE_U(synfig::Color &color, float amount) { color.set_u(amount-0.5f); }
void
ColorSlider::slider_color_TYPE_V(synfig::Color &color, float amount) { color.set_v(amount-0.5f); }
void
ColorSlider::slider_color_TYPE_HUE(synfig::Color &color, float amount) { color.set_uv_angle(Angle::rot(amount)); }
void
ColorSlider::slider_color_TYPE_SAT(synfig::Color &color, float amount) { color.set_s(amount*0.5f); }
void
ColorSlider::slider_color_TYPE_A(synfig::Color &color, float amount) { color.set_a(amount); }

void
ColorSlider::adjust_color(Type type, synfig::Color &color, float amount)
{
	static const slider_color_func jump_table[int(TYPE_END)] =
	{
		slider_color_TYPE_R,
		slider_color_TYPE_G,
		slider_color_TYPE_B,
		slider_color_TYPE_Y,
		slider_color_TYPE_U,
		slider_color_TYPE_V,
		slider_color_TYPE_HUE,
		slider_color_TYPE_SAT,
		slider_color_TYPE_A,
	};
	jump_table[int(type)](color,amount);
}

void
ColorSlider::draw_arrow(
	const Cairo::RefPtr<Cairo::Context> &cr,
	double x, double y,
	double width, double height,
	int size,
	bool fill)
{
	// hardcoded colors
	Color dark(0, 0, 0);
	Color light(1, 1, 1);

	// Upper black pointing down arrow
	cr->set_source_rgb(dark.get_r(), dark.get_g(), dark.get_b());
	cr->set_line_width(1.0);
	cr->move_to(x, y);
	cr->line_to(x - 0.5*width, y - height);
	cr->line_to(x + 0.5*width, y - height);
	cr->close_path();
	if (fill)
		cr->fill();
	else
		cr->stroke();

	// Bottom light pointing up arrow
	cr->set_source_rgb(light.get_r(), light.get_g(), light.get_b());
	cr->set_line_width(1.0);
	cr->move_to(x, size - height);
	cr->line_to(x - 0.5*width, size);
	cr->line_to(x + 0.5*width, size);
	cr->close_path();
	if (fill)
		cr->fill();
	else
		cr->stroke();
}

bool
ColorSlider::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	Color color = color_;

	static const slider_color_func jump_table[int(TYPE_END)] =
	{
		slider_color_TYPE_R,
		slider_color_TYPE_G,
		slider_color_TYPE_B,
		slider_color_TYPE_Y,
		slider_color_TYPE_U,
		slider_color_TYPE_V,
		slider_color_TYPE_HUE,
		slider_color_TYPE_SAT,
		slider_color_TYPE_A,
	};

	slider_color_func color_func = jump_table[int(type)];

	Gamma gamma = App::get_selected_canvas_gamma().get_inverted();

	float amount;
	switch(type)
	{
		case TYPE_R: amount=color.get_r(); break;
		case TYPE_G: amount=color.get_g(); break;
		case TYPE_B: amount=color.get_b(); break;
		case TYPE_Y: amount=color.get_y(); break;
		case TYPE_U: amount=color.get_u()+0.5; break;
		case TYPE_V: amount=color.get_v()+0.5; break;
		case TYPE_HUE: amount=Angle::rot(color.get_uv_angle()).get(); amount-=floor(amount); break;
		case TYPE_SAT: amount=color.get_s()*2.0; break;
		case TYPE_A: amount=color.get_a(); break;
		default: amount=0; break;
	}

	const int height(get_height());
	const int width(get_width());

	Gdk::Rectangle ca(0,0,width,height);

	const Color bg1(0.75, 0.75, 0.75);
	const Color bg2(0.5, 0.5, 0.5);
	for(int i = width-1; i >= 0; --i)
	{
		Color c = color;
		color_func(c, i/float(width));

		const Color c1 = gamma.apply(
				Color::blend(c,bg1,1.0).clamped() );
		const Color c2 = gamma.apply(
				Color::blend(c,bg2,1.0).clamped() );
		assert(c1.is_valid());
		assert(c2.is_valid());

		if((i*2/height)&1)
		{
	        cr->set_source_rgb(c1.get_r(), c1.get_g(), c1.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y(), 1, height/2);
	        cr->fill();

	        cr->set_source_rgb(c2.get_r(), c2.get_g(), c2.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y()+height/2, 1, height/2);
	        cr->fill();
		}
		else
		{
	        cr->set_source_rgb(c2.get_r(), c2.get_g(), c2.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y(), 1, height/2);
	        cr->fill();

	        cr->set_source_rgb(c1.get_r(), c1.get_g(), c1.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y()+height/2, 1, height/2);
	        cr->fill();
		}
	}

    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(ca.get_x()+1, ca.get_y()+1, width-3, height-3);
    cr->stroke();

    cr->set_source_rgb(0, 0, 0);
    cr->rectangle(ca.get_x(), ca.get_y(), width-1, height-1);
    cr->stroke();

    // Draw face to face contrasted arrows
    draw_arrow(cr, int(amount*width), height/2, height/2, height/2, height, 1);

	return true;
}

bool
ColorSlider::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	const int width(get_width());
	float x = 0;
	if( GDK_SCROLL == event->type ){
		Color color(color_);
		float amount;
		switch(type)
		{
			case TYPE_R: amount=color.get_r(); break;
			case TYPE_G: amount=color.get_g(); break;
			case TYPE_B: amount=color.get_b(); break;
			case TYPE_Y: amount=color.get_y(); break;
			case TYPE_U: amount=color.get_u()+0.5; break;
			case TYPE_V: amount=color.get_v()+0.5; break;
			case TYPE_HUE: amount=Angle::rot(color.get_uv_angle()).get(); amount-=floor(amount); break;
			case TYPE_SAT: amount=color.get_s()*2.0; break;
			case TYPE_A: amount=color.get_a(); break;
			default: amount=0; break;
		}
		x = amount*width;
		switch(event->scroll.direction){
			case GDK_SCROLL_UP:
			case GDK_SCROLL_RIGHT:
				x+=1.0;
				break;
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT:
				x-=1.0;
				break;
			default:
				break;
		}
	} else {
		x = float(event->button.x);
	}

	float pos(x/width);
	if (pos > 1) pos = 1;
	if (pos < 0 || x <= 0 || event->button.x <= 0) pos=0;

	switch(event->type)
	{
	case GDK_SCROLL:
		signal_slider_moved_(type,pos);
		queue_draw();
		signal_activated_();
		return true;

	case GDK_BUTTON_RELEASE:
		signal_activated_();
		return true;

	case GDK_BUTTON_PRESS:
	case GDK_MOTION_NOTIFY:
		signal_slider_moved_(type,pos);
		queue_draw();
		return true;
		break;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}
