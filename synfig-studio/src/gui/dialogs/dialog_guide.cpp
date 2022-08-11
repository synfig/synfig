/* === S Y N F I G ========================================================= */
/*!	\file Dialog_Guide.cpp
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <gui/dialogs/dialog_guide.h>

#include <gui/localization.h>

#include <gui/widgets/widget_waypoint.h> //should probably be replaced with just box header
#include <gui/widgets/widget_value.h>
#include <gui/workarea.h>
#include <gui/app.h>


#include <cmath>//remove probably not needed

#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
const double pi = std::acos(-1);
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Guide::Dialog_Guide(Gtk::Window& parent, etl::handle<synfig::Canvas> canvas,WorkArea *work_area):
	Dialog(_("Guide Editor"),parent),
	canvas(canvas),
	angle_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	center_x_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	center_y_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	point_x_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	point_y_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0))
{
	this->set_resizable(false);
	assert(canvas);

	//Box start
	Gtk::Box *guide_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	//ok so now we need to add to the two grids then we worry about frames

	angle_widget=manage(new class Gtk::SpinButton(angle_adjustment,15,2));
	angle_widget->set_value(5.5);//this is not neeeded remove
	angle_widget->show();

	center_x_widget=manage(new class Gtk::SpinButton(center_x_widget_adjust,15,2));
	center_x_widget->show();
	center_x_widget->set_hexpand();
	center_y_widget=manage(new class Gtk::SpinButton(center_y_widget_adjust,15,2));
	center_y_widget->show();
	center_y_widget->set_hexpand();
	point_x_widget=manage(new class Gtk::SpinButton(point_x_widget_adjust,15,2));
	point_x_widget->show();
	point_x_widget->set_hexpand();
	point_y_widget=manage(new class Gtk::SpinButton(point_y_widget_adjust,15,2));
	point_y_widget->show();
	point_y_widget->set_hexpand();

	auto guideGrid = manage(new Gtk::Grid());
	guideGrid->get_style_context()->add_class("dialog-secondary-content");
	guideGrid->set_row_spacing(6);
	guideGrid->set_column_spacing(8);

	Gtk::Label *rotationAngleLabel = manage(new Gtk::Label(_("_Rotation Angle"), true));
	rotationAngleLabel->set_halign(Gtk::ALIGN_CENTER);
	rotationAngleLabel->set_mnemonic_widget(*angle_widget);
	guideGrid->attach(*rotationAngleLabel, 0, 0, 1, 1);
	guideGrid->attach(*angle_widget      , 1, 0, 1, 1);

	auto coordGrid = manage(new Gtk::Grid());
	coordGrid->get_style_context()->add_class("dialog-secondary-content");
	coordGrid->set_row_spacing(6);
	coordGrid->set_column_spacing(8);

	Gtk::Label *centerCoordLabel = manage(new Gtk::Label(_("_Center Point"), true));
	centerCoordLabel->set_halign(Gtk::ALIGN_CENTER);
	centerCoordLabel->set_margin_end(15);
	centerCoordLabel->set_mnemonic_widget(*center_x_widget);
	coordGrid->attach(*centerCoordLabel, 0, 0, 1, 1);
	coordGrid->attach(*center_x_widget   , 1, 0, 1, 1);
	coordGrid->attach(*center_y_widget   , 2, 0, 1, 1);

	Gtk::Label *otherCoordLabel = manage(new Gtk::Label(_("_Other Point"), true));
	otherCoordLabel->set_mnemonic_widget(*point_x_widget);
	otherCoordLabel->set_margin_end(15);
	coordGrid->attach(*otherCoordLabel, 0, 1, 1, 1);
	coordGrid->attach(*point_x_widget,  1, 1, 1, 1);
	coordGrid->attach(*point_y_widget,  2, 1, 1, 1);



	guide_box->add(*guideGrid);
	guide_box->add(*coordGrid);
	guide_box->set_margin_bottom(5);

	//Box end
	get_content_area()->pack_start(*guide_box);// to get the box an then pack in the beginning the box

	Gtk::Button *apply_button(manage(new Gtk::Button(_("_Apply"), true)));
	apply_button->show();
	add_action_widget(*apply_button,0);
	apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Guide::on_apply_pressed));

	Gtk::Button *ok_button(manage(new Gtk::Button(_("_OK"), true)));
	ok_button->show();
	add_action_widget(*ok_button,1);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Guide::on_ok_pressed));

	guide_box->show_all();

	signal_changed().connect(sigc::mem_fun(*this, &Dialog_Guide::set_new_coordinates));

	current_work_area = work_area;
}

Dialog_Guide::~Dialog_Guide()
{
}

void
Dialog_Guide::on_ok_pressed()
{
	if (angle_deg != angle_widget->get_value()) {
		angle_deg = angle_widget->get_value();
		angle_rad = (angle_deg * pi )/(180);
		hide();
		rotate_ruler();
	} else {
		hide();
		signal_changed_(); //we can use this as a signal to connect to something in workarea to handle whatever
	}
}

void
Dialog_Guide::on_apply_pressed()
{
	if (angle_deg != angle_widget->get_value()) {
		angle_deg = angle_widget->get_value();
		angle_rad = (angle_deg * pi )/(180);
		rotate_ruler();
	} else
		signal_changed_();
	current_work_area->get_drawing_area()->queue_draw();
}

void
Dialog_Guide::set_current_guide_iterators(GuideList::iterator curr_guide_workarea,
								 GuideList::iterator curr_guide_accomp_duckamtic_workarea,
								 GuideList::iterator curr_guide_accomp_duckamtic_other_workarea)
{
	curr_guide = curr_guide_workarea;
	curr_guide_accomp_duckamtic = curr_guide_accomp_duckamtic_workarea;
	curr_guide_accomp_duckamtic_other = curr_guide_accomp_duckamtic_other_workarea;
}

void
Dialog_Guide::rotate_ruler() //--connceted to on ok pressed through signal changed
{
	//rortate works by rotating around the center so basically we are only changing the accomp coords here here

	//coordinates
	float center_x, center_y, new_rotated_x, new_rotated_y;

	// equation of line with entered slope and passing through the point which is center
	// y-y1 = slope(x-x1)
	// y = y1 + slope(x - x1)
	// x = ((y-y1)/slope) + x1

	if (menu_guide_is_x) {
		center_x = *curr_guide;
		center_y = ((1.0/2.0)*(current_work_area->drawing_area_height)*(current_work_area->pheight)) + current_work_area->window_starty;
		new_rotated_x = center_x + 3;
		new_rotated_y = center_y + (std::tan(angle_rad))*(new_rotated_x - center_x);
		*curr_guide_accomp_duckamtic_other = new_rotated_x;
		*curr_guide_accomp_duckamtic = new_rotated_y;
	}
	else {
		center_x = ((1.0/2.0)*(current_work_area->drawing_area_width)*(current_work_area->pwidth)) + current_work_area->window_startx;
		center_y = *curr_guide;
		//handling angle being zero or 180
		if ( angle_rad == 0 ){
			new_rotated_y = center_y;
			new_rotated_x = center_x + 3;
		} else if ( angle_rad == (float)pi ){
			new_rotated_y = center_y;
			new_rotated_x = center_x - 3;
		} else {
			new_rotated_y = center_y + 3;
			new_rotated_x = ((new_rotated_y - center_y)/(std::tan(angle_rad))) + center_x;
		}
		*curr_guide_accomp_duckamtic_other = new_rotated_y;
		*curr_guide_accomp_duckamtic = new_rotated_x;
	}
}

void
Dialog_Guide::set_new_coordinates()
{
	//converting ruler coordinates back to the mouse coordinates to store
	float center_x_new =  synfig::Distance(center_x_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	float center_y_new = synfig::Distance(center_y_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	float point_x_new = synfig::Distance(point_x_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	float point_y_new = synfig::Distance(point_y_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());
	if (menu_guide_is_x) {
		*curr_guide = center_x_new;
		*curr_guide_accomp_duckamtic_other = point_x_new;
		*curr_guide_accomp_duckamtic = point_y_new;
	} else {
		*curr_guide = center_y_new;
		*curr_guide_accomp_duckamtic_other = point_y_new;
		*curr_guide_accomp_duckamtic = point_x_new;
	}
	//check first to see if the value changed before setting the iterators
}

void
Dialog_Guide::set_rotation_angle(bool curr_guide_is_x)//should probably be renamed to something like set value as this will set values for both the angle and the coords
{
	menu_guide_is_x = curr_guide_is_x;
	float center_x, center_y, rotated_x, rotated_y;
	if (menu_guide_is_x) {
		center_x = *curr_guide;
		center_y = ((1.0/2.0)*(current_work_area->drawing_area_height)*(current_work_area->pheight)) + current_work_area->window_starty;
		rotated_x = *curr_guide_accomp_duckamtic_other;//rotate cords not needed
		rotated_y = *curr_guide_accomp_duckamtic; //this isnt safe it isnt initilized before rotation
	}
	else
	{
		center_x = ((1.0/2.0)*(current_work_area->drawing_area_width)*(current_work_area->pwidth)) + current_work_area->window_startx;
		center_y = *curr_guide;
		rotated_x = *curr_guide_accomp_duckamtic;
		rotated_y = *curr_guide_accomp_duckamtic_other;
	}

	//if they arent rotated yet they are either horizontal or vertical so 0 or 90 deg
	//from the values of the accomp we can know if they arent rotated
	//but we need to know if x_ruler or y_ruler

	if (rotated_x > -900) {  /*this means it is rotated*/
		//slope = tan(theta)... theta = atan(slope)
		float slope = (rotated_y - center_y)/(rotated_x - center_x);
		float angle_rad_float = std::atan(slope);


		int quadrant=0; //unused var remove
		// we want the range to be from 0 to 360 but atan has range -90 to 90
		//p.s. this works only for when ruler was set by mouse rotation, dialog rotation shows 180 as 0 etc.. think of what to do here
		if (rotated_x > center_x){
			if (rotated_y > center_y)
				quadrant = 1;
			else {
				quadrant = 4;
				angle_rad_float += 2.0 * pi;
			}
		} else if (rotated_x < center_x) {
			if (rotated_y > center_y){
				quadrant = 2;
				angle_rad_float += pi;
			}
			else if (rotated_y < center_y){
				quadrant = 3;
				angle_rad_float += pi;
			}
			else
				angle_rad_float = pi;
		}
		else {
			if(rotated_y < center_y)
				angle_rad_float = (3.0*pi)/(2.0);
		}

		angle_rad = angle_rad_float;
		angle_deg = (angle_rad * 180)/(pi);
	}
	else
	{
		if (curr_guide_is_x)
			angle_deg = 90;
		else
			angle_deg = 0;

	}
	std::cout<<"angle in degrees: "<<angle_deg<<std::endl;

	angle_widget->set_value(angle_deg);

	double center_x_ruler_unit = synfig::Distance(center_x , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());
	double center_y_ruler_unit = synfig::Distance(center_y , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());
	double rotated_x_ruler_unit = synfig::Distance(rotated_x , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());
	double rotated_y_ruler_unit = synfig::Distance(rotated_y , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());

	if (menu_guide_is_x && rotated_x > -900) {
		center_x_widget->set_value(center_x_ruler_unit);
		point_x_widget->set_value(rotated_x_ruler_unit);
		point_y_widget->set_value(rotated_y_ruler_unit);
		center_y_widget->hide();
	} else if (!menu_guide_is_x &&  rotated_x > -900) {
		center_y_widget->set_value(center_y_ruler_unit);
		point_x_widget->set_value(rotated_x_ruler_unit);
		point_y_widget->set_value(rotated_y_ruler_unit);
		center_x_widget->hide();
	} else if (rotated_x < -900) {
		if (menu_guide_is_x) {
			center_x_widget->show();
			center_x_widget->set_value(center_x_ruler_unit);
			center_y_widget->hide();
		} else {
			center_y_widget->show();
			center_y_widget->set_value(center_y_ruler_unit);
			center_x_widget->hide();
		}
	}
}
