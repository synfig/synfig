/* === S Y N F I G ========================================================= */
/*!	\file widget_canvastimeslider.cpp
**	\brief Canvas Time Slider Widget Implementation File
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
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

#include <cmath>

#include <ETL/misc>

#include <synfig/general.h>

#include <gui/localization.h>
#include <gui/canvasview.h>
#include <gui/workarea.h>
#include <gui/workarearenderer/renderer_canvas.h>

#include "widget_canvastimeslider.h"


#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Widget_CanvasTimeslider::Widget_CanvasTimeslider():
	tooltip(Gtk::WINDOW_POPUP)
{
	thumb.show();

	tooltip.set_type_hint(Gdk::WINDOW_TYPE_HINT_TOOLTIP);
	tooltip.set_border_width(1);
	tooltip.add(thumb);

	add_events(Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK);
}

Widget_CanvasTimeslider::~Widget_CanvasTimeslider()
	{ set_canvas_view( CanvasView::LooseHandle() ); }

void
Widget_CanvasTimeslider::set_canvas_view(const CanvasView::LooseHandle &x)
{
	if (canvas_view == x) return;

	rendering_change.disconnect();
	canvas_view = x;
	if (canvas_view && canvas_view->get_work_area())
		rendering_change = canvas_view->get_work_area()->signal_rendering().connect(
			sigc::mem_fun(*this, &Widget_CanvasTimeslider::queue_draw) );

	queue_draw();
}

bool
Widget_CanvasTimeslider::on_motion_notify_event(GdkEventMotion* event)
{
	Cairo::RefPtr<Cairo::ImageSurface> surface;
	if ( get_width()
	  && adj_timescale
	  && canvas_view
	  && canvas_view->get_canvas()
	  && canvas_view->get_work_area()
	  && canvas_view->get_work_area()->get_renderer_canvas() )
	{
		double x     = event->x;
		double w     = (double)get_width();
		double start = adj_timescale->get_lower();
		double end   = adj_timescale->get_upper();
		float  fps   = canvas_view->get_canvas()->rend_desc().get_frame_rate();
		if (approximate_less_lp(start, end) && approximate_greater_lp(fps, 0.f)) {
			synfig::Time time(x/w*(end - start) + start);
			time = time.round(fps);
			surface = canvas_view->get_work_area()->get_renderer_canvas()->get_thumb(time);
		}
	}

	thumb.set(surface);
	if (surface && get_screen() && get_width() > 0) {
		const int space = 20;
		int tooltip_w = surface->get_width();
		int tooltip_h = surface->get_height();

		int screen_h = get_screen()->get_height();

		int w    = get_width();
		int h    = get_height();
		int left = (int)round(event->x_root - event->x);
		int top  = (int)round(event->y_root - event->y);

		int x = left;
		if (w > tooltip_w)
			x += (int)round(event->x/(double)w*(w - tooltip_w));

		int y = 0;
		bool visible = false;
		if (top > 2*space + tooltip_h) {
			y = top - space - tooltip_h;
			visible = true;
		} else
		if (screen_h - top - h > 2*space + tooltip_h) {
			y = top + h + space;
			visible = true;
		}

		if (visible) {
			tooltip.set_screen(get_screen());
			tooltip.move(x, y);
			tooltip.show();
		} else tooltip.hide();
	} else tooltip.hide();

	return true;
}

bool
Widget_CanvasTimeslider::on_leave_notify_event(GdkEventCrossing*)
{
	tooltip.hide();
	return true;
}


void
Widget_CanvasTimeslider::draw_background(const Cairo::RefPtr<Cairo::Context> &cr)
{
	Widget_Timeslider::draw_background(cr);

	if (!canvas_view || !canvas_view->get_work_area()) return;
	Canvas::Handle canvas = canvas_view->get_canvas();
	Renderer_Canvas::Handle renderer_canvas = canvas_view->get_work_area()->get_renderer_canvas();
	if (!canvas || !renderer_canvas) return;

	Glib::RefPtr<Gdk::Window> window = get_window();
	int w = get_width(), h = get_height();
	if (w == 0 || h == 0) return;

	if (!adj_timescale) return;
	double start = adj_timescale->get_lower();
	double end   = adj_timescale->get_upper();
	if (approximate_less_or_equal_lp(end, start)) return;

	RendDesc desc = canvas->rend_desc();
	float fps = desc.get_frame_rate();
	if (approximate_less_or_equal_lp(fps, 0.f)) return;
	double frame_duration = 1.0/(double)fps;

	Renderer_Canvas::StatusMap status_map;
	renderer_canvas->get_render_status(status_map);
	double k = (double)w/(end - start);
	double top = 0.0;
	double width = frame_duration*k + 1.0;
	double height = (double)h;
	for(Renderer_Canvas::StatusMap::const_iterator i = status_map.begin(); i != status_map.end(); ++i) {
		cr->save();
		switch(i->second) {
		case Renderer_Canvas::FS_PartiallyDone : cr->set_source_rgba(0.4, 0.4, 0.4, 1.0); break;
		case Renderer_Canvas::FS_InProcess     : cr->set_source_rgba(1.0, 1.0, 0.0, 1.0); break;
		case Renderer_Canvas::FS_Done          : cr->set_source_rgba(0.0, 0.8, 0.0, 1.0); break;
		default                                : cr->set_source_rgba(0.0, 0.0, 0.0, 0.0); break;
		}
		cr->rectangle(((double)i->first - start)*k, top, width, height);
		cr->fill();
		cr->restore();
	}
}
