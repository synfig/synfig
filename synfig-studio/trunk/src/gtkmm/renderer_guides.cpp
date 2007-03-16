/* === S Y N F I G ========================================================= */
/*!	\file renderer_guides.cpp
**	\brief Template File
**
**	$Id$
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

#include "renderer_guides.h"
#include "workarea.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Guides::Renderer_Guides():
	dragging(false)
{

}

Renderer_Guides::~Renderer_Guides()
{
}

bool
Renderer_Guides::get_enabled_vfunc()const
{
	return get_work_area()->get_show_guides();
}

std::list<float>&
Renderer_Guides::get_guide_list_x()
{
	return get_work_area()->get_guide_list_x();
}

std::list<float>&
Renderer_Guides::get_guide_list_y()
{
	return get_work_area()->get_guide_list_y();
}

bool
Renderer_Guides::event_vfunc(GdkEvent* event)
{
	synfig::Point mouse_pos;
	// float bezier_click_pos;
	// const float radius((abs(get_pw())+abs(get_ph()))*4);
	int button_pressed(0);
	float pressure(0);
	bool is_mouse(false);
	Gdk::ModifierType modifier(Gdk::ModifierType(0));

	// Handle input stuff
	if(
		event->any.type==GDK_MOTION_NOTIFY ||
		event->any.type==GDK_BUTTON_PRESS ||
		event->any.type==GDK_2BUTTON_PRESS ||
		event->any.type==GDK_3BUTTON_PRESS ||
		event->any.type==GDK_BUTTON_RELEASE
	)
	{
		GdkDevice *device;
		if(event->any.type==GDK_MOTION_NOTIFY)
		{
			device=event->motion.device;
			modifier=Gdk::ModifierType(event->motion.state);
		}
		else
		{
			device=event->button.device;
			modifier=Gdk::ModifierType(event->button.state);
		}

		// Make sure we recognise the device
		/*if(curr_input_device)
		{
			if(curr_input_device!=device)
			{
				assert(device);
				curr_input_device=device;
				signal_input_device_changed()(curr_input_device);
			}
		}
		else*/ if(device)
		{
			//curr_input_device=device;
			//signal_input_device_changed()(curr_input_device);
		}

		//assert(curr_input_device);

		// Calculate the position of the
		// input device in canvas coordinates
		// and the buttons
		if(!event->button.axes)
		{
			mouse_pos=synfig::Point(screen_to_comp_coords(synfig::Point(event->button.x,event->button.y)));
			button_pressed=event->button.button;
			pressure=1.0f;
			is_mouse=true;
			if(isnan(event->button.x) || isnan(event->button.y))
				return false;
		}
		else
		{
			double x(event->button.axes[0]);
			double y(event->button.axes[1]);
			if(isnan(x) || isnan(y))
				return false;

			pressure=event->button.axes[2];
			//synfig::info("pressure=%f",pressure);
			pressure-=0.04f;
			pressure/=1.0f-0.04f;


			assert(!isnan(pressure));

			mouse_pos=synfig::Point(screen_to_comp_coords(synfig::Point(x,y)));

			button_pressed=event->button.button;

			if(button_pressed==1 && pressure<0 && (event->any.type!=GDK_BUTTON_RELEASE && event->any.type!=GDK_BUTTON_PRESS))
				button_pressed=0;
			if(pressure<0)
				pressure=0;

			//if(event->any.type==GDK_BUTTON_PRESS && button_pressed)
			//	synfig::info("Button pressed on input device = %d",event->button.button);

			//if(event->button.axes[2]>0.1)
			//	button_pressed=1;
			//else
			//	button_pressed=0;
		}
	}
	switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		break;
	case GDK_MOTION_NOTIFY:
		break;
	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}

	return false;
}

void
Renderer_Guides::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& expose_area
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	// const synfig::RendDesc &rend_desc(get_work_area()->get_canvas()->rend_desc());

	const synfig::Vector focus_point(get_work_area()->get_focus_point());

	//std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& tile_book(get_tile_book());

	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);

	// Calculate the window coordinates of the top-left
	// corner of the canvas.
	// const synfig::Vector::value_type
	// 	x(focus_point[0]/get_pw()+drawable_w/2-get_w()/2),
	// 	y(focus_point[1]/get_ph()+drawable_h/2-get_h()/2);

	/*const synfig::Vector::value_type window_startx(window_tl[0]);
	const synfig::Vector::value_type window_endx(window_br[0]);
	const synfig::Vector::value_type window_starty(window_tl[1]);
	const synfig::Vector::value_type window_endy(window_br[1]);
	*/
	// const int
	// 	tile_w(get_work_area()->get_tile_w()),
	// 	tile_h(get_work_area()->get_tile_h());

	// const int
	// 	w(get_w()),
	// 	h(get_h());

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(drawable));

	//const synfig::Vector grid_size(get_grid_size());

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	// const synfig::Vector::value_type window_endx(get_work_area()->get_window_br()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	// const synfig::Vector::value_type window_endy(get_work_area()->get_window_br()[1]);
	const float pw(get_pw()),ph(get_ph());

	// Draw out the guides
	{
		gc->set_function(Gdk::COPY);
		gc->set_rgb_fg_color(Gdk::Color("#9f9fff"));
		gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

		Duckmatic::GuideList::const_iterator iter;

		// vertical
		for(iter=get_guide_list_x().begin();iter!=get_guide_list_x().end();++iter)
		{
			const float x((*iter-window_startx)/pw);

			if(iter==get_work_area()->curr_guide)
				gc->set_rgb_fg_color(Gdk::Color("#ff6f6f"));
			else
				gc->set_rgb_fg_color(Gdk::Color("#6f6fff"));

			drawable->draw_line(gc,
				round_to_int(x),
				0,
				round_to_int(x),
				drawable_h
			);
		}
		// horizontal
		for(iter=get_guide_list_y().begin();iter!=get_guide_list_y().end();++iter)
		{
			const float y((*iter-window_starty)/ph);

			if(iter==get_work_area()->curr_guide)
				gc->set_rgb_fg_color(Gdk::Color("#ff6f6f"));
			else
				gc->set_rgb_fg_color(Gdk::Color("#6f6fff"));

			drawable->draw_line(gc,
				0,
				round_to_int(y),
				drawable_w,
				round_to_int(y)
			);
		}
	}
}
