/* === S Y N F I G ========================================================= */
/*!	\file mainwindow.h
**	\brief MainWindow
**
**	\legal
**	......... ... 2013 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_MAINWINDOW_H
#define __SYNFIG_STUDIO_MAINWINDOW_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/applicationwindow.h>

#include <synfig/filesystem_path.h>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	class Dockable;
	class DockBook;
	class WorkspaceHandler;

	class MainWindow: public Gtk::ApplicationWindow
	{
	private:
		Gtk::Bin *bin_;
		DockBook *main_dock_book_;

		//! Constructor Helper - Initializes all of the menus
		void init_menus();

		//! Mandatory for constructing custom widgets from GTKBuilder files
		void register_custom_widget_types();

		static void show_dialog_input();
		void on_recent_files_changed();
		void on_custom_workspaces_changed();
		void on_dockable_registered(Dockable* dockable);
		void on_dockable_unregistered(Dockable* dockable);
		void toggle_show_menubar();
		void toggle_show_toolbar();

		static void save_all();

		static std::unique_ptr<studio::WorkspaceHandler> workspaces;
		static const std::vector<std::string> get_workspaces();

		void save_custom_workspace();
		static void edit_custom_workspace_list();

		void refresh_menu_icon_offset();

	protected:
		virtual bool on_key_press_event(GdkEventKey *key_event);

	public:
		MainWindow(const Glib::RefPtr<Gtk::Application>& application);
		virtual ~MainWindow();

		Gtk::Bin& root() { return *bin_; }
		const Gtk::Bin& root() const { return *bin_; }

		DockBook& main_dock_book() { return *main_dock_book_; }
		const DockBook& main_dock_book() const { return *main_dock_book_; }

		static void set_workspace_default();
		static void set_workspace_compositing();
		static void set_workspace_animating();
		static void set_workspace_from_template(const std::string &tpl);
		static void set_workspace_from_name(const std::string &name);
		static void load_custom_workspaces();
		static void save_custom_workspaces();
		static WorkspaceHandler* get_workspace_handler() { return workspaces.get(); }

		static sigc::signal<void>& signal_custom_workspaces_changed();

		static void make_short_filenames(
			const std::vector<synfig::filesystem::Path>& fullnames,
			std::vector<synfig::String>& shortnames );
	};
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
