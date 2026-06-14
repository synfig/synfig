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
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>

#include <gui/actiondatabase.h>
#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/dialogs/dialog_input.h>
#include <gui/dialogs/dialog_workspaces.h>
#include <gui/docks/dockable.h>
#include <gui/docks/dockbook.h>
#include <gui/docks/dockmanager.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/widgets/widget_link.h>
#include <gui/widgets/widget_time.h>
#include <gui/widgets/widget_vector.h>
#include <gui/workspacehandler.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

std::unique_ptr<studio::WorkspaceHandler> studio::MainWindow::workspaces = nullptr;
static sigc::signal<void> signal_custom_workspaces_changed_;

/* === P R O C E D U R E S ================================================= */

// replace _ in menu item labels with __ or it won't show up in the menu
static std::string
escape_underline(const std::string& raw)
{
	std::string quoted;
	size_t pos = 0, last_pos = 0;
	for (pos = last_pos = 0; (pos = raw.find('_', pos)) != std::string::npos; last_pos = pos)
		quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
	quoted += raw.substr(last_pos);
	return quoted;
}

static void
on_show_panel_actionated(const Glib::VariantBase& v)
{
	if (!App::dock_manager)
		return;

	const auto panel_name_vrt = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v);
	if (panel_name_vrt)
		App::dock_manager->present(panel_name_vrt.get());
	else
		synfig::warning(_("Action show-panel: panel name should be a string"));
}

static void
on_set_workspace_actionated(const Glib::VariantBase& v)
{
	if (!App::main_window)
		return;

	const auto workspace_name_vrt = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v);
	if (workspace_name_vrt)
		App::main_window->set_workspace_from_name(workspace_name_vrt.get());
	else
		synfig::warning(_("Action show-panel: panel name should be a string"));
}

/* === M E T H O D S ======================================================= */

