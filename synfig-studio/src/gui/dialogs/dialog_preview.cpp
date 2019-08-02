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

#include <synfig/general.h>

#include "dialogs/dialog_preview.h"
#include <gtkmm/adjustment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>

#include <gui/widgets/widget_time.h>
#include <gui/localization.h>

#include "gui/resourcehelper.h"
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

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
	preview.on_dialog_show();
}

void Dialog_Preview::on_hide()
{
	Window::on_hide();
	preview.on_dialog_hide();
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

static Glib::RefPtr<Gtk::Builder> load_interface(const char *filename) {
	auto refBuilder = Gtk::Builder::create();
	try
	{
		refBuilder->add_from_file(ResourceHelper::get_ui_path(filename));
	}
	catch(const Glib::FileError& ex)
	{
		synfig::error("FileError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Glib::MarkupError& ex)
	{
		synfig::error("MarkupError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Gtk::BuilderError& ex)
	{
		synfig::error("BuilderError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	return refBuilder;
}

Dialog_PreviewOptions::Dialog_PreviewOptions(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
	Gtk::Dialog(cobject),
	settings(this, "prevoptions"),
	builder(refGlade)
{
	refGlade->get_widget("check_overbegin", check_overbegin);
	refGlade->get_widget("check_overend", check_overend);
	refGlade->get_widget("time_begin", time_begin);
	refGlade->get_widget("time_end", time_end);

	if (check_overbegin)
		check_overbegin->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_PreviewOptions::on_overbegin_toggle));
	if (check_overend)
		check_overend->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_PreviewOptions::on_overend_toggle));

	Gtk::SpinButton * zoomSpinner;
	refGlade->get_widget("zoom_spinner", zoomSpinner);
	if (zoomSpinner)
		adj_zoom = zoomSpinner->get_adjustment();

	Gtk::SpinButton * fpsSpinner;
	refGlade->get_widget("fps_spinner", fpsSpinner);
	if (fpsSpinner)
		adj_fps = fpsSpinner->get_adjustment();

	Gtk::Button *button = nullptr;

	refGlade->get_widget("cancel_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PreviewOptions::on_cancel_pressed));

	refGlade->get_widget("ok_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PreviewOptions::on_ok_pressed));

	show_all();
}

Dialog_PreviewOptions* Dialog_PreviewOptions::create(/*Gtk::Window& parent*/)
{
	auto refBuilder = load_interface("preview_options.ui");
	if (!refBuilder)
		return nullptr;
	Dialog_PreviewOptions * dialog = nullptr;
	refBuilder->get_widget_derived("preview_options", dialog);
//	if (dialog) {
//		dialog->set_transient_for(parent);
//	}
	return dialog;
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

void
Dialog_PreviewOptions::on_cancel_pressed()
{
	hide();
}

void Dialog_PreviewOptions::on_overbegin_toggle()
{
	time_begin->set_sensitive(get_begin_override());
}

void Dialog_PreviewOptions::on_overend_toggle()
{
	time_end->set_sensitive(get_end_override());
}

void studio::Dialog_PreviewOptions::set_global_fps(float f)
{
	globalfps = f;
	time_begin->set_fps(f);
	time_end->set_fps(f);
}
