/* === S Y N F I G ========================================================= */
/*!	\file dock_navigator.cpp
**	\brief Dock Nagivator File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**  Copyright (c) 2011 Nikita Kitaev
**  ......... ... 2018 Ivan Mahonin
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

#include <cassert>
#include <algorithm>

#include <gdkmm/general.h>
#include <gtkmm/separator.h>

#include <synfig/general.h>

#include <gui/localization.h>
#include <workarea.h>
#include <canvasview.h>
#include <workarearenderer/renderer_canvas.h>

#include "dock_navigator.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */


Widget_NavView::Widget_NavView():
	adj_zoom(Gtk::Adjustment::create(0, -4, 4, 1, 2)),
	scrolling(0)
{
	//zooming stuff
	zoom_print.set_size_request(40,-1);
	Gtk::HScale *hs = manage(new Gtk::HScale(adj_zoom));
	hs->set_draw_value(false);

	Gtk::HSeparator *sep = manage(new Gtk::HSeparator());

	attach(drawto,     0, 4, 0, 1);
	attach(*sep,       0, 4, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
	attach(zoom_print, 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
	attach(*hs,        1, 4, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
	show_all();

	adj_zoom->signal_value_changed().connect(sigc::mem_fun(*this, &Widget_NavView::on_number_modify));

	drawto.signal_draw().connect(sigc::mem_fun(*this, &Widget_NavView::on_drawto_draw));
	drawto.signal_event().connect(sigc::mem_fun(*this, &Widget_NavView::on_mouse_event));
	drawto.add_events(Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_PRESS_MASK);

	adj_zoom->set_value(0);
}

Widget_NavView::~Widget_NavView()
	{ set_canvas_view( CanvasView::LooseHandle() ); }

void
Widget_NavView::set_canvas_view(const etl::loose_handle<CanvasView> &x)
{
	if (canvas_view == x) return;

	view_window_changed.disconnect();
	rendering_tile_finished.disconnect();
	time_changed.disconnect();

	canvas_view = x;

	if (canvas_view)
	if (WorkArea *work_area = canvas_view->get_work_area()) {
		view_window_changed = work_area->signal_view_window_changed().connect(
			sigc::mem_fun(*this, &Widget_NavView::on_view_window_changed) );
		rendering_tile_finished = work_area->signal_rendering_tile_finished().connect(
			sigc::mem_fun(*this, &Widget_NavView::on_rendering_tile_finished) );
		time_changed = canvas_view->time_model()->signal_time_changed().connect(
			sigc::mem_fun(*this, &Widget_NavView::queue_draw) );
	}

	queue_draw();
}

bool
Widget_NavView::on_drawto_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if (!canvas_view) return false;
	Canvas::Handle canvas = canvas_view->get_canvas();
	if (!canvas) return false;
	WorkArea *work_area = canvas_view->get_work_area();
	if (!work_area) return false;
	Renderer_Canvas::Handle renderer_canvas = work_area->get_renderer_canvas();
	if (!renderer_canvas) return false;

	synfig::Time time = canvas_view->get_time();
	Cairo::RefPtr<Cairo::ImageSurface> new_surface = renderer_canvas->get_thumb(time);
	if (new_surface) surface = new_surface;
	if (!surface)
		return false;

	//axis transform from units to pixel coords
	const RendDesc &desc = canvas->rend_desc();
	int canvw = desc.get_w();
	Real pw   = desc.get_pw();
	Real ph   = desc.get_ph();
	int w     = surface->get_width();
	int h     = surface->get_height();
	if (w == 0 || h == 0)
		return false;

	// round to smallest scale (fit entire thing in window without distortion)
	Real scale = std::min( drawto.get_width()/(Real)w, drawto.get_height()/(Real)h );

	//scale to a new pixmap and then copy over to the window
	int nw = (int)(w*scale);
	int nh = (int)(h*scale);
	if (nw == 0 || nh == 0)
		return false;

	// scaling and stuff
	// the point to navpixel space conversion should be:
	//      (navpixels / canvpixels) * (canvpixels / canvsize)
	//   or (navpixels / prevpixels) * (prevpixels / navpixels)
	Real xaxis = scale*w/(Real)canvw;
	Real yaxis = xaxis/ph;
	xaxis /= pw;

	//must now center to be cool
	Real offx = 0.5*(drawto.get_width() - nw);
	Real offy = 0.5*(drawto.get_height() - nh);

	// draw background
    cr->save();
    cr->rectangle((int)offx, (int)offy, nw, nh);
    cr->clip();
	cr->translate(nw/2, nh/2);
    cr->set_source(work_area->get_background_pattern());
    cr->paint();
    cr->restore();

	// draw surface
	cr->save();
	cr->translate((int)offx, (int)offy);
	cr->scale(nw/(Real)w, nh/(Real)h);
	cr->rectangle(0.0, 0.0, w, h);
	cr->clip();
	cr->set_source(surface, 0.0, 0.0);
	cr->paint();
	cr->restore();

	// draw fancy red rectangles around focus point and image
	const Point &wtl = get_canvas_view()->get_work_area()->get_window_tl(),
				&wbr = get_canvas_view()->get_work_area()->get_window_br();

	// it must be clamped to the drawing area though
	const Point fp = -get_canvas_view()->get_work_area()->get_focus_point();

	// get focus point in normal space
	int rw = (int)(fabs((wtl[0] - wbr[0])*xaxis));
	int rh = (int)(fabs((wtl[1] - wbr[1])*yaxis));

	// transform into pixel space
	int l = (int)(0.5*drawto.get_width()  + fp[0]*xaxis - 0.5*rw);
	int t = (int)(0.5*drawto.get_height() + fp[1]*yaxis - 0.5*rh);

	// coord system:
	//   tl : (offx,offy)
	//   axis multipliers = xaxis,yaxis
	cr->save();
	cr->set_line_width(2.0);
	cr->set_line_cap(Cairo::LINE_CAP_BUTT);
	cr->set_line_join(Cairo::LINE_JOIN_MITER);
	cr->set_antialias(Cairo::ANTIALIAS_NONE);
	cr->set_source_rgb(1.0, 0.0, 0.0);
	cr->rectangle(l, t, rw, rh);
    cr->rectangle((int)offx, (int)offy, nw, nh);
	cr->stroke();
	cr->restore();

	// draw everything else too
	return false;
}

void
Widget_NavView::on_number_modify()
{
	// zoom slider is on exponential scale
	// map: -4,4 -> small number,1600 with 100 at 0
	// f(x) = 100*2^x
	double z = pow(2.0, adj_zoom->get_value());
	zoom_print.set_text(etl::strprintf("%.1f%%", z*100.0));
	if(get_canvas_view() && z != get_canvas_view()->get_work_area()->get_zoom()) {
		struct Lock {
			int &i;
			Lock(int &i): i(i) { ++i; }
			~Lock() { --i; }
		} lock(scrolling);
		get_canvas_view()->get_work_area()->set_zoom(z);
	}
}

void
Widget_NavView::on_view_window_changed()
{
	// inverted calculations of on_number_modify()
	double wz = get_canvas_view()->get_work_area()->get_zoom();
	double z = approximate_greater_lp(wz, 0.0) ? log(wz)/log(2.0) : -999999.0;
	if (!scrolling && z != adj_zoom->get_value())
		adj_zoom->set_value(z);
	queue_draw();
}

void
Widget_NavView::on_rendering_tile_finished(synfig::Time time)
{
	if (canvas_view && canvas_view->get_time() == time)
		queue_draw();
}

bool
studio::Widget_NavView::on_mouse_event(GdkEvent * e)
{
	int dw = drawto.get_width();
	int dh = drawto.get_height();

	Point p;
	bool setpos = false;
	if(e->type == GDK_BUTTON_PRESS && e->button.button == 1) {
		p[0] = e->button.x - 0.5*dw;
		p[1] = e->button.y - 0.5*dh;
		setpos = true;
	}
	if(e->type == GDK_MOTION_NOTIFY && (Gdk::ModifierType(e->motion.state) & Gdk::BUTTON1_MASK)) {
		p[0] = e->motion.x - 0.5*dw;
		p[1] = e->motion.y - 0.5*dh;
		setpos = true;
	}

	if(setpos && surface && get_canvas_view()) {
		const Point &tl = get_canvas_view()->get_canvas()->rend_desc().get_tl();
		const Point &br = get_canvas_view()->get_canvas()->rend_desc().get_br();
		if (tl[0] < br[0]) p[0] = -p[0];
		if (tl[1] < br[1]) p[1] = -p[1];

		int w = surface->get_width();
		int h = surface->get_height();
		Real max = dw*h < dh*w
				 ? fabs((br[0] - tl[0])/(Real)dw)
				 : fabs((br[1] - tl[1])/(Real)dh);
		
		get_canvas_view()->get_work_area()->set_focus_point(p*max);
		return true;
	}

	return false;
}


// Navigator Dock Definitions

Dock_Navigator::Dock_Navigator():
	Dock_CanvasSpecific("navigator", _("Navigator"),Gtk::StockID("synfig-navigator"))
{
	add(navview);
}

Dock_Navigator::~Dock_Navigator()
{ }

void
Dock_Navigator::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	navview.set_canvas_view(canvas_view);
}
