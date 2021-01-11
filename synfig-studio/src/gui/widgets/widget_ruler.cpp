/* === S Y N F I G ========================================================= */
/*!	\file widget_ruler.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include <gui/widgets/widget_ruler.h>

#include <ETL/stringf>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Ruler::Widget_Ruler(bool is_vertical):
	is_vertical(is_vertical),
	layout(Pango::Layout::create(get_pango_context())),
	min(0.0),
	max(0.0),
	position(0.0)
{ }

Widget_Ruler::~Widget_Ruler() { }

void
Widget_Ruler::set_min(synfig::Real value)
	{ if (min != value) { min = value; this->queue_draw(); } }

void
Widget_Ruler::set_max(synfig::Real value)
	{ if (max != value) { max = value; this->queue_draw(); } }

void
Widget_Ruler::set_position(synfig::Real value)
	{ if (position != value) { position = value; this->queue_draw(); } }

void
Widget_Ruler::draw_line(
	const ::Cairo::RefPtr< ::Cairo::Context>& cr,
	synfig::Real position,
	synfig::Real size,
	const Gdk::RGBA &color,
	synfig::Real width,
	synfig::Real height )
{
	position = round(position) - 0.5;

	cr->set_line_width(0.5);
	cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
	if (is_vertical) {
		cr->move_to(width - size, position);
		cr->line_to(width, position);
	}
	else
	{
		cr->move_to(position, height - size);
		cr->line_to(position, height);
	}
	cr->stroke();
}

void
Widget_Ruler::draw_text(
	const ::Cairo::RefPtr< ::Cairo::Context>& cr,
	synfig::Real position,
	const synfig::String &text,
	int size,
	const Gdk::RGBA &color,
	synfig::Real offset,
	synfig::Real width,
	synfig::Real height )
{
	layout->set_text(text);

	int w = 0, h = 0;
	Pango::AttrList attr_list;
	Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*size));
	pango_size.set_start_index(0);
	pango_size.set_end_index(64);
	attr_list.change(pango_size);
	layout->set_attributes(attr_list);
	layout->get_pixel_size(w, h);

	if (is_vertical) {
		cr->save();
		cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
		cr->rotate_degrees(-90.f);
		cr->move_to(-position + 3, width - offset - h);
		layout->show_in_cairo_context(cr);
		cr->restore();
	} else {
		cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
		cr->move_to(position + 3, height - offset - h);
		layout->show_in_cairo_context(cr);
	}
}

bool
Widget_Ruler::on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr)
{
	const Real min_screen_text_mark_distance = 50.0;
	const Real mark_1_size = 18.0;
	const Real mark_2_size = 18.0;
	const Real mark_3_size = 5.0;
	const Real mark_4_size = 3.0;
	const int text_size = 8;
	const Real text_offset = 5.0;

	Real screen_min = get_screen_min();
	Real screen_max = get_screen_max();

	Gdk::RGBA color = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);

	// Make ruler less visually noisy
	// TODO: this should probably be done via Gtk styling instead
	color.set_alpha(0.6);

	Real min_text_mark_distance = fabs(distance_from_screen(min_screen_text_mark_distance));
	int text_degree = (int)round(ceil(log10(min_text_mark_distance)));
	Real text_mark_distance = exp(log(10)*(Real)text_degree);
	Real screen_text_mark_distance = fabs(distance_to_screen(text_mark_distance));

	int mode = 0.2*screen_text_mark_distance >= min_screen_text_mark_distance ? 2
			 : 0.5*screen_text_mark_distance >= min_screen_text_mark_distance ? 5
			 : 10;
	int sub_divisions_count = 100/mode;

	Real mark_distance = text_mark_distance/(Real)sub_divisions_count;
	Real screen_mark_distance = fabs(distance_to_screen(mark_distance));

	Real begin, end;
	if (min < max)
	{
		begin = floor(min/(10.0*text_mark_distance))*(10.0*text_mark_distance);
		end = ceil(max/(10.0*text_mark_distance))*(10.0*text_mark_distance);
	}
	else
	{
		begin = ceil(min/(10.0*text_mark_distance))*(10.0*text_mark_distance);
		end = floor(max/(10.0*text_mark_distance))*(10.0*text_mark_distance);
	}

	Real screen_begin = position_to_screen(begin);
	Real screen_position = get_screen_position();

	int total_marks_count = sub_divisions_count*(int)round(fabs(end - begin)/text_mark_distance) + 1;
	if (total_marks_count > 16384) return true;

	Real width = (Real)get_width();
	Real height = (Real)get_height();

	// draw background
	//if (is_vertical)
	//	cr->rectangle(0.0, screen_min, width, screen_max - screen_min);
	//else
	//	cr->rectangle(screen_min, 0.0, screen_max - screen_min, height);
	//cr->set_source_rgb(1, 1, 1);
	//cr->fill();

	// draw bounds
	//draw_line(cr, screen_min, is_vertical ? width : height, width, height);
	//draw_line(cr, screen_max, is_vertical ? width : height, width, height);

	// draw marks
	for(int i = 0; i < total_marks_count; ++i)
	{
		Real screen_pos = screen_begin + (Real)i*screen_mark_distance;
		if (screen_pos <= screen_min || screen_pos >= screen_max)
			continue;

		Real pos = min < max
				 ? begin + (Real)i*mark_distance
				 : begin - (Real)i*mark_distance;
		if ((int)round(pos/mark_distance) == 0)
			pos = 0.0;

		if (i%sub_divisions_count == 0)
		{
			draw_line(cr, screen_pos, mark_1_size, color, width, height);
			String format = etl::strprintf("%%.%df", text_degree < 0 ? -text_degree : 0);
			String text = etl::strprintf(format.c_str(), pos);
			draw_text(cr, screen_pos, text, text_size, color, text_offset, width, height);
		}
		else
		if ( (mode == 5 && i%10 == 0)
		  || (mode == 2 && i%10 == 0) )
		{
			draw_line(cr, screen_pos, mark_2_size, color, width, height);
			String format = etl::strprintf("%%.%df", text_degree < 1 ? 1-text_degree : 0);
			String text = etl::strprintf(format.c_str(), pos);
			draw_text(cr, screen_pos, text, text_size, color, text_offset, width, height);
		}
		else
		if ( (mode == 10 && i%2 == 0)
		  || (mode ==  5 && i%2 == 0)
		  || (mode ==  2 && i%5 == 0) )
		{
			draw_line(cr, screen_pos, mark_3_size, color, width, height);
		}
		else
		{
			draw_line(cr, screen_pos, mark_4_size, color, width, height);
		}
	}

	// draw current position
	if (screen_position > screen_min && screen_position < screen_max)
		draw_line(cr, screen_position, is_vertical ? width : height, color, width, height);

	return true;
}

