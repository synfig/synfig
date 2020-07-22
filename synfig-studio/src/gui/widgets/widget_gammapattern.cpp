/* === S Y N F I G ========================================================= */
/*!	\file widget_gammapattern.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
/* ========================================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif


#include "widgets/widget_gammapattern.h"

#endif

using namespace synfig;
using namespace studio;

Widget_GammaPattern::Widget_GammaPattern() :
	Glib::ObjectBase("widget_gammapattern"),
	tile_w          (80),
	tile_h          (80),
	gradient_h      (20)
{
	init();
}

Widget_GammaPattern::Widget_GammaPattern(BaseObjectType *cobject) :
	Glib::ObjectBase("widget_gammapattern"),
	Gtk::DrawingArea(cobject),
	tile_w          (80),
	tile_h          (80),
	gradient_h      (20)
{
	init();
}

void
Widget_GammaPattern::init()
{
	set_size_request(tile_w*4, tile_h*2 + gradient_h*2);

	// make pattern
	Cairo::RefPtr<Cairo::ImageSurface> surface =
			Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 2, 2);
	Cairo::RefPtr<Cairo::Context> context =
			Cairo::Context::create(surface);
	context->set_operator(Cairo::OPERATOR_SOURCE);
	context->set_source_rgba(0, 0, 0, 0);
	context->rectangle(0, 0, 1, 1);
	context->fill();
	context->set_source_rgba(0, 0, 0, 1);
	context->rectangle(0, 0, 1, 1);
	context->rectangle(1, 1, 1, 1);
	context->fill();
	surface->flush();

	pattern = Cairo::SurfacePattern::create(surface);
	pattern->set_filter(Cairo::FILTER_NEAREST);
	pattern->set_extend(Cairo::EXTEND_REPEAT);
}

void
Widget_GammaPattern::set_gamma(const Gamma &x)
{
	if (gamma == x) return;
	gamma = x;
	queue_draw();
}

const Gamma
&Widget_GammaPattern::get_gamma() const
{
	return gamma;
}

bool
Widget_GammaPattern::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	cr->save();
		
	// prepare colors
	ColorReal values[3] = {0.25, 0.5, 1};
	Color colors[3][4];
	for(int i = 0; i < 3; ++i) {
		ColorReal v = values[i];
		Color *c = colors[i];
		c[0] = Color(v, v, v);
		c[1] = Color(v, 0, 0);
		c[2] = Color(0, v, 0);
		c[3] = Color(0, 0, v);
		for(int j = 0; j < 4; ++j)
			c[j] = gamma.apply(c[j]);
	}
	Color *gray25 = colors[0];
	Color *gray50 = colors[1];
	Color *white  = colors[2];
	
	// 50% pattern
	for(int i = 0; i < 4; ++i) {
		cr->set_source_rgb(white[i].get_r(), white[i].get_g(), white[i].get_b());
		cr->rectangle(i*tile_w, 0, tile_w, tile_h);
		cr->fill();

		cr->set_source(pattern);
		cr->rectangle(i*tile_w, 0, tile_w, tile_h);
		cr->fill();

		cr->set_source_rgb(gray50[i].get_r(), gray50[i].get_g(), gray50[i].get_b());
		cr->rectangle(i*tile_w+tile_w/4, tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
		cr->fill();
	}

	// 25% pattern
	for(int i = 0; i < 4; ++i) {
		cr->set_source_rgb(gray50[i].get_r(), gray50[i].get_g(), gray50[i].get_b());
		cr->rectangle(i*tile_w, tile_h, tile_w, tile_h);
		cr->fill();

		cr->set_source(pattern);
		cr->rectangle(i*tile_w, tile_h, tile_w, tile_h);
		cr->fill();

		cr->set_source_rgb(gray25[i].get_r(), gray25[i].get_g(), gray25[i].get_b());
		cr->rectangle(i*tile_w+tile_w/4, tile_h+tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
		cr->fill();
	}

	// black and white level pattern
	cr->set_source_rgb(0, 0, 0);
	cr->rectangle(0, tile_h*2, tile_w*4, gradient_h);
	cr->fill();
	cr->set_source_rgb(1, 1, 1);
	cr->rectangle(0, tile_h*2 + gradient_h, tile_w*4, gradient_h);
	cr->fill();
	ColorReal level = 1;
	for(int i = 0; i < 8; ++i) {
		level *= 0.5;
		Color black = gamma.apply(Color(level, level, level));
		Color white = gamma.apply(Color(1-level, 1-level, 1-level));
		double x = tile_w*4*(i/8.0 + 1/16.0);
		double yb = tile_h*2 + gradient_h/2.0;
		double yw = yb + gradient_h;
		double r = gradient_h/4.0;
		
		cr->set_source_rgb(black.get_r(), black.get_g(), black.get_b());
		cr->arc(x, yb, r, 0, 2*M_PI);
		cr->fill();

		cr->set_source_rgb(white.get_r(), white.get_g(), white.get_b());
		cr->arc(x, yw, r, 0, 2*M_PI);
		cr->fill();
	}
	cr->restore();
	return true;
}

GType Widget_GammaPattern::gtype = 0;

Glib::ObjectBase
*Widget_GammaPattern::wrap_new(GObject *o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new Widget_GammaPattern(GTK_DRAWING_AREA(o));
	else
		return Gtk::manage(new Widget_GammaPattern(GTK_DRAWING_AREA(o)));
}

void
Widget_GammaPattern::register_type()
{
	if (gtype)
		return;

	Widget_GammaPattern dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	Glib::wrap_register(gtype, Widget_GammaPattern::wrap_new);
}
