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

#include <synfig/general.h>

#include <gui/localization.h>

#include "mainwindow.h"
#include "canvasview.h"
#include "docks/dockable.h"
#include "docks/dockbook.h"
#include "docks/dockmanager.h"
#include "docks/dockdroparea.h"
#include "dialogs/dialog_input.h"

#include <synfigapp/main.h>

#include <gtkmm/menubar.h>
#include <gtkmm/box.h>

#include <gtkmm/textview.h>

#include "gui/widgets/widget_time.h"
#include "gui/widgets/widget_vector.h"

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
	register_custom_widget_types();

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
	};

	bin_ = manage((Gtk::Bin*)new Bin());
	bin_->add(*main_dock_book_);
	bin_->show();

	Gtk::VBox *vbox = manage(new Gtk::VBox());

	Gtk::Widget* menubar = App::ui_manager()->get_widget("/menubar-main");
	if (menubar != NULL)
	{
		vbox->pack_start(*menubar, false, false, 0);
	}

	vbox->pack_end(*bin_, true, true, 0);
	vbox->show();
	if(!App::enable_mainwin_menubar && menubar) menubar->hide();

	add(*vbox);

	init_menus();
	window_action_group = Gtk::ActionGroup::create("mainwindow-window");
	App::ui_manager()->insert_action_group(window_action_group);

	App::signal_recent_files_changed().connect(
		sigc::mem_fun(*this, &MainWindow::on_recent_files_changed) );

	signal_delete_event().connect(
		sigc::ptr_fun(App::shutdown_request) );

	App::dock_manager->signal_dockable_registered().connect(
		sigc::mem_fun(*this,&MainWindow::on_dockable_registered) );

	App::dock_manager->signal_dockable_unregistered().connect(
		sigc::mem_fun(*this,&MainWindow::on_dockable_unregistered) );

	GRAB_HINT_DATA("mainwindow");
}

MainWindow::~MainWindow(){ }

void
MainWindow::show_dialog_input()
{
	App::dialog_input->reset();
	App::dialog_input->present();
}

void
MainWindow::init_menus()
{
	Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create("mainwindow");

	// file
	action_group->add( Gtk::Action::create("new", Gtk::StockID("synfig-new_doc"), _("New"), _("Create a new document")),
		sigc::hide_return(sigc::ptr_fun(&studio::App::new_instance))
	);
	action_group->add( Gtk::Action::create("open", Gtk::StockID("synfig-open"), _("Open"), _("Open an existing document")),
		sigc::hide_return(sigc::bind(sigc::ptr_fun(&studio::App::dialog_open), ""))
	);
	action_group->add( Gtk::Action::create("quit", Gtk::StockID("gtk-quit"), _("Quit")),
		sigc::hide_return(sigc::ptr_fun(&studio::App::quit))
	);

	// Edit menu
	action_group->add( Gtk::Action::create("input-devices", _("Input Devices...")),
		sigc::ptr_fun(&MainWindow::show_dialog_input)
	);
	action_group->add( Gtk::Action::create("setup", _("Preferences...")),
		sigc::ptr_fun(&studio::App::show_setup)
	);

	// View menu
	Glib::RefPtr<Gtk::ToggleAction> toggle_menubar = Gtk::ToggleAction::create("toggle-mainwin-menubar", _("Show Menubar"));
	toggle_menubar->set_active(App::enable_mainwin_menubar);
	action_group->add(toggle_menubar, sigc::mem_fun(*this, &studio::MainWindow::toggle_show_menubar));

	// pre defined workspace (window ui layout)
	action_group->add( Gtk::Action::create("workspace-compositing", _("Compositing")),
		sigc::ptr_fun(App::set_workspace_compositing)
	);
	action_group->add( Gtk::Action::create("workspace-animating", _("Animating")),
		sigc::ptr_fun(App::set_workspace_animating)
	);
	action_group->add( Gtk::Action::create("workspace-default", _("Default")),
		sigc::ptr_fun(App::set_workspace_default)
	);

	// help
	#define URL(action_name,title,url) \
		action_group->add( Gtk::Action::create(action_name, title), \
			sigc::bind(sigc::ptr_fun(&studio::App::open_uri),url))
	#define WIKI(action_name,title,page) \
		URL(action_name,title, "http://synfig.org/wiki" + String(page))

	action_group->add( Gtk::Action::create("help", Gtk::Stock::HELP),
		sigc::ptr_fun(studio::App::dialog_help)
	);

	// TRANSLATORS:         | Help menu entry:              | A wiki page:          |
	WIKI("help-tutorials",	_("Tutorials"),					_("/Category:Tutorials"));
	WIKI("help-reference",	_("Reference"),					_("/Category:Reference"));
	WIKI("help-faq",		_("Frequently Asked Questions"),_("/FAQ")				);
	URL("help-support",		_("Get Support"),				_("https://forums.synfig.org/")	);

	action_group->add( Gtk::Action::create(
			"help-about", Gtk::StockID("synfig-about"), _("About Synfig Studio")),
		sigc::ptr_fun(studio::App::dialog_about)
	);

	// TODO: open recent
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));

	App::ui_manager()->insert_action_group(action_group);
}

