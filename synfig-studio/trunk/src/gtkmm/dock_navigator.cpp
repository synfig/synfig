/* === S Y N F I G ========================================================= */
/*!	\file dock_navigator.cpp
**	\brief Dock Nagivator File
**
**	$Id: dock_navigator.cpp,v 1.3 2005/01/12 00:31:11 darco Exp $
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

#include "dock_navigator.h"
#include "canvasview.h"
#include "workarea.h"

#include <cassert>
#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/target_scanline.h>
#include <synfig/surface.h>

#include <gtkmm/separator.h>

#include "asyncrenderer.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

const double log_10_2 = log(2.0);

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
studio::Widget_NavView::Widget_NavView(CanvasView::LooseHandle cv)
:canvview(cv),
adj_zoom(0,-4,4,1,2),
scrolling(false),
surface(new synfig::Surface)
{
	attach(drawto,0,4,0,1);

	attach(*manage(new Gtk::HSeparator),0,4,1,2,Gtk::SHRINK|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	//zooming stuff
	attach(zoom_print,0,1,2,3,Gtk::SHRINK|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);
	zoom_print.set_size_request(40,-1);

	Gtk::HScale *s = manage(new Gtk::HScale(adj_zoom));
	s->set_draw_value(false);
	//s->set_update_policy(Gtk::UPDATE_DELAYED);
	//s->signal_event().connect(sigc::mem_fun(*this,&Dock_Navigator::on_scroll_event));
	attach(*s,1,4,2,3,Gtk::EXPAND|Gtk::FILL,Gtk::SHRINK|Gtk::FILL);

	show_all();

	adj_zoom.signal_value_changed().connect(sigc::mem_fun(*this,&Widget_NavView::on_number_modify));

	if(cv)
	{
		drawto.signal_expose_event().connect(sigc::mem_fun(*this,&Widget_NavView::on_expose_draw));
		drawto.signal_event().connect(sigc::mem_fun(*this,&Widget_NavView::on_mouse_event));

		drawto.add_events(Gdk::BUTTON_MOTION_MASK|Gdk::BUTTON_PRESS_MASK);

		//get_canvas_view()->canvas_interface()->signal_dirty_preview()
		//				.connect(sigc::mem_fun(*this,&Widget_NavView::on_dirty_preview));
		get_canvas_view()->work_area->signal_rendering()
						.connect(sigc::mem_fun(*this,&Widget_NavView::on_dirty_preview));

		get_canvas_view()->work_area->signal_view_window_changed()
						.connect(sigc::mem_fun(*this,&Widget_NavView::on_workarea_view_change));

		//update with this canvas' view
		on_workarea_view_change();

		dirty = true;
		queue_draw();
	}

	adj_zoom.set_value(0);
}

studio::Widget_NavView::~Widget_NavView()
{
}


static void freegu8(const guint8 *p)
{
	delete [] p;
}

void studio::Widget_NavView::on_start_render()
{
	if(dirty)
	{
		//synfig::warning("Nav: Starting render");
		//synfig::warning("Nav: Rendering canvas");
		etl::handle<Target_Scanline>	targ = surface_target(surface.get());

		targ->set_canvas(get_canvas_view()->get_canvas());
		targ->set_remove_alpha();
		targ->set_avoid_time_sync();
		targ->set_quality(get_canvas_view()->get_work_area()->get_quality());
		//synfig::info("Set the quality level to: %d", get_canvas_view()->get_work_area()->get_quality());

		//this should set it to render a single frame
		RendDesc	r = get_canvas_view()->get_canvas()->rend_desc();
		r.set_time(get_canvas_view()->canvas_interface()->get_time());

		//this changes the size of the canvas to the closest thing we can find
		int sw = r.get_w(), sh = r.get_h();

		//synfig::warning("Nav: source image is %d x %d", sw,sh);

		//resize so largest dimension is 128
		int dw = sw > sh ? 128 : sw*128/sh,
			dh = sh > sw ? 128 : sh*128/sw;

		//synfig::warning("Nav: dest image is %d x %d", dw,dh);

		r.set_w(dw);
		r.set_h(dh);

		//get the pw and ph
		//float pw = r.get_pw();
		//float ph = r.get_ph();

		//synfig::warning("Nav: pixel size is %f x %f", pw,ph);

		//this renders that single frame
		targ->set_rend_desc(&r);

		//synfig::warning("Nav: Building async renderer and starting it...");

		renderer = new AsyncRenderer(targ);
		renderer->signal_success().connect(sigc::mem_fun(*this,&Widget_NavView::on_finish_render));
		renderer->start();
		dirty = false;
	}
}

void studio::Widget_NavView::on_finish_render()
{
	//convert it into our pixmap
	PixelFormat pf(PF_RGB);

	//synfig::warning("Nav: It hath succeeded!!!");

	//assert(renderer && renderer->has_success());
	DEBUGPOINT();
	//synfig::warning("Nav: now we know it really succeeded");
	if(!*surface)
	{
		synfig::warning("dock_navigator: Bad surface");
		return;
	}

	int w = 0, h = 0;
	int dw = surface->get_w();
	int dh = surface->get_h();

	if(prev)
	{
		w = prev->get_width();
		h = prev->get_height();
	}

	if(w != dw || h != dh || !prev)
	{
		const int total_bytes(dw*dh*synfig::channels(pf));

		//synfig::warning("Nav: Updating the pixbuf to be the right size, etc. (%d bytes)", total_bytes);

		prev.clear();
		guint8 *bytes = new guint8[total_bytes]; //24 bits per pixel

		//convert into our buffered dataS
		//synfig::warning("Nav: converting color format into buffer");
		convert_color_format((unsigned char *)bytes, (*surface)[0], dw*dh, pf, App::gamma);

		prev =
		Gdk::Pixbuf::create_from_data(
			bytes,	// pointer to the data
			Gdk::COLORSPACE_RGB, // the colorspace
			((pf&PF_A)==PF_A), // has alpha?
			8, // bits per sample
			dw,	// width
			dh,	// height
			dw*synfig::channels(pf), // stride (pitch)
			SigC::slot(freegu8)
		);
	}
	else
	{
		//synfig::warning("Nav: Don't need to resize");
		//convert into our buffered dataS
		//synfig::warning("Nav: converting color format into buffer");
		if(prev) //just in case we're stupid
		{
			convert_color_format((unsigned char *)prev->get_pixels(), (*surface)[0], dw*dh, pf, App::gamma);
		}
	}
	queue_draw();
}

/*	zoom slider is on exponential scale

	map: -4,4 -> small number,1600 with 100 at 0

	f(x) = 100*2^x
*/

