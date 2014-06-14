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

#include "docks/dock_navigator.h"
#include "canvasview.h"
#include "workarea.h"

#include <cassert>
#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/target_scanline.h>
#include <synfig/surface.h>
#include <gdkmm/general.h>

#include <gtkmm/separator.h>

#include "asyncrenderer.h"

#include "general.h"

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
adj_zoom(Gtk::Adjustment::create(0,-4,4,1,2)),
scrolling(false),
surface(new synfig::Surface),
cairo_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1))
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

	adj_zoom->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_NavView::on_number_modify));

	if(cv)
	{
		drawto.signal_draw().connect(sigc::mem_fun(*this,&Widget_NavView::on_drawto_draw));
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

	adj_zoom->set_value(0);
}

studio::Widget_NavView::~Widget_NavView()
{
	cairo_surface_destroy(cairo_surface);
}


static void freegu8(const guint8 *p)
{
	delete [] p;
}

void studio::Widget_NavView::on_start_render()
{
	if(dirty)
	{
		//this should set it to render a single frame
		RendDesc	r = get_canvas_view()->get_canvas()->rend_desc();
		r.set_time(get_canvas_view()->canvas_interface()->get_time());

		//this changes the size of the canvas to the closest thing we can find
		int sw = r.get_w(), sh = r.get_h();
		
		//resize so largest dimension is 128
		int dw = sw > sh ? 128 : sw*128/sh,
		dh = sh > sw ? 128 : sh*128/sw;
		
		r.set_w(dw);
		r.set_h(dh);

		if(studio::App::navigator_uses_cairo)
		{
			// Create a cairo_image_target
			etl::handle<Target_Cairo> targ = cairo_image_target(&cairo_surface);
			// Fill the target with the proper information
			targ->set_canvas(get_canvas_view()->get_canvas());
			targ->set_alpha_mode(TARGET_ALPHA_MODE_FILL);
			targ->set_avoid_time_sync();
			targ->set_quality(get_canvas_view()->get_work_area()->get_quality());
			targ->set_rend_desc(&r);
			// Sets up a Asynchronous renderer
			renderer = new AsyncRenderer(targ);
		}
		else
		{
			// Create a surface_target
			etl::handle<Target_Scanline>	targ = surface_target(surface.get());
			// Fill the target with the proper information
			targ->set_canvas(get_canvas_view()->get_canvas());
			targ->set_alpha_mode(TARGET_ALPHA_MODE_FILL);
			targ->set_avoid_time_sync();
			targ->set_quality(get_canvas_view()->get_work_area()->get_quality());
			targ->set_rend_desc(&r);
			// Sets up a Asynchronous renderer
			renderer = new AsyncRenderer(targ);
		}
		// connnect the renderer success to the finish render handler
		renderer->signal_success().connect(sigc::mem_fun(*this,&Widget_NavView::on_finish_render));
		// Mark it as clean since we are to start to render
		dirty = false;
		// start the asynchronous rendering
		renderer->start();
	}
}

void studio::Widget_NavView::on_finish_render()
{
	if(studio::App::navigator_uses_cairo)
	{
		if(cairo_surface_status(cairo_surface))
			return;
		Target_Cairo::gamma_filter(cairo_surface, studio::App::gamma);
	}
	else
	{
		//convert it into our pixmap
		PixelFormat pf(PF_RGB);

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
				sigc::ptr_fun(freegu8)
			);
		}
		else
		{
			if(prev) //just in case we're stupid
			{
				convert_color_format((unsigned char *)prev->get_pixels(), (*surface)[0], dw*dh, pf, App::gamma);
			}
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

bool studio::Widget_NavView::on_drawto_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
#ifdef SINGLE_THREADED
	// don't redraw if the previous redraw is still running single-threaded
	// or we end up destroying the renderer that's rendering it
	if (App::single_threaded && renderer && renderer->updating)
		return false;
#endif

	//draw the good stuff
	on_start_render();

	//if we've got a preview etc. display it...
	if(get_canvas_view())
	{
		//axis transform from units to pixel coords
		float xaxis = 0, yaxis = 0;

		int canvw = get_canvas_view()->get_canvas()->rend_desc().get_w();
		int w, h;
	
		float pw = get_canvas_view()->get_canvas()->rend_desc().get_pw();
		float ph = get_canvas_view()->get_canvas()->rend_desc().get_ph();
		if(prev && !studio::App::navigator_uses_cairo)
		{
			w = prev->get_width();
			h = prev->get_height();
		}
		if(studio::App::navigator_uses_cairo)
		{
			w=cairo_image_surface_get_width(cairo_surface);
			h=cairo_image_surface_get_height(cairo_surface);
		}

		//scale up/down to the nearest pixel ratio...
		//and center in center
		float offx=0, offy=0;

		float sx, sy;
		int nw,nh;

		sx = drawto.get_width() / (float)w;
		sy = drawto.get_height() / (float)h;

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
		if(prev && !studio::App::navigator_uses_cairo)
		{
			Glib::RefPtr<Gdk::Pixbuf> scalepx = prev->scale_simple(nw,nh,Gdk::INTERP_NEAREST);

			cr->save();

			//synfig::warning("Nav: Drawing scaled bitmap");
			Gdk::Cairo::set_source_pixbuf(
				cr, //cairo context
				scalepx, //pixbuf
				(int)offx, (int)offy //coordinates to place upper left corner of pixbuf
				);
			cr->paint();
			cr->restore();
		}
		if(studio::App::navigator_uses_cairo)
		{
			cr->save();
			cr->scale(sx, sx);
			cairo_set_source_surface(cr->cobj(), cairo_surface, offx/sx, offy/sx);
			cairo_pattern_set_filter(cairo_get_source(cr->cobj()), CAIRO_FILTER_NEAREST);
			cr->paint();
			cr->restore();	
		}
		cr->save();
		//draw fancy red rectangle around focus point
		const Point &wtl = get_canvas_view()->work_area->get_window_tl(),
					&wbr = get_canvas_view()->work_area->get_window_br();

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

		cr->set_line_width(2.0);
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);
		// Visually distinguish when using Cairo on Navigator or not.
		if(!studio::App::navigator_uses_cairo)
			cr->set_source_rgb(1,0,0);
		else
			cr->set_source_rgb(0,1,0);
		cr->rectangle(l,t,rw,rh);
		cr->stroke();

		cr->restore();
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
		double z = unit_to_zoom(adj_zoom->get_value());

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
	double z = unit_to_zoom(adj_zoom->get_value());
	zoom_print.set_text(strprintf("%.1f%%",z*100.0));
	//synfig::warning("Updating zoom to %f",adj_zoom->get_value());

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
	if(!scrolling && z != adj_zoom->get_value())
	{
		adj_zoom->set_value(z);
		//adj_zoom->value_changed();
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

	if(setpos && (prev||studio::App::navigator_uses_cairo) && get_canvas_view())
	{
		const Point &tl = get_canvas_view()->get_canvas()->rend_desc().get_tl();
		const Point &br = get_canvas_view()->get_canvas()->rend_desc().get_br();
		int w,h;
		if(prev && !studio::App::navigator_uses_cairo)
		{
			w = prev->get_width();
			h = prev->get_height();
		}
		if(studio::App::navigator_uses_cairo)
		{
			w=cairo_image_surface_get_width(cairo_surface);
			h=cairo_image_surface_get_height(cairo_surface);
		}
		float max = abs((br[0]-tl[0]) / drawto.get_width());

		if((float(w) / drawto.get_width()) < (float(h) / drawto.get_height()))
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
