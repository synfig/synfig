/* === S Y N F I G ========================================================= */
/*!	\file renderer_dragbox.cpp
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

#include "renderer_dragbox.h"
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

Renderer_Dragbox::~Renderer_Dragbox()
{
}

const synfig::Point&
Renderer_Dragbox::get_drag_point()const
{
	return get_work_area()->get_drag_point();
}

const synfig::Point&
Renderer_Dragbox::get_curr_point()const
{
	return get_work_area()->get_cursor_pos();
}

bool
Renderer_Dragbox::get_enabled_vfunc()const
{
	return get_work_area()->get_dragging_mode()==WorkArea::DRAG_BOX;
}

bool
Renderer_Dragbox::event_vfunc(GdkEvent* event)
{
    switch(event->type)
    {
    case GDK_BUTTON_PRESS:
        {
// Seems to be not received ! event_mask ?
        }
        break;
    case GDK_MOTION_NOTIFY:
    {
        if(get_work_area()->get_dragmode() == WorkArea::DRAG_BOX)
        {
            if (drag_paused)
            {
                selected_handles_= get_work_area()->get_selected_ducks();
                drag_paused = false;
            }
            const synfig::Point& curr_point(get_curr_point());
            const synfig::Point& drag_point(get_drag_point());
            Gdk::ModifierType modifier(Gdk::ModifierType(0));

            modifier = Gdk::ModifierType(event->button.state);
            // when dragging a box around some ducks:
            // SHIFT selects; CTRL toggles; SHIFT+CTRL unselects; <none> clears all then selects
            if(modifier&GDK_SHIFT_MASK)
            {

                DuckList::const_iterator iter;
                get_work_area()->clear_selected_ducks();
                for(iter=selected_handles_.begin();iter!=selected_handles_.end();++iter)
                {
                    get_work_area()->select_duck((*iter));
                }

                get_work_area()->select_ducks_in_box(drag_point,curr_point);

            }

            if(modifier&GDK_CONTROL_MASK)
                get_work_area()->toggle_select_ducks_in_box(drag_point,curr_point);
            else if(!(modifier&GDK_SHIFT_MASK))
            {
                get_work_area()->clear_selected_ducks();
                get_work_area()->select_ducks_in_box(drag_point,curr_point);
            }
        }
    }
    break;
    case GDK_BUTTON_RELEASE:
    {
        drag_paused = true;
    }
        break;

    default:
        break;
    }

    return false;
}

void
Renderer_Dragbox::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	// const synfig::Vector focus_point(get_work_area()->get_focus_point());
    // Warning : Unused focus_point
    //int drawable_w = drawable->get_width();
    // Warning : Unused drawable_w
    //int drawable_h = drawable->get_height();
    // Warning : Unused drawable_h

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const float pw(get_pw()),ph(get_ph());

	const synfig::Point& curr_point(get_curr_point());
	const synfig::Point& drag_point(get_drag_point());

	{
		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

		cr->set_line_width(1.0);
		cr->set_source_rgb(0,0,0);
		std::valarray<double> dashes(2);
		dashes[0]=5.0;
		dashes[1]=5.0;
		cr->set_dash(dashes, 0);

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

		cr->rectangle(
			tl[0],
			tl[1],
			br[0]-tl[0],
			br[1]-tl[1]
		);
		cr->stroke();

		cr->restore();
	}
}