void MainWindow::register_custom_widget_types()
{
	Widget_Vector::register_type();
	Widget_Time::register_type();
}

void
MainWindow::toggle_show_menubar()
{
	Gtk::Widget* menubar = App::ui_manager()->get_widget("/menubar-main");

	App::enable_mainwin_menubar = !App::enable_mainwin_menubar;

	if(App::enable_mainwin_menubar)
		menubar->show();
	else
		menubar->hide();
}

bool
MainWindow::on_key_press_event(GdkEventKey* key_event)
{
	Gtk::Widget * widget = get_focus();
	if (widget && (dynamic_cast<Gtk::Editable*>(widget) || dynamic_cast<Gtk::TextView*>(widget))) {
		bool handled = gtk_window_propagate_key_event(this->gobj(), key_event);
		if (handled)
			return true;
	}
	return Gtk::Window::on_key_press_event(key_event);
}

void
MainWindow::make_short_filenames(
	const std::vector<synfig::String> &fullnames,
	std::vector<synfig::String> &shortnames )
{
	if (fullnames.size() == 1)
	{
		shortnames.resize(1);
		shortnames[0] = etl::basename(fullnames[0]);
		return;
	}

	const int count = (int)fullnames.size();
	vector< vector<String> > dirs(count);
	vector< vector<bool> > dirflags(count);
	shortnames.clear();
	shortnames.resize(count);

	// build dir lists
	for(int i = 0; i < count; ++i) {
		int j = 0;
		String fullname = fullnames[i];
		if (fullname.substr(0, 7) == "file://")
			fullname = fullname.substr(7);
		while(j < (int)fullname.size())
		{
			size_t k = fullname.find_first_of(ETL_DIRECTORY_SEPARATORS, j);
			if (k == string::npos) k = fullname.size();
			string sub = fullname.substr(j, k - j);
			if (!sub.empty() && sub != "...")
				dirs[i].insert(dirs[i].begin(), sub);
			j = (int)k + 1;
		}

		dirflags[i].resize(dirs.size(), false);
	}

	// find shotest paths which shows that files are different
	for(int i = 0; i < count; ++i) {
		for(int j = 0; j < count; ++j) {
			if (i == j) continue;
			for(int k = 0; k < (int)dirs[i].size(); ++k) {
				dirflags[i][k] = true;
				if (k >= (int)dirs[j].size() || dirs[i][k] != dirs[j][k])
					break;
			}
		}
	}

	// remove non-informative dirs from middle of shortest paths
	// holes will shown as "/.../" at final stage
	for(int i = 0; i < count; ++i) {
		for(int k = 1; k < (int)dirs[i].size() && dirflags[i][k]; ++k) {
			dirflags[i][k] = false;
			for(int j = 0; j < count; ++j) {
				if (i == j) continue;
				int index = -1;
				for(int l = 0; l < (int)dirs[i].size(); ++l) {
					if (dirflags[i][l]) {
						++index;
						while(index < (int)dirs[j].size() && dirs[i][l] != dirs[j][index])
							++index;
					}
				}
				if (index < (int)dirs[j].size()) {
					dirflags[i][k] = true;
					break;
				}
			}
		}
	}

	// concatenate dir-lists to short names
	for(int i = 0; i < count; ++i) {
		int prevk = 0;
		for(int k = 0; k < (int)dirs[i].size(); ++k) {
			if (dirflags[i][k]) {
				if (prevk < k) shortnames[i] = "/"+shortnames[i];
				if (prevk < k-1) shortnames[i] = "/..."+shortnames[i];
				shortnames[i] = dirs[i][k] + shortnames[i];
				prevk = k;
			}
		}
	}
}

