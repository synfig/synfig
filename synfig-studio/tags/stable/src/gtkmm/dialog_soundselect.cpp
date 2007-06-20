/* === S Y N F I G ========================================================= */
/*!	\file dialog_soundselect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "dialog_soundselect.h"
#include <gtkmm/table.h>

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
:Dialog(_("Sound Select")),
okbutton(_("Ok")),
canvas_interface(ci)
{
	Gtk::Table *table = manage(new Gtk::Table);

	table->attach(soundfile,0,1,0,1);
	table->attach(offset,1,2,0,1);
	table->attach(okbutton,0,2,1,2);

	table->show_all();
	get_vbox()->pack_start(*table);

	offset.set_value(0);

	okbutton.signal_clicked().connect(sigc::mem_fun(*this,&Dialog_SoundSelect::on_ok));
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
