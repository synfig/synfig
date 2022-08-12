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
		Duckmatic::GuideList::const_iterator iter;
		Duckmatic::GuideList::const_iterator accomp_iter;
		Duckmatic::GuideList::const_iterator accomp_iter_other;

		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

		cr->set_line_width(1.0);
		std::valarray<double> dashes(2);
		dashes[0]=5.0;
		dashes[1]=5.0;
		cr->set_dash(dashes, 0);

		accomp_iter=get_work_area()->get_x_list_accomp_cord().begin();
		accomp_iter_other=get_work_area()->get_x_list_accomp_cord_other().begin();

		// vertical
		for(iter=get_guide_list_x().begin();iter!=get_guide_list_x().end();++iter)
		{
			const float x((*iter-window_startx)/pw);
			const float x_rotate((*accomp_iter_other-window_startx)/pw);
			const float y((*accomp_iter-window_starty)/ph);

			float x_temp = x_rotate;
			float y_temp = y;
			bool current_guide = false;
			if(iter==get_work_area()->curr_guide){
				cr->set_source_rgb(GDK_COLOR_TO_RGB(GUIDE_COLOR_CURRENT));
				current_guide = true;
			}
			else{
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());
				current_guide = false;
			}

			if(*accomp_iter > -900){ //meaning this is a rotated ruler render it diffrently

				//first we move the point to the center of rotation which for vert. is 1/2 drawable_height and x before rotate for nows sake lets just consider case of one ruler
				float center_x = x; //this has to be x_before rotation which I think would be so much easier to disable x when rotate and have sep x rotate
				float center_y = (1.0/2.0)*(drawable_h);

				//draw the center of rotation for the selected guide
				if (current_guide) {//probably should show as well when dialog is open
					cr->save();
					cr->unset_dash();
					cr->set_source_rgb(0,0,1.0);
					cr->set_line_width(1.0);
					cr->move_to(center_x + 6.0, center_y);//make a var for the four and name it center_label_width and have it be 8
					cr->line_to(center_x - 6.0, center_y);
					cr->stroke();
					cr->move_to(center_x, center_y + 6.0);
					cr->line_to(center_x, center_y - 6.0);
//					cr->set_source_rgb(0,1,0); //should it instead be a circle ?
//					cr->arc(center_x, center_y, 3.0, 0, 6.82); //make a pi const or use one
					cr->stroke();
					cr->restore();
				}

				//then we get the point where the mouse is which is (x , y)
				//determine cordinate of point relative to center
				std::string cordinate;
				if(x_rotate>center_x){
					if(y<center_y)
						cordinate="first";
					else
						cordinate="fourth";
				}
				else{
					if(y<center_y)
						cordinate="second";
					else
						cordinate="third";
				}
				float slope= (y-center_y)/(x_rotate-center_x);
				//if slope is greter than 300 absoulute then its just vertical and should be drawn as such
				if ( std::fabs(slope) > 300 ) {

					cr->move_to(
						x,
						0
						); //now the point is x,0
					cr->line_to(
						x,
						drawable_h
					);
					cr->stroke();
					accomp_iter++;
					accomp_iter_other++;
					continue;
				}
				float y_inc= slope;

				if(cordinate == "first" || cordinate == "third" ){
						//loop until y less than zero or x greater than width
						//loop on cordinate

					while( (y_temp>0) && (x_temp<drawable_w)){
						x_temp +=1/*/2*/;
						y_temp +=slope/*/2*/;

				}
				cr->move_to(center_x,center_y);

				//now point is inc succesfully we should draw
				cr->line_to(
					x_temp,
					y_temp
				);
				cr->stroke();
				cr->move_to(center_x,center_y);
				while ((y_temp<drawable_h) && (x_temp>0)) {
					x_temp -=1;
					y_temp -=slope;
					}
				cr->line_to(
					x_temp,
					y_temp
				);
				cr->stroke();
			}
				else if(cordinate == "second" || cordinate == "fourth"){

					//loop until y greater than height
					while( (y_temp<drawable_h) && (x_temp<drawable_w)){
						x_temp +=1/*/2*/;
						y_temp +=slope/*/2*/;

						}
					cr->move_to(center_x,center_y);

					// point is inc succesfully... draw
					cr->line_to(
						x_temp,
						y_temp
					);
					cr->stroke();
					cr->move_to(center_x,center_y);

					while( (y_temp>0) && (x_temp>0)){
							x_temp -=1;
							y_temp -=slope;
					}
					cr->line_to(
						x_temp,
						y_temp
					);
					cr->stroke();
				}
			} else {
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

			accomp_iter++;
			accomp_iter_other++;
		}

		accomp_iter=get_work_area()->get_y_list_accomp_cord().begin();
		accomp_iter_other=get_work_area()->get_y_list_accomp_cord_other().begin();

		// horizontal
		for(iter=get_guide_list_y().begin();iter!=get_guide_list_y().end();++iter)
		{
			const float x((*accomp_iter-window_startx)/pw);
			const float y((*iter-window_starty)/ph);
			const float y_rotate((*accomp_iter_other-window_starty)/ph);

			float x_temp = x;
			float y_temp = y_rotate;

			bool current_guide = false; //unnecessaryy just move center def higher up
			if(iter==get_work_area()->curr_guide){
				cr->set_source_rgb(GDK_COLOR_TO_RGB(GUIDE_COLOR_CURRENT));
				current_guide = true;
			}
			else{
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());
				current_guide = false;
			}

			if(*accomp_iter > -1000){

				float center_x = (1.0/2.0)*(drawable_w);
				float center_y = y;

				if(current_guide) {
					cr->save();
					cr->unset_dash();
					cr->set_source_rgb(0,0,1.0);
					cr->set_line_width(1.0);
					cr->move_to(center_x + 6.0, center_y);//make a var for the four and name it center_label_width and have it be 8
					cr->line_to(center_x - 6.0, center_y);
					cr->stroke();
					cr->move_to(center_x, center_y + 6.0);
					cr->line_to(center_x, center_y - 6.0);
//					cr->set_source_rgb(0,1,0); //should it instead be a circle ?
//					cr->arc(center_x, center_y, 3.0, 0, 6.82); //make a pi const or use one
					cr->stroke();
					cr->restore();
				}

				std::string cordinate;
				if(x>center_x){
					if(y_rotate<center_y)
						cordinate="first";
					else
						cordinate="fourth";
				}
				else{
					if(y_rotate<center_y)
						cordinate="second";
					else
						cordinate="third";
				}

				//determine slope
				float slope= (y_rotate-center_y)/(x-center_x);

				if (std::fabs(slope) > 300) {

					cr->move_to(
						x,
						0
						); //now the point is x,0
					cr->line_to(
						x,
						drawable_h
					);
					cr->stroke();
					accomp_iter++;
					accomp_iter_other++;
					continue;
				}
				float y_inc= slope;

				if(cordinate == "first" || cordinate == "third" ){
						//loop until y less than zero or x greater than width
						//loop on cordinate

					while( (y_temp>0) && (x_temp<drawable_w)){
						x_temp +=1;
						y_temp +=slope;

				}
						cr->move_to(center_x,center_y);

						//now point is inc succesfully we should draw
						cr->line_to(
							x_temp,
							y_temp
						);
						cr->stroke();

						cr->move_to(center_x,center_y);

						while( (y_temp<drawable_h) && (x_temp>0)){
							x_temp -=1;
							y_temp -=slope;

					}
						cr->line_to(
							x_temp,
							y_temp
						);
						cr->stroke();
			}
				else if(cordinate == "second" || cordinate == "fourth"){

					//loop until y greater than height
					while( (y_temp<drawable_h) && (x_temp<drawable_w)){
						x_temp +=1;
						y_temp +=slope;

						}
					cr->move_to(center_x,center_y);

					//now point is inc succesfully we should draw
					cr->line_to(
						x_temp,
						y_temp
					);
					cr->stroke();
					cr->move_to(center_x,center_y);

					while( (y_temp>0) && (x_temp>0)){
						x_temp -=1;
						y_temp -=slope;
					}
					cr->line_to(
						x_temp,
						y_temp
					);
					cr->stroke();
				}

					}else{

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
			accomp_iter++;
			accomp_iter_other++;
		}

		cr->restore();
	}
}
