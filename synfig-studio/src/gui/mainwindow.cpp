/* === S Y N F I G ========================================================= */
/*!	\file mainwindow.cpp
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/mainwindow.h>

#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/dialogs/dialog_input.h>
#include <gui/docks/dockable.h>
#include <gui/docks/dockbook.h>
#include <gui/docks/dockmanager.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/widgets/widget_time.h>
#include <gui/widgets/widget_vector.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

MainWindow::MainWindow() :
	save_workspace_merge_id(0), custom_workspaces_merge_id(0)
{
	register_custom_widget_types();

	set_default_size(600, 400);

	main_dock_book_ = manage(new DockBook());
	main_dock_book_->allow_empty = true;
	main_dock_book_->show();

	class Bin : public Gtk::Bin {
	public:
		Bin() = default;
	protected:
		void on_size_allocate(Gtk::Allocation &allocation) override {
			Gtk::Bin::on_size_allocate(allocation);
			if (get_child() != nullptr)
				get_child()->size_allocate(allocation);
		}
	};

	bin_ = manage((Gtk::Bin*)new Bin());
	bin_->add(*main_dock_book_);
	bin_->show();

	auto visible_vbox = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	auto hidden_box   = manage(new Gtk::Box());

	auto visible_menubar = App::ui_manager()->get_widget("/menubar-main");
	auto hidden_menubar  = App::ui_manager()->get_widget("/menubar-hidden");
	if (visible_menubar != NULL)
	{
		hidden_box->add(*hidden_menubar);
		hidden_box->hide();

		visible_vbox->add(*hidden_box);
		visible_vbox->pack_start(*visible_menubar, false, false, 0);
	}

	visible_vbox->pack_end(*bin_, true, true, 0);
	visible_vbox->show();
	if(!App::enable_mainwin_menubar && visible_menubar) visible_menubar->hide();

	add(*visible_vbox);

	init_menus();
	window_action_group = Gtk::ActionGroup::create("mainwindow-window");
	App::ui_manager()->insert_action_group(window_action_group);

	App::signal_recent_files_changed().connect(
		sigc::mem_fun(*this, &MainWindow::on_recent_files_changed) );

	App::signal_custom_workspaces_changed().connect(
		sigc::mem_fun(*this, &MainWindow::on_custom_workspaces_changed) );

	signal_delete_event().connect(
		sigc::ptr_fun(App::shutdown_request) );

	App::dock_manager->signal_dockable_registered().connect(
		sigc::mem_fun(*this,&MainWindow::on_dockable_registered) );

	App::dock_manager->signal_dockable_unregistered().connect(
		sigc::mem_fun(*this,&MainWindow::on_dockable_unregistered) );

	set_type_hint(Gdk::WindowTypeHint(synfigapp::Main::settings().get_value("pref.mainwindow_hints", Gdk::WindowTypeHint())));
}

MainWindow::~MainWindow() = default;

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
	/*
	action_group->add( Gtk::Action::create("input-devices", _("Input Devices...")),
		sigc::ptr_fun(&MainWindow::show_dialog_input)
	);
	action_group->add( Gtk::Action::create("setup", _("Preferences...")),
		sigc::ptr_fun(&studio::App::show_setup)
	);
	*/
	//Gtk::Builder
	App::canvas_action_group()->add_action(App::instance()->add_action("input-devices",
		sigc::ptr_fun(&MainWindow::show_dialog_input)
	));
	App::canvas_action_group()->add_action(App::instance()->add_action("setup",
		sigc::ptr_fun(&studio::App::show_setup)
	));

	// View menu
	Glib::RefPtr<Gtk::ToggleAction> toggle_menubar = Gtk::ToggleAction::create("toggle-mainwin-menubar", _("Show Menubar"));
	toggle_menubar->set_active(App::enable_mainwin_menubar);
	action_group->add(toggle_menubar, sigc::mem_fun(*this, &studio::MainWindow::toggle_show_menubar));
	App::instance()->add_action_bool("show-menubar", 
		sigc::mem_fun(*this, &studio::MainWindow::appmenu_toggle_show_menubar), true);

	Glib::RefPtr<Gtk::ToggleAction> toggle_toolbar = Gtk::ToggleAction::create("toggle-mainwin-toolbar", _("Toolbar"));
	toggle_toolbar->set_active(App::enable_mainwin_toolbar);
	action_group->add(toggle_toolbar, sigc::mem_fun(*this, &studio::MainWindow::toggle_show_toolbar));
	App::instance()->add_action_bool("show-toolbar", 
		sigc::mem_fun(*this, &studio::MainWindow::appmenu_toggle_show_toolbar), true);
	
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
	action_group->add( Gtk::Action::create("save-workspace", Gtk::StockID("synfig-save_as"), _("Save workspace...")),
		sigc::ptr_fun(App::save_custom_workspace)
	);

	action_group->add( Gtk::Action::create("edit-workspacelist", _("Edit workspaces...")),
		sigc::ptr_fun(App::edit_custom_workspace_list)
	);

	// help
	#define URL(action_name,title,url) \
		action_group->add( Gtk::Action::create(action_name, title), \
			sigc::bind(sigc::ptr_fun(&studio::App::open_uri),url))
	#define WIKI(action_name,title,page) \
		URL(action_name,title, "https://wiki.synfig.org/" + String(page))

	action_group->add( Gtk::Action::create("help", Gtk::Stock::HELP),
		sigc::ptr_fun(studio::App::dialog_help)
	);

