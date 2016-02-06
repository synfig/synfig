/* === S Y N F I G ========================================================= */
/*!	\file dialog_preview.cpp
**	\brief Preview dialog File
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

#include "dialogs/dialog_preview.h"
#include "preview.h"
#include <gtkmm/spinbutton.h>
#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include "general.h"

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
:settings(this,"preview"),preview_table(1, 1, true)

{
	set_title(_("Preview Window"));
	set_keep_above();
	add(preview_table);
	preview_table.attach(preview, 0, 1, 0, 1);
	preview.show();
	preview_table.show();

	//catch key press event
	signal_key_press_event().connect(sigc::mem_fun(*this, &Dialog_Preview::on_key_pressed));
}

Dialog_Preview::~Dialog_Preview()
{
}

void Dialog_Preview::set_preview(etl::handle<Preview> prev)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(window) window.clear();

	preview.set_preview(prev);
	//preview.update();
}

void Dialog_Preview::on_show()
{
	Window::on_show();
	preview.on_show();
}

void Dialog_Preview::on_hide()
{
	Window::on_hide();
	preview.on_hide();
}

//press escape key to close window
bool Dialog_Preview::on_key_pressed(GdkEventKey *ev)
{
	if (ev->keyval == gdk_keyval_from_name("Escape") )
	{
		close_window_handler();
		return true;
	}

	return false;
}

void Dialog_Preview::close_window_handler()
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return;

	if ((window->get_state() & Gdk::WINDOW_STATE_MAXIMIZED) != 0)
	{
	unmaximize();
	}

	hide();
}

//dialog_previewoptions stuff
Dialog_PreviewOptions::Dialog_PreviewOptions()
:Dialog(_("Preview Options")),
adj_zoom(Gtk::Adjustment::create(0.5,0.1,5.0,0.1,0.2)),
adj_fps(Gtk::Adjustment::create(15,1,120,1,5)),
check_use_cairo(_("Use _Cairo render"), false),
check_overbegin(_("_Begin time"),false),
check_overend(_("_End time"),false),
settings(this,"prevoptions")
{
	//framerate = 15.0f;
	//zoom = 0.2f;

	//set the fps of the time widgets
	Gtk::Alignment *dialogPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	dialogPadding->set_padding(12, 12, 12, 12);
	get_vbox()->add(*dialogPadding);

	Gtk::VBox *dialogBox = manage(new Gtk::VBox(false, 12));
	dialogPadding->add(*dialogBox);

	Gtk::Frame *generalFrame = manage(new Gtk::Frame(_("General settings")));
	generalFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) generalFrame->get_label_widget())->set_markup(_("<b>General settings</b>"));
	dialogBox->pack_start(*generalFrame, false, false, 0);

	Gtk::Alignment *generalPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	generalPadding->set_padding(6, 0, 24, 0);
	generalFrame->add(*generalPadding);

	Gtk::Table *generalTable = manage(new Gtk::Table(2, 2, false));
	generalTable->set_row_spacings(6);
	generalTable->set_col_spacings(12);
	generalPadding->add(*generalTable);

	Gtk::Label *zoomLabel = manage(new Gtk::Label(_("_Quality")));
	zoomLabel->set_alignment(0, 0.5);
	zoomLabel->set_use_underline(TRUE);
	Gtk::SpinButton *zoomSpinner = manage(new Gtk::SpinButton(adj_zoom, 0.1, 2));
	zoomLabel->set_mnemonic_widget(*zoomSpinner);
	zoomSpinner->set_alignment(1);
	generalTable->attach(*zoomLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	generalTable->attach(*zoomSpinner, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Label *fpsLabel = manage(new Gtk::Label(_("_FPS")));
	fpsLabel->set_alignment(0, 0.5);
	fpsLabel->set_use_underline(TRUE);
	Gtk::SpinButton *fpsSpinner = manage(new Gtk::SpinButton(adj_fps, 1, 1));
	fpsLabel->set_mnemonic_widget(*fpsSpinner);
	fpsSpinner->set_alignment(1);
	generalTable->attach(*fpsLabel, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	generalTable->attach(*fpsSpinner, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	check_use_cairo.set_alignment(0, 0.5);
	check_use_cairo.set_use_underline(TRUE);
	generalTable->attach(check_use_cairo, 0, 1, 2, 3, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Frame *timeFrame = manage(new Gtk::Frame(_("Time settings")));
	timeFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) timeFrame->get_label_widget())->set_markup(_("<b>Time settings</b>"));
	dialogBox->pack_start(*timeFrame, false, false, 0);

	Gtk::Alignment *timePadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	timePadding->set_padding(6, 0, 24, 0);
	timeFrame->add(*timePadding);

	Gtk::Table *timeTable = manage(new Gtk::Table(2, 2, false));
	timeTable->set_row_spacings(6);
	timeTable->set_col_spacings(12);
	timePadding->add(*timeTable);

	check_overbegin.set_alignment(0, 0.5);
	check_overbegin.set_use_underline(TRUE);
	check_overend.set_alignment(0, 0.5);
	check_overend.set_use_underline(TRUE);
	time_begin.set_alignment(1);
	time_end.set_alignment(1);
	timeTable->attach(check_overbegin, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	timeTable->attach(time_begin, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	timeTable->attach(check_overend, 0, 1, 1, 2, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	timeTable->attach(time_end, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	check_overbegin.signal_toggled().connect(sigc::mem_fun(*this,&Dialog_PreviewOptions::on_overbegin_toggle));
	check_overend.signal_toggled().connect(sigc::mem_fun(*this,&Dialog_PreviewOptions::on_overend_toggle));

	Gtk::Button *cancelButton = manage(new Gtk::Button(Gtk::StockID("gtk-cancel")));
	cancelButton->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PreviewOptions::on_cancel_pressed));
	add_action_widget(*cancelButton, 1);
	cancelButton->show();

	Gtk::Button *okbutton = manage(new Gtk::Button(Gtk::StockID("gtk-go-forward")));
	okbutton->set_label(_("Preview"));
	okbutton->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_PreviewOptions::on_ok_pressed));
	add_action_widget(*okbutton, 0);
	okbutton->show();

	time_begin.set_sensitive(false);
	time_end.set_sensitive(false);
	show_all();
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
	i.use_cairo = get_use_cairo();
	if(i.overbegin) i.begintime = (float)get_begintime();
	if(i.overend)	i.endtime = (float)get_endtime();

	hide();
	signal_finish_(i);
	signal_finish_.clear();
}

void
Dialog_PreviewOptions::on_cancel_pressed()
{
	hide();
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
