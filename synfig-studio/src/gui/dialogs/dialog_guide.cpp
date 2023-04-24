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

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>

#include <gui/workarea.h>
#include <gui/duckmatic.h>
#include <gui/app.h>

#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Guide::Dialog_Guide(Gtk::Window& parent, etl::handle<synfig::Canvas> canvas, WorkArea* work_area):
	Dialog(_("Guide Editor"),parent),
	canvas(canvas),
	current_work_area(work_area),
	angle_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
/*	center_x_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	center_y_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	point_x_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	point_y_widget_adjust (Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)), */
	degrees(true)
{
	this->set_resizable(false);
	assert(canvas);

	//Box start
	Gtk::Box *guide_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

	angle_widget=manage(new Gtk::SpinButton(angle_adjustment,15,2));
	angle_widget->show();

/*	center_x_widget=manage(new Gtk::SpinButton(center_x_widget_adjust,15,2));
	center_x_widget->show();
	center_x_widget->set_hexpand();
	center_y_widget=manage(new Gtk::SpinButton(center_y_widget_adjust,15,2));
	center_y_widget->show();
	center_y_widget->set_hexpand();
	point_x_widget=manage(new Gtk::SpinButton(point_x_widget_adjust,15,2));
	point_x_widget->show();
	point_x_widget->set_hexpand();
	point_y_widget=manage(new Gtk::SpinButton(point_y_widget_adjust,15,2));
	point_y_widget->show();
	point_y_widget->set_hexpand(); */

	Gtk::Frame* angleFrame = manage(new Gtk::Frame(_("Rotate Guide")));
	angleFrame->set_shadow_type(Gtk::SHADOW_NONE);
	(static_cast<Gtk::Label*>(angleFrame->get_label_widget()))->set_markup(_("<b>Rotate Guide</b>"));
	angleFrame->set_margin_bottom(5);
	angleFrame->set_margin_top(5);


	auto guideGrid = manage(new Gtk::Grid());
	guideGrid->get_style_context()->add_class("dialog-secondary-content");
	guideGrid->set_row_spacing(6);
	guideGrid->set_column_spacing(8);

	angle_type_picker.append("Degree", _("Degree"));
	angle_type_picker.append("Radian", _("Radian"));
	angle_type_picker.set_active(0);
	angle_type_picker.signal_changed().connect(sigc::mem_fun(*this, &Dialog_Guide::set_angle_type));

	Gtk::Label* rotationAngleLabel = manage(new Gtk::Label(_("_Rotation Angle"), true));
	rotationAngleLabel->set_halign(Gtk::ALIGN_CENTER);
	rotationAngleLabel->set_mnemonic_widget(*angle_widget);
	guideGrid->attach(*rotationAngleLabel, 0, 0, 1, 1);
	guideGrid->attach(*angle_widget      , 1, 0, 1, 1);
	guideGrid->attach(angle_type_picker  , 2, 0, 1, 1);

/*	auto coordGrid = manage(new Gtk::Grid());
	coordGrid->get_style_context()->add_class("dialog-secondary-content");
	coordGrid->set_row_spacing(6);
	coordGrid->set_column_spacing(8);

	Gtk::Frame *coordFrame = manage(new Gtk::Frame(_("Rotate By Setting Coordinates")));
	coordFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) coordFrame->get_label_widget())->set_markup(_("<b>Rotate By Setting Coordinates</b>"));
	coordFrame->set_margin_bottom(5);
	coordFrame->set_margin_top(5);

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
*/

	guide_box->add(*angleFrame);
	guide_box->add(*guideGrid);
//	guide_box->add(*coordFrame);
//	guide_box->add(*coordGrid);
	guide_box->set_margin_bottom(5);

	//Box end
	get_content_area()->pack_start(*guide_box);

	Gtk::Button *apply_button(manage(new Gtk::Button(_("_Apply"), true)));
	apply_button->show();
	add_action_widget(*apply_button, Gtk::RESPONSE_APPLY);
	apply_button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &Dialog_Guide::on_ok_or_apply_pressed), false));

	Gtk::Button *ok_button(manage(new Gtk::Button(_("_OK"), true)));
	ok_button->show();
	add_action_widget(*ok_button, Gtk::RESPONSE_OK);
	ok_button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &Dialog_Guide::on_ok_or_apply_pressed), true));

	guide_box->show_all();

