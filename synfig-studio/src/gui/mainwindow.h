/* === S Y N F I G ========================================================= */
/*!	\file mainwindow.h
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_MAINWINDOW_H
#define __SYNFIG_STUDIO_MAINWINDOW_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/actiongroup.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	class Dockable;

	class MainWindow: public Gtk::Window
	{
	private:
		Gtk::Notebook *notebook_;
		Glib::RefPtr<Gtk::ActionGroup> panels_action_group;

		//! Constructor Helper - Initializes all of the menus
		void init_menus();

		void on_switch_page(GtkNotebookPage* page, guint page_num);

		static void create_stock_dialog1();
		static void create_stock_dialog2();
		static void save_all();
		static void show_dialog_input();
		void on_recent_files_changed();
		void on_dockable_registered(Dockable* dockable);

	public:
		MainWindow();
		virtual ~MainWindow();

		Gtk::Notebook& notebook() { return *notebook_; }
		const Gtk::Notebook& notebook() const { return *notebook_; }
	};
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