void
MainWindow::on_recent_files_changed()
{
	Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create("mainwindow-recentfiles");

	vector<String> fullnames(App::get_recent_files().begin(), App::get_recent_files().end());
	vector<String> shortnames;
	make_short_filenames(fullnames, shortnames);

	std::string menu_items;
	for(int i = 0; i < (int)fullnames.size(); ++i)
	{
		std::string raw = shortnames[i];
		std::string quoted;
		size_t pos = 0, last_pos = 0;

		// replace _ in filenames by __ or it won't show up in the menu
		for (pos = last_pos = 0; (pos = raw.find('_', pos)) != string::npos; last_pos = pos)
			quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
		quoted += raw.substr(last_pos);

		std::string action_name = strprintf("file-recent-%d", i);
		menu_items += "<menuitem action='" + action_name +"' />";

		action_group->add( Gtk::Action::create(action_name, quoted, fullnames[i]),
			sigc::hide_return(sigc::bind(sigc::ptr_fun(&App::open),fullnames[i]))
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

	// remove group if exists
	typedef std::vector< Glib::RefPtr<Gtk::ActionGroup> > ActionGroupList;
	ActionGroupList groups = App::ui_manager()->get_action_groups();
	for(ActionGroupList::const_iterator i = groups.begin(); i != groups.end(); ++i)
		if ((*i)->get_name() == action_group->get_name())
			App::ui_manager()->remove_action_group(*i);
	groups.clear();

	App::ui_manager()->insert_action_group(action_group);
	App::ui_manager()->add_ui_from_string(ui_info_popup);
	App::ui_manager()->add_ui_from_string(ui_info_menubar);
}

void
MainWindow::on_dockable_registered(Dockable* dockable)
{

	// replace _ in panel names (filenames) by __ or it won't show up in the menu,
	// this block code is just a copy from MainWindow::on_recent_files_changed().
	std::string raw = dockable->get_local_name();
	std::string quoted;
	size_t pos = 0, last_pos = 0;
	for (pos = last_pos = 0; (pos = raw.find('_', pos)) != string::npos; last_pos = pos)
		quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
	quoted += raw.substr(last_pos);

	window_action_group->add( Gtk::Action::create("panel-" + dockable->get_name(), quoted),
		sigc::mem_fun(*dockable, &Dockable::present)
	);

	std::string ui_info =
		"<menu action='menu-window'>"
	    "<menuitem action='panel-" + dockable->get_name() + "' />"
	    "</menu>";
	std::string ui_info_popup =
		"<ui><popup action='menu-main'>" + ui_info + "</popup></ui>";
	std::string ui_info_menubar =
		"<ui><menubar action='menubar-main'>" + ui_info + "</menubar></ui>";

	Gtk::UIManager::ui_merge_id merge_id_popup = App::ui_manager()->add_ui_from_string(ui_info_popup);
	Gtk::UIManager::ui_merge_id merge_id_menubar = App::ui_manager()->add_ui_from_string(ui_info_menubar);

	// record CanvasView toolbar and popup id's
	CanvasView *canvas_view = dynamic_cast<CanvasView*>(dockable);
	if(canvas_view)
	{
		canvas_view->set_popup_id(merge_id_popup);
		canvas_view->set_toolbar_id(merge_id_menubar);
	}
}

void
MainWindow::on_dockable_unregistered(Dockable* dockable)
{
	// remove the document from the menus
	CanvasView *canvas_view = dynamic_cast<CanvasView*>(dockable);
	if(canvas_view)
	{
		App::ui_manager()->remove_ui(canvas_view->get_popup_id());
		App::ui_manager()->remove_ui(canvas_view->get_toolbar_id());
	}
}
/* === E N T R Y P O I N T ================================================= */