#if GTK_CHECK_VERSION(3, 20, 0)
	action_group->add( Gtk::Action::create(
			"help-shortcuts", _("Keyboard Shortcuts")),
		sigc::ptr_fun(studio::App::window_shortcuts)
	);
#endif

	// TRANSLATORS:         | Help menu entry:              | A wiki page:          |
	URL("help-tutorials",	_("Tutorials"),					_("https://synfig.readthedocs.io/en/latest/tutorials.html"));
	WIKI("help-reference",	_("Reference"),					_("Category:Reference"));
	URL("help-faq",		_("Frequently Asked Questions"),	_("https://wiki.synfig.org/FAQ")				);
	URL("help-support",		_("Get Support"),				_("https://forums.synfig.org/")	);

	action_group->add( Gtk::Action::create(
			"help-about", Gtk::StockID("synfig-about"), _("About Synfig Studio")),
		sigc::ptr_fun(studio::App::dialog_about)
	);

	// TODO: open recent
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));

	App::ui_manager()->insert_action_group(action_group);

	add_custom_workspace_menu_item_handlers();
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

void
MainWindow::appmenu_toggle_show_menubar()
{
	bool isActive = false;
	auto toggle_action = App::instance()->lookup_action("show-menubar");
	if (auto action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(toggle_action)) {
		action->get_state(isActive);
		isActive = !isActive;
		action->change_state(isActive);
		MainWindow::toggle_show_menubar();
	}
}

void
MainWindow::toggle_show_toolbar()
{
	App::enable_mainwin_toolbar = !App::enable_mainwin_toolbar;
	
	for(std::list<etl::handle<Instance> >::iterator iter1 = App::instance_list.begin(); iter1 != App::instance_list.end(); iter1++){
			const Instance::CanvasViewList &views = (*iter1)->canvas_view_list();
			for(Instance::CanvasViewList::const_iterator iter2 = views.begin(); iter2 != views.end(); ++iter2)
				(*iter2)->toggle_show_toolbar();
	}
}

void
MainWindow::appmenu_toggle_show_toolbar()
{
	bool isActive = false;
	auto toggle_action = App::instance()->lookup_action("show-toolbar");
	if (auto action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(toggle_action)) {
		action->get_state(isActive);
		isActive = !isActive;
		action->change_state(isActive);
		MainWindow::toggle_show_toolbar();
	}
}

void MainWindow::add_custom_workspace_menu_item_handlers()
{
	std::string ui_info_menu =
			"<menu action='menu-window'>"
			"	<menu action='menu-workspace'>"
			"	    <separator name='sep-window2'/>"
			"		<menuitem action='save-workspace' />"
			"		<menuitem action='edit-workspacelist' />"
			"	</menu>"
			"</menu>";

	std::string ui_info =
			"<ui>"
			"  <popup name='menu-main' action='menu-main'>" + ui_info_menu + "</popup>"
			"  <menubar name='menubar-main' action='menubar-main'>" + ui_info_menu + "</menubar>"
			"</ui>";

	save_workspace_merge_id = App::ui_manager()->add_ui_from_string(ui_info);
}

void MainWindow::remove_custom_workspace_menu_item_handlers()
{
	App::ui_manager()->remove_ui(save_workspace_merge_id);
}