MainWindow::MainWindow(const Glib::RefPtr<Gtk::Application>& application)
	: Gtk::ApplicationWindow(application)
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
	if (visible_menubar)
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

	init_actions();
	init_menus();

	App::signal_recent_files_changed().connect(
		sigc::mem_fun(*this, &MainWindow::on_recent_files_changed) );

	signal_custom_workspaces_changed().connect(
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
	action_group->add( Gtk::Action::create_with_icon_name("new", "action_doc_new_icon", _("New"), _("Create a new document")),
		sigc::hide_return(sigc::ptr_fun(&studio::App::new_instance))
	);
	action_group->add( Gtk::Action::create_with_icon_name("open", "action_doc_open_icon", _("Open"), _("Open an existing document")),
		sigc::hide_return(sigc::bind(sigc::ptr_fun(&studio::App::dialog_open), filesystem::Path{}))
	);
	action_group->add( Gtk::Action::create_with_icon_name("save-all", "action_doc_saveall_icon", _("Save All"), _("Save all opened documents")),
		sigc::ptr_fun(&save_all)
	);
	action_group->add( Gtk::Action::create_with_icon_name("quit", "application-exit", _("_Quit"), _("Quit")),
		sigc::hide_return(sigc::ptr_fun(&studio::App::quit))
	);

	// Edit menu
	action_group->add( Gtk::Action::create("input-devices", _("Input Devices...")),
		sigc::ptr_fun(&MainWindow::show_dialog_input)
	);
	action_group->add( Gtk::Action::create("preferences", _("Preferences...")),
		sigc::ptr_fun(&studio::App::show_setup)
	);

	// View menu
	Glib::RefPtr<Gtk::ToggleAction> toggle_menubar = Gtk::ToggleAction::create("toggle-mainwin-menubar", _("Show Menubar"));
	toggle_menubar->set_active(App::enable_mainwin_menubar);
	action_group->add(toggle_menubar, sigc::mem_fun(*this, &studio::MainWindow::toggle_show_menubar));

	Glib::RefPtr<Gtk::ToggleAction> toggle_toolbar = Gtk::ToggleAction::create("toggle-mainwin-toolbar", _("Toolbar"));
	toggle_toolbar->set_active(App::enable_mainwin_toolbar);
	action_group->add(toggle_toolbar, sigc::mem_fun(*this, &studio::MainWindow::toggle_show_toolbar));
	
	// plugins
	if (App::menu_plugins) {
		for (const auto& plugin : studio::App::plugin_manager.plugins())
			App::menu_plugins->append(plugin.name.get(), strprintf("doc.run-plugin('%s')", plugin.id.c_str()));

		auto menuitem_plugins = dynamic_cast<Gtk::MenuItem*>(App::ui_manager()->get_widget("/menubar-main/menu-plugins"));
		auto menuitem_plugins2 = dynamic_cast<Gtk::MenuItem*>(App::ui_manager()->get_widget("/menu-main/menu-plugins"));
		auto menu = Gtk::manage(new Gtk::Menu(App::menu_plugins));
		menuitem_plugins->set_submenu(*menu);
		auto menu2 = Gtk::manage(new Gtk::Menu(App::menu_plugins));
		menuitem_plugins2->set_submenu(*menu2);
	}

	//animation tabs
	for (int i = 1; i <= 8; ++i) {
		const std::string tab = std::to_string(i);
		action_group->add(Gtk::Action::create("switch-to-tab-" + tab, _("Switch to Tab ") + tab),
			sigc::track_obj([this, i]() { main_dock_book().set_current_page(i-1); }, this)
		);
	}
	action_group->add(Gtk::Action::create("switch-to-rightmost-tab", _("Switch to Rightmost Tab")),
		sigc::track_obj([this]() { main_dock_book().set_current_page(-1); }, this)
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

	action_group->add( Gtk::Action::create_with_icon_name(
			"about", "about_icon", _("About Synfig Studio"), _("About Synfig Studio")),
		sigc::ptr_fun(studio::App::dialog_about)
	);

	// TODO: open recent
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));

	App::ui_manager()->insert_action_group(action_group);

	// Workspace menu
	auto menu_window = Gio::Menu::create();
	auto menu_window_workspaces = Gio::Menu::create();
	App::menu_window_custom_workspaces = Gio::Menu::create();
	App::menu_window_docks = Gio::Menu::create();
	App::menu_window_canvases = Gio::Menu::create();
	auto menu_window_default_workspaces = Gio::Menu::create();
	menu_window_default_workspaces->append(_("Default"), "win.workspace-default");
	menu_window_default_workspaces->append(_("Compositing"), "win.workspace-compositing");
	menu_window_default_workspaces->append(_("Animating"), "win.workspace-animating");
	menu_window_workspaces->append_section(menu_window_default_workspaces);
	menu_window_workspaces->append_section(App::menu_window_custom_workspaces);
	auto menu_window_workspaces_actions = Gio::Menu::create();
	menu_window_workspaces_actions->append(_("Save workspace..."), "win.save-workspace");
	menu_window_workspaces_actions->append(_("Edit workspaces..."), "win.edit-workspacelist");
	menu_window_workspaces->append_section(menu_window_workspaces_actions);
	menu_window->append_submenu(_("Workspace"), menu_window_workspaces);
	menu_window->append_section(App::menu_window_docks);
	menu_window->append_section(App::menu_window_canvases);
	auto menuitem_window = dynamic_cast<Gtk::MenuItem*>(App::ui_manager()->get_widget("/menubar-main/menu-window"));
	menuitem_window->set_submenu(*Gtk::manage(new Gtk::Menu(menu_window)));
	auto menuitem_window2 = dynamic_cast<Gtk::MenuItem*>(App::ui_manager()->get_widget("/menu-main/menu-window"));
	menuitem_window2->set_submenu(*Gtk::manage(new Gtk::Menu(menu_window)));
}

