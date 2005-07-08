/* === S Y N F I G ========================================================= */
/*!	\file dialog_preview.cpp
**	\brief Preview dialog File
**
**	$Id: dialog_preview.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "dialog_preview.h"
#include "preview.h"
#include <gtkmm/spinbutton.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;
using namespace Gtk;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//dialog_preview stuff...
Dialog_Preview::Dialog_Preview()
:Dialog(_("Preview Window"),false,true),
settings(this,"preview")
{
	get_vbox()->pack_start(preview);
}

Dialog_Preview::~Dialog_Preview()
{
}

void Dialog_Preview::set_preview(handle<Preview>	prev)
{
	get_window().clear();
	preview.set_preview(prev);
	//preview.update();
}

void Dialog_Preview::on_hide()
{
	Dialog::on_hide();
	preview.stop();
	preview.stoprender();
}

//dialog_previewoptions stuff
Dialog_PreviewOptions::Dialog_PreviewOptions()
:Dialog(_("Preview Options"),false,true),
adj_zoom(0.5,0.1,5.0,0.1,0.2),
adj_fps(15,1,120,1,5),
check_overbegin(_("Begin Time"),false),
check_overend(_("End Time"),false),
settings(this,"prevoptions")
{
	//framerate = 15.0f;
	//zoom = 0.2f;
	
	//set the fps of the time widgets	
	Gtk::Table	*ot = manage(new class Gtk::Table);
	
	ot->attach(*manage(new class Gtk::Label(_("Zoom"))),0,1,0,1);	
	ot->attach(*manage(new class Gtk::Label(_("FPS"))),1,2,0,1);
	
	ot->attach(*manage(new class Gtk::SpinButton(adj_zoom,0.1,2)),0,1,1,2);	
	ot->attach(*manage(new class Gtk::SpinButton(adj_fps,1,1)),1,2,1,2);
	
	ot->attach(check_overbegin,0,1,2,3);
	ot->attach(check_overend,1,2,2,3);
	check_overbegin.signal_toggled().connect(sigc::mem_fun(*this,&Dialog_PreviewOptions::on_overbegin_toggle));
	check_overend.signal_toggled().connect(sigc::mem_fun(*this,&Dialog_PreviewOptions::on_overend_toggle));
		
	ot->attach(time_begin,0,1,3,4);
	ot->attach(time_end,1,2,3,4);
	
	Gtk::Button *okbutton = manage(new Gtk::Button(_("Preview")));
	okbutton->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_PreviewOptions::on_ok_pressed));
	ot->attach(*okbutton,0,2,4,5);
	
	ot->show_all();
	
	get_vbox()->pack_start(*ot);
	
	time_begin.set_sensitive(false);
	time_end.set_sensitive(false);
}

Dialog_PreviewOptions::~Dialog_PreviewOptions()
{
}

void Dialog_PreviewOptions::on_ok_pressed()
{
	PreviewInfo	i;
	i.zoom = get_zoom();
	i.fps = get_fps();
	i.overbegin = get_begin_override();
	i.overend = get_end_override();
	if(i.overbegin) i.begintime = (float)get_begintime();
	if(i.overend)	i.endtime = (float)get_endtime();
	
	hide();
	signal_finish_(i);
	signal_finish_.clear();
}

void Dialog_PreviewOptions::on_overbegin_toggle()
{
	time_begin.set_sensitive(get_begin_override());
}

void Dialog_PreviewOptions::on_overend_toggle()
{
	time_end.set_sensitive(get_end_override());
}

void studio::Dialog_PreviewOptions::set_global_fps(float f) 
{ 
	globalfps = f; 
	time_begin.set_fps(f); 
	time_end.set_fps(f);
}
