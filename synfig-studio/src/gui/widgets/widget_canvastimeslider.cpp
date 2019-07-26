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
	thumb.signal_draw().connect(sigc::mem_fun(*this, &Widget_CanvasTimeslider::draw_thumb));
	thumb.show();

	tooltip.set_type_hint(Gdk::WINDOW_TYPE_HINT_TOOLTIP);
	tooltip.set_border_width(1);
	tooltip.add(thumb);

	add_events(Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK);
}

Widget_CanvasTimeslider::~Widget_CanvasTimeslider()
{
	rendering_change.disconnect();
}

void
Widget_CanvasTimeslider::set_canvas_view(const CanvasView::LooseHandle &x)
{
	if (canvas_view == x) return;

	rendering_change.disconnect();
	lock_ducks.reset();
	canvas_view = x;

	set_time_model(canvas_view ? canvas_view->time_model() : etl::handle<TimeModel>());

	if (canvas_view && canvas_view->get_work_area())
		rendering_change = canvas_view->get_work_area()->signal_rendering().connect(
			sigc::mem_fun(*this, &Widget_CanvasTimeslider::queue_draw) );

	queue_draw();
}

void
Widget_CanvasTimeslider::show_tooltip(const synfig::Point &p, const synfig::Point &root)
{
	thumb_background.clear();
	thumb_surface.clear();

	Cairo::RefPtr<Cairo::SurfacePattern> pattern;
	Cairo::RefPtr<Cairo::ImageSurface> surface;
	if ( get_width()
	  && time_model
	  && canvas_view
	  && canvas_view->get_canvas()
	  && canvas_view->get_work_area()
	  && canvas_view->get_work_area()->get_renderer_canvas() )
	{
		double x   = p[0];
		double w   = (double)get_width();
		Time lower = time_model->get_visible_lower();
		Time upper = time_model->get_visible_upper();
		if (lower < upper) {
			synfig::Time time(x/w*(upper - lower) + lower);
			time = time_model->round_time(time);
			surface = canvas_view->get_work_area()->get_renderer_canvas()->get_thumb(time);
			pattern = canvas_view->get_work_area()->get_background_pattern();
		}
	}

	if (pattern && surface && get_screen() && get_width() > 0) {
		const int space = 20;
		int tooltip_w = surface->get_width();
		int tooltip_h = surface->get_height();

		int screen_h = get_screen()->get_height();

		int w    = get_width();
		int h    = get_height();
		int left = (int)round(root[0] - p[0]);
		int top  = (int)round(root[1] - p[1]);

		int x = left;
		if (w > tooltip_w)
			x += (int)round(p[0]/(double)w*(w - tooltip_w));

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
			thumb_background = pattern;
			thumb_surface = surface;
			thumb.set_size_request(thumb_surface->get_width(), thumb_surface->get_height());
			thumb.queue_draw();
			tooltip.set_screen(get_screen());
			tooltip.move(x, y);
			tooltip.show();
		} else tooltip.hide();
	} else tooltip.hide();
}

bool
Widget_CanvasTimeslider::on_button_press_event(GdkEventButton *event)
{
	if (event->button == 1 && canvas_view && canvas_view->get_work_area()) {
		lock_ducks = new LockDucks(etl::handle<CanvasView>(canvas_view));
		canvas_view->get_work_area()->clear_ducks();
		canvas_view->queue_rebuild_ducks();
	}

	//Clicking on the timeline while the animation is playing should stop the playback #415
	if (canvas_view->is_playing()) canvas_view->stop_async();

	if (event->button == 1 || event->button == 2)
		tooltip.hide();
	return Widget_Timeslider::on_button_press_event(event);
}

bool
Widget_CanvasTimeslider::on_button_release_event(GdkEventButton *event)
{
	if (event->button == 1 && canvas_view)
		lock_ducks.reset();
	//if ( (event->button == 1 && !(event->state & Gdk::BUTTON2_MASK))
	//  || (event->button == 2 && !(event->state & Gdk::BUTTON1_MASK)) )
	//	show_tooltip(Point(event->x, event->y), Point(event->x_root, event->y_root));
	return Widget_Timeslider::on_button_release_event(event);
}

bool
Widget_CanvasTimeslider::on_motion_notify_event(GdkEventMotion* event)
{
	if ((event->state & (Gdk::BUTTON1_MASK | Gdk::BUTTON2_MASK)) == 0)
		show_tooltip(Point(event->x, event->y), Point(event->x_root, event->y_root));
	return Widget_Timeslider::on_motion_notify_event(event);
}

bool
Widget_CanvasTimeslider::on_leave_notify_event(GdkEventCrossing*)
{
	tooltip.hide();
	return true;
}

bool
Widget_CanvasTimeslider::draw_thumb(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if (!thumb_background || !thumb_surface)
		return false;

	cr->save();
	cr->translate(tooltip.get_width()/2, tooltip.get_height()/2);
	cr->set_source(thumb_background);
	cr->paint();
	cr->restore();

	cr->save();
	cr->set_source(thumb_surface, 0, 0);
	cr->paint();
	cr->restore();

	return true;
}

void
Widget_CanvasTimeslider::draw_background(const Cairo::RefPtr<Cairo::Context> &cr)
{
	Widget_Timeslider::draw_background(cr);

	if (!time_model || !canvas_view || !canvas_view->get_work_area()) return;
	Renderer_Canvas::Handle renderer_canvas = canvas_view->get_work_area()->get_renderer_canvas();
	if (!renderer_canvas) return;

	int w = get_width(), h = get_height();
	if (w <= 0 || h <= 0) return;

	Time lower = time_model->get_visible_lower();
	Time upper = time_model->get_visible_upper();
	Time frame_duration = time_model->get_frame_duration();
	if (lower >= upper) return;

	double k = (double)w/(double)(upper - lower);
	double extra_pixels = 1.0;
	Time extra_time = extra_pixels*k;
	Time lower_ex = lower - frame_duration - 2.0*extra_time;
	Time upper_ex = upper + 2.0*extra_time;

	double top = 0.0;
	double width = frame_duration*k + 1.0;
	double height = (double)h;

	Renderer_Canvas::StatusMap status_map;
	renderer_canvas->get_render_status(status_map);
	for(Renderer_Canvas::StatusMap::const_iterator i = status_map.begin(); i != status_map.end(); ++i) {
		if (i->first < lower_ex || i->first > upper_ex) continue;
		cr->save();
		switch(i->second) {
		case Renderer_Canvas::FS_PartiallyDone : cr->set_source_rgba(0.4, 0.4, 0.4, 1.0); break;
		case Renderer_Canvas::FS_InProcess     : cr->set_source_rgba(1.0, 1.0, 0.0, 1.0); break;
		case Renderer_Canvas::FS_Done          : cr->set_source_rgba(0.0, 0.8, 0.0, 1.0); break;
		default                                : cr->set_source_rgba(0.0, 0.0, 0.0, 0.0); break;
		}
		cr->rectangle(((double)i->first - lower)*k, top, width, height);
		cr->fill();
		cr->restore();
	}
}
