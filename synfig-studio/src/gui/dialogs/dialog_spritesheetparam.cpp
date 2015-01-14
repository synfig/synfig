/* === S Y N F I G ========================================================= */
/*!	\file dialog_spritesheetparam.cpp
**	\brief Implementation for the SpriteSheetParam Dialog
**
**	$Id$
**
**	\legal
**	Copyright (c) 2015 Denis Zdorovtsov
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
#include "dialogs/dialog_spritesheetparam.h"
#include "general.h"

namespace studio
{

Dialog_SpriteSheetParam::Dialog_SpriteSheetParam(Gtk::Window &parent):
	Dialog_TargetParam(parent, _("Sprite sheet parameters"))
{
	Gtk::Label* offset_x_label(manage(new Gtk::Label(_("Offset X:"))));
	offset_x_label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	offset_x_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 0.0,10000.0)));
	Gtk::Label* offset_y_label(manage(new Gtk::Label(_("Offset Y:"))));
	offset_y_label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	offset_y_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 0.0,10000.0)));
	Gtk::Label* rows_label(manage(new Gtk::Label(_("Rows:"))));
	rows_label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	rows_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 0.0,1000.0)));
	Gtk::Label* columns_label(manage(new Gtk::Label(_("Columns:"))));
	columns_label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	columns_box = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 0.0,1000.0)));	
	get_vbox()->pack_start(*offset_x_label, true, true, 0);
	get_vbox()->pack_start(*offset_x_box, true, true, 0);
	get_vbox()->pack_start(*offset_y_label, true, true, 0);
	get_vbox()->pack_start(*offset_y_box, true, true, 0);
	get_vbox()->pack_start(*rows_label, true, true, 0);
	get_vbox()->pack_start(*rows_box, true, true, 0);
	get_vbox()->pack_start(*columns_label, true, true, 0);
	get_vbox()->pack_start(*columns_box, true, true, 0);
	get_vbox()->show_all();
}

Dialog_SpriteSheetParam::~Dialog_SpriteSheetParam()
{
}

void
Dialog_SpriteSheetParam::init()
{
	offset_x_box->set_value(get_tparam().offset_x);
	offset_y_box->set_value(get_tparam().offset_y);
	rows_box->set_value(get_tparam().rows);
	columns_box->set_value(get_tparam().columns);
}

void
Dialog_SpriteSheetParam::write_tparam(synfig::TargetParam & tparam_)
{
	tparam_.offset_x = offset_x_box->get_value();
	tparam_.offset_y = offset_y_box->get_value();
	tparam_.rows = rows_box->get_value();
	tparam_.columns = columns_box->get_value();
}

}