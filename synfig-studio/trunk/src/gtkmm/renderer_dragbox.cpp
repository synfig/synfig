/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: renderer_dragbox.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "renderer_dragbox.h"
#include "workarea.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Dragbox::~Renderer_Dragbox()
{
}

const sinfg::Point&
Renderer_Dragbox::get_drag_point()const
{
	return get_work_area()->get_drag_point();
}

const sinfg::Point&
Renderer_Dragbox::get_curr_point()const
{
	return get_work_area()->get_cursor_pos();
}

bool
Renderer_Dragbox::get_enabled_vfunc()const
{
	return get_work_area()->get_dragging_mode()==WorkArea::DRAG_BOX;
}


void
Renderer_Dragbox::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& expose_area
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;
	
	const sinfg::RendDesc &rend_desc(get_work_area()->get_canvas()->rend_desc());
	
	const sinfg::Vector focus_point(get_work_area()->get_focus_point());

//	std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& tile_book(get_tile_book());
	
	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);
	
	// Calculate the window coordinates of the top-left
	// corner of the canvas.
	const sinfg::Vector::value_type
		x(focus_point[0]/get_pw()+drawable_w/2-get_w()/2),
		y(focus_point[1]/get_ph()+drawable_h/2-get_h()/2);

	/*const sinfg::Vector::value_type window_startx(window_tl[0]);
	const sinfg::Vector::value_type window_endx(window_br[0]);
	const sinfg::Vector::value_type window_starty(window_tl[1]);
	const sinfg::Vector::value_type window_endy(window_br[1]);
	*/
	const int
		tile_w(get_work_area()->get_tile_w()),
		tile_h(get_work_area()->get_tile_h());

	const int
		w(get_w()),
		h(get_h());
	
	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(drawable));
	
	//const sinfg::Vector grid_size(get_grid_size());

	const sinfg::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const sinfg::Vector::value_type window_endx(get_work_area()->get_window_br()[0]);
	const sinfg::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const sinfg::Vector::value_type window_endy(get_work_area()->get_window_br()[1]);
	const float pw(get_pw()),ph(get_ph());
	
	const sinfg::Point& curr_point(get_curr_point());
	const sinfg::Point& drag_point(get_drag_point());
	
	{
		gc->set_function(Gdk::COPY);
		gc->set_rgb_fg_color(Gdk::Color("#000000"));
		gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
		//gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

		Point tl(std::min(drag_point[0],curr_point[0]),std::min(drag_point[1],curr_point[1]));
		Point br(std::max(drag_point[0],curr_point[0]),std::max(drag_point[1],curr_point[1]));

		tl[0]=(tl[0]-window_startx)/pw;
		tl[1]=(tl[1]-window_starty)/ph;
		br[0]=(br[0]-window_startx)/pw;
		br[1]=(br[1]-window_starty)/ph;
		if(tl[0]>br[0])
			swap(tl[0],br[0]);
		if(tl[1]>br[1])
			swap(tl[1],br[1]);
		
		drawable->draw_rectangle(gc,false,
			round_to_int(tl[0]),
			round_to_int(tl[1]),
			round_to_int(br[0]-tl[0]),
			round_to_int(br[1]-tl[1])
		);
	}
}
