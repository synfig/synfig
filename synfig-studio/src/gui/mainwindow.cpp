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
#include "docks/dockable.h"
#include "docks/dockbook.h"
#include "docks/dockmanager.h"
#include "docks/dockdroparea.h"

#include <synfigapp/main.h>

#include <gtkmm/menubar.h>
#include <gtkmm/box.h>

#include <gtkmm/inputdialog.h>

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

MainWindow::MainWindow()
{
	set_default_size(600, 400);

	main_dock_book_ = manage(new DockBook());
	main_dock_book_->allow_empty = true;
	main_dock_book_->show();

	class Bin : public Gtk::Bin {
	public:
		Bin() { }
	protected:
		void on_size_allocate(Gtk::Allocation &allocation) {
			Gtk::Bin::on_size_allocate(allocation);
			if (get_child() != NULL)
				get_child()->size_allocate(allocation);
		}
		void on_size_request(Gtk::Requisition *requisition) {
			Gtk::Bin::on_size_request(requisition);
			if (get_child() != NULL && requisition != NULL)
				*requisition = get_child()->size_request();
		}
	};

	bin_ = manage((Gtk::Bin*)new Bin());
	bin_->add(*main_dock_book_);
	bin_->show();

	Gtk::VBox *vbox = manage(new Gtk::VBox());

	Gtk::Widget* menubar = App::ui_manager()->get_widget("/menubar-main");
	if (menubar != NULL)
	{
		menubar->show();
		vbox->pack_start(*menubar, false, false, 0);
	}

	vbox->pack_end(*bin_, true, true, 0);
	vbox->show();
	add(*vbox);

	add_accel_group(App::ui_manager()->get_accel_group());

	init_menus();
	panels_action_group = Gtk::ActionGroup::create("mainwindow-recentfiles");
	App::ui_manager()->insert_action_group(panels_action_group);

	App::signal_recent_files_changed().connect(
		sigc::mem_fun(*this, &MainWindow::on_recent_files_changed) );

	signal_delete_event().connect(
		sigc::ptr_fun(App::shutdown_request) );

	App::dock_manager->signal_dockable_registered().connect(
		sigc::mem_fun(*this,&MainWindow::on_dockable_registered) );

	GRAB_HINT_DATA("mainwindow");
}

MainWindow::~MainWindow() { }

void MainWindow::create_stock_dialog1()
{
	// Canvases, History

	std::string layout =
		"[vert|200"
			"|[book|canvases]"
			"|[vert|200|[book|history]|[book|layers]]"
		"]";
	Gtk::Widget *widget = App::dock_manager->load_widget_from_string(layout);
	if (widget != NULL)
		DockManager::add_widget(App::main_window->main_dock_book(), *widget, false, false);
}

void MainWindow::create_stock_dialog2()
{
	// Layers, Library, Parameters

	std::string layout =
		"[hor|200"
			"|[book|params]"
			"|[book|keyframes]"
		"]";
	Gtk::Widget *widget = App::dock_manager->load_widget_from_string(layout);
	if (widget != NULL)
		DockManager::add_widget(App::main_window->main_dock_book(), *widget, true, false);
}

void
MainWindow::save_all()
{
	std::list<etl::handle<Instance> >::iterator iter;
	for(iter=App::instance_list.begin();iter!=App::instance_list.end();iter++)
		(*iter)->save();
}

void
MainWindow::show_dialog_input()
{
	App::dialog_input->present();
}

