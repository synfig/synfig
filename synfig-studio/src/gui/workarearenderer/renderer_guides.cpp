/* === S Y N F I G ========================================================= */
/*!	\file renderer_guides.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "renderer_guides.h"
#include "workarea.h"
#include <ETL/misc>

#include "general.h"

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

Renderer_Guides::Renderer_Guides()//:
	//dragging(false)
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
		}
		else
		{
			device=event->button.device;
		}

		// Make sure we recognize the device
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
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;


	int drawable_w = drawable->get_width();
	int drawable_h = drawable->get_height();

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const float pw(get_pw()),ph(get_ph());

	synfig::Color guides_color(get_work_area()->get_guides_color());

	// Draw out the guides
	{
		Duckmatic::GuideList::const_iterator iter;

		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

		cr->set_line_width(1.0);
		std::valarray<double> dashes(2);
		dashes[0]=5.0;
		dashes[1]=5.0;
		cr->set_dash(dashes, 0);

		// vertical
		for(iter=get_guide_list_x().begin();iter!=get_guide_list_x().end();++iter)
		{
			const float x((*iter-window_startx)/pw);

			//!TODO Remove hardcoded hightlight/selected guide color
			if(iter==get_work_area()->curr_guide)
				cr->set_source_rgb(1.0,111.0/255.0,111.0/255.0);
			else
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());

			cr->move_to(
				x,
				0
				);
			cr->line_to(
				x,
				drawable_h
			);
			cr->stroke();
		}
		// horizontal
		for(iter=get_guide_list_y().begin();iter!=get_guide_list_y().end();++iter)
		{
			const float y((*iter-window_starty)/ph);
            //!TODO Remove hardcoded hightlight/selected guide color
			if(iter==get_work_area()->curr_guide)
				cr->set_source_rgb(1.0,111.0/255.0,111.0/255.0);
			else
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());

			cr->move_to(
				0,
				y
				);
			cr->line_to(
				drawable_w,
				y
			);
			cr->stroke();
		}

		cr->restore();
	}
}