//	current_work_area = work_area;
}

Dialog_Guide::~Dialog_Guide()
{
}

void
Dialog_Guide::set_current_guide(GuideList::iterator current_guide)
{
	curr_guide = current_guide;
	init_widget_values();
}


void
Dialog_Guide::on_ok_or_apply_pressed(bool ok)
{
	if (synfig::Angle::deg(curr_guide->angle).get() != angle_widget->get_value() && degrees) {
		curr_guide->angle = synfig::Angle::deg(angle_widget->get_value());
	} else if (curr_guide->angle.get() != angle_widget->get_value() && !degrees) {
		curr_guide->angle = synfig::Angle::rad(angle_widget->get_value());
	} /*else
		set_new_coordinates();*/

	if (ok)
		hide();
	else
		current_work_area->get_drawing_area()->queue_draw();
}

void
Dialog_Guide::set_angle_type()
{
	Glib::ustring text = angle_type_picker.get_active_text();
	if (text == "Degree")
		degrees=true;
	else if (text == "Radian")
		degrees=false;
}

/*
void
Dialog_Guide::set_new_coordinates()
{
	float center_x_new = synfig::Distance(center_x_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	float center_y_new = synfig::Distance(center_y_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	float point_x_new = synfig::Distance(point_x_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	float point_y_new = synfig::Distance(point_y_widget->get_value() , App::distance_system).get(synfig::Distance::SYSTEM_UNITS, canvas->rend_desc());

	if (curr_guide->isVertical) {
		float slope = (point_y_new - curr_guide->point[1])/(point_x_new - center_x_new);
		curr_guide->point[0] = center_x_new;
		curr_guide->angle = synfig::Angle::rad(atan(slope));
	} else {
		float slope = (point_y_new - center_y_new)/(point_x_new - curr_guide->point[0]);
		curr_guide->point[1] = center_y_new;
		curr_guide->angle = synfig::Angle::rad(atan(slope));
	}
}
*/

void
Dialog_Guide::init_widget_values()
{
//	float center_x = curr_guide->point[0];
//	float center_y = curr_guide->point[1];
//	float rotated_x = center_x + 2;
//	float rotated_y = center_y + (2.0)*(tan(curr_guide->angle.get()));

	if(degrees)
		angle_widget->set_value(synfig::Angle::deg(curr_guide->angle).get());
	else
		angle_widget->set_value(curr_guide->angle.get());
/*
	double center_x_ruler_unit = synfig::Distance(center_x , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());
	double center_y_ruler_unit = synfig::Distance(center_y , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());
	double rotated_x_ruler_unit = synfig::Distance(rotated_x , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());
	double rotated_y_ruler_unit = synfig::Distance(rotated_y , synfig::Distance::SYSTEM_UNITS).get(App::distance_system, canvas->rend_desc());

	if (curr_guide->isVertical) {
		center_x_widget->set_sensitive(true);
		center_x_widget->set_value(center_x_ruler_unit);
		point_x_widget->set_value(rotated_x_ruler_unit);
		point_y_widget->set_value(rotated_y_ruler_unit);
		center_y_widget->set_sensitive(false);
	} else {
		center_y_widget->set_sensitive(true);
		center_y_widget->set_value(center_y_ruler_unit);
		point_x_widget->set_value(rotated_x_ruler_unit);
		point_y_widget->set_value(rotated_y_ruler_unit);
		center_x_widget->set_sensitive(false);
	}
*/
}