void
MainWindow::init_menus()
{
	Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create("mainwindow");

	// file
	action_group->add( Gtk::Action::create("new", Gtk::Stock::NEW),
		sigc::hide_return(sigc::ptr_fun(&studio::App::new_instance))
	);
	action_group->add( Gtk::Action::create("open", Gtk::Stock::OPEN),
		sigc::hide_return(sigc::bind(sigc::ptr_fun(&studio::App::dialog_open), ""))
	);
	action_group->add( Gtk::Action::create("save-all", Gtk::StockID("synfig-saveall")),
		sigc::ptr_fun(save_all)
	);
	action_group->add( Gtk::Action::create("input-devices", _("Input Devices...")),
		sigc::ptr_fun(&MainWindow::show_dialog_input)
	);
	action_group->add( Gtk::Action::create("setup", _("Setup...")),
		sigc::ptr_fun(&studio::App::show_setup)
	);
	action_group->add( Gtk::Action::create("reset-initial-preferences", _("Reset to default Setup values")),
		sigc::ptr_fun(&studio::App::reset_initial_preferences)
	);
	action_group->add( Gtk::Action::create("quit", Gtk::StockID("gtk-quit"), _("Quit")),
		sigc::hide_return(sigc::ptr_fun(&studio::App::quit))
	);

	// file -> panels
	action_group->add( Gtk::Action::create("panels-vertical", _("Vertical Docks: Canvases, History, Layers")),
		sigc::ptr_fun(&MainWindow::create_stock_dialog1)
	);
	action_group->add( Gtk::Action::create("panels-horizontal", _("Horizontal Docks: Parameters, Keyframes")),
		sigc::ptr_fun(&MainWindow::create_stock_dialog2)
	);
	action_group->add( Gtk::Action::create("panels-reset", _("Reset Panels to Original Layout")),
		sigc::ptr_fun(App::reset_initial_window_configuration)
	);

	// help
	#define URL(action_name,title,url) \
		action_group->add( Gtk::Action::create(action_name, title), \
			sigc::bind(sigc::ptr_fun(&studio::App::open_url),url))
	#define WIKI(action_name,title,page) \
		URL(action_name,title, "http://synfig.org/wiki" + String(page))
	#define SITE(action_name,title,page) \
		URL(action_name,title, "http://synfig.org/cms" + String(page))

	action_group->add( Gtk::Action::create("help", Gtk::Stock::HELP),
		sigc::ptr_fun(studio::App::dialog_help)
	);

	// TRANSLATORS:         | Help menu entry:              | A wiki page:          |
	WIKI("help-tutorials",	_("Tutorials"),					_("/Category:Tutorials"));
	WIKI("help-reference",	_("Reference"),					_("/Category:Reference"));
	WIKI("help-faq",		_("Frequently Asked Questions"),_("/FAQ")				);
	SITE("help-support",	_("Get Support"),				_("/en/support")		);

	action_group->add( Gtk::Action::create("help-about", Gtk::StockID("synfig-about")),
		sigc::ptr_fun(studio::App::dialog_about)
	);

	// TODO: open recent
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));

	App::ui_manager()->insert_action_group(action_group);
}


void
MainWindow::on_recent_files_changed()
{
	Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create("mainwindow-recentfiles");

	int index = 0;
	std::string menu_items;
	for(list<string>::const_iterator i=App::get_recent_files().begin();i!=App::get_recent_files().end();i++)
	{
		std::string raw = basename(*i);
		std::string quoted;
		size_t pos = 0, last_pos = 0;

		// replace _ in filenames by __ or it won't show up in the menu
		for (pos = last_pos = 0; (pos = raw.find('_', pos)) != string::npos; last_pos = pos)
			quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
		quoted += raw.substr(last_pos);

		std::string action_name = strprintf("file-recent-%d", index++);
		menu_items += "<menuitem action='" + action_name +"' />";

		action_group->add( Gtk::Action::create(action_name, quoted),
			sigc::hide_return(sigc::bind(sigc::ptr_fun(&App::open),*i))
		);
	}

	std::string ui_info =
		"<menu action='menu-file'><menu action='menu-open-recent'>"
	  + menu_items
	  + "</menu></menu>";
	std::string ui_info_popup =
		"<ui><popup action='menu-main'>" + ui_info + "</popup></ui>";
	std::string ui_info_menubar =
		"<ui><menubar action='menubar-main'>" + ui_info + "</menubar></ui>";

	App::ui_manager()->insert_action_group(action_group);
	App::ui_manager()->add_ui_from_string(ui_info_popup);
	App::ui_manager()->add_ui_from_string(ui_info_menubar);
}

void
MainWindow::on_dockable_registered(Dockable* dockable)
{
	panels_action_group->add( Gtk::Action::create("panel-" + dockable->get_name(), dockable->get_local_name()),
		sigc::mem_fun(*dockable, &Dockable::present)
	);

	std::string ui_info =
		"<menu action='menu-file'><menu action='menu-panels'>"
	    "<menuitem action='panel-" + dockable->get_name() + "' />"
	    "</menu></menu>";
	std::string ui_info_popup =
		"<ui><popup action='menu-main'>" + ui_info + "</popup></ui>";
	std::string ui_info_menubar =
		"<ui><menubar action='menubar-main'>" + ui_info + "</menubar></ui>";

	App::ui_manager()->add_ui_from_string(ui_info_popup);
	App::ui_manager()->add_ui_from_string(ui_info_menubar);
}

/* === E N T R Y P O I N T ================================================= */
