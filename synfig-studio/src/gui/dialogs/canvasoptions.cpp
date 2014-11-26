/* === S Y N F I G ========================================================= */
/*!	\file canvasoptions.cpp
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

#include "canvasoptions.h"
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/alignment.h>
#include "canvasview.h"
#include "workarea.h"

#include "general.h"

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

CanvasOptions::CanvasOptions(Gtk::Window &window,etl::loose_handle<CanvasView> canvas_view):
	Gtk::Dialog(_("Canvas Options"),window),
	canvas_view_(canvas_view),
	toggle_grid_snap(_("_Snap to grid"), true),
	toggle_grid_show(_("S_how grid"), true),
	toggle_time_snap(_("Snap to _frame"), true)
{
	vector_grid_size.set_canvas(canvas_view->get_canvas());

	Gtk::Alignment *dialogPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	dialogPadding->set_padding(12, 12, 12, 12);

	Gtk::Notebook *notebook=manage(new class Gtk::Notebook());
	dialogPadding->add(*notebook);

	toggle_grid_snap.signal_toggled().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_grid_snap_toggle));
	toggle_grid_show.signal_toggled().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_grid_show_toggle));

	Gtk::Alignment *gridPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	gridPadding->set_padding(12, 12, 12, 12);
	notebook->append_page(*gridPadding, _("Grid"));

	Gtk::VBox *gridBox = manage(new Gtk::VBox(false, 12));
	gridPadding->add(*gridBox);

	Gtk::Table *gridTable = manage(new Gtk::Table(3, 2, false));
	gridTable->set_row_spacings(6);
	gridTable->set_col_spacings(12);
	gridBox->pack_start(*gridTable, false, false, 0);

	Gtk::Label *gridSizeLabel = manage(new Gtk::Label(_("_Grid size"), true));
	gridSizeLabel->set_alignment(0, 0.5);
	gridSizeLabel->set_mnemonic_widget(vector_grid_size);

	toggle_grid_show.set_alignment(0, 0.5);
	toggle_grid_snap.set_alignment(0, 0.5);

	gridTable->attach(*gridSizeLabel, 0, 1, 0, 1, Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	gridTable->attach(vector_grid_size, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	gridTable->attach(toggle_grid_show, 0, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);
	gridTable->attach(toggle_grid_snap, 0, 2, 2, 3, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 0, 0);

	Gtk::Alignment *timePadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	timePadding->set_padding(12, 12, 12, 12);
	notebook->append_page(*timePadding, _("Time"));

	Gtk::VBox *timeBox = manage(new Gtk::VBox(false, 12));
	timePadding->add(*timeBox);

	timeBox->pack_start(toggle_time_snap, false, false, 0);

	Gtk::Alignment *unitPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	unitPadding->set_padding(12, 12, 12, 12);
	notebook->append_page(*unitPadding, _("Units"));
	unitPadding->add(*manage(new Gtk::Label(_("Not yet implemented!"))));

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


	get_vbox()->pack_start(*dialogPadding);
	get_vbox()->show_all();

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

	toggle_time_snap.set_tooltip_text(_("Not yet implemented"));
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
	canvas_view_->set_grid_snap_toggle(toggle_grid_snap.get_active());
	if(toggle_grid_snap.get_active())
		canvas_view_->work_area->enable_grid_snap();
	else
		canvas_view_->work_area->disable_grid_snap();

	canvas_view_->set_grid_show_toggle(toggle_grid_show.get_active());
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
