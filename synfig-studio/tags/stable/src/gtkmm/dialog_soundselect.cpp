/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: dialog_soundselect.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

studio::Dialog_SoundSelect::Dialog_SoundSelect(Gtk::Window &parent, etl::handle<sinfgapp::CanvasInterface> ci)
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
