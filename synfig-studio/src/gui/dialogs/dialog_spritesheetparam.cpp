/* === S Y N F I G ========================================================= */
/*!	\file dialog_spritesheetparam.cpp
**	\brief Implementation for the SpriteSheetParam Dialog
**
**	$Id$
**
**	\legal
**	Copyright (c) 2015 Denis Zdorovtsov
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
#include <gui/dialogs/dialog_spritesheetparam.h>

#include <cmath>

#include <gtkmm/grid.h>

#include <gui/localization.h>

namespace studio
{

Dialog_SpriteSheetParam::Dialog_SpriteSheetParam(Gtk::Window &parent):
	Dialog_TargetParam(parent, _("Sprite sheet parameters")), 
	frame_count(0)
{
	this->set_resizable(false);
	//Checkbox
	check_button = Gtk::manage(new Gtk::CheckButton(_("Add into an existing file"),true));

	//Offset X
	Gtk::Label* offset_x_label(manage(new Gtk::Label(_("Offset X:"))));
	offset_x_label->set_halign(Gtk::ALIGN_START);
	offset_x_label->set_valign(Gtk::ALIGN_CENTER);
	offset_x_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 0.0,10000.0)));

	//Offset Y
	Gtk::Label* offset_y_label(manage(new Gtk::Label(_("Offset Y:"))));
	offset_y_label->set_halign(Gtk::ALIGN_START);
	offset_y_label->set_valign(Gtk::ALIGN_CENTER);
	offset_y_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 0.0,10000.0)));

	//Dirrection
	Gtk::Label* direction_label(manage(new Gtk::Label(_("Direction:"))));
	direction_label->set_halign(Gtk::ALIGN_START);
	direction_label->set_valign(Gtk::ALIGN_CENTER);
	direction_box = Gtk::manage(new Gtk::ComboBoxText());
	direction_box->append("horizontal");
	direction_box->append("vertical");
	direction_box->set_active(0);
	direction_box->signal_changed().connect(sigc::mem_fun(*this, &Dialog_SpriteSheetParam::on_dir_change));
	
	//Row count
	Gtk::Label* rows_label(manage(new Gtk::Label(_("Rows:"))));
	rows_label->set_halign(Gtk::ALIGN_START);
	rows_label->set_valign(Gtk::ALIGN_CENTER);
	rows_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 1.0,1000.0)));
	rows_box->signal_value_changed().connect(sigc::mem_fun(*this, &Dialog_SpriteSheetParam::on_rows_change));

	//Column count
	Gtk::Label* columns_label(manage(new Gtk::Label(_("Columns:"))));
	columns_label->set_halign(Gtk::ALIGN_START);
	columns_label->set_valign(Gtk::ALIGN_CENTER);
	columns_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 1.0,1000.0)));
	columns_box->signal_value_changed().connect(sigc::mem_fun(*this, &Dialog_SpriteSheetParam::on_cols_change));

	//Grid
	Gtk::Grid* grid = Gtk::manage(new Gtk::Grid());
	grid->attach(*check_button,0,0,2,1);
	grid->attach(*offset_x_label,0,1,1,1);
	grid->attach(*offset_x_box,1,1,1,1);
	grid->attach(*offset_y_label,2,1,1,1);
	grid->attach(*offset_y_box,3,1,1,1);
	grid->attach(*direction_label,0,2,2,1);
	grid->attach(*direction_box,1,2,3,1);
	grid->attach(*rows_label,0,3,1,1);
	grid->attach(*rows_box,1,3,1,1);
	grid->attach(*columns_label,2,3,1,1);
	grid->attach(*columns_box,3,3,1,1);
	grid->set_row_spacing (4);
	grid->set_column_spacing (2);
	grid->set_border_width(8);
	grid->show_all();

	get_content_area()->pack_start(*grid, true, true, 3);
	get_content_area()->show_all();
}

Dialog_SpriteSheetParam::~Dialog_SpriteSheetParam()
{
}

void
Dialog_SpriteSheetParam::init()
{
	frame_count = get_desc().get_frame_end() - get_desc().get_frame_start() + 1;
	
	offset_x_box->set_value(get_tparam().offset_x);
	offset_y_box->set_value(get_tparam().offset_y);
	rows_box->set_value(get_tparam().rows);
	columns_box->set_value(get_tparam().columns);
	direction_box->set_active(get_tparam().dir);
	check_button->set_active(get_tparam().append);
	on_dir_change(); //Update boxes
}

void
Dialog_SpriteSheetParam::on_dir_change()
{
	rows_box->set_sensitive(direction_box->get_active_row_number() == 1);
	rows_box->set_value(direction_box->get_active_row_number() == 1 ? frame_count : 1);
	columns_box->set_sensitive(direction_box->get_active_row_number() == 0);
	columns_box->set_value(direction_box->get_active_row_number() == 0 ? frame_count : 1);
}

void
Dialog_SpriteSheetParam::on_rows_change()
{
	static bool flag = false;
	if (direction_box->get_active_row_number() == 0)
		return;
	if ((flag = !flag))
		columns_box->set_value(ceil((double)frame_count / rows_box->get_value()));
}

void
Dialog_SpriteSheetParam::on_cols_change()
{
	static bool flag = false;
	if (direction_box->get_active_row_number() == 1)
		return;
	if ((flag = !flag))
		rows_box->set_value(ceil((double)frame_count / columns_box->get_value()));
}

void
Dialog_SpriteSheetParam::write_tparam(synfig::TargetParam & tparam_)
{
	tparam_.offset_x = offset_x_box->get_value();
	tparam_.offset_y = offset_y_box->get_value();
	tparam_.rows = rows_box->get_value();
	tparam_.columns = columns_box->get_value();
	tparam_.dir = (synfig::TargetParam::Direction)direction_box->get_active_row_number ();
	tparam_.append = check_button->get_active();
}

}
