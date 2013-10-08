/* === S Y N F I G ========================================================= */
/*!	\file mainwindow.cpp
**	\brief MainWindow
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "mainwindow.h"
#include "canvasview.h"

#include <synfigapp/main.h>

#include <gtkmm/menubar.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y)	{ \
		String x; \
		if(synfigapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str()));	\
		} \
	}

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

MainWindow::MainWindow():
	menu_added_(false)
{
	set_default_size(600, 400);

	notebook_.show();
	vbox_.pack_end(notebook_, true, true, 0);
	vbox_.show();
	add(vbox_);

	add_accel_group(App::ui_manager()->get_accel_group());

	notebook_.signal_switch_page().connect(
		sigc::mem_fun(*this, &studio::MainWindow::on_switch_page) );
	App::ui_manager()->signal_add_widget().connect(
		sigc::mem_fun(*this, &studio::MainWindow::on_ui_manager_add_widget) );

	GRAB_HINT_DATA("canvas_view");
}

MainWindow::~MainWindow() { }

void
MainWindow::on_ui_manager_add_widget(Gtk::Widget*)
{
	if (menu_added_) return;
	Gtk::Widget* menubar = App::ui_manager()->get_widget("/menubar-main");
	if (menubar != NULL)
	{
		menubar->show();
		vbox_.pack_start(*menubar, false, false, 0);
		menu_added_ = true;
	}
}


void
MainWindow::on_switch_page(GtkNotebookPage* /* page */, guint page_num)
{
	Gtk::Notebook::PageList::iterator i = App::main_window->notebook().pages().find(page_num);
	if (i == App::main_window->notebook().pages().end())
		App::set_selected_canvas_view(NULL);
	else
		App::set_selected_canvas_view(dynamic_cast<CanvasView*>(i->get_child()));
}


/* === E N T R Y P O I N T ================================================= */


