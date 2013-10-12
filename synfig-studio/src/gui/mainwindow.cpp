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

	notebook_ = manage(new Gtk::Notebook());

	DockDropArea *dock_area = manage(new DockDropArea(notebook_));
	dock_area->show();

	notebook_->set_action_widget(dock_area, Gtk::PACK_END);
	notebook_->set_scrollable(true);
	notebook_->show();

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

	Gtk::Bin *bin = manage((Gtk::Bin*)new Bin());
	bin->add(*notebook_);
	bin->show();

	Gtk::VBox *vbox = manage(new Gtk::VBox());

	Gtk::Widget* menubar = App::ui_manager()->get_widget("/menubar-main");
	if (menubar != NULL)
	{
		menubar->show();
		vbox->pack_start(*menubar, false, false, 0);
	}

	vbox->pack_end(*bin, true, true, 0);
	vbox->show();
	add(*vbox);

	add_accel_group(App::ui_manager()->get_accel_group());

	init_menus();

	notebook_->signal_switch_page().connect(
		sigc::mem_fun(*this, &studio::MainWindow::on_switch_page) );

	GRAB_HINT_DATA("canvas_view");
}

MainWindow::~MainWindow() { }

void MainWindow::create_stock_dialog1()
{
	// TODO:
	//DockDialog* dock_dialog(new DockDialog);
	//dock_dialog->set_contents("canvases history");
	//dock_dialog->set_composition_selector(true);
	//dock_dialog->present();
}
void MainWindow::create_stock_dialog2()
{
	// TODO:
	//DockDialog* dock_dialog(new DockDialog);
	//dock_dialog->set_contents("layers children keyframes | params");
	//dock_dialog->present();
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
	action_group->add( Gtk::Action::create("panels-vertical", _("Vertical Dock: Canvases, History")),
		sigc::ptr_fun(&MainWindow::create_stock_dialog1)
	);
	action_group->add( Gtk::Action::create("panels-horizontal", _("Horizontal Dock: Layers, Library, Parameters")),
		sigc::ptr_fun(&MainWindow::create_stock_dialog2)
	);
	action_group->add( Gtk::Action::create("panels-reset", _("Reset Windows to Original Layout")),
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

	action_group->add( Gtk::Action::create("help", Gtk::Stock::HELP),
		sigc::ptr_fun(studio::App::dialog_help)
	);
	action_group->add( Gtk::Action::create("help-about", Gtk::StockID("synfig-about")),
		sigc::ptr_fun(studio::App::dialog_about)
	);

	// TODO: open recent
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));

	App::ui_manager()->insert_action_group(action_group);
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