void
MainWindow::init_actions()
{
	struct ActionMetadata {
		std::string name;
		std::string icon;
		std::string shortcut;
		std::string label;
		std::string tooltip;
		std::function<void()> slot;
	};

	const std::vector<ActionMetadata> action_list = {
		{"save-all",                "action_doc_saveall_icon",   "<Primary>e", N_("Save All"),           N_("Save all open documents"),  sigc::ptr_fun(save_all)},

		// pre defined workspace (window ui layout)
		{"workspace-compositing",   "",                          "",           N_("Compositing"),        "",                             sigc::ptr_fun(MainWindow::set_workspace_compositing)},
		{"workspace-animating",     "",                          "",           N_("Animating"),          "",                             sigc::ptr_fun(MainWindow::set_workspace_animating)},
		{"workspace-default",       "",                          "",           N_("Default"),            "",                             sigc::ptr_fun(MainWindow::set_workspace_default)},
		{"save-workspace",          "action_doc_saveas_icon",    "",           N_("Save workspace..."),  N_("Save workspace..."),                            sigc::mem_fun(*this, &MainWindow::save_custom_workspace)},
		{"edit-workspacelist",      "",                          "",           N_("Edit workspaces..."), N_("Edit workspaces..."),       sigc::ptr_fun(MainWindow::edit_custom_workspace_list)},

		// animation tabs
		{"switch-to-tab-1",         "",                          "<Primary>1", N_("Switch to Tab 1"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(0); }, this)},
		{"switch-to-tab-2",         "",                          "<Primary>2", N_("Switch to Tab 2"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(1); }, this)},
		{"switch-to-tab-3",         "",                          "<Primary>3", N_("Switch to Tab 3"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(2); }, this)},
		{"switch-to-tab-4",         "",                          "<Primary>4", N_("Switch to Tab 4"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(3); }, this)},
		{"switch-to-tab-5",         "",                          "<Primary>5", N_("Switch to Tab 5"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(4); }, this)},
		{"switch-to-tab-6",         "",                          "<Primary>6", N_("Switch to Tab 6"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(5); }, this)},
		{"switch-to-tab-7",         "",                          "<Primary>7", N_("Switch to Tab 7"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(6); }, this)},
		{"switch-to-tab-8",         "",                          "<Primary>8", N_("Switch to Tab 8"),         "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(7); }, this)},
		{"switch-to-rightmost-tab", "",                          "<Primary>9", N_("Switch to Rightmost Tab"), "",                        sigc::track_obj([this]() { main_dock_book().set_current_page(-1); }, this)},

		// input devices
		{"input-devices",           "",                          "",           N_("Input Devices..."),   "",                             sigc::ptr_fun(&MainWindow::show_dialog_input)},

		// docks (including open file tabs): set via MainWindow::on_dockable_registered()
	};

	for (const auto& item : action_list) {
		App::get_action_database()->add(ActionDatabase::Entry{"win." + item.name, item.label, item.shortcut, item.icon, item.tooltip});
		add_action(item.name, item.slot);
	}

	struct BoolActionMetadata {
		const std::string name;
		const std::string icon;
		const std::string label;
		const std::string tooltip;
		// bool (WorkArea::*slot_to_get)(void) const;
		void (MainWindow::*slot_to_toogle)(void);
	};

	const std::vector<BoolActionMetadata> bool_action_list = {
		{"show-menubar", "", N_("Show Menubar"), "", &studio::MainWindow::toggle_show_menubar},
		{"show-toolbar", "", N_("Show Toolbar"), "", &studio::MainWindow::toggle_show_toolbar},
	};

	for (const auto& item : bool_action_list) {
		bool current_value = true;
		auto action_name = item.name;
		add_action_bool(action_name, sigc::mem_fun(*this, item.slot_to_toogle), current_value);

		App::get_action_database()->add(ActionDatabase::Entry{"win." + action_name, item.label, "", item.icon, item.tooltip});
	}

	add_action_with_parameter("show-panel", Glib::Variant<Glib::ustring>().variant_type(), sigc::ptr_fun(on_show_panel_actionated));
	add_action_with_parameter("set-workspace", Glib::Variant<Glib::ustring>().variant_type(), sigc::ptr_fun(on_set_workspace_actionated));
}

