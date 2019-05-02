/* === S Y N F I G ========================================================= */
/*!	\file renderer_bbox.cpp
**  \brief Renderer_BBox class is used to render in the workarea the bounding box
** of the selected layer(s)
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

#include <synfig/general.h>

#include "renderer_bbox.h"
#include "workarea.h"
#include "canvasview.h"
#include <gui/localization.h>

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

Renderer_BBox::Renderer_BBox()
{
}

Renderer_BBox::~Renderer_BBox()
{
}

const synfig::Rect&
Renderer_BBox::get_bbox()
{
	return get_work_area()->get_canvas_view()->get_bbox();
}

void
Renderer_BBox::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;
	if (!get_work_area()->get_canvas_view())
		return;
	if (get_work_area()->get_canvas_view()->is_ducks_locked())
		return;

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const float pw(get_pw()),ph(get_ph());

	const synfig::Point curr_point(get_bbox().get_min());
	const synfig::Point drag_point(get_bbox().get_max());
	if(get_bbox().area()<10000000000000000.0 && get_bbox().area()>0.00000000000000001)
	{
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

		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);

		cr->set_line_width(1.0);
		cr->set_source_rgb(GDK_COLOR_TO_RGB(BBOX_COLOR_OUTLINE));

		// Operator difference was added in Cairo 1.9.4
		// It currently isn't supported by Cairomm
#if CAIRO_VERSION >= 10904
		cairo_set_operator(cr->cobj(), CAIRO_OPERATOR_DIFFERENCE);
#else
		// Fallback: set color to black
        cr->set_source_rgb(GDK_COLOR_TO_RGB(BBOX_COLOR_FAILBACK));
#endif

		cr->rectangle(
			int(tl[0])+0.5,
			int(tl[1])+0.5,
			int(br[0]-tl[0]+1),
			int(br[1]-tl[1]+1)
		);
		cr->stroke();

		cr->restore();
	}
}
