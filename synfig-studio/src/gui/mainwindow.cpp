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
#include <gtkmm/cssprovider.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/textview.h>

#include <gui/actionmanagers/actionmanager.h>
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

	add(*bin_);

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

	signal_direction_changed().connect(sigc::hide(sigc::mem_fun(*this, &MainWindow::refresh_menu_icon_offset)));
	signal_map().connect(sigc::mem_fun(*this, &MainWindow::refresh_menu_icon_offset));

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
		App::get_action_manager()->add(ActionManager::Entry{"win." + item.name, item.label, item.shortcut, item.icon, item.tooltip});
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

		App::get_action_manager()->add(ActionManager::Entry{"win." + action_name, item.label, "", item.icon, item.tooltip});
	}

	// plugins
	if (App::menu_plugins) {
		for (const auto& plugin : studio::App::plugin_manager.plugins()) {
			App::menu_plugins->append(plugin.name.get(), "doc.plugin-" + plugin.id);
		}
	}



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
	App::enable_mainwin_menubar = !App::enable_mainwin_menubar;
	property_show_menubar() = App::enable_mainwin_menubar;

	auto action = this->lookup_action("show-menubar");
	if (action)
		action->change_state(App::enable_mainwin_menubar);
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

	auto action = this->lookup_action("show-toolbar");
	if (action)
		action->change_state(App::enable_mainwin_toolbar);
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

		const std::string action_name = synfig::strprintf("file-recent-%d", i);

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
MainWindow::refresh_menu_icon_offset()
{
	static auto provider = Gtk::CssProvider::create();
	auto screen   = Gdk::Screen::get_default();
	const bool is_ltr = App::main_window->get_direction() == Gtk::TEXT_DIR_LTR;

	int icon_offset = -24; // How to discover it in runtime?
	int margin_left = is_ltr ? icon_offset :   0;
	int margin_right = is_ltr ? 0 : icon_offset;
	provider->load_from_data(strprintf("menubar menuitem image, .shift-icon menuitem image { margin-left:%ipx; margin-right:%ipx}",  margin_left, margin_right));
	Gtk::StyleContext::remove_provider_for_screen(screen, provider);
	Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void
MainWindow::on_custom_workspaces_changed()
{
	App::menu_window_custom_workspaces->remove_all();
	auto workspace_group = Gio::SimpleActionGroup::create();

	std::vector<std::string> workspaces = get_workspaces();

	unsigned int num_custom_workspaces = 0;
	for (const auto& workspace_name : workspaces) {
		std::string action_name = synfig::strprintf("custom-workspace-%d", num_custom_workspaces);

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

	const CanvasView* canvas_view = dynamic_cast<CanvasView*>(dockable);

//	auto panel_group = Gio::SimpleActionGroup::create();
	/*panel_group->*/add_action("panel-"+dockable->get_name(), sigc::mem_fun(*dockable, &Dockable::present));
	App::get_action_manager()->add({"win.panel-"+dockable->get_name(), strprintf(_("Panel %s"), escaped_local_name.c_str()), ""});

	if (canvas_view)
		App::menu_window_canvas->append(escaped_local_name, "win.panel-" + dockable->get_name());
	else
		App::menu_window_docks->append(escaped_local_name, "win.panel-" + dockable->get_name());

//FIXME
//	insert_action_group("panel", panel_group);
}

void
MainWindow::on_dockable_unregistered(Dockable* dockable)
{
	// remove the document from the menus

	const auto dock_action_name = "win.panel-" + dockable->get_name();
	auto menu_window_docks = App::menu_window_docks;
	int i = menu_window_docks->get_n_items();
	while (--i >= 0) {
		auto attribute = menu_window_docks->get_item_attribute(i, Gio::MENU_ATTRIBUTE_ACTION, Glib::VARIANT_TYPE_STRING);
		auto action_name = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(attribute).get();
		if (action_name == dock_action_name) {
			menu_window_docks->remove(i);
			// TODO: remove action?
			break;
		}
	}
}
/* === E N T R Y P O I N T ================================================= */
