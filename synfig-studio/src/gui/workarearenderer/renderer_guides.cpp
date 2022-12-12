/* === S Y N F I G ========================================================= */
/*!	\file renderer_guides.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2011 Nikita Kitaev
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

#define GUIDE_COLOR_CURRENT     Gdk::RGBA("#ff6f6f")

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Guides::~Renderer_Guides()
{
}

bool
Renderer_Guides::get_enabled_vfunc()const
{
	return get_work_area()->get_show_guides();
}

bool
Renderer_Guides::event_vfunc(GdkEvent* /*event*/)
{
	// TODO : All the guides stuff done in WorkArea::on_drawing_area_event(GdkEvent *event)
	// could be done here for better code maintenance (or not).
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
		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

		cr->set_line_width(1.0);
		std::valarray<double> dashes(2);
		dashes[0]=5.0;
		dashes[1]=5.0;
		cr->set_dash(dashes, 0);

		Duckmatic::GuideList::const_iterator iter;

		for (iter = get_work_area()->get_guide_list().begin(); iter!=get_work_area()->get_guide_list().end(); ++iter)
		{

			const float x_center((iter->point[0]-window_startx)/pw);
			const float y_center((iter->point[1]-window_starty)/ph);
			bool current_guide = false;

			if(iter==get_work_area()->curr_guide){
				cr->set_source_rgb(GDK_COLOR_TO_RGB(GUIDE_COLOR_CURRENT));
				current_guide = true;
			}
			else{
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());
				current_guide = false;
			}
			if(iter->angle.get() != 0 && synfig::Angle::deg(iter->angle).get() != 90){
				//draw the center of rotation for the selected guide
				if (current_guide) {
					cr->save();
					cr->unset_dash();
					cr->set_source_rgb(0,0,1.0);
					cr->set_line_width(1.0);
					cr->move_to(x_center + 6.0, y_center);
					cr->line_to(x_center - 6.0, y_center);
					cr->stroke();
					cr->move_to(x_center, y_center + 6.0);
					cr->line_to(x_center, y_center - 6.0);
//					cr->set_source_rgb(0,1,0); //should it instead be a circle ?
//					cr->arc(x_center, y_center, 3.0, 0, 6.82);
					cr->stroke();
					cr->restore();
				}
				float slope = -tan(iter->angle.get());
				synfig::Point point1;
				synfig::Point point2;
				bool point1_done = false, point2_done = false;
				float temp =0;
				// equation of guide: y = slope*(x - x_center) + y_center
				// intersection with x = 0
				temp =  slope * (-x_center) + y_center;
				if (temp>0 && temp < drawable_h){
					point1_done = true;
					point1 = synfig::Point(0.0, temp);
				}
				//intersection with x = drawable_w
				temp =  slope * (drawable_w - x_center) + y_center;
				if (temp>0 && temp < drawable_h){
					if (point1_done){
						point2 = synfig::Point(drawable_w, temp);
						point2_done = true;
					} else{
						point1 = synfig::Point(drawable_w, temp);
						point1_done = true;
					}
				}
				if (!point2_done) {
					//intersection with y = 0
					temp = x_center + (-y_center)/slope;
					if (temp>0.0 && temp < drawable_w){
						if (point1_done){
							point2 = synfig::Point(temp, 0.0);
							point2_done = true;
						} else {
							point1 = synfig::Point(temp, 0.0);
						}
					}
				}
				if (!point2_done){
					//intersection with y = drawable_h
					temp = x_center + (drawable_h - y_center)/slope;
					if (temp>0.0 && temp < drawable_w){
						point2 = synfig::Point(temp, drawable_h);
					}
				}
				cr->move_to(point1[0],point1[1]);
				cr->line_to(
				point2[0],
				point2[1]
				);
				cr->stroke();
			} else {
				if (iter->isVertical){
					cr->move_to(
						x_center,
						0
						);
					cr->line_to(
						x_center,
						drawable_h
					);
					cr->stroke();
				} else {
					cr->move_to(
						0,
						y_center
						);
					cr->line_to(
						drawable_w,
						y_center
					);
					cr->stroke();
				}
			}
		}

		cr->restore();
	}
}
