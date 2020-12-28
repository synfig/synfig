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

#include <cmath>
#include <gdkmm/rgba.h>

#include <synfig/interpolation.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;

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

static Gdk::RGBA
color_shift(Gdk::RGBA x, double amount)
{
	x.set_red(x.get_red() + amount);
	x.set_green(x.get_green() + amount);
	x.set_blue(x.get_blue() + amount);
	return x;
}

static Gdk::RGBA
get_black(bool hover)
{
	if (!hover)
		return Gdk::RGBA("#2e3436"); // it's black, trust me
	return Gdk::RGBA("#4e5456");
}

void
WaypointRenderer::render_time_point_to_window(
	const Cairo::RefPtr<Cairo::Context> &cr,
	const Gdk::Rectangle& area,
	const TimePoint &tp,
	bool selected,
	bool hover,
	bool double_outline)
{
	const Gdk::RGBA outline_color = get_black(hover);

	if(selected)
		cr->set_line_width(2.0);
	else
		cr->set_line_width(1.0);

	Gdk::RGBA color;

/*-	BEFORE ------------------------------------- */

	color=get_interp_color(tp.get_before());
	color=color_darken(color,1.0f);
	if(selected)color=color_darken(color,1.3f);
	if(hover) color = color_shift(color, 0.2);
	cr->set_source_rgb(color.get_red(),color.get_green(),color.get_blue());

	const double double_outline_margin_pixels = 2.5;
	double double_outline_margin = double_outline_margin_pixels;
	if (double_outline)
		double_outline_margin /= std::max(area.get_width(), area.get_height());

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
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->save();
			cr->translate(area.get_x(), area.get_y());
			cr->scale(area.get_width(), area.get_height());
			cr->arc(0.5, 0.5, 0.5+double_outline_margin, 90*M_PI/180.0, 270*M_PI/180.0);
			cr->restore();
			cr->stroke();
		}
		break;

	case INTERPOLATION_HALT:
		cr->save();
		cr->translate(area.get_x(), area.get_y());
		cr->scale(area.get_width(), area.get_height()*2);
		cr->move_to(0.5, 0.5);
		cr->arc(0.5, 0.5, 0.5, 180*M_PI/180.0, 270*M_PI/180.0);
		cr->fill_preserve();
		cr->restore();

		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->save();
			cr->translate(area.get_x(), area.get_y());
			cr->scale(area.get_width(), area.get_height()*2);
			cr->move_to(0.5, 0.5+double_outline_margin/2);
			cr->line_to(-double_outline_margin, 0.5+double_outline_margin/2);
			cr->arc(0.5-double_outline_margin/2, 0.5, 0.5+double_outline_margin/2, 180*M_PI/180.0, 270*M_PI/180.0);
			cr->line_to(0.5, -double_outline_margin/2);
			cr->restore();
			cr->stroke();
		}
		break;

	case INTERPOLATION_LINEAR:
		cr->move_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x(),area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->move_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.-double_outline_margin_pixels/2,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()-double_outline_margin_pixels*1.5,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;

	case INTERPOLATION_CONSTANT:
		cr->move_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x()+area.get_width()/4.,area.get_y());
		cr->line_to(area.get_x()+area.get_width()/4.,area.get_y()+area.get_height()/2.);
		cr->line_to(area.get_x(),area.get_y()+area.get_height()/2.);
		cr->line_to(area.get_x(),area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->move_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/4.-double_outline_margin_pixels,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/4.-double_outline_margin_pixels,area.get_y()+area.get_height()/2.-double_outline_margin_pixels);
			cr->line_to(area.get_x()-double_outline_margin_pixels,area.get_y()+area.get_height()/2.-double_outline_margin_pixels);
			cr->line_to(area.get_x()-double_outline_margin_pixels,area.get_y()+area.get_height() + double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height() + double_outline_margin_pixels);
			cr->stroke();
		}
		break;

	case INTERPOLATION_CLAMPED:
		cr->move_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x(),area.get_y()+area.get_height()/2.);
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->move_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()-double_outline_margin_pixels,area.get_y()+area.get_height()/2.);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;

	default:
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x()+area.get_width()/3.,area.get_y());
		cr->line_to(area.get_x(),area.get_y()+area.get_height()/3.);
		cr->line_to(area.get_x(),area.get_y()+area.get_height()-area.get_height()/3.);
		cr->line_to(area.get_x()+area.get_width()/3.,area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/3.-double_outline_margin_pixels/2,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()-double_outline_margin_pixels,area.get_y()+area.get_height()/3.-double_outline_margin_pixels/2);
			cr->line_to(area.get_x()-double_outline_margin_pixels,area.get_y()+area.get_height()-area.get_height()/3.+double_outline_margin_pixels/2);
			cr->line_to(area.get_x()+area.get_width()/3.-double_outline_margin_pixels/2,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;
	}

