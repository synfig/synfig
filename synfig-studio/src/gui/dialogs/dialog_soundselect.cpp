/* === S Y N F I G ========================================================= */
/*!	\file dialog_soundselect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 David Roden
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

#include <gui/dialogs/dialog_soundselect.h>

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

studio::Dialog_SoundSelect::Dialog_SoundSelect(Gtk::Window &parent, etl::handle<synfigapp::CanvasInterface> ci)
:Dialog(_("Sound Select"), parent),
canvas_interface(ci)
{

	Gtk::Frame *soundFrame = manage(new Gtk::Frame(_("Sound Parameters")));
	((Gtk::Label *) soundFrame->get_label_widget())->set_markup(_("<b>Sound Parameters</b>"));
	soundFrame->get_style_context()->add_class("dialog-main-content");
	soundFrame->set_shadow_type(Gtk::SHADOW_NONE);
	soundFrame->set_vexpand(true);
	soundFrame->set_hexpand(true);
	get_content_area()->pack_start(*soundFrame, false, false, 0);

	Gtk::Label *fileLabel = manage(new Gtk::Label(_("_Sound File"), true));
	fileLabel->set_halign(Gtk::ALIGN_START);
	fileLabel->set_valign(Gtk::ALIGN_CENTER);
	fileLabel->set_mnemonic_widget(soundfile);
	Gtk::Label *offsetLabel = manage(new Gtk::Label(_("Time _Offset"), true));
	offsetLabel->set_halign(Gtk::ALIGN_START);
	offsetLabel->set_valign(Gtk::ALIGN_CENTER);
	offsetLabel->set_mnemonic_widget(offset);

	Gtk::Table *table = manage(new Gtk::Table(2, 2, false));
	table->get_style_context()->add_class("dialog-secondary-content");
	table->set_row_spacings(6);
	table->set_col_spacings(12);
	table->set_vexpand(true);
	table->set_hexpand(true);
	soundFrame->add(*table);

	table->attach(*fileLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);
	table->attach(soundfile, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);
	table->attach(*offsetLabel, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);
	table->attach(offset, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);

	okbutton = manage(new Gtk::Button(Gtk::StockID("gtk-ok")));
	add_action_widget(*okbutton, 0);

	get_content_area()->show_all();

	offset.set_value(0);

	okbutton->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_SoundSelect::on_ok));
}

studio::Dialog_SoundSelect::~Dialog_SoundSelect()
{
}

void studio::Dialog_SoundSelect::on_file()
{
	signal_file_changed_(soundfile.get_value());
}

void studio::Dialog_SoundSelect::on_offset()
{
	signal_offset_changed_(offset.get_value());
}

void studio::Dialog_SoundSelect::on_ok()
{
	hide();

	//signal_finish_(a);
	signal_file_changed_(soundfile.get_value());
	signal_offset_changed_(offset.get_value());
}

void studio::Dialog_SoundSelect::set_global_fps(float f)
{
	offset.set_fps(f);
}
