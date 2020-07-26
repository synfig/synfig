/* === S Y N F I G ========================================================= */
/*!	\file dock_info.cpp
**	\brief Dock Info File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "app.h"
#include <synfig/general.h>

#include "docks/dock_info.h"
#include "canvasview.h"
#include "workarea.h"

#include <synfig/canvas.h>
#include <synfig/color.h>		// for gamma_in()
#include <synfig/context.h>

#include <gtkmm/separator.h>
#include <gtkmm/invisible.h>
#include <gtkmm/progressbar.h>

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

void studio::Dock_Info::on_mouse_move()
{
	etl::loose_handle<CanvasView> canvas_view(get_canvas_view());
	if(!canvas_view) return;
	Point pos = canvas_view->get_work_area()->get_cursor_pos();

	Distance xv(pos[0],Distance::SYSTEM_UNITS);
	xv.convert(App::distance_system, canvas_view->get_canvas()->rend_desc());

	Distance yv(pos[1],Distance::SYSTEM_UNITS);
	yv.convert(App::distance_system, canvas_view->get_canvas()->rend_desc());

	//get the color and set the labels

	x.set_text(xv.get_string(3));
	y.set_text(yv.get_string(3));

	Color c = canvas_view->get_canvas()->get_context( canvas_view->get_context_params() ).get_color(pos);
	Gamma gamma = canvas_view->get_canvas()->rend_desc().get_gamma();

	if ( approximate_equal_lp(gamma.get_r(), ColorReal(1))
	  && approximate_equal_lp(gamma.get_g(), ColorReal(1))
	  && approximate_equal_lp(gamma.get_b(), ColorReal(1)) )
	{
		r.set_text(strprintf("%.1f%%", c.get_r()*100));
		g.set_text(strprintf("%.1f%%", c.get_g()*100));
		b.set_text(strprintf("%.1f%%", c.get_b()*100));
	} else {
		Color cg = gamma.apply(c);
		r.set_text(strprintf("%.1f%% (%.1f%%)", c.get_r()*100, cg.get_r()*100));
		g.set_text(strprintf("%.1f%% (%.1f%%)", c.get_g()*100, cg.get_g()*100));
		b.set_text(strprintf("%.1f%% (%.1f%%)", c.get_b()*100, cg.get_b()*100));
	}
	a.set_text(strprintf("%.1f%%", c.get_a()*100));
}

studio::Dock_Info::Dock_Info()
:Dock_CanvasSpecific("info",_("Info"),Gtk::StockID("synfig-info"))
{
	set_use_scrolled(false);

	Gtk::Grid *table = manage(new Gtk::Grid);
	table->set_column_spacing(3);

	//pos labels
	Gtk::Label *x_label = manage(new Gtk::Label());
	x_label->set_markup(etl::strprintf("<b>%s</b>", _("X: ")));
	x_label->set_hexpand(false);
	Gtk::Label *y_label = manage(new Gtk::Label());
	y_label->set_markup(etl::strprintf("<b>%s</b>", _("Y: ")));
	y_label->set_hexpand(false);
	table->attach(*x_label, 0,0,1,1);
	table->attach(*y_label, 0,1,1,1);

	//pos
	x.property_xalign() = 1.0;
	y.property_xalign() = 1.0;
	table->attach_next_to(x, *x_label, Gtk::POS_RIGHT, 1,1);
	table->attach_next_to(y, *y_label, Gtk::POS_RIGHT, 1,1);

	//separator
	Gtk::Label *separator1 = manage(new Gtk::Label("  "));
	separator1->set_hexpand(true);
	table->attach_next_to(*separator1, x, Gtk::POS_RIGHT, 1,4);

	//color label
	Gtk::Label *r_label = manage(new Gtk::Label());
	r_label->set_markup(etl::strprintf("<b>%s</b>", _("R: ")));
	r_label->set_hexpand(false);
	Gtk::Label *g_label = manage(new Gtk::Label());
	g_label->set_markup(etl::strprintf("<b>%s</b>", _("G: ")));
	g_label->set_hexpand(false);
	Gtk::Label *b_label = manage(new Gtk::Label());
	b_label->set_markup(etl::strprintf("<b>%s</b>", _("B: ")));
	b_label->set_hexpand(false);
	Gtk::Label *a_label = manage(new Gtk::Label());
	a_label->set_markup(etl::strprintf("<b>%s</b>", _("A: ")));
	a_label->set_hexpand(false);
	table->attach(*r_label, 3,0,1,1);
	table->attach(*g_label, 3,1,1,1);
	table->attach(*b_label, 3,2,1,1);
	table->attach(*a_label, 3,3,1,1);

	//color
	r.property_xalign() = 1.0f;
	g.property_xalign() = 1.0f;
	b.property_xalign() = 1.0f;
	a.property_xalign() = 0.0f;
	r.set_tooltip_text(_("Red component value of color\nThe value after gamma correction, if different, is given in brackets"));
	g.set_tooltip_text(_("Green component value of color\nThe value after gamma correction, if different, is given in brackets"));
	b.set_tooltip_text(_("Blue component value of color\nThe value after gamma correction, if different, is given in brackets"));
	a.set_tooltip_text(_("Alpha component value of color, ie. opacity"));
	table->attach_next_to(r, *r_label, Gtk::POS_RIGHT, 1,1);
	table->attach_next_to(g, *g_label, Gtk::POS_RIGHT, 1,1);
	table->attach_next_to(b, *b_label, Gtk::POS_RIGHT, 1,1);
	table->attach_next_to(a, *a_label, Gtk::POS_RIGHT, 1,1);

	Gtk::Label *separator2 = manage(new Gtk::Label(" "));
	table->attach(*separator2, 0,4,8,1);

	//Render Progress Bar
	Gtk::Label *render_progress_label = manage(new Gtk::Label());
	render_progress_label->set_markup(etl::strprintf("<b>%s</b>", _("Render Progress: ")));
	table->attach_next_to(*render_progress_label, *separator2, Gtk::POS_BOTTOM, 8,1);

	table->attach_next_to(render_progress, *render_progress_label, Gtk::POS_BOTTOM, 7,1);
	render_progress.set_hexpand(true);
	render_progress.set_vexpand(false);
	render_progress.set_show_text(true);
	render_progress.set_text(strprintf("%.1f%%", 0.0));
	render_progress.set_fraction(0.0);

	stop_button.set_label("Stop rendering");
	stop_button.signal_clicked().connect(sigc::mem_fun(*this, &studio::Dock_Info::on_stop_button_clicked));

	table->attach_next_to(stop_button, render_progress, Gtk::POS_BOTTOM, 7, 1);

	table->set_margin_start(5);
	table->set_margin_end(5);
	table->set_margin_top(5);
	table->set_margin_bottom(5);

	
	table->show_all();

	add(*table);
	
	//Render progress
	set_n_passes_requested(1); //Default
	set_n_passes_pending  (0); //Default
	set_render_progress (0.0); //Default, 0.0%
}

studio::Dock_Info::~Dock_Info()
{
}

void studio::Dock_Info::on_stop_button_clicked()
{
	if(!async_renderer)
            return;

	if(App::dialog_message_2b(
		_("Stop rendering"),
		_("The rendering process will be stopped. Are you sure?"),
		Gtk::MESSAGE_QUESTION,
		_("Cancel"),
		_("Stop")))
		async_renderer->stop();
}

void studio::Dock_Info::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	mousecon.disconnect();

	if(canvas_view && canvas_view->get_work_area())
	{
		mousecon = get_canvas_view()->get_work_area()->signal_cursor_moved().connect(sigc::mem_fun(*this,&Dock_Info::on_mouse_move));
	}
}

void studio::Dock_Info::set_async_render(etl::handle<studio::AsyncRenderer> ar)
{
	async_renderer = ar;
}

void studio::Dock_Info::set_n_passes_requested(int value)
{
	n_passes_requested = value;
}

void studio::Dock_Info::set_n_passes_pending(int value)
{
	n_passes_pending = value;
}

void studio::Dock_Info::set_render_progress(float value)
{
	float coeff        = (1.000 / (float)n_passes_requested);  //% of fraction for 1 pass if more than 1 pass
	float already_done = coeff * (float)(n_passes_requested - n_passes_pending -1); 
	float r            = ( coeff * value ) + already_done;

	render_progress.set_text( strprintf( "%.1f%%", r*100 ));
	render_progress.set_fraction(r);

	if(n_passes_pending > 0)
		stop_button.set_sensitive(true);
	else
		stop_button.set_sensitive(false);
}