/*-	AFTER -------------------------------------- */

	color=get_interp_color(tp.get_after());
	color=color_darken(color,0.8f);
	if(selected)color=color_darken(color,1.3f);
	if(hover) color = color_shift(color, 0.2);
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
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->save();
			cr->translate(area.get_x(), area.get_y());
			cr->scale(area.get_width(), area.get_height());
			cr->arc(0.5, 0.5, 0.5+double_outline_margin, -90*M_PI/180.0, 90*M_PI/180.0);
			cr->restore();
			cr->stroke();
		}
		break;

	case INTERPOLATION_HALT:
		cr->save();
		cr->translate(area.get_x(), area.get_y());
		cr->scale(area.get_width(), area.get_height()*2);
		cr->move_to(0.5, 0.0);
		cr->arc(0.5, 0.0, 0.5, 0*M_PI/180.0, 90*M_PI/180.0);
		cr->fill_preserve();
		cr->restore();

		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->save();
			cr->translate(area.get_x(), area.get_y());
			cr->scale(area.get_width(), area.get_height()*2);
			cr->move_to(0.5, 0.0-double_outline_margin/2);
			cr->line_to(1.0+double_outline_margin, 0.0-double_outline_margin/2);
			cr->arc(0.5+double_outline_margin/2, 0.0, 0.5+double_outline_margin/2, 0*M_PI/180.0, 90*M_PI/180.0);
			cr->line_to(0.5, 0.5+double_outline_margin/2);
			cr->restore();
			cr->stroke();
		}
		break;

	case INTERPOLATION_LINEAR:
		cr->move_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y());
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->move_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()+1.5*double_outline_margin_pixels,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.+double_outline_margin_pixels/2,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;

	case INTERPOLATION_CONSTANT:
		cr->move_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()/2.);
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/4.,area.get_y()+area.get_height()/2.);
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/4.,area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->move_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()+double_outline_margin_pixels,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()+double_outline_margin_pixels,area.get_y()+area.get_height()/2.+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()-area.get_width()/4.+double_outline_margin_pixels,area.get_y()+area.get_height()/2.+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()-area.get_width()/4.+double_outline_margin_pixels,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;

	case INTERPOLATION_CLAMPED:
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()/2.);
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()+double_outline_margin_pixels,area.get_y()+area.get_height()/2.);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;

	default:
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y());
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/3.,area.get_y());
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()/3.);
		cr->line_to(area.get_x()+area.get_width(),area.get_y()+area.get_height()-area.get_height()/3.);
		cr->line_to(area.get_x()+area.get_width()-area.get_width()/3.,area.get_y()+area.get_height());
		cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height());
		cr->fill_preserve();
		cr->set_source_rgb(outline_color.get_red(),outline_color.get_green(),outline_color.get_blue());
		cr->stroke();

		if (double_outline) {
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()-area.get_width()/3.+double_outline_margin_pixels/2,area.get_y()-double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()+double_outline_margin_pixels,area.get_y()+area.get_height()/3.-double_outline_margin_pixels/2);
			cr->line_to(area.get_x()+area.get_width()+double_outline_margin_pixels,area.get_y()+area.get_height()-area.get_height()/3.+double_outline_margin_pixels/2);
			cr->line_to(area.get_x()+area.get_width()-area.get_width()/3.+double_outline_margin_pixels/2,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->line_to(area.get_x()+area.get_width()/2.,area.get_y()+area.get_height()+double_outline_margin_pixels);
			cr->stroke();
		}
		break;
	}
}


static Time
get_time_offset_from_vdesc(const ValueDesc &v)
{
#ifdef ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	if (v.get_value_type() != type_canvas || getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS"))
		return Time::zero();

	if (!v.get_value().get(Canvas::Handle()))
		return Time::zero();

	if (!v.parent_is_layer())
		return Time::zero();

	synfig::Layer::Handle layer = v.get_layer();

	if (!etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
		return Time::zero();

	return layer->get_param("time_offset").get(Time());
#else // ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	return synfig::Time::zero();
#endif
}

static Time
get_time_dilation_from_vdesc(const ValueDesc &v)
{
#ifdef ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	if (v.get_value_type() != type_canvas || getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS"))
		return Time(1.0);

	if (!v.get_value().get(Canvas::Handle()))
		return Time(1.0);

	if (!v.parent_is_layer())
		return Time(1.0);

	Layer::Handle layer = v.get_layer();

	if (!etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
		return Time(1.0);

	return layer->get_param("time_dilation").get(Time());
#else // ADJUST_WAYPOINTS_FOR_TIME_OFFSET
	return Time(1.0);
#endif
}

static const Node::time_set empty_time_set {};

const Node::time_set&
WaypointRenderer::get_times_from_valuedesc(const ValueDesc &v)
{
	if (v.get_value_type() == type_canvas && !getenv("SYNFIG_SHOW_CANVAS_PARAM_WAYPOINTS"))
		if(Canvas::Handle canvasparam = v.get_value().get(Canvas::Handle()))
			return canvasparam->get_times();

	//we want a dynamic list entry to override the normal...
	if (v.parent_is_value_node())
		if (ValueNode_DynamicList *parent_value_node = dynamic_cast<ValueNode_DynamicList *>(v.get_parent_value_node().get()))
			return parent_value_node->list[v.get_index()].get_times();

	if (ValueNode *base_value = v.get_value_node().get()) //don't render stuff if it's just animated...
		return base_value->get_times();

	return empty_time_set;
}

void
WaypointRenderer::foreach_visible_waypoint(const synfigapp::ValueDesc &value_desc,
		const studio::TimePlotData &time_plot_data,
		std::function<ForeachCallback> foreach_callback,
		void* data)
{
	const Node::time_set & tset = get_times_from_valuedesc(value_desc);
	if (!tset.empty()) {
		const Time time_offset = get_time_offset_from_vdesc(value_desc);
		const Time time_dilation = get_time_dilation_from_vdesc(value_desc);
		const double time_k = time_dilation == Time::zero() ? 1.0 : 1.0/time_dilation;

		for (const auto & timepoint : tset) {
			Time t = (timepoint.get_time() - time_offset)*time_k;
			if (time_plot_data.is_time_visible_extra(t)) {
				if (foreach_callback(timepoint, t, data))
					break;
			}
		}
	}
}

}
