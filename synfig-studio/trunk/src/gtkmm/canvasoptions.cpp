/* === S Y N F I G ========================================================= */
/*!	\file canvasoptions.cpp
**	\brief Template File
**
**	$Id: canvasoptions.cpp,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#include "canvasoptions.h"
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include "canvasview.h"
#include "workarea.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CanvasOptions::CanvasOptions(loose_handle<studio::CanvasView> canvas_view):
	Gtk::Dialog(_("Canvas Options"),*canvas_view,false,true),
	canvas_view_(canvas_view),
	toggle_grid_snap(_("Grid Snap")),
	toggle_grid_show(_("Grid Show")),
	toggle_time_snap(_("Snap-To-Frame"))
{
	vector_grid_size.set_canvas(canvas_view->get_canvas());
	
	Gtk::Notebook *notebook=manage(new class Gtk::Notebook());

	toggle_grid_snap.signal_toggled().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_grid_snap_toggle));
	toggle_grid_show.signal_toggled().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_grid_show_toggle));

	Gtk::Table *grid_page=manage(new class Gtk::Table(2,2,false));
	notebook->append_page(*grid_page,_("Grids"));
	grid_page->attach(vector_grid_size, 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	grid_page->attach(toggle_grid_snap, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	grid_page->attach(toggle_grid_show, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	
	Gtk::Table *time_page=manage(new class Gtk::Table(2,2,false));
	notebook->append_page(*time_page,_("Time"));
	time_page->attach(toggle_time_snap, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	
	Gtk::Table *unit_page=manage(new class Gtk::Table(2,2,false));
	notebook->append_page(*unit_page,_("Units"));
	unit_page->attach(*manage(new Gtk::Label(_("Not yet implemented"))), 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	
	
	Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_ok_pressed));

	Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
	apply_button->show();
	add_action_widget(*apply_button,1);
	apply_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_apply_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_cancel_pressed));

	//set_default_response(1);
	
	
	get_vbox()->pack_start(*notebook);
	notebook->show_all();
	
	signal_show().connect(sigc::mem_fun(*this, &studio::CanvasOptions::refresh));

	vector_grid_size.set_digits(5);
	
	update_title();
}

CanvasOptions::~CanvasOptions()
{
}

void
CanvasOptions::update_title()
{
	set_title(_("Options")+String(" - ")+canvas_view_->get_canvas()->get_name());
}

void
CanvasOptions::refresh()
{
	if(canvas_view_->work_area->grid_status())
		toggle_grid_show.set_active(true);
	else
		toggle_grid_show.set_active(false);
		
	if(canvas_view_->work_area->get_grid_snap())
		toggle_grid_snap.set_active(true);
	else
		toggle_grid_snap.set_active(false);
	
	vector_grid_size.set_value(canvas_view_->work_area->get_grid_size());
	
	tooltips.set_tip(toggle_time_snap,_("Not yet implemented"));
	toggle_time_snap.set_sensitive(false);

	update_title();
}

void
CanvasOptions::on_grid_snap_toggle()
{
}

void
CanvasOptions::on_grid_show_toggle()
{
}

void
CanvasOptions::on_apply_pressed()
{
	if(toggle_grid_snap.get_active())
		canvas_view_->work_area->enable_grid_snap();
	else
		canvas_view_->work_area->disable_grid_snap();
		
	if(toggle_grid_show.get_active())
		canvas_view_->work_area->enable_grid();
	else
		canvas_view_->work_area->disable_grid();

	canvas_view_->work_area->set_grid_size(vector_grid_size.get_value());
}

void
CanvasOptions::on_ok_pressed()
{
	on_apply_pressed();
	hide();
}

void
CanvasOptions::on_cancel_pressed()
{
	refresh();
	hide();
}
