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

#include "dialogs/dialog_soundselect.h"
#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
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
	Gtk::Alignment *dialogPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	dialogPadding->set_padding(12, 12, 12, 12);
	get_vbox()->pack_start(*dialogPadding, false, false, 0);

	Gtk::Frame *soundFrame = manage(new Gtk::Frame(_("Sound Parameters")));
	((Gtk::Label *) soundFrame->get_label_widget())->set_markup(_("<b>Sound Parameters</b>"));
	soundFrame->set_shadow_type(Gtk::SHADOW_NONE);
	dialogPadding->add(*soundFrame);

	Gtk::Alignment *framePadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	framePadding->set_padding(6, 0, 24, 0);
	soundFrame->add(*framePadding);

	Gtk::Label *fileLabel = manage(new Gtk::Label(_("_Sound File"), true));
	fileLabel->set_alignment(0, 0.5);
	fileLabel->set_mnemonic_widget(soundfile);
	Gtk::Label *offsetLabel = manage(new Gtk::Label(_("Time _Offset"), true));
	offsetLabel->set_alignment(0, 0.5);
	offsetLabel->set_mnemonic_widget(offset);

	Gtk::Table *table = manage(new Gtk::Table(2, 2, false));
	table->set_row_spacings(6);
	table->set_col_spacings(12);
	framePadding->add(*table);

	table->attach(*fileLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);
	table->attach(soundfile, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);
	table->attach(*offsetLabel, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);
	table->attach(offset, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL, 0, 0);

	okbutton = manage(new Gtk::Button(Gtk::StockID("gtk-ok")));
	add_action_widget(*okbutton, 0);

	get_vbox()->show_all();

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
