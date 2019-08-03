/* === S Y N F I G ========================================================= */
/*!	\file waypointrenderer.cpp
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  ......... ... 2019 Rodolfo R. Gomes
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
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

#include "waypointrenderer.h"

#include <gdkmm/rgba.h>

#include <synfig/interpolation.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S ======================================================= */

namespace studio {

static Gdk::RGBA
get_interp_color(Interpolation x)
{
	switch(x) {
	case INTERPOLATION_TCB:
		return Gdk::RGBA("#73d216");
	case INTERPOLATION_LINEAR:
		return Gdk::RGBA("#edd400");
	case INTERPOLATION_CONSTANT:
		return Gdk::RGBA("#cc0000");
	case INTERPOLATION_HALT:
		return Gdk::RGBA("#3465a4");
	case INTERPOLATION_MANUAL:
		return Gdk::RGBA("#75507b");
	case INTERPOLATION_CLAMPED:
		return Gdk::RGBA("#c17d11");
	case INTERPOLATION_UNDEFINED:
	default:
		break;
	}
	return Gdk::RGBA("#555753");
}

static Gdk::RGBA
color_darken(Gdk::RGBA x, float amount)
{
	x.set_red(x.get_red() * amount);
	x.set_green(x.get_green() * amount);
	x.set_blue(x.get_blue() * amount);
	return x;
}

void
WaypointRenderer::render_time_point_to_window(
	const Cairo::RefPtr<Cairo::Context> &cr,
	const Gdk::Rectangle& area,
	const TimePoint &tp,
	bool selected )
{
	const Gdk::RGBA black("#2e3436"); // it's black, trust me

	if(selected)
		cr->set_line_width(2.0);
	else
		cr->set_line_width(1.0);

	Gdk::RGBA color;

/*-	BEFORE ------------------------------------- */

	color=get_interp_color(tp.get_before());
	color=color_darken(color,1.0f);
	if(selected)color=color_darken(color,1.3f);
	cr->set_source_rgb(color.get_red(),color.get_green(),color.get_blue());

	switch(tp.get_before())
	{
	case INTERPOLATION_TCB:
		cr->save();
		cr->translate(area.get_x(), area.get_y());
		cr->scale(area.get_width(), area.get_height());
		cr->move_to(0.5, 1);
		cr->arc(0.5, 0.5, 0.5, 90*M_PI/180.0, 270*M_PI/180.0);
		cr->fill_preserve();
		cr->restore();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		break;

	case INTERPOLATION_HALT:
		cr->save();
		cr->translate(area.get_x(), area.get_y());
		cr->scale(area.get_width(), area.get_height()*2);
		cr->move_to(0.5, 0.5);
		cr->arc(0.5, 0.5, 0.5, 180*M_PI/180.0, 270*M_PI/180.0);
		cr->fill();
		cr->arc(0.5, 0.5, 0.5, 180*M_PI/180.0, 270*M_PI/180.0);
		cr->restore();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();

		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->move_to(area.get_x(),area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->stroke();
		break;

	case INTERPOLATION_LINEAR:
		cr->save();
		cr->move_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x(),area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;

	case INTERPOLATION_CONSTANT:
		cr->save();
		cr->move_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width()/4,area.get_y());
		cr->line_to(area.get_x()+area.get_width()/4,area.get_y()+area.get_height()/2);
		cr->line_to(area.get_x(),area.get_y()+area.get_height()/2);
		cr->line_to(area.get_x(),area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;

	case INTERPOLATION_CLAMPED:
		cr->save();
		cr->move_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x(),area.get_y()+area.get_height()/2);
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;

	default:
		cr->save();
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width()/3,area.get_y());
		cr->line_to(area.get_x(),area.get_y()+area.get_height()/3);
		cr->line_to(area.get_x(),area.get_y()+area.get_height()-area.get_height()/3);
		cr->line_to(area.get_x()+area.get_width()/3,area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;
	}

/*-	AFTER -------------------------------------- */

	color=get_interp_color(tp.get_after());
	color=color_darken(color,0.8f);
	if(selected)color=color_darken(color,1.3f);
	cr->set_source_rgb(color.get_red(),color.get_green(),color.get_blue());

	switch(tp.get_after())
	{
	case INTERPOLATION_TCB:
		cr->save();
		cr->translate(area.get_x(), area.get_y());
		cr->scale(area.get_width(), area.get_height());
		cr->arc(0.5, 0.5, 0.5, -90*M_PI/180.0, 90*M_PI/180.0);
		cr->fill_preserve();
		cr->restore();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		break;

	case INTERPOLATION_HALT:
		cr->save();
		cr->translate(area.get_x(), area.get_y());
		cr->scale(area.get_width(), area.get_height()*2);
		cr->move_to(0.5, 0.0);
		cr->arc(0.5, 0.0, 0.5, 0*M_PI/180.0, 90*M_PI/180.0);
		cr->fill();
		cr->arc(0.5, 0.0, 0.5, 0*M_PI / 180.0, 90*M_PI / 180.0);
		cr->restore();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();

		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->move_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y());
		cr->stroke();
		break;

	case INTERPOLATION_LINEAR:
		cr->save();
		cr->move_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;

	case INTERPOLATION_CONSTANT:
		cr->save();
		cr->move_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()/2);
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/4,area.get_y()+area.get_height()/2);
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/4,area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;

	case INTERPOLATION_CLAMPED:
		cr->save();
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()/2);
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;

	default:
		cr->save();
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y());
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/3,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()/3);
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()-area.get_height()/3);
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/3,area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(black.get_red(),black.get_green(),black.get_blue());
		cr->stroke();
		cr->restore();
		break;
	}
}


}