static double unit_to_zoom(double f)
{
	return pow(2.0,f);
}

static double zoom_to_unit(double f)
{
	if(f > 0)
	{
		return log(f) / log_10_2;
	}else return -999999.0;
}

bool studio::Widget_NavView::on_expose_draw(GdkEventExpose *exp)
{
	//print out the zoom
	//HACK kind of...
	//zoom_print.set_text(strprintf("%.1f%%",100*unit_to_zoom(adj_zoom.get_value())));

	//draw the good stuff
	on_start_render();

	//if we've got a preview etc. display it...
	if(get_canvas_view() && prev)
	{
		//axis transform from units to pixel coords
		float xaxis = 0, yaxis = 0;

		int canvw = get_canvas_view()->get_canvas()->rend_desc().get_w();
		//int canvh = get_canvas_view()->get_canvas()->rend_desc().get_h();

		float pw = get_canvas_view()->get_canvas()->rend_desc().get_pw();
		float ph = get_canvas_view()->get_canvas()->rend_desc().get_ph();

		int w = prev->get_width();
		int h = prev->get_height();

		//scale up/down to the nearest pixel ratio...
		//and center in center
		int offx=0, offy=0;

		float sx, sy;
		int nw,nh;

		sx = drawto.get_width() / (float)w;
		sy = drawto.get_height() / (float)h;

		//synfig::warning("Nav redraw: now to scale the bitmap: %.3f x %.3f",sx,sy);

		//round to smallest scale (fit entire thing in window without distortion)
		if(sx > sy) sx = sy;
		//else sy = sx;

		//scaling and stuff
		// the point to navpixel space conversion should be:
		//		(navpixels / canvpixels) * (canvpixels / canvsize)
		//	or (navpixels / prevpixels) * (prevpixels / navpixels)
		xaxis = sx * w / (float)canvw;
		yaxis = xaxis/ph;
		xaxis /= pw;

		//scale to a new pixmap and then copy over to the window
		nw = (int)(w*sx);
		nh = (int)(h*sx);

		//must now center to be cool
		offx = (drawto.get_width() - nw)/2;
		offy = (drawto.get_height() - nh)/2;

		//trivial escape
		if(nw == 0 || nh == 0)return true;

		//draw to drawing area
		Glib::RefPtr<Gdk::GC>	gc = Gdk::GC::create(drawto.get_window());

		//synfig::warning("Nav: Scaling pixmap to off (%d,%d) with size (%d,%d)", offx,offy,nw, nh);
		Glib::RefPtr<Gdk::Pixbuf> scalepx = prev->scale_simple(nw,nh,Gdk::INTERP_NEAREST);

		//synfig::warning("Nav: Drawing scaled bitmap");
		drawto.get_window()->draw_pixbuf(
			gc, //GC
			scalepx, //pixbuf
			0, 0,	// Source X and Y
			offx, offy,	// Dest X and Y
			-1,-1,	// Width and Height
			Gdk::RGB_DITHER_MAX, // RgbDither
			2, 2 // Dither offset X and Y
		);

		//draw fancy red rectangle around focus point
		const Point &wtl = get_canvas_view()->work_area->get_window_tl(),
					&wbr = get_canvas_view()->work_area->get_window_br();

		gc->set_rgb_fg_color(Gdk::Color("#ff0000"));
		gc->set_line_attributes(2,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

		//it must be clamped to the drawing area though
		int l=0,rw=0,t=0,rh=0;
		const Point fp = -get_canvas_view()->work_area->get_focus_point();

		//get focus point in normal space
		rw = (int)(abs((wtl[0]-wbr[0])*xaxis));
		rh = (int)(abs((wtl[1]-wbr[1])*yaxis));

		//transform into pixel space
		l = (int)(drawto.get_width()/2 + fp[0]*xaxis - rw/2);
		t = (int)(drawto.get_height()/2 + fp[1]*yaxis - rh/2);

		//coord system:
		// tl : (offx,offy)
		// axis multipliers = xaxis,yaxis
		//synfig::warning("Nav: tl (%f,%f), br (%f,%f)", wtl[0],wtl[1],wbr[0],wbr[1]);
		//synfig::warning("Nav: tl (%f,%f), br (%f,%f)", wtl[0],wtl[1],wbr[0],wbr[1]);
		//synfig::warning("Nav: Drawing Rectangle (%d,%d) with dim (%d,%d)", l,t,rw,rh);
		drawto.get_window()->draw_rectangle(gc,false,l,t,rw,rh);
	}

	return false; //draw everything else too
}

void studio::Widget_NavView::on_dirty_preview()
{
	dirty = true;
	queue_draw();
}

bool studio::Widget_NavView::on_scroll_event(GdkEvent *event)
{
	if(get_canvas_view() && get_canvas_view()->get_work_area())
	{
		double z = unit_to_zoom(adj_zoom.get_value());

		switch(event->type)
		{
			case GDK_BUTTON_PRESS:
			{
				if(event->button.button == 1)
				{
					scrolling = true;
					get_canvas_view()->get_work_area()->set_zoom(z);
					scrolling = false;
				}
				break;
			}

			case GDK_MOTION_NOTIFY:
			{
				if(Gdk::ModifierType(event->motion.state) & Gdk::BUTTON1_MASK)
				{
					scrolling = true;
					get_canvas_view()->get_work_area()->set_zoom(z);
					scrolling = false;
				}
				break;
			}

			default:
				break;
		}
	}

	return false;
}

void studio::Widget_NavView::on_number_modify()
{
	double z = unit_to_zoom(adj_zoom.get_value());
	zoom_print.set_text(strprintf("%.1f%%",z*100.0));
	//synfig::warning("Updating zoom to %f",adj_zoom.get_value());

	if(get_canvas_view() && z != get_canvas_view()->get_work_area()->get_zoom())
	{
		scrolling = true;
		get_canvas_view()->get_work_area()->set_zoom(z);
		scrolling = false;
	}
}

void studio::Widget_NavView::on_workarea_view_change()
{
	double wz = get_canvas_view()->get_work_area()->get_zoom();
	double z = zoom_to_unit(wz);

	//synfig::warning("Updating zoom to %f -> %f",wz,z);
	if(!scrolling && z != adj_zoom.get_value())
	{
		adj_zoom.set_value(z);
		//adj_zoom.value_changed();
	}
	queue_draw();
}

bool studio::Widget_NavView::on_mouse_event(GdkEvent * e)
{
	Point p;
	bool	setpos = false;

	if(e->type == GDK_BUTTON_PRESS && e->button.button == 1)
	{
		p[0] = e->button.x - drawto.get_width()/2;
		p[1] = e->button.y - drawto.get_height()/2;

		setpos = true;
	}

	if(e->type == GDK_MOTION_NOTIFY && (Gdk::ModifierType(e->motion.state) & Gdk::BUTTON1_MASK))
	{
		p[0] = e->motion.x - drawto.get_width()/2;
		p[1] = e->motion.y - drawto.get_height()/2;

		setpos = true;
	}

	if(setpos && prev && get_canvas_view())
	{
		const Point &tl = get_canvas_view()->get_canvas()->rend_desc().get_tl();
		const Point &br = get_canvas_view()->get_canvas()->rend_desc().get_br();

		float max = abs((br[0]-tl[0]) / drawto.get_width());

		if((prev->get_width() / drawto.get_width()) < (prev->get_height() / drawto.get_height()))
			max = abs((br[1]-tl[1]) / drawto.get_height());

		float signx = (br[0]-tl[0]) < 0 ? -1 : 1;
		float signy = (br[1]-tl[1]) < 0 ? -1 : 1;

		Point pos;

		pos[0] = p[0] * max * signx;
		pos[1] = p[1] * max * signy;

		get_canvas_view()->get_work_area()->set_focus_point(-pos);

		return true;
	}

	return false;
}

//Navigator Dock Definitions

studio::Dock_Navigator::Dock_Navigator()
:Dock_CanvasSpecific("navigator",_("Navigator"),Gtk::StockID("synfig-navigator"))
{
	add(dummy);
}

studio::Dock_Navigator::~Dock_Navigator()
{
}

void studio::Dock_Navigator::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(canvas_view)
	{
		Widget *v = canvas_view->get_ext_widget("navview");

		if(!v)
		{
			v = new Widget_NavView(canvas_view);
			canvas_view->set_ext_widget("navview",v);
		}

		add(*v);
	}else
	{
		clear_previous();
		//add(dummy);
	}
}
