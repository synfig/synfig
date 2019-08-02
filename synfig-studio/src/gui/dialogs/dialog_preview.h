/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_preview.h
**	\brief Preview dialog Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_DIALOG_PREVIEW_H
#define __SYNFIG_GTKMM_DIALOG_PREVIEW_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

#include <gui/dialogsettings.h>

#include "preview.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
class Adjustment;
class CheckButton;
}

namespace studio {

class Widget_Time;

struct PreviewInfo
{
	float zoom,fps,begintime,endtime;
	bool overbegin,overend;
};

class Dialog_Preview : public Gtk::Window
{
	Widget_Preview 	preview;
	DialogSettings	settings;

	//etl::handle<synfig::Canvas> canvas;

public:
	Dialog_Preview();
	~Dialog_Preview();

    void set_preview(etl::handle<Preview> prev);

	Widget_Preview &get_widget() {return preview;}
	const Widget_Preview &get_widget() const {return preview;}

	virtual void on_show();
	virtual void on_hide();
	//other forwarding functions...

	//child widgets:

private:
	bool on_key_pressed(GdkEventKey*);
	void close_window_handler();

protected:
	Gtk::Table preview_table;

}; // END of Dialog_Preview

class Dialog_PreviewOptions : public Gtk::Dialog
{
	//all the info needed to construct a render description...
	Glib::RefPtr<Gtk::Adjustment> adj_zoom;	// factor at which to resize the window...
	Glib::RefPtr<Gtk::Adjustment> adj_fps;	// how often to take samples of the animation
	
	studio::Widget_Time * time_begin;
	studio::Widget_Time * time_end;

	Gtk::CheckButton * check_overbegin;
	Gtk::CheckButton * check_overend;

	DialogSettings	settings;

	float globalfps;

	// for finishing
	void on_ok_pressed();
	void on_cancel_pressed();

	// for ui stuff
	void on_overbegin_toggle();
	void on_overend_toggle();

	sigc::signal<void,const PreviewInfo &>	signal_finish_;

	// private to force the use of create()
	Dialog_PreviewOptions();
	const Glib::RefPtr<Gtk::Builder>& builder;
public:
	Dialog_PreviewOptions(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	static Dialog_PreviewOptions * create(/*Gtk::Window& parent*/);
	~Dialog_PreviewOptions();

	float get_zoom() const { return adj_zoom->get_value(); }
	void set_zoom(float z) { adj_zoom->set_value(z); }

	float get_fps() const { return adj_fps->get_value(); }
	void set_fps(float z) { adj_fps->set_value(z); }

	float get_global_fps() const { return globalfps; }
	void set_global_fps(float f);

	synfig::Time get_begintime() const { return time_begin->get_value(); }
	void set_begintime(const synfig::Time &t) { time_begin->set_value(t); }

	synfig::Time get_endtime() const { return time_end->get_value(); }
	void set_endtime(const synfig::Time &t) { time_end->set_value(t); }

	bool get_begin_override() const { return check_overbegin->get_active(); }
	void set_begin_override(bool o) { check_overbegin->set_active(o); }

	bool get_end_override() const { return check_overend->get_active(); }
	void set_end_override(bool o) { check_overend->set_active(o); }
	
	sigc::signal<void,const PreviewInfo &>	&signal_finish() {return signal_finish_;}
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
