/* === S Y N F I G ========================================================= */
/*!	\file dialog_targetparam.cpp
**	\brief Implementation for the TargetParam Dialog
**
**	$Id$
**
**	\legal
**	Copyright (c) 2010 Carlos López González
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

#include <gui/dialogs/dialog_targetparam.h>

#include <gui/localization.h>

/* === U S I N G =========================================================== */

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace studio {

Dialog_TargetParam::Dialog_TargetParam(Gtk::Window &parent,  
                                       const char* title = _("Target Parameters")):
	Gtk::Dialog(title, parent)
{
	cancel_button = manage(new class Gtk::Button(Gtk::StockID("gtk-cancel")));
	cancel_button->show();
	add_action_widget(*cancel_button,Gtk::RESPONSE_CANCEL);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_TargetParam::on_cancel));

	ok_button = manage(new class Gtk::Button(Gtk::StockID("gtk-ok")));
	ok_button->show();
	add_action_widget(*ok_button,Gtk::RESPONSE_OK);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_TargetParam::on_ok));
}

int Dialog_TargetParam::run()
{
	init();
	return Gtk::Dialog::run();
}

void
Dialog_TargetParam::on_ok()
{
	write_tparam(tparam_);
	hide();
}

void
Dialog_TargetParam::on_cancel()
{
	hide();
}

}
