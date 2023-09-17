/* === S Y N F I G ========================================================= */
/*!	\file Dialog_Guide.cpp
**	\brief Dialog for editing guides.
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
#include <synfig/distance.h>
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
	x_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	y_adjustment(Gtk::Adjustment::create(0,-2000000000,2000000000,1,1,0)),
	degrees(true)
{
	this->set_resizable(false);
	assert(canvas);

	//Box start
	Gtk::Box *guide_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

	angle_widget=manage(new Gtk::SpinButton(angle_adjustment,15,2));
	angle_widget->show();

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

	distance_system_picker.append("0", _("Units"));
	distance_system_picker.append("1", _("Pixels"));
	distance_system_picker.append("2", _("Points"));
	distance_system_picker.append("3", _("Inches"));
	distance_system_picker.append("4", _("Meters"));
	distance_system_picker.append("5", _("Millimeters"));
	distance_system_picker.append("6", _("Centimeters"));
	distance_system_picker.set_active(0);
	distance_system_picker.signal_changed().connect(sigc::mem_fun(*this, &Dialog_Guide::set_distance_system));

	Gtk::Frame* pivotFrame = manage(new Gtk::Frame(_("Set Pivot Position (px)")));
	pivotFrame->set_shadow_type(Gtk::SHADOW_NONE);
	(static_cast<Gtk::Label*>(pivotFrame->get_label_widget()))->set_markup(_("<b>Set Pivot Position</b>"));
	pivotFrame->set_margin_bottom(5);
	pivotFrame->set_margin_top(5);
	pivotFrame->set_margin_left(5);

	auto posGrid = manage(new Gtk::Grid());
	posGrid->get_style_context()->add_class("dialog-secondary-content");
	posGrid->set_row_spacing(6);
	posGrid->set_column_spacing(8);
	
	Gtk::Label* xPosLabel = manage(new Gtk::Label(_("_X:"), true));
	Gtk::Label* yPosLabel = manage(new Gtk::Label(_("_Y:"), true));
	x_widget = new Widget_Distance();
	x_widget->show();
	y_widget = new Widget_Distance();
	y_widget->show();
	
	posGrid->attach(*xPosLabel, 0, 0, 1, 1);
	posGrid->attach(*x_widget, 1, 0, 1, 1);
	posGrid->attach(*yPosLabel, 0, 1, 1, 1);
	posGrid->attach(*y_widget, 1, 1, 1, 1);
	posGrid->attach(distance_system_picker, 2, 0, 1, 1);

	guide_box->add(*angleFrame);
	guide_box->add(*guideGrid);
	guide_box->add(*pivotFrame);
	guide_box->add(*posGrid);
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
	if (degrees && synfig::Angle::deg(curr_guide->angle).get() != angle_widget->get_value()) {
		curr_guide->angle = synfig::Angle::deg(angle_widget->get_value());
	} else if (!degrees && curr_guide->angle.get() != angle_widget->get_value()) {
		curr_guide->angle = synfig::Angle::rad(angle_widget->get_value());
	}

	const synfig::RendDesc& rend_desc(canvas->rend_desc());
	Distance x_cord(x_widget->get_value(), prev_system);
	Distance y_cord(y_widget->get_value(), prev_system);

	x_cord.convert(Distance::SYSTEM_UNITS, rend_desc);
	y_cord.convert(Distance::SYSTEM_UNITS, rend_desc);

	curr_guide->point[0] = x_cord;
	curr_guide->point[1] = y_cord;
	
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

void
Dialog_Guide::set_distance_system()
{
	const synfig::RendDesc& rend_desc(canvas->rend_desc());
	curr_system = (synfig::Distance::System) stratoi(distance_system_picker.get_active_id());
	Distance x_cord(x_widget->get_value(), prev_system);
	Distance y_cord(y_widget->get_value(), prev_system);
	x_cord.convert(curr_system, rend_desc);
	y_cord.convert(curr_system, rend_desc);
	x_widget->set_value(x_cord);
	y_widget->set_value(y_cord); 
	prev_system = (synfig::Distance::System) stratoi(distance_system_picker.get_active_id());
}

void
Dialog_Guide::init_widget_values()
{
	if(degrees)
		angle_widget->set_value(synfig::Angle::deg(curr_guide->angle).get());
	else
		angle_widget->set_value(curr_guide->angle.get());

	const synfig::RendDesc& rend_desc(canvas->rend_desc());
	prev_system = App::distance_system;
	distance_system_picker.set_active(prev_system);
	Distance x_cord(curr_guide->point[0], Distance::SYSTEM_UNITS);
	Distance y_cord(curr_guide->point[1], Distance::SYSTEM_UNITS);

	x_cord.convert(prev_system, rend_desc);
	y_cord.convert(prev_system, rend_desc);

	x_widget->set_value(x_cord);
	y_widget->set_value(y_cord);
}