void MainWindow::register_custom_widget_types()
{
	Widget_Link::register_type();
	Widget_Time::register_type();
	Widget_Vector::register_type();
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
MainWindow::toggle_show_toolbar()
{
	App::enable_mainwin_toolbar = !App::enable_mainwin_toolbar;
	
	for (const auto& instance : App::instance_list) {
		const Instance::CanvasViewList& views = instance->canvas_view_list();
		for (auto& canvas_view : views)
			canvas_view->set_show_toolbars(App::enable_mainwin_toolbar);
	}
}

void
MainWindow::save_all()
{
	for (auto& instance : App::instance_list)
		instance->save();
}

const std::vector<std::string>
MainWindow::get_workspaces()
{
	std::vector<std::string> list;
	if (workspaces)
		workspaces->get_name_list(list);
	return list;
}

bool
MainWindow::on_key_press_event(GdkEventKey* key_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	Gtk::Widget * widget = get_focus();
	if (widget && (dynamic_cast<Gtk::Editable*>(widget) || dynamic_cast<Gtk::TextView*>(widget) || dynamic_cast<Gtk::DrawingArea*>(widget))) {
		bool handled = gtk_window_propagate_key_event(GTK_WINDOW(this->gobj()), key_event);
		if (handled)
			return true;
	}
	return Gtk::Window::on_key_press_event(key_event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
MainWindow::make_short_filenames(
	const std::vector<synfig::filesystem::Path> &fullnames,
	std::vector<synfig::String> &shortnames )
{
	if (fullnames.size() == 1)
	{
		shortnames.resize(1);
		shortnames[0] = fullnames[0].filename().u8string();
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
		String fullname = fullnames[i].u8string();
		if (fullname.substr(0, 7) == "file://")
			fullname = fullname.substr(7);
		while(j < (int)fullname.size())
		{
			size_t dir_separator_pos = fullname.find_first_of("/\\", j);
			if (dir_separator_pos == std::string::npos) dir_separator_pos = fullname.size();
			std::string sub = fullname.substr(j, dir_separator_pos - j);
			if (!sub.empty() && sub != "...")
				dirs[i].insert(dirs[i].begin(), sub);
			j = (int)dir_separator_pos + 1;
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

	const std::vector<filesystem::Path> fullnames(App::get_recent_files().begin(), App::get_recent_files().end());
	std::vector<String> shortnames;
	make_short_filenames(fullnames, shortnames);

	App::menu_recent_files->remove_all();

	for (int i = 0; i < (int)fullnames.size(); ++i) {
		const std::string raw = shortnames[i];
		const std::string quoted_shortname = escape_underline(raw);
		const filesystem::Path filename = fullnames[i];
		App::menu_recent_files->append_item(Gio::MenuItem::create(quoted_shortname, strprintf("app.open-recent-file(\"%s\")", filename.c_str())));
	}
}

void
MainWindow::set_workspace_default()
{
	std::string tpl =
	"[mainwindow|%0X|%0Y|%100x|%90y|"
		"[hor|%75x"
			"|[vert|%70y"
				"|[hor|%10x"
					"|[book|toolbox]"
					"|[mainnotebook]"
				"]"
				"|[hor|%25x"
					"|[book|params|keyframes]"
					"|[book|timetrack|curves|children|meta_data|soundwave]"
				"]"
			"]"
			"|[vert|%20y"
				"|[book|canvases|pal_edit|navigator|info]"
				"|[vert|%25y"
					"|[book|tool_options|history]"
										"|[book|layers|groups]"
				"]"
			"]"
		"]"
	"]";

	set_workspace_from_template(tpl);
}

void
MainWindow::set_workspace_compositing()
{
	std::string tpl =
	"[mainwindow|%0X|%0Y|%100x|%90y|"
		"[hor|%1x"
			"|[vert|%1y|[book|toolbox]|[book|tool_options]]"
			"|[hor|%60x|[mainnotebook]"
				"|[hor|%50x|[book|params]"
					"|[vert|%30y|[book|history|groups]|[book|layers|canvases]]"
			"]"
		"]"
	"]";

	set_workspace_from_template(tpl);
}

void
MainWindow::set_workspace_animating()
{
	std::string tpl =
	"[mainwindow|%0X|%0Y|%100x|%90y|"
		"[hor|%70x"
			"|[vert|%1y"
				"|[hor|%1x|[book|toolbox]|[mainnotebook]]"
				"|[hor|%25x|[book|params|children]|[book|timetrack|curves|soundwave|]]"
			"]"
			"|[vert|%30y"
				"|[book|keyframes|history|groups]|[book|layers|canvases]]"
			"]"
		"]"
	"]";

	set_workspace_from_template(tpl);
}

void
MainWindow::set_workspace_from_template(const std::string& tpl)
{
	Glib::RefPtr<Gdk::Display> display(Gdk::Display::get_default());
	Glib::RefPtr<const Gdk::Screen> screen(display->get_default_screen());
	Gdk::Rectangle rect;
	// A proper way to obtain the primary monitor is to use the
	// Gdk::Screen::get_primary_monitor () const member. But as it
	// was introduced in gtkmm 2.20 I assume that the monitor 0 is the
	// primary one.
	screen->get_monitor_geometry(0,rect);
	float dx = (float)rect.get_x();
	float dy = (float)rect.get_y();
	float sx = (float)rect.get_width();
	float sy = (float)rect.get_height();

	std::string layout = DockManager::layout_from_template(tpl, dx, dy, sx, sy);
	App::dock_manager->load_layout_from_string(layout);
	App::dock_manager->show_all_dock_dialogs();
}

void
MainWindow::set_workspace_from_name(const std::string& name)
{
	if (!workspaces)
		return;
	std::string tpl;
	bool ok = workspaces->get_workspace(name, tpl);
	if (!ok)
		return;
	set_workspace_from_template(tpl);
}

void
MainWindow::save_custom_workspace()
{
	if (!App::dock_manager || !workspaces) {
		Gtk::MessageDialog dialog(*this, _("Internal error: Dock Manager or Workspace Handler not set"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, true);
		return;
	}

	Gtk::MessageDialog dialog(*this, _("Type a name for this custom workspace:"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);

	dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	Gtk::Button * ok_button = dialog.add_button(_("Ok"), Gtk::RESPONSE_OK);
	ok_button->set_sensitive(false);

	Gtk::Entry * name_entry = Gtk::manage(new Gtk::Entry());
	name_entry->set_margin_start(16);
	name_entry->set_margin_end(16);
	name_entry->signal_changed().connect(sigc::track_obj([&](){
		std::string name = synfig::trim(name_entry->get_text());
		bool has_equal_sign = name.find('=') != std::string::npos;
		ok_button->set_sensitive(!name.empty() && !has_equal_sign);
		if (ok_button->is_sensitive())
			ok_button->grab_default();
	}, dialog));
	name_entry->signal_activate().connect(sigc::mem_fun(*ok_button, &Gtk::Button::clicked));

	dialog.get_content_area()->set_spacing(12);
	dialog.get_content_area()->add(*name_entry);

	ok_button->set_can_default(true);

	dialog.show_all();

	int response = dialog.run();
	if (response != Gtk::RESPONSE_OK)
		return;

	std::string name = synfig::trim(name_entry->get_text());

	std::string tpl = App::dock_manager->save_layout_to_string();
	if (!workspaces->has_workspace(name))
		workspaces->add_workspace(name, tpl);
	else {
		Gtk::MessageDialog confirm_dlg(dialog, _("Do you want to overwrite this workspace?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
		if (confirm_dlg.run() != Gtk::RESPONSE_OK)
			return;
		workspaces->set_workspace(name, tpl);
	}
}

void
MainWindow::load_custom_workspaces()
{
	if (!workspaces) {
		workspaces = std::unique_ptr<WorkspaceHandler>(new WorkspaceHandler());
		workspaces->signal_list_changed().connect( sigc::mem_fun(signal_custom_workspaces_changed_, &sigc::signal<void>::emit) );
	}
	workspaces->clear();
	filesystem::Path filename = App::get_config_file("workspaces");
	workspaces->load(filename);
}

void
MainWindow::save_custom_workspaces()
{
	if (workspaces) {
		filesystem::Path filename = App::get_config_file("workspaces");
		workspaces->save(filename);
	}
}

sigc::signal<void>&
MainWindow::signal_custom_workspaces_changed()
{
	return signal_custom_workspaces_changed_;
}

void
MainWindow::edit_custom_workspace_list()
{
	Dialog_Workspaces* dlg = Dialog_Workspaces::create(*App::main_window);
	if (!dlg) {
		synfig::warning("Can't load Dialog_Workspaces");
		return;
	}
	dlg->run();
	delete dlg;
}

void
MainWindow::on_custom_workspaces_changed()
{
	App::menu_window_custom_workspaces->remove_all();
	auto workspace_group = Gio::SimpleActionGroup::create();

	std::vector<std::string> workspaces = get_workspaces();

	unsigned int num_custom_workspaces = 0;
	for (const auto& workspace_name : workspaces) {
		const std::string action_name = synfig::strprintf("custom-workspace-%d", num_custom_workspaces);

		workspace_group->add_action(action_name, sigc::bind(sigc::ptr_fun(&MainWindow::set_workspace_from_name), workspace_name));
		App::menu_window_custom_workspaces->append(workspace_name, "workspace." + action_name);

		++num_custom_workspaces;
	}

	// remove group if exists
	remove_action_group("workspace");
	insert_action_group("workspace", workspace_group);
}

void
MainWindow::on_dockable_registered(Dockable* dockable)
{
	const std::string local_name = dockable->get_local_name();
	const std::string escaped_local_name = escape_underline(local_name);

	// Action created to help add shortcuts to it
	add_action("panel-"+dockable->get_name(), sigc::mem_fun(*dockable, &Dockable::present));
	App::get_action_database()->add({"win.panel-"+dockable->get_name(), strprintf(_("Panel %s"), escaped_local_name.c_str()), ""});

	const bool is_canvas_view = dynamic_cast<CanvasView*>(dockable);
	const std::string detailed_action_name = "win.show-panel('" + dockable->get_name() + "')";

	if (is_canvas_view) {
		App::menu_window_canvases->append(escaped_local_name, detailed_action_name);
	} else {
		// Sort docks by local name
		const int max_items = App::menu_window_docks->get_n_items();
		bool inserted = false;
		for (int i = 0; i < max_items; ++i) {
			const auto attribute = App::menu_window_docks->get_item_attribute(i, Gio::MENU_ATTRIBUTE_LABEL, Glib::VARIANT_TYPE_STRING);
			const auto label = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(attribute).get();
			if (label.compare(escaped_local_name) > 0) {
				App::menu_window_docks->insert(i, escaped_local_name, detailed_action_name);
				inserted = true;
				break;
			}
		}

		if (!inserted)
			App::menu_window_docks->append(escaped_local_name, detailed_action_name);
	}
}

void
MainWindow::on_dockable_unregistered(Dockable* dockable)
{
	const std::string dock_action_name = "win.show-panel";
	const bool is_canvas_view = dynamic_cast<CanvasView*>(dockable);

	auto menu_section = is_canvas_view ? App::menu_window_canvases : App::menu_window_docks;
	int i = menu_section->get_n_items();
	while (--i >= 0) {
		const auto attribute = menu_section->get_item_attribute(i, Gio::MENU_ATTRIBUTE_ACTION, Glib::VARIANT_TYPE_STRING);
		const auto action_name = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(attribute).get();
		if (action_name == dock_action_name) {
			const auto attribute = menu_section->get_item_attribute(i, Gio::MENU_ATTRIBUTE_TARGET, Glib::VARIANT_TYPE_STRING);
			const auto target_name = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(attribute).get();
			if (target_name == dockable->get_name()) {
				menu_section->remove(i);
				// TODO: remove action?
				break;
			}
		}
	}
}
/* === E N T R Y P O I N T ================================================= */