bool
MainWindow::on_key_press_event(GdkEventKey* key_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	Gtk::Widget * widget = get_focus();
	if (widget && (dynamic_cast<Gtk::Editable*>(widget) || dynamic_cast<Gtk::TextView*>(widget) || dynamic_cast<Gtk::DrawingArea*>(widget))) {
		bool handled = gtk_window_propagate_key_event(this->gobj(), key_event);
		if (handled)
			return true;
	}
	return Gtk::Window::on_key_press_event(key_event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
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
	std::vector< std::vector<String> > dirs(count);
	std::vector< std::vector<bool> > dirflags(count);
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
			if (k == std::string::npos) k = fullname.size();
			std::string sub = fullname.substr(j, k - j);
			if (!sub.empty() && sub != "...")
				dirs[i].insert(dirs[i].begin(), sub);
			j = (int)k + 1;
		}

		dirflags[i].resize(dirs[i].size(), false);
	}

	// find shortest paths which shows that files are different
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
	// TODO(ice0): switch to GtkRecentChooserMenu?
	//Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create("mainwindow-recentfiles");

	std::vector<String> fullnames(App::get_recent_files().begin(), App::get_recent_files().end());
	std::vector<String> shortnames;
	make_short_filenames(fullnames, shortnames);

	auto menu_object = App::builder()->get_object("recent-file");
	auto recent_file_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(menu_object);
	recent_file_menu->remove_all();

	std::string menu_items;
	for(int i = 0; i < (int)fullnames.size(); ++i)
	{
		std::string raw = shortnames[i];
		std::string quoted;
		size_t pos = 0, last_pos = 0;

		// replace _ in filenames by __ or it won't show up in the menu
		for (pos = last_pos = 0; (pos = raw.find('_', pos)) != std::string::npos; last_pos = pos)
			quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
		quoted += raw.substr(last_pos);

		//const std::string action_name = etl::strprintf("file-recent-%d", i);
		//menu_items += "<menuitem action='" + action_name +"' />";

		std::string filename = fullnames[i];
		/*
		action_group->add( Gtk::Action::create(action_name, quoted, fullnames[i]),
			[filename](){App::open_recent(filename);}
		);
		*/
		
		replace(raw.begin(), raw.end(), ' ', '-');
		recent_file_menu->append(raw, "app.open-recent-"+raw);
		App::instance()->add_action("open-recent-"+raw, [filename](){App::open_recent(filename);});
	}

/*
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
	*/
}

void
MainWindow::on_custom_workspaces_changed()
{
	Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create("mainwindow-customworkspaces");

	std::vector<std::string> workspaces = App::get_workspaces();

	std::string menu_items;
	unsigned int num_custom_workspaces = 0;
	for (auto it = workspaces.cbegin(); it != workspaces.cend(); ++it, ++num_custom_workspaces) {
		std::string raw = *it;
		std::string quoted;
		size_t pos = 0, last_pos = 0;

		// replace _ in names by __ or it won't show up in the menu
		for (pos = last_pos = 0; (pos = raw.find('_', pos)) != std::string::npos; last_pos = pos)
			quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
		quoted += raw.substr(last_pos);

		std::string action_name = etl::strprintf("custom-workspace-%d", num_custom_workspaces);
		menu_items += "<menuitem action='" + action_name +"' />";

		action_group->add( Gtk::Action::create(action_name, quoted),
			sigc::bind(sigc::ptr_fun(&App::set_workspace_from_name), workspaces[num_custom_workspaces])
		);
	}
	if (num_custom_workspaces > 0)
		menu_items = "<separator name='sep-window1' />" + menu_items;

	remove_custom_workspace_menu_item_handlers();
	if (custom_workspaces_merge_id)
		App::ui_manager()->remove_ui(custom_workspaces_merge_id);

	std::string ui_info =
		"<menu action='menu-window'><menu action='menu-workspace'>"
	  + menu_items
	  + "</menu></menu>";
	std::string ui_info_popup =
		"<popup action='menu-main'>" + ui_info + "</popup>";
	std::string ui_info_menubar =
		"<menubar action='menubar-main'>" + ui_info + "</menubar>";

	// remove group if exists
	typedef std::vector< Glib::RefPtr<Gtk::ActionGroup> > ActionGroupList;
	ActionGroupList groups = App::ui_manager()->get_action_groups();
	for(ActionGroupList::const_iterator it = groups.begin(); it != groups.end(); ++it)
		if ((*it)->get_name() == action_group->get_name()) {
			App::ui_manager()->remove_action_group(*it);
			break;
		}
	groups.clear();
	App::ui_manager()->ensure_update();

	App::ui_manager()->insert_action_group(action_group);
	custom_workspaces_merge_id = App::ui_manager()->add_ui_from_string("<ui>" + ui_info_popup + ui_info_menubar + "</ui>");

	add_custom_workspace_menu_item_handlers();
}

void
MainWindow::on_dockable_registered(Dockable* dockable)
{

	// replace _ in panel names (filenames) by __ or it won't show up in the menu,
	// this block code is just a copy from MainWindow::on_recent_files_changed().
	std::string raw = dockable->get_local_name();
	std::string quoted;
	size_t pos = 0, last_pos = 0;
	for (pos = last_pos = 0; (pos = raw.find('_', pos)) != std::string::npos; last_pos = pos)
		quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
	quoted += raw.substr(last_pos);

	window_action_group->add( Gtk::Action::create("panel-" + dockable->get_name(), quoted),
		sigc::mem_fun(*dockable, &Dockable::present)
	);

	const std::string ui_info =
		"<menu action='menu-window'>"
	    "<menuitem action='panel-" + dockable->get_name() + "' />"
	    "</menu>";
	const std::string ui_info_popup =
		"<ui><popup action='menu-main'>" + ui_info + "</popup></ui>";
	const std::string ui_info_menubar =
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
