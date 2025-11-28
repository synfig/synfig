/* === S Y N F I G ========================================================= */
/*!	\file app.cpp
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008 Gerald Young
**	Copyright (c) 2008, 2010-2013 Carlos López
**	Copyright (c) 2009, 2011 Nikita Kitaev
**	Copyright (c) 2012-2015 Konstantin Dmitriev
**	Copyright (c) 2013-2016 Jerome Blanchi
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

#include <fstream>
#include <iostream>

#ifdef __OpenBSD__
#include <errno.h>
#elif defined(HAVE_SYS_ERRNO_H)
#include <sys/errno.h>
#endif

#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/init.h>
#include <glibmm/main.h>
#include <glibmm/miscutils.h>
#include <glibmm/timer.h>
#include <glibmm/shell.h>
#include <glibmm/spawn.h>

#include <gtkmm/accelmap.h>
#include <gtkmm/builder.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/settings.h>
#if GTK_CHECK_VERSION(3, 20, 0)
#include <gtkmm/shortcutswindow.h>
#endif
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

#include <gui/app.h>
#include <gui/autorecover.h>
#include <gui/canvasview.h>
#include <gui/devicetracker.h>

#include <gui/dialogs/about.h>
#include <gui/dialogs/dialog_color.h>
#include <gui/dialogs/dialog_gradient.h>
#include <gui/dialogs/dialog_input.h>
#include <gui/dialogs/dialog_setup.h>
#include <gui/dialogs/vectorizersettings.h>

#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dockmanager.h>
#include <gui/docks/dock_canvases.h>
#include <gui/docks/dock_children.h>
#include <gui/docks/dock_curves.h>
#include <gui/docks/dock_history.h>
#include <gui/docks/dock_info.h>
#include <gui/docks/dock_keyframes.h>
#include <gui/docks/dock_layers.h>
#include <gui/docks/dock_layergroups.h>
#include <gui/docks/dock_params.h>
#include <gui/docks/dock_metadata.h>
#include <gui/docks/dock_navigator.h>
#include <gui/docks/dock_soundwave.h>
#include <gui/docks/dock_timetrack.h>
#include <gui/docks/dock_timetrack2.h>
#include <gui/docks/dock_toolbox.h>

#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/modules/mod_palette/mod_palette.h>
#include <gui/onemoment.h>
#include <gui/resourcehelper.h>
#include <gui/splash.h>

#include <gui/statemanager.h>
#include <gui/states/state_bline.h>
#include <gui/states/state_bone.h>
#include <gui/states/state_brush.h>
#include <gui/states/state_circle.h>
#include <gui/states/state_draw.h>
#include <gui/states/state_eyedrop.h>
#include <gui/states/state_fill.h>
#include <gui/states/state_gradient.h>
#include <gui/states/state_lasso.h>
#include <gui/states/state_mirror.h>
#include <gui/states/state_normal.h>
#include <gui/states/state_polygon.h>
#include <gui/states/state_rectangle.h>
#include <gui/states/state_rotate.h>
#include <gui/states/state_scale.h>
#include <gui/states/state_sketch.h>
#include <gui/states/state_smoothmove.h>
#include <gui/states/state_star.h>
#include <gui/states/state_text.h>
#include <gui/states/state_width.h>
#include <gui/states/state_zoom.h>

#include <gui/actionmanagers/actionmanager.h>

#include <gui/widgets/widget_enum.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/color.h>
#include <synfig/filesystemnative.h>
#include <synfig/general.h>
#include <synfig/layer.h>
#include <synfig/loadcanvas.h>
#include <synfig/os.h>
#include <synfig/savecanvas.h>
#include <synfig/soundprocessor.h>
#include <synfig/string_helper.h>
#include <synfig/version.h>
#include <gui/exception_guard.h>

#include <synfigapp/action.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>
#include <synfigapp/settings.h>

#include <thread>
#include <gtkmm/icontheme.h>

#ifdef _WIN32

#include <gui/main_win32.h>

#define WINVER 0x0500
#include <windows.h>

//#define USE_WIN32_FILE_DIALOGS 1
#ifdef USE_WIN32_FILE_DIALOGS
 #include <gdk/gdkwin32.h>
 #include <cstring>
 static OPENFILENAME ofn={};
#endif // USE_WIN32_FILE_DIALOGS

#endif // _WIN32


#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DEFAULT_ICON_THEME_NAME "classic"
/* === S I G N A L S ======================================================= */

static sigc::signal<void> signal_present_all_;
sigc::signal<void>&
App::signal_present_all() { return signal_present_all_; }

static sigc::signal<void> signal_recent_files_changed_;
sigc::signal<void>&
App::signal_recent_files_changed() { return signal_recent_files_changed_; }

static sigc::signal<void,CanvasView::LooseHandle > signal_canvas_view_focus_;
sigc::signal<void,CanvasView::LooseHandle >&
App::signal_canvas_view_focus() { return signal_canvas_view_focus_; }

static sigc::signal<void,etl::handle<Instance> > signal_instance_selected_;
sigc::signal<void,etl::handle<Instance> >&
App::signal_instance_selected() { return signal_instance_selected_; }

static sigc::signal<void,etl::handle<Instance> > signal_instance_created_;
sigc::signal<void,etl::handle<Instance> >&
App::signal_instance_created() { return signal_instance_created_; }

static sigc::signal<void,etl::handle<Instance> > signal_instance_deleted_;
sigc::signal<void,etl::handle<Instance> >&
App::signal_instance_deleted() { return signal_instance_deleted_; }

/* === G L O B A L S ======================================================= */

static std::list<synfig::filesystem::Path> recent_files;
const std::list<synfig::filesystem::Path>& App::get_recent_files() { return recent_files; }

int	 App::Busy::count;
bool App::shutdown_in_progress;

int        App::jack_locks_ = 0;
synfig::Distance::System  App::distance_system;

std::list<etl::handle<Instance> > App::instance_list;

static etl::handle<synfigapp::UIInterface>           ui_interface_;
const  etl::handle<synfigapp::UIInterface>& App::get_ui_interface() { return ui_interface_; }

etl::handle<Instance>   App::selected_instance;
CanvasView::Handle      App::selected_canvas_view;

studio::About              *studio::App::about          = nullptr;
studio::AutoRecover        *studio::App::auto_recover   = nullptr;
studio::DeviceTracker      *studio::App::device_tracker = nullptr;
studio::MainWindow         *studio::App::main_window    = nullptr;

studio::Dialog_Color       *studio::App::dialog_color;
studio::Dialog_Gradient    *studio::App::dialog_gradient;
studio::Dialog_Input       *studio::App::dialog_input;
//studio::Dialog_Palette     *studio::App::dialog_palette;
studio::Dialog_Setup       *studio::App::dialog_setup = nullptr;
studio::Dialog_ToolOptions *studio::App::dialog_tool_options;
studio::VectorizerSettings *studio::App::vectorizerpopup;

       studio::DockManager   *App::dock_manager = nullptr;
static studio::Dock_Canvases      *dock_canvases;
static studio::Dock_Children      *dock_children;
static studio::Dock_Curves        *dock_curves;
static studio::Dock_History       *dock_history;
static studio::Dock_Info          *dock_info;
               Dock_Info     *App::dock_info_ = nullptr;
static studio::Dock_Keyframes     *dock_keyframes;
static studio::Dock_Layers        *dock_layers;
static studio::Dock_LayerGroups   *dock_layer_groups;
static studio::Dock_MetaData      *dock_meta_data;
static studio::Dock_Params        *dock_params;
static studio::Dock_Navigator     *dock_navigator;
static studio::Dock_SoundWave     *dock_soundwave;
static studio::Dock_Timetrack_Old *dock_timetrack_old;
static studio::Dock_Timetrack2    *dock_timetrack;
       studio::Dock_Toolbox  *App::dock_toolbox = nullptr;

static std::list< etl::handle< studio::Module > > module_list_;

bool   studio::App::restrict_radius_ducks        = true;
bool   studio::App::resize_imported_images       = false;
bool   studio::App::animation_thumbnail_preview  = true;
bool   studio::App::enable_experimental_features = false;
bool   studio::App::use_dark_theme               = false;
String studio::App::icon_theme_name              = "";
bool   studio::App::show_file_toolbar            = true;
String studio::App::custom_filename_prefix       (DEFAULT_FILENAME_PREFIX);
int    studio::App::preferred_x_size             = 480;
int    studio::App::preferred_y_size             = 270;
String studio::App::predefined_size              (DEFAULT_PREDEFINED_SIZE);
String studio::App::predefined_fps               (DEFAULT_PREDEFINED_FPS);
float  studio::App::preferred_fps                = 24.0;
PluginManager studio::App::plugin_manager;
std::set< filesystem::Path > studio::App::brushes_path;
String studio::App::image_editor_path;

String studio::App::sequence_separator(".");
int    studio::App::number_of_threads = std::thread::hardware_concurrency();
String studio::App::navigator_renderer;
String studio::App::workarea_renderer;

String        studio::App::default_background_layer_type  = "none";
synfig::Color studio::App::default_background_layer_color =
	synfig::Color(1.000000, 1.000000, 1.000000, 1.000000);  //White
String        studio::App::default_background_layer_image = "undefined";
synfig::Color studio::App::preview_background_color =
	synfig::Color(0.742187, 0.742187, 0.742187, 1.000000);  //X11 Gray

Glib::RefPtr<Gio::Menu> studio::App::menu_recent_files;
Glib::RefPtr<Gio::Menu> studio::App::menu_plugins;
Glib::RefPtr<Gio::Menu> studio::App::menu_layer;
Glib::RefPtr<Gio::Menu> studio::App::menu_layers;
Glib::RefPtr<Gio::Menu> studio::App::menu_selected_layers;
Glib::RefPtr<Gio::Menu> studio::App::menu_special_layers;
Glib::RefPtr<Gio::Menu> studio::App::menu_tools;
Glib::RefPtr<Gio::Menu> studio::App::menu_window_custom_workspaces;
Glib::RefPtr<Gio::Menu> studio::App::menu_window_docks;
Glib::RefPtr<Gio::Menu> studio::App::menu_window_canvas;
Glib::RefPtr<Gio::Menu> studio::App::menu_keyframe;

bool   studio::App::enable_mainwin_menubar = true;
bool   studio::App::enable_mainwin_toolbar = true;
String studio::App::ui_language ("os_LANG");
long   studio::App::ui_handle_tooltip_flag(Duck::STRUCT_DEFAULT);

static int max_recent_files_=25;
int    studio::App::get_max_recent_files()      { return max_recent_files_; }
void   studio::App::set_max_recent_files(int x) {        max_recent_files_ = x; }

static synfig::String app_base_path_;

SoundProcessor *App::sound_render_done = nullptr;
bool App::use_render_done_sound = true;

static StateManager* state_manager;

static ActionManager* action_manager = nullptr;

static bool
really_delete_widget(Gtk::Widget *widget)
{
	delete widget;
	return false;
}

// nasty workaround - when we've finished with a popup menu, we want to delete it
// attaching to the signal_hide() signal gets us here before the action on the menu has run,
// so schedule the real delete to happen in 50ms, giving the action a chance to run
void
studio::delete_widget(Gtk::Widget *widget)
{
	Glib::signal_timeout().connect(sigc::bind(sigc::ptr_fun(&really_delete_widget), widget), 50);
}

class GlobalUIInterface : public synfigapp::UIInterface
{
public:

	virtual Response confirmation(
			const std::string &message,
			const std::string &details,
			const std::string &confirm,
			const std::string &cancel,
			Response dflt
	)
	{
		Gtk::MessageDialog dialog(
			message,
			false,
			Gtk::MESSAGE_WARNING,
			Gtk::BUTTONS_NONE,
			true
		);

		if (! details.empty())
			dialog.set_secondary_text(details);

		dialog.add_button(cancel,  RESPONSE_CANCEL);
		dialog.add_button(confirm, RESPONSE_OK);
		dialog.set_default_response(dflt);

		dialog.show_all();
		int response = dialog.run();
		if (response != RESPONSE_OK)
			return RESPONSE_CANCEL;
		return RESPONSE_OK;
	}


	virtual Response yes_no_cancel(
				const std::string &message,
				const std::string &details,
				const std::string &button1,
				const std::string &button2,
				const std::string &button3,
				bool hasDestructiveAction,
				Response dflt=RESPONSE_YES
	)
	{
		Gtk::MessageDialog dialog(
			message,
			false,
			Gtk::MESSAGE_QUESTION,
			Gtk::BUTTONS_NONE,
			true
		);

		dialog.set_secondary_text(details);
		dialog.add_button(button1, RESPONSE_NO);
		dialog.add_button(button2, RESPONSE_CANCEL);
		dialog.add_button(button3, RESPONSE_YES);

		dialog.set_default_response(dflt);
		dialog.show();
		int response = dialog.run();
		if (response != RESPONSE_YES && response != RESPONSE_NO)
			return RESPONSE_CANCEL;
		return Response(response);
	}


	virtual bool
	task(const std::string &task)
	{
		std::cerr<<task.c_str()<<std::endl;
		App::process_all_events();
		return true;
	}

	virtual bool
	error(const std::string &err)
	{
		Gtk::MessageDialog dialog(err, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		dialog.set_transient_for(*App::main_window);
		dialog.show();
		dialog.run();
		return true;
	}

	virtual bool
	warning(const std::string &err)
	{
		std::cerr<<"warning: "<<err.c_str()<<std::endl;
		App::process_all_events();
		return true;
	}

	virtual bool
	amount_complete(int /*current*/, int /*total*/)
	{
		App::process_all_events();
		return true;
	}
};

class SetsModel : public Gtk::TreeModel::ColumnRecord
{
	public:
		SetsModel() {
			add(entry_set_name);
		}

		Gtk::TreeModelColumn<Glib::ustring> entry_set_name;
};

/* === P R O C E D U R E S ================================================= */

class App::Preferences : public synfigapp::Settings
{
public:
	virtual bool get_raw_value(const synfig::String& key, synfig::String& value)const override
	{
		try
		{
			synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
			if(key=="time_format")
			{
				value=strprintf("%i",App::get_time_format());
				return true;
			}
			if(key=="file_history.size")
			{
				value=strprintf("%i",App::get_max_recent_files());
				return true;
			}
			if(key=="distance_system")
			{
				value=strprintf("%s",Distance::system_name(App::distance_system).c_str());
				return true;
			}
			if(key=="autosave_backup")
			{
				value=strprintf("%i",App::auto_recover->get_enabled());
				return true;
			}
			if(key=="autosave_backup_interval")
			{
				value=strprintf("%i",App::auto_recover->get_timeout_ms());
				return true;
			}
			if(key=="restrict_radius_ducks")
			{
				value=strprintf("%i",(int)App::restrict_radius_ducks);
				return true;
			}
			if(key=="resize_imported_images")
			{
				value=strprintf("%i",(int)App::resize_imported_images);
				return true;
			}
			if(key=="animation_thumbnail_preview")
			{
				value=strprintf("%i",(int)App::animation_thumbnail_preview);
				return true;
			}
			if(key=="enable_experimental_features")
			{
				value=strprintf("%i",(int)App::enable_experimental_features);
				return true;
			}
			if(key=="use_dark_theme")
			{
				value=strprintf("%i",(int)App::use_dark_theme);
				return true;
			}
			if(key=="icon_theme_name")
			{
				value=strprintf("%s", App::get_raw_icon_theme_name().c_str());
				return true;
			}
			if(key=="show_file_toolbar")
			{
				value=strprintf("%i",(int)App::show_file_toolbar);
				return true;
			}
			//! "Keep brushes_path" preferences entry for backward compatibility (15/12 - v1.0.3)
			//! Now brush path(s) are hold by input preferences : brush.path_count & brush.path_%d
			if(key=="brushes_path")
			{
				value="";
				if (!App::brushes_path.empty())
					value = App::brushes_path.begin()->u8string();
				return true;
			}
			if(key=="custom_filename_prefix")
			{
				value=App::custom_filename_prefix;
				return true;
			}
			if(key=="preferred_x_size")
			{
				value=strprintf("%i",App::preferred_x_size);
				return true;
			}
			if(key=="preferred_y_size")
			{
				value=strprintf("%i",App::preferred_y_size);
				return true;
			}
			if(key=="predefined_size")
			{
				value=strprintf("%s",App::predefined_size.c_str());
				return true;
			}
			if(key=="preferred_fps")
			{
				value=strprintf("%f",App::preferred_fps);
				return true;
			}
			if(key=="predefined_fps")
			{
				value=strprintf("%s",App::predefined_fps.c_str());
				return true;
			}
			if(key=="sequence_separator")
			{
				value=App::sequence_separator;
				return true;
			}
			if(key=="number_of_threads")
			{
				value=strprintf("%i",App::number_of_threads);
				return true;
			}
			if(key=="navigator_renderer")
			{
				value=App::navigator_renderer;
				return true;
			}
			if(key=="workarea_renderer")
			{
				value=App::workarea_renderer;
				return true;
			}
			if (key == "default_background_layer_type")
			{
				value = strprintf("%s", App::default_background_layer_type.c_str());
				return true;
			}
			if (key == "default_background_layer_color")
			{
				value = strprintf("%f %f %f %f",
					App::default_background_layer_color.get_r(),
					App::default_background_layer_color.get_g(),
					App::default_background_layer_color.get_b(),
					App::default_background_layer_color.get_a()
					);
				return true;
			}
			if (key == "default_background_layer_image")
			{
				value = strprintf("%s", App::default_background_layer_image.c_str());
				return true;
			}
			if (key == "preview_background_color")
			{
				value = strprintf("%f %f %f %f",
					App::preview_background_color.get_r(),
					App::preview_background_color.get_g(),
					App::preview_background_color.get_b(),
					App::preview_background_color.get_a()
					);
				return true;
			}
			if(key=="use_render_done_sound")
			{
				value=strprintf("%i",(int)App::use_render_done_sound);
				return true;
			}
			if(key=="enable_mainwin_menubar")
			{
				value=strprintf("%i", (int)App::enable_mainwin_menubar);
				return true;
			}
			if(key=="ui_handle_tooltip_flag")
			{
				value=strprintf("%ld", (long)App::ui_handle_tooltip_flag);
				return true;
			}
			if(key=="image_editor_path")
			{
				value=App::image_editor_path;
				return true;
			}
		}
		catch(...)
		{
			synfig::warning("Preferences: Caught exception when attempting to get value.");
		}
		return synfigapp::Settings::get_raw_value(key,value);
	}

	virtual bool set_value(const synfig::String& key,const synfig::String& value) override
	{
		try
		{
			synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
			if(key=="time_format")
			{
				int i(atoi(value.c_str()));
				App::set_time_format(static_cast<synfig::Time::Format>(i));
				return true;
			}
			if(key=="autosave_backup")
			{
				int i(atoi(value.c_str()));
				App::auto_recover->set_enabled(i);
				return true;
			}
			if(key=="autosave_backup_interval")
			{
				int i(atoi(value.c_str()));
				App::auto_recover->set_timeout_ms(i);
				return true;
			}
			if(key=="file_history.size")
			{
				int i(atoi(value.c_str()));
				App::set_max_recent_files(i);
				return true;
			}
			if(key=="distance_system")
			{
				App::distance_system=Distance::ident_system(value);
				return true;
			}
			if(key=="restrict_radius_ducks")
			{
				int i(atoi(value.c_str()));
				App::restrict_radius_ducks=i;
				return true;
			}
			if(key=="resize_imported_images")
			{
				int i(atoi(value.c_str()));
				App::resize_imported_images=i;
				return true;
			}
			if(key=="animation_thumbnail_preview")
			{
				int i(atoi(value.c_str()));
				App::animation_thumbnail_preview=i;
				return true;
			}
			if(key=="enable_experimental_features")
			{
				int i(atoi(value.c_str()));
				App::enable_experimental_features=i;
				return true;
			}
			if(key=="use_dark_theme")
			{
				int i(atoi(value.c_str()));
				App::use_dark_theme=i;
				return true;
			}
			if(key=="icon_theme_name")
			{
				App::icon_theme_name = value;
				return true;
			}
			if(key=="show_file_toolbar")
			{
				int i(atoi(value.c_str()));
				App::show_file_toolbar=i;
				return true;
			}
			//! "Keep brushes_path" preferences entry for backward compatibility (15/12 - v1.0.3)
			//! Now brush path(s) are hold by input preferences : brush.path_count & brush.path_%d
			if(key=="brushes_path")
			{
				App::brushes_path.insert(value);
				return true;
			}
			if(key=="custom_filename_prefix")
			{
				App::custom_filename_prefix=value;
				return true;
			}
			if(key=="preferred_x_size")
			{
				int i(atoi(value.c_str()));
				App::preferred_x_size=i;
				return true;
			}
			if(key=="preferred_y_size")
			{
				int i(atoi(value.c_str()));
				App::preferred_y_size=i;
				return true;
			}
			if(key=="predefined_size")
			{
				App::predefined_size=value;
				return true;
			}
			if(key=="preferred_fps")
			{
				float i(atof(value.c_str()));
				App::preferred_fps=i;
				return true;
			}
			if(key=="predefined_fps")
			{
				App::predefined_fps=value;
				return true;
			}
			if(key=="sequence_separator")
			{
				App::sequence_separator=value;
				return true;
			}
			if(key=="number_of_threads")
			{
				App::number_of_threads=atoi(value.c_str());
				return true;
			}
			if(key=="navigator_renderer")
			{
				App::navigator_renderer=value;
				return true;
			}
			if(key=="workarea_renderer")
			{
				App::workarea_renderer=value;
				return true;
			}
			if (key == "default_background_layer_type")
			{
				App::default_background_layer_type = value;
				return true;
			}
			if (key == "default_background_layer_color")
			{
				float r,g,b,a;
				strscanf(value,"%f %f %f %f", &r,&g,&b,&a);
				App::default_background_layer_color = synfig::Color(r,g,b,a);
				return true;
			}
			if (key == "default_background_layer_image")
			{
				App::default_background_layer_image = value;
				return true;
			}
			if (key == "preview_background_color")
			{
				float r,g,b,a;
				strscanf(value,"%f %f %f %f", &r,&g,&b,&a);
				App::preview_background_color = synfig::Color(r,g,b,a);
				return true;
			}
			if(key=="use_render_done_sound")
			{
				int i(atoi(value.c_str()));
				App::use_render_done_sound=i;
				return true;
			}
			if(key=="enable_mainwin_menubar")
			{
				int i(atoi(value.c_str()));
				App::enable_mainwin_menubar = i;
				return true;
			}
			if(key=="ui_handle_tooltip_flag")
			{
				long l(atol(value.c_str()));
				App::ui_handle_tooltip_flag = l;
				return true;
			}
			if(key=="image_editor_path")
			{
				App::image_editor_path=value;
				return true;
			}
			if(key=="clear_redo_stack_on_new_action")
			{
				for (auto& instance : App::instance_list)
					instance->set_clear_redo_stack_on_new_action(value != "0");
			}
		}
		catch(...)
		{
			synfig::warning("Preferences: Caught exception when attempting to set value.");
		}
		return synfigapp::Settings::set_value(key,value);
	}

	virtual KeyList get_key_list()const override
	{
		KeyList ret(synfigapp::Settings::get_key_list());
		ret.push_back("time_format");
		ret.push_back("distance_system");
		ret.push_back("file_history.size");
		ret.push_back("autosave_backup");
		ret.push_back("autosave_backup_interval");
		ret.push_back("restrict_radius_ducks");
		ret.push_back("resize_imported_images");
		ret.push_back("animation_thumbnail_preview");
		ret.push_back("enable_experimental_features");
		ret.push_back("use_dark_theme");
		ret.push_back("icon_theme_name");
		ret.push_back("show_file_toolbar");
		ret.push_back("brushes_path");
		ret.push_back("custom_filename_prefix");
		ret.push_back("ui_language");
		ret.push_back("preferred_x_size");
		ret.push_back("preferred_y_size");
		ret.push_back("predefined_size");
		ret.push_back("preferred_fps");
		ret.push_back("predefined_fps");
		ret.push_back("sequence_separator");
		ret.push_back("number_of_threads");
		ret.push_back("navigator_renderer");
		ret.push_back("workarea_renderer");
		ret.push_back("default_background_layer_type");
		ret.push_back("default_background_layer_color");
		ret.push_back("default_background_layer_image");
		ret.push_back("preview_background_color");
		ret.push_back("use_render_done_sound");
		ret.push_back("enable_mainwin_menubar");
		ret.push_back("ui_handle_tooltip_flag");
		ret.push_back("image_editor_path");


		return ret;
	}

	using synfigapp::Settings::set_value;
};

App::Preferences App::_preferences;

static const ActionManager::EntryList app_action_db =
{
	{"app.new",            N_("New"),            {"<Primary>n"}, "action_doc_new_icon", N_("Create a new document")},
	{"app.open",           N_("Open"),           {"<Primary>o"}, "action_doc_open_icon", N_("Open an existing document")},
	{"app.quit",           N_("Quit"),           {"<Primary>q"}, "application-exit", N_("Quit application")},
	{"app.preferences",    N_("Preferences..."), {},             "application-preferences"},
	{"app.help",           N_("Help"),           {"F1"},         "help-contents"},
#if GTK_CHECK_VERSION(3, 20, 0)
	{"app.help-shortcuts", N_("Keyboard Shortcuts"),         {}, ""},
#endif
	{"app.help-tutorials", N_("Tutorials"),                  {}, ""},
	{"app.help-reference", N_("Reference"),                  {}, ""},
	{"app.help-faq",       N_("Frequently Asked Questions"), {}, "help-faq"},
	{"app.help-support",   N_("Get Support"),                {}, ""},
	{"app.about",          N_("About Synfig Studio"),        {}, "help-about"},
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static void
init_app_actions()
{
	Glib::RefPtr<Gio::ActionMap> action_map = App::instance();
	action_map->add_action("new", sigc::hide_return(sigc::ptr_fun(&App::new_instance)));
	action_map->add_action("open", sigc::hide_return(sigc::bind(sigc::ptr_fun(&App::dialog_open), synfig::filesystem::Path{})));
	action_map->add_action("quit", sigc::hide_return(sigc::ptr_fun(&App::quit)));
	action_map->add_action("preferences", sigc::ptr_fun(&App::show_setup));
	action_map->add_action("help", sigc::ptr_fun(App::dialog_help));
#if GTK_CHECK_VERSION(3, 20, 0)
	action_map->add_action("help-shortcuts", sigc::ptr_fun(App::window_shortcuts));
#endif
	action_map->add_action("help-tutorials", sigc::bind(sigc::ptr_fun(&App::open_uri), _("https://synfig.readthedocs.io/en/latest/tutorials.html")));
	action_map->add_action("help-reference", sigc::bind(sigc::ptr_fun(&App::open_uri), _("https://wiki.synfig.org/Category:Reference")));
	action_map->add_action("help-faq", sigc::bind(sigc::ptr_fun(&App::open_uri), _("https://wiki.synfig.org/FAQ")));
	action_map->add_action("help-support", sigc::bind(sigc::ptr_fun(&App::open_uri), _("https://forums.synfig.org/")));
	action_map->add_action("about", sigc::ptr_fun(App::dialog_about));

	auto open_recent_slot = [](const Glib::VariantBase& v) {
		Glib::Variant<Glib::ustring> filename = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v);
		App::open_recent(synfig::filesystem::Path(filename.get()));
	};
	action_map->add_action_with_parameter("open-recent-file", Glib::VARIANT_TYPE_STRING, open_recent_slot);

	for (const auto& entry : app_action_db)
		App::get_action_manager()->add(entry);
}

Glib::RefPtr<App> App::instance() {
	static Glib::RefPtr<studio::App> app_reference = Glib::RefPtr<App>(new App());
	return app_reference;
}

/* === M E T H O D S ======================================================= */
App::App()
	: Gtk::Application("org.synfig.SynfigStudio", Gio::APPLICATION_HANDLES_OPEN)
{
	add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "console", 'c', N_("Opens a console that shows some Synfig Studio output"));
	signal_handle_local_options().connect(sigc::mem_fun(*this, &App::on_handle_local_options));
	signal_shutdown().connect(sigc::mem_fun(*this, &App::on_shutdown));
}

int App::on_handle_local_options(const Glib::RefPtr<Glib::VariantDict> &options)
{
	// To continue, return -1 to let the default option processing continue.
	// https://developer-old.gnome.org/glibmm/unstable/classGio_1_1Application.html#a46b21ab9629b123b7524fe906290d32b
	if (!options)
		return -1;
	if (options->contains("console")) {
#ifdef _WIN32
		redirectIOToConsole();
#else
		warning(_("Option --console is valid on MS Windows only"));
#endif
	}
	return -1;
}

void App::on_activate()
{
	if (!getenv("SYNFIG_DISABLE_AUTOMATIC_DOCUMENT_CREATION") && !get_selected_instance())
		new_instance();

	if (get_windows().size() == 0) {
		add_window(*main_window);
	} else {
		main_window->present();
	}
}

void App::on_open(const type_vec_files &files, const Glib::ustring &hint)
{
	if (get_windows().size() == 0)
		add_window(*main_window);

	OneMoment one_moment;
	for (const auto& file : files) {
		open(file->get_path());
	}
}

void App::init(const synfig::String& rootpath)
{
	app_base_path_=rootpath;

	// Set ui language
	load_language_settings();
	if (ui_language != "os_LANG")
	{
		Glib::setenv ("LANGUAGE",  App::ui_language.c_str(), 1);
	}

	// paths
	filesystem::Path path_to_icons = ResourceHelper::get_icon_path();

	String path_to_plugins = ResourceHelper::get_plugin_path();

	filesystem::Path path_to_user_plugins = synfigapp::Main::get_user_app_directory() / filesystem::Path("plugins");

	ui_interface_=new GlobalUIInterface();

	// don't call thread_init() if threads are already initialized
	// on some machines bonobo_init() initialized threads before we get here
	//if (!Glib::thread_supported())
	//	Glib::thread_init();

	distance_system=Distance::SYSTEM_PIXELS;

#ifdef _WIN32
	// Do not show "No disc in drive" errors
	// - https://github.com/synfig/synfig/issues/489
	// - https://github.com/synfig/synfig/issues/724
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif

	if (FileSystemNative::instance()->directory_create(synfigapp::Main::get_user_app_directory().u8string())) {
		synfig::info("Created directory \"%s\"", synfigapp::Main::get_user_app_directory().u8_str());
	} else {
		synfig::error("UNABLE TO CREATE \"%s\"", synfigapp::Main::get_user_app_directory().u8_str());
	}


	if(!SYNFIG_CHECK_VERSION())
	{
		std::cerr << "FATAL: Synfig Version Mismatch" << std::endl;
		dialog_message_1b(
			"ERROR",
			_("Synfig version mismatched!"),
			_("This copy of Synfig Studio was compiled against a "
			"different version of libsynfig than what is currently "
			"installed. Synfig Studio will now abort. Try downloading "
			"the latest version from the Synfig website at "
			"https://www.synfig.org/download-development/"),
			_("Close"));

		throw 40;
	}
	Glib::set_application_name(_("Synfig Studio"));

	Splash splash_screen;
	splash_screen.show();

	shutdown_in_progress=false;
	SuperCallback synfig_init_cb(splash_screen.get_callback(),0,9000,10000);
	SuperCallback studio_init_cb(splash_screen.get_callback(),9000,10000,10000);

	// Initialize the Synfig library
	try { synfigapp_main = std::make_shared<synfigapp::Main>(rootpath,&synfig_init_cb); }
	catch(std::runtime_error &x)
	{
		get_ui_interface()->error(strprintf("%s\n\n%s", _("Failed to initialize synfig!"), x.what()));
		throw;
	}
	catch(...)
	{
		get_ui_interface()->error(_("Failed to initialize synfig!"));
		throw;
	}


	// add the preferences to the settings
	synfigapp::Main::settings().add_domain(&_preferences,"pref");

	// icons
	init_icon_themes();
	init_icons(path_to_icons);

	auto builder = Gtk::Builder::create();
	try
	{
		builder->add_from_file(ResourceHelper::get_ui_path("studio_menubar.xml"));
	}
	catch (const Glib::Error& ex)
	{
		std::cerr << "Building menus failed: " << ex.what();
	}

	auto object = builder->get_object("studio_menubar");
	auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
	if (!gmenu) {
		g_warning("GMenu not found");
	} else {
		set_menubar(gmenu);
		menu_recent_files = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-recent-files"));
		menu_plugins = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-plugins"));
		menu_layer = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-layer"));
		menu_layers = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-layer-new"));
		menu_selected_layers = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("selected-layer-actions"));
		menu_special_layers = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("special-layer-actions"));
		menu_tools = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-tools"));
		menu_window_custom_workspaces = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-window-custom-workspaces"));
		menu_window_docks = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-window-docks"));
		menu_window_canvas = Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("menu-window-canvas"));
		menu_keyframe = Gio::Menu::create();
	}

	try
	{
		// Try to load settings early to get access to some important
		// values, like "enable_experimental_features".
		studio_init_cb.task(_("Loading Basic Settings..."));

		load_settings("pref.use_dark_theme");

		App::apply_gtk_settings();

		load_settings("pref.show_file_toolbar");

		// Set experimental features
		load_settings("pref.enable_experimental_features");

		// Set main window menu and toolbar
		load_settings("pref.enable_mainwin_menubar");

		load_settings("pref.default_background_layer_type");
		load_settings("pref.default_background_layer_color");
		load_settings("pref.default_background_layer_image");
		load_settings("pref.preview_background_color");
		load_settings("pref.image_editor_path");

		studio_init_cb.task(_("Loading Plugins..."));
		plugin_manager.load_dir(path_to_plugins);
		plugin_manager.load_dir(path_to_user_plugins.u8string());

		studio_init_cb.task(_("Init UI Manager..."));
		action_manager = new ActionManager(App::instance());

		init_app_actions();

		studio_init_cb.task(_("Init Dock Manager..."));
		dock_manager=new studio::DockManager();

		studio_init_cb.task(_("Init State Manager..."));
		state_manager=new StateManager();

		studio_init_cb.task(_("Init Main Window..."));
		main_window=new studio::MainWindow(App::instance());

		studio_init_cb.task(_("Init Toolbox..."));
		dock_toolbox=new studio::Dock_Toolbox();
		dock_manager->register_dockable(*dock_toolbox);

		studio_init_cb.task(_("Init About Dialog..."));
		about=new studio::About();

		studio_init_cb.task(_("Init Tool Options..."));
		dialog_tool_options=new studio::Dialog_ToolOptions();
		dock_manager->register_dockable(*dialog_tool_options);

		studio_init_cb.task(_("Init History..."));
		dock_history=new studio::Dock_History();
		dock_manager->register_dockable(*dock_history);

		studio_init_cb.task(_("Init Canvases..."));
		dock_canvases=new studio::Dock_Canvases();
		dock_manager->register_dockable(*dock_canvases);

		studio_init_cb.task(_("Init Keyframes..."));
		dock_keyframes=new studio::Dock_Keyframes();
		dock_manager->register_dockable(*dock_keyframes);

//! Must be done before Dock_Timetrack and Dock_Curves :
//! both are connected to a studio::LayerTree::param_tree_view_'s signal, and
//! studio::LayerTree is created from Dock_Layers::init_canvas_view_vfunc
		studio_init_cb.task(_("Init Layers..."));
		dock_layers=new studio::Dock_Layers();
		dock_manager->register_dockable(*dock_layers);

		studio_init_cb.task(_("Init Parameters..."));
		dock_params=new studio::Dock_Params();
		dock_manager->register_dockable(*dock_params);

		studio_init_cb.task(_("Init MetaData..."));
		dock_meta_data=new studio::Dock_MetaData();
		dock_manager->register_dockable(*dock_meta_data);

		studio_init_cb.task(_("Init Library..."));
		dock_children=new studio::Dock_Children();
		dock_manager->register_dockable(*dock_children);

		studio_init_cb.task(_("Init Info..."));
		dock_info = new studio::Dock_Info();
		dock_manager->register_dockable(*dock_info);

		studio_init_cb.task(_("Init Navigator..."));
		dock_navigator = new studio::Dock_Navigator();
		dock_manager->register_dockable(*dock_navigator);

		studio_init_cb.task(_("Init SoundWave..."));
		dock_soundwave = new studio::Dock_SoundWave();
		dock_manager->register_dockable(*dock_soundwave);

		studio_init_cb.task(_("Init Timetrack (old)..."));
		dock_timetrack_old = new studio::Dock_Timetrack_Old();
		dock_manager->register_dockable(*dock_timetrack_old);

		studio_init_cb.task(_("Init Timetrack..."));
		dock_timetrack = new studio::Dock_Timetrack2();
		dock_manager->register_dockable(*dock_timetrack);

		studio_init_cb.task(_("Init Curve Editor..."));
		dock_curves = new studio::Dock_Curves();
		dock_manager->register_dockable(*dock_curves);

		studio_init_cb.task(_("Init Layer Sets..."));
		dock_layer_groups = new studio::Dock_LayerGroups();
		dock_manager->register_dockable(*dock_layer_groups);


		studio_init_cb.task(_("Init Color Dialog..."));
		dialog_color=new studio::Dialog_Color();

		studio_init_cb.task(_("Init Gradient Dialog..."));
		dialog_gradient=new studio::Dialog_Gradient();

		studio_init_cb.task(_("Init DeviceTracker..."));
		device_tracker=new studio::DeviceTracker();

		//Init Tools...was here

		studio_init_cb.task(_("Init ModPalette..."));
		module_list_.push_back(new ModPalette()); module_list_.back()->start();

		studio_init_cb.task(_("Init Setup Dialog..."));
		dialog_setup=new studio::Dialog_Setup(*App::main_window);

		studio_init_cb.task(_("Init Input Dialog..."));
		dialog_input=new studio::Dialog_Input(*App::main_window);
		dialog_input->signal_apply().connect( sigc::mem_fun( *device_tracker, &DeviceTracker::save_preferences) );

		studio_init_cb.task(_("Loading Custom Workspace List..."));
		MainWindow::load_custom_workspaces();

		studio_init_cb.task(_("Init auto recovery..."));
		auto_recover=new AutoRecover();

		studio_init_cb.amount_complete(9250,10000);
		studio_init_cb.task(_("Loading Settings..."));
		if (!load_settings())
			MainWindow::set_workspace_default();
		if (!load_settings("workspace.layout"))
			MainWindow::set_workspace_default();
		load_recent_files();

		studio_init_cb.task(_("Init Tools..."));
		/* editing tools */
		state_manager->add_state(&state_normal);
		state_manager->add_state(&state_smooth_move);
		state_manager->add_state(&state_scale);
		state_manager->add_state(&state_rotate);
		state_manager->add_state(&state_mirror);

		/* geometry */
		state_manager->add_state(&state_circle);
		state_manager->add_state(&state_rectangle);
		state_manager->add_state(&state_star);
		if(!getenv("SYNFIG_DISABLE_POLYGON")) state_manager->add_state(&state_polygon); // Enabled - for working without ducks
		state_manager->add_state(&state_gradient);

		/* bline tools */
		state_manager->add_state(&state_bline);
		if(!getenv("SYNFIG_DISABLE_DRAW"   )) state_manager->add_state(&state_draw ); // Enabled for now.  Let's see whether they're good enough yet.
		state_manager->add_state(&state_lasso);
		if(!getenv("SYNFIG_DISABLE_WIDTH"  )) state_manager->add_state(&state_width); // Enabled since 0.61.09
		state_manager->add_state(&state_fill);
		state_manager->add_state(&state_eyedrop);

		/* skeleton tool*/
		state_manager->add_state(&state_bone);

		/* other */
		state_manager->add_state(&state_text);
		if(!getenv("SYNFIG_DISABLE_SKETCH" )) state_manager->add_state(&state_sketch);
		if(!getenv("SYNFIG_DISABLE_BRUSH"  ) && App::enable_experimental_features) state_manager->add_state(&state_brush);
		state_manager->add_state(&state_zoom);

		// Load the user shortcuts/accel keys
		load_accel_map();

		device_tracker->load_preferences();
		// If the default bline width is modified before focus a canvas
		// window, the Distance widget doesn't understand the given value
		// and produces this message:
		// Distance::ident_system(): Unknown distance system ".00pt"
		// setting the default bline width to 1 unit.
		// This line fixes that.
		synfigapp::Main::set_bline_width(synfigapp::Main::get_selected_input_device()->get_bline_width());

		studio_init_cb.task(_("Checking auto-recover..."));

		studio_init_cb.amount_complete(9900,10000);

		if (!getenv("SYNFIG_DISABLE_AUTO_RECOVERY") && auto_recover->recovery_needed())
		{
			splash_screen.hide();
			if (get_ui_interface()->confirmation(
					_("Auto recovery file(s) found. Do you want to recover unsaved changes?"),
					_("Synfig Studio seems to have crashed before you could save all your files."),
					_("Recover"),
					_("Ignore")
				) == synfigapp::UIInterface::RESPONSE_OK)
			{
				int number_recovered;
				if(!auto_recover->recover(number_recovered))
					if (number_recovered)
						get_ui_interface()->error(_("Unable to fully recover from previous crash"));
					else
						get_ui_interface()->error(_("Unable to recover from previous crash"));
				else
					dialog_message_1b(
						"WARNING",
						_("It would be a good idea to review and save recovered files now."),
						_("Synfig Studio has attempted to recover from a previous crash. "
						"The files just recovered are NOT YET SAVED."),
						_("Thanks"));
			}
			else
			{
				auto_recover->clear_backups();
			}
			splash_screen.show();
		}

		studio_init_cb.task(_("Done."));
		studio_init_cb.amount_complete(10000,10000);

		// To avoid problems with some window managers and gtk >= 2.18
		// we should show dock dialogs after the settings load.
		// If dock dialogs are shown before the settings are loaded,
		// the windows manager can act over it.
		// See discussions here:
		// * https://synfig.org/forums/viewtopic.php?f=1&t=1131&st=0&sk=t&sd=a&start=30
		// * https://synfig.org/forums/viewtopic.php?f=15&t=1062
		dock_manager->show_all_dock_dialogs();

		main_window->present();

		splash_screen.hide();
	}
	catch(String &x)
	{
		get_ui_interface()->error(_("Unknown exception caught when constructing App.\nThis software may be unstable.") + String("\n\n") + x);
	}
	catch(const std::runtime_error& re)	{
		// specific handling for runtime_error
		get_ui_interface()->error(std::string("Runtime error: ") + re.what());
		std::cerr << "Runtime error: " << re.what() << std::endl;
	}
	catch(const std::exception& ex)	{
		// specific handling for all exceptions extending std::exception, except
		// std::runtime_error which is handled explicitly
		get_ui_interface()->error(std::string("Exception: ") + ex.what());
		std::cerr << "Error occurred: " << ex.what() << std::endl;
	}
	catch(...)
	{
		get_ui_interface()->error(_("Unknown exception caught when constructing App.\nThis software may be unstable."));
	}

	// Load sound effects
	sound_render_done = new SoundProcessor();
	sound_render_done->addSound(
		SoundProcessor::PlayOptions(),
		SoundProcessor::Sound(ResourceHelper::get_sound_path("renderdone.wav")));

	App::dock_info_ = dock_info;
}

StateManager* App::get_state_manager() { return state_manager; }

ActionManager*
App::get_action_manager()
{
	return action_manager;
}

void
App::on_shutdown()
{
	shutdown_in_progress=true;

	save_settings();
	save_accel_map();

	synfigapp::Main::settings().remove_domain("pref");

	selected_instance=nullptr;

	// Unload all of the modules
	for(;!module_list_.empty();module_list_.pop_back())
		module_list_.back()->stop();

	delete state_manager;

	delete action_manager;

	delete auto_recover;

	delete about;

	main_window->hide();

	delete dialog_setup;

	delete dialog_gradient;

	delete dialog_color;

	delete dialog_input;

	delete dock_manager;

	instance_list.clear();

	if (sound_render_done) delete sound_render_done;
	sound_render_done = nullptr;

	process_all_events();
	delete main_window;
}

synfig::filesystem::Path
App::get_config_file(const synfig::String& file)
{
	return synfigapp::Main::get_user_app_directory() / filesystem::Path(file);
}

void
App::add_recent_file(const etl::handle<Instance> instance)
{
	add_recent_file(filesystem::absolute(instance->get_file_name()), true);
}

void
App::add_recent_file(const synfig::filesystem::Path& file_name, bool emit_signal = true)
{
	filesystem::Path filename(file_name.cleanup());

	assert(!filename.empty());

	if(filename.empty())
		return;

	// Toss out any "hidden" files
	if (filename.filename().u8string()[0] == '.')
		return;

	// If we aren't an absolute path, turn ourselves into one
	filename = filesystem::absolute(filename);

	// Check to see if the file is already on the list.
	// If it is, then remove it from the list
	for (auto iter = recent_files.begin(); iter != recent_files.end(); ++iter) {
		if (*iter == filename) {
			recent_files.erase(iter);
			break;
		}
	}

	// Push the filename to the front of the list
	recent_files.push_front(filename);

	// Clean out the files at the end of the list.
	while(recent_files.size()>(unsigned)get_max_recent_files())
	{
		recent_files.pop_back();
	}

	if (emit_signal) {
		signal_recent_files_changed_();
	}

}

static Time::Format _App_time_format(Time::FORMAT_FRAMES);

bool App::jack_is_locked()
{
	return jack_locks_ > 0;
}

void App::jack_lock()
{
	++jack_locks_;
	if (jack_locks_ == 1)
	{
		// lock jack in instances
		for (const auto& instance : instance_list) {
			for (const auto& canvas_view : instance->canvas_view_list())
				canvas_view->jack_lock();
		}
	}
}

void App::jack_unlock()
{
	--jack_locks_;
	assert(jack_locks_ >= 0);
	if (jack_locks_ == 0)
	{
		// unlock jack in instances
		for (const auto& instance : instance_list) {
			for (const auto& canvas_view : instance->canvas_view_list())
				canvas_view->jack_unlock();
		}
	}
}


Time::Format
App::get_time_format()
{
	return _App_time_format;
}

void
App::set_time_format(synfig::Time::Format x)
{
	_App_time_format=x;
}


void
App::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		{
			filesystem::Path filename = get_config_file("language");

			std::ofstream file(filename.c_str());

			if(!file)
			{
				synfig::warning("Unable to save %s", filename.u8_str());
			} else {
				file<<App::ui_language.c_str()<<std::endl;
			}
		}
		do{
			filesystem::Path filename = get_config_file("recentfiles");

			std::ofstream file(filename.c_str());

			if(!file)
			{
				synfig::warning("Unable to save %s", filename.u8_str());
				break;
			}

			for (auto r_iter = recent_files.rbegin(); r_iter != recent_files.rend(); ++r_iter)
				file << r_iter->u8string() << std::endl;
		} while (false);
		filesystem::Path filename = get_config_file("settings-1.4");
		synfigapp::Main::settings().save_to_file(filename);

		MainWindow::save_custom_workspaces();
	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to save settings.");
	}
}

bool
App::load_settings(const synfig::String& key_filter)
{
	bool ret=false;
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		filesystem::Path filename = get_config_file("settings-1.4");
		ret=synfigapp::Main::settings().load_from_file(filename, key_filter);
	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load settings.");
	}
	return ret;
}

void
App::load_accel_map()
{
	UserShortcutList list;
	list.restore_to_defaults(*App::get_action_manager());
	list.load_from_file(get_config_file("shortcuts"), false);
	list.apply(App::instance(), *App::get_action_manager());
}

void
App::save_accel_map()
{
	// only save those shortcuts customized by user, i.e., without default values
	UserShortcutList list;
	for (const auto& entry : action_manager->get_entries()) {
		auto user_accels = App::instance()->get_accels_for_action(entry.name_);
		if (user_accels.empty() && !entry.accelerators_.empty()) {
			if (!entry.accelerators_[0].empty())
				list.shortcuts[entry.name_] = "";
		} else if (!user_accels.empty() && entry.accelerators_.empty()) {
			if (!user_accels[0].empty())
				list.shortcuts[entry.name_] = user_accels[0];
		} else {
			std::sort(user_accels.begin(), user_accels.end());
			auto default_accels = entry.accelerators_;
			std::sort(default_accels.begin(), default_accels.end());
			if (user_accels[0] != default_accels[0]) {
				list.shortcuts[entry.name_] = user_accels[0];
			}
		}
	}
	list.save_to_file(get_config_file("shortcuts"));
}

void
App::load_recent_files()
{
	try
	{
		filesystem::Path filename = get_config_file("recentfiles");
		std::ifstream file(filename.c_str());

		while(file)
		{
			std::string recent_file;
			getline(file,recent_file);
			if(!recent_file.empty() && FileSystemNative::instance()->is_file(recent_file))
				add_recent_file(recent_file, false);
		}
		signal_recent_files_changed()();
	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load recent file list.");
	}
}

void
App::load_language_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		{
			filesystem::Path filename = get_config_file("language");
			std::ifstream file(filename.c_str());

			while(file)
			{
				std::string language;
				getline(file,language);
				if(!language.empty())
					App::ui_language=language;
			}
		}

	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load language settings.");
	}
}

void
App::apply_gtk_settings()
{
	Glib::RefPtr<Gtk::Settings> gtk_settings = Gtk::Settings::get_default();

	const std::string theme_name = Glib::getenv("SYNFIG_GTK_THEME");
	if (!theme_name.empty())
		gtk_settings->property_gtk_theme_name() = theme_name;

	// dark theme
	gtk_settings->property_gtk_application_prefer_dark_theme() = App::use_dark_theme;

	// enable menu icons
	gtk_settings->property_gtk_menu_images() = true;

	auto provider = Gtk::CssProvider::create();
	auto screen   = Gdk::Screen::get_default();
	auto css_file = Gio::File::create_for_path(ResourceHelper::get_css_path("synfig.css"));

#ifdef __APPLE__
		if ( gtk_settings->property_gtk_theme_name() == "Adwaita" )
			css_file = Gio::File::create_for_path(ResourceHelper::get_css_path("synfig.mac.css"));
#endif

	try {
		provider->load_from_file(css_file);
	} catch (Glib::Error &e) {
		synfig::warning("%s", e.what().c_str());
	}

	Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void
App::init_icon_themes()
{
	// If environment is not set then read theme name from preferences
	if (Glib::getenv("SYNFIG_ICON_THEME").empty()) {
		load_settings("pref.icon_theme_name");
	}
	auto icon_theme = Gtk::IconTheme::get_default();
	icon_theme->prepend_search_path(ResourceHelper::get_themes_path());

	set_icon_theme(App::icon_theme_name);
}

std::string
App::get_icon_theme_name()
{
	// If not explicitly set at runtime, environment variable has priority
	if (!icon_theme_name.empty())
		return icon_theme_name;

	auto env_icon_theme = Glib::getenv("SYNFIG_ICON_THEME");
	if (!env_icon_theme.empty()) {
		return env_icon_theme;
	}

	return DEFAULT_ICON_THEME_NAME;
}

std::string App::get_raw_icon_theme_name()
{
	return icon_theme_name;
}

// Almost all GTK methods using icons from the default GTK theme.
// Unfortunately, we can't change the IconTheme name if we get it
// from the default screen with the `Gtk::IconTheme::get_default()`
// method.
// https://docs.gtk.org/gtk3/method.IconTheme.set_custom_theme.html
// > Sets the name of the icon theme that the GtkIconTheme object uses
// > overriding system configuration. This function cannot be called on
// > the icon theme objects returned from gtk_icon_theme_get_default()
// > and gtk_icon_theme_get_for_screen().
//
// Also, I didn't find a way to change the app IconTheme to a custom
// one. However, we can change the IconTheme for the default screen
// using the `Gtk::Settings` object.
void
App::set_icon_theme(const std::string &theme_name)
{
	icon_theme_name = theme_name;
	Gtk::Settings::get_default()->property_gtk_icon_theme_name() = get_icon_theme_name();
}

bool
App::shutdown_request(GdkEventAny*)
{
	quit();
	return true;
	//return !shutdown_in_progress;
}

void
App::quit()
{
	if (shutdown_in_progress) return;

	get_ui_interface()->task(_("Quit Request"));

	if(Busy::count)
	{
		dialog_message_1b(
			"ERROR",
			_("Tasks are currently running. Please cancel the current tasks and try again"),
			"details",
			_("Close"));
		return;
	}

	while(!instance_list.empty())
		if (!instance_list.front()->safe_close())
			return;
	process_all_events();

	App::instance()->remove_window(*main_window);

	get_ui_interface()->task(_("Quit Request sent"));
}

void
App::show_setup()
{
	dialog_setup->refresh();
	dialog_setup->show();
}

static std::unique_ptr<Gtk::FileChooserDialog>
create_dialog_open_file(const std::string& title, const filesystem::Path& filename, const filesystem::Path& prev_path, const std::vector<Glib::RefPtr<Gtk::FileFilter>>& filters)
{
	std::unique_ptr<Gtk::FileChooserDialog> dialog(new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_OPEN));

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path.u8string());
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Load"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

	for (const auto& filter : filters)
		dialog->add_filter(filter);

	filesystem::Path full_path = prev_path;
	if (!filename.empty())
		full_path /= filename;

	dialog->set_filename(filesystem::absolute(full_path).u8string());

	return dialog;
}

bool
App::dialog_open_file_ext(const std::string& title, std::vector<synfig::filesystem::Path>& filenames, const std::string& preference, bool allow_multiple_selection)
{
	// info("App::dialog_open_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());
	// TODO: Win32 native dialod not ready yet
#ifdef USE_WIN32_FILE_DIALOGS
	static TCHAR szFilter[] = TEXT (_("All Files (*.*)\0*.*\0\0")) ;

	GdkWindow *gdkWinPtr=toolbox->get_window()->gobj();
	HINSTANCE hInstance=static_cast<HINSTANCE>(GetModuleHandle(nullptr));
	HWND hWnd=static_cast<HWND>(GDK_WINDOW_HWND(gdkWinPtr));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = szFilter;
//	ofn.lpstrCustomFilter=nullptr;
//	ofn.nMaxCustFilter=0;
//	ofn.nFilterIndex=0;
//	ofn.lpstrFile=nullptr;
	ofn.nMaxFile=MAX_PATH;
//	ofn.lpstrFileTitle=nullptr;
//	ofn.lpstrInitialDir=nullptr;
//	ofn.lpstrTitle=nullptr;
	ofn.Flags=OFN_HIDEREADONLY;
//	ofn.nFileOffset=0;
//	ofn.nFileExtension=0;
	ofn.lpstrDefExt=TEXT("sif");
//	ofn.lCustData = 0l;
	ofn.lpfnHook=nullptr;
//	ofn.lpTemplateName=nullptr;

	CHAR szFilename[MAX_PATH];
	CHAR szTitle[500];
	strcpy(szFilename,filename.c_str());
	strcpy(szTitle,title.c_str());

	ofn.lpstrFile=szFilename;
	ofn.lpstrFileTitle=szTitle;

	if(GetOpenFileName(&ofn))
	{
		filename=szFilename;
		return true;
	}
	return false;

#else   // not USE_WIN32_FILE_DIALOGS
	filesystem::Path prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// 0 All supported files
	// 0.1 Synfig documents. sfg is not supported to import
	Glib::RefPtr<Gtk::FileFilter> filter_supported = Gtk::FileFilter::create();
	filter_supported->set_name(_("All supported files"));
	filter_supported->add_mime_type("application/x-sif");
	filter_supported->add_pattern("*.sif");
	filter_supported->add_pattern("*.sifz");
	// 0.2 Image files
	filter_supported->add_mime_type("image/png");
	filter_supported->add_mime_type("image/jpeg");
	filter_supported->add_mime_type("image/jpg");
	filter_supported->add_mime_type("image/bmp");
	filter_supported->add_mime_type("image/svg+xml");
	filter_supported->add_mime_type("application/x-krita");
	filter_supported->add_mime_type("image/openraster");
	filter_supported->add_pattern("*.png");
	filter_supported->add_pattern("*.jpeg");
	filter_supported->add_pattern("*.jpg");
	filter_supported->add_pattern("*.bmp");
	filter_supported->add_pattern("*.svg");
	filter_supported->add_pattern("*.lst");
	filter_supported->add_pattern("*.kra");
	filter_supported->add_pattern("*.ora");
	// 0.3 Audio files
	filter_supported->add_mime_type("audio/x-vorbis+ogg");
	filter_supported->add_mime_type("audio/mpeg");
	filter_supported->add_mime_type("audio/x-wav");
	filter_supported->add_pattern("*.ogg");
	filter_supported->add_pattern("*.mp3");
	filter_supported->add_pattern("*.wav");
	// 0.4 Video files
	filter_supported->add_pattern("*.avi");
	filter_supported->add_pattern("*.mp4");
	filter_supported->add_pattern("*.gif");
	// 0.5 lipsync files
	filter_supported->add_pattern("*.pgo");
	filter_supported->add_pattern("*.tsv");
	filter_supported->add_pattern("*.xml");

	// Sub filters
	// 1 Synfig documents. sfg is not supported to import
	Glib::RefPtr<Gtk::FileFilter> filter_synfig = Gtk::FileFilter::create();
	filter_synfig->set_name(_("Synfig files (*.sif, *.sifz)"));
	filter_synfig->add_mime_type("application/x-sif");
	filter_synfig->add_pattern("*.sif");
	filter_synfig->add_pattern("*.sifz");

	// 2.1 Image files
	Glib::RefPtr<Gtk::FileFilter> filter_image = Gtk::FileFilter::create();
	filter_image->set_name(_("Images (*.png, *.jpeg, *.bmp, *.svg)"));
	filter_image->add_mime_type("image/png");
	filter_image->add_mime_type("image/jpeg");
	filter_image->add_mime_type("image/jpg");
	filter_image->add_mime_type("image/bmp");
	filter_image->add_mime_type("image/svg+xml");
	filter_image->add_pattern("*.png");
	filter_image->add_pattern("*.jpeg");
	filter_image->add_pattern("*.jpg");
	filter_image->add_pattern("*.bmp");
	filter_image->add_pattern("*.svg");

	// 2.2 Image sequence/list files
	Glib::RefPtr<Gtk::FileFilter> filter_image_list = Gtk::FileFilter::create();
	filter_image_list->set_name(_("Image sequence files (*.lst)"));
	filter_image_list->add_pattern("*.lst");

	// 3 Audio files
	Glib::RefPtr<Gtk::FileFilter> filter_audio = Gtk::FileFilter::create();
	filter_audio->set_name(_("Audio (*.ogg, *.mp3, *.wav)"));
	filter_audio->add_mime_type("audio/x-vorbis+ogg");
	filter_audio->add_mime_type("audio/mpeg");
	filter_audio->add_mime_type("audio/x-wav");
	filter_audio->add_pattern("*.ogg");
	filter_audio->add_pattern("*.mp3");
	filter_audio->add_pattern("*.wav");

	// 4 Video files
	Glib::RefPtr<Gtk::FileFilter> filter_video = Gtk::FileFilter::create();
	filter_video->set_name(_("Video (*.avi, *.mp4)"));
	filter_video->add_mime_type("video/x-msvideo");
	filter_video->add_mime_type("video/mp4");
	filter_video->add_pattern("*.avi");
	filter_video->add_pattern("*.mp4");

	// 5 Lipsync files
	Glib::RefPtr<Gtk::FileFilter> filter_lipsync = Gtk::FileFilter::create();
	filter_lipsync->set_name(_("Lipsync (*.pgo, *.tsv, *.xml)"));
	filter_lipsync->add_pattern("*.pgo");
	filter_lipsync->add_pattern("*.tsv");
	filter_lipsync->add_pattern("*.xml");

	// 6 Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	filesystem::Path filename = filenames.empty() ? filesystem::Path() : *filenames.begin();

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_supported, filter_synfig, filter_image, filter_image_list, filter_audio, filter_video, filter_lipsync, filter_any});
	static_cast<Gtk::Button*>(dialog->get_widget_for_response(Gtk::RESPONSE_ACCEPT))->set_label(_("Import"));
	dialog->set_select_multiple(allow_multiple_selection);
	dialog->set_extra_widget(*scale_imported_box());

	if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
		std::vector<std::string> files = dialog->get_filenames();
		filenames.assign(files.begin(), files.end());
		if (!filenames.empty()) {
			_preferences.set_value(preference, filenames.front().parent_path());
		}
		return true;
	}
	return false;
#endif   // not USE_WIN32_FILE_DIALOGS
}

bool
App::dialog_open_file(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	std::vector<synfig::filesystem::Path> filenames;
	if (!filename.empty())
		filenames.push_back(filename);
	if(dialog_open_file_ext(title, filenames, preference, false)) {
		filename = filenames.front();
		return true;
	}
	return false;
}

bool App::dialog_open_file(const std::string& title, std::vector<synfig::filesystem::Path>& filenames, const std::string& preference)
{
	return dialog_open_file_ext(title, filenames, preference, true);
}

bool
App::dialog_open_file_spal(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	filesystem::Path prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	Glib::RefPtr<Gtk::FileFilter> filter_supported = Gtk::FileFilter::create();
	filter_supported->set_name(_("Palette files (*.spal, *.gpl)"));
	filter_supported->add_pattern("*.spal");
	filter_supported->add_pattern("*.gpl");

	// show only Synfig color palette file (*.spal)
	Glib::RefPtr<Gtk::FileFilter> filter_spal = Gtk::FileFilter::create();
	filter_spal->set_name(_("Synfig palette files (*.spal)"));
	filter_spal->add_pattern("*.spal");

	// ...and add GIMP color palette file too (*.gpl)
	Glib::RefPtr<Gtk::FileFilter> filter_gpl = Gtk::FileFilter::create();
	filter_gpl->set_name(_("GIMP palette files (*.gpl)"));
	filter_gpl->add_pattern("*.gpl");

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_supported, filter_spal, filter_gpl});

	if (dialog->run() != Gtk::RESPONSE_ACCEPT)
		return false;

	filename = dialog->get_filename();
	_preferences.set_value(preference, filename.parent_path());
	return true;
}

bool
App::dialog_open_file_sketch(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// show only Synfig sketch file (*.sketch)
	Glib::RefPtr<Gtk::FileFilter> filter_sketch = Gtk::FileFilter::create();
	filter_sketch->set_name(_("Synfig sketch files (*.sketch)"));
	filter_sketch->add_pattern("*.sketch");

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_sketch});

	if(dialog->run() == Gtk::RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, filename.parent_path());
		return true;
	}

	return false;
}


bool
App::dialog_open_file_image(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// show only images
	Glib::RefPtr<Gtk::FileFilter> filter_image = Gtk::FileFilter::create();
	filter_image->set_name(_("Images and sequence files (*.png, *.jpg, *.jpeg, *.bmp, *.svg, *.lst)"));
	filter_image->add_mime_type("image/png");
	filter_image->add_mime_type("image/jpeg");
	filter_image->add_mime_type("image/jpg");
	filter_image->add_mime_type("image/bmp");
	filter_image->add_mime_type("image/svg+xml");
	filter_image->add_pattern("*.png");
	filter_image->add_pattern("*.jpeg");
	filter_image->add_pattern("*.jpg");
	filter_image->add_pattern("*.bmp");
	filter_image->add_pattern("*.svg");
	filter_image->add_pattern("*.lst");

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_image, filter_any});

	dialog->set_extra_widget(*scale_imported_box());

	if(dialog->run() == Gtk::RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, filename.parent_path());
		return true;
	}

	return false;
}


bool
App::dialog_open_file_audio(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// Audio files
	Glib::RefPtr<Gtk::FileFilter> filter_audio = Gtk::FileFilter::create();
	filter_audio->set_name(_("Audio (*.ogg, *.mp3, *.wav)"));
	filter_audio->add_mime_type("audio/x-vorbis+ogg");
	filter_audio->add_mime_type("audio/mpeg");
	filter_audio->add_mime_type("audio/x-wav");
	filter_audio->add_pattern("*.ogg");
	filter_audio->add_pattern("*.mp3");
	filter_audio->add_pattern("*.wav");

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_audio, filter_any});

	if(dialog->run() == Gtk::RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, filename.parent_path());
		return true;
	}

	return false;
}

bool
App::dialog_open_file_image_sequence(const std::string& title, std::set<synfig::filesystem::Path>& filenames, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// show only images
	Glib::RefPtr<Gtk::FileFilter> filter_image = Gtk::FileFilter::create();
	filter_image->set_name(_("Images files (*.png, *.jpg, *.jpeg, *.bmp)"));
	filter_image->add_mime_type("image/png");
	filter_image->add_mime_type("image/jpeg");
	filter_image->add_mime_type("image/jpg");
	filter_image->add_mime_type("image/bmp");
	filter_image->add_mime_type("image/svg+xml");
	filter_image->add_pattern("*.png");
	filter_image->add_pattern("*.jpeg");
	filter_image->add_pattern("*.jpg");
	filter_image->add_pattern("*.bmp");

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	filesystem::Path filename = filenames.empty() ? filesystem::Path() : *filenames.begin();

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_image, filter_any});
	dialog->set_select_multiple(true);
	dialog->set_extra_widget(*scale_imported_box());

	filenames.clear();

	if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
		std::vector<std::string> files = dialog->get_filenames();
		filenames.insert(files.begin(), files.end());
		_preferences.set_value(preference, filesystem::Path(dialog->get_filename()).parent_path());
		return true;
	}

	return false;
}

void
on_open_dialog_with_history_selection_changed(Gtk::FileChooserDialog *dialog, Gtk::Button* history_button)
{
	// activate the history button when something is selected
	filesystem::Path path(dialog->get_filename());
	history_button->set_sensitive(path.extension().u8string() == ".sfg");
}

/*

Finds which importer to use for the given filename

Returns false if the user has canceled the import.

plugin is set to the script identifier for the importer,
or an empty string if the file should be opened as a normal sif file

*/
bool
App::dialog_select_importer(const synfig::filesystem::Path& filename, std::string& plugin)
{
	synfig::String ext = filename.extension().u8string();

	Gtk::Dialog dialog(_("Select importer"), true);
	dialog.add_button(_("Cancel"), Gtk::RESPONSE_REJECT)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog.add_button(_("OK"), Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-ok", Gtk::ICON_SIZE_BUTTON);

	Gtk::Label label(_("Please select the importer to use for this file"));
	dialog.get_content_area()->pack_start(label);

	Gtk::ComboBoxText combo_importers;
	dialog.get_content_area()->pack_end(combo_importers);

	int count = 0;
	plugin = "";
	for ( const auto& importer : App::plugin_manager.importers() )
	{
		if ( importer.has_extension(ext) )
		{
			plugin = importer.id;
			combo_importers.append(importer.id, importer.description.get());
			if ( count == 0 )
				combo_importers.set_active_id(importer.id);
			count++;
		}
	}

	if ( count == 1 )
		return true;

	if ( count == 0 )
		return true;

	dialog.show_all();

	if ( dialog.run() != Gtk::RESPONSE_ACCEPT )
	{
		plugin = "";
		return false;
	}

	plugin = combo_importers.get_active_id();
	return true;

}

bool
App::dialog_open_file_with_history_button(const std::string& title, filesystem::Path& filename, bool& show_history, const std::string& preference, std::string& plugin_importer)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// File filters
	// Synfig Documents
	Glib::RefPtr<Gtk::FileFilter> filter_builtin = Gtk::FileFilter::create();
	filter_builtin->set_name(_("Synfig files (*.sif, *.sifz, *.sfg)"));
	filter_builtin->add_mime_type("application/x-sif");
	filter_builtin->add_pattern("*.sif");
	filter_builtin->add_pattern("*.sifz");
	filter_builtin->add_pattern("*.sfg");

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	// Supported files
	Glib::RefPtr<Gtk::FileFilter> filter_supported = Gtk::FileFilter::create();
	filter_supported->set_name(_("All supported files"));
	filter_supported->add_pattern("*.sif");
	filter_supported->add_pattern("*.sifz");
	filter_supported->add_pattern("*.sfg");

	auto dialog = create_dialog_open_file(title, filename, prev_path, {filter_builtin, filter_any, filter_supported});
	dialog->set_filter(filter_supported);

	for ( const ImportExport& exp : plugin_manager.importers() )
	{
		Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
		filter->set_name(exp.description.get());
		for ( const std::string& extension : exp.extensions )
		{
			filter->add_pattern("*" + extension);
			filter_supported->add_pattern("*" + extension);
		}
		dialog->add_filter(filter);
	}

	Gtk::Button* history_button = dialog->add_button(_("Open history"), RESPONSE_ACCEPT_WITH_HISTORY);
	// TODO: the Open history button should be file type sensitive one.
	dialog->set_response_sensitive(RESPONSE_ACCEPT_WITH_HISTORY, true);
	static_cast<Gtk::Button*>(dialog->get_widget_for_response(Gtk::RESPONSE_ACCEPT))->set_label(_("Open"));

	// this ptr is't available to a static member fnc, connect to global function.
	sigc::connection connection_sc = dialog->signal_selection_changed().connect(sigc::bind(sigc::ptr_fun(on_open_dialog_with_history_selection_changed), dialog.get(), history_button));

	int response = dialog->run();
	if (response == Gtk::RESPONSE_ACCEPT || response == RESPONSE_ACCEPT_WITH_HISTORY) {
		filename = dialog->get_filename();
		show_history = response == RESPONSE_ACCEPT_WITH_HISTORY;

		auto filter = dialog->get_filter();

		plugin_importer = "";
		if ( filter == filter_any || filter == filter_supported )
		{
			if ( !dialog_select_importer(filename, plugin_importer) )
				return false;
		}
		else
		{
			for ( const auto& importer : App::plugin_manager.importers() )
			{
				if ( filter->get_name() == importer.description.get() )
				{
					plugin_importer = importer.id;
					break;
				}
			}
		}

		_preferences.set_value(preference, filename.parent_path());
		return true;
	}

	connection_sc.disconnect();
	return false;
}

bool
App::dialog_open_folder(const std::string& title, filesystem::Path& foldername, const std::string& preference, Gtk::Window& transientwind)
{
	synfig::String prev_path;
	synfigapp::Settings settings;
	prev_path = settings.get_value(preference, ".");
	prev_path = filesystem::Path::absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
			title, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

	dialog->set_transient_for(transientwind);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Open"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

	if(dialog->run() == Gtk::RESPONSE_ACCEPT)
	{
		foldername = dialog->get_filename();
		delete dialog;
		return true;
	}
	delete dialog;
	return false;
}

static std::unique_ptr<Gtk::FileChooserDialog>
create_dialog_save_file(const std::string& title, const filesystem::Path& filename, const filesystem::Path& prev_path, const std::vector<Glib::RefPtr<Gtk::FileFilter>>& filters)
{
	std::unique_ptr<Gtk::FileChooserDialog> dialog(new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_SAVE));

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path.u8string());
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Save"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-save", Gtk::ICON_SIZE_BUTTON);

	for (const auto& filter : filters)
		dialog->add_filter(filter);

	filesystem::Path full_path = prev_path;
	if (!filename.empty())
		full_path /= filename;

	dialog->set_filename(filesystem::absolute(full_path).u8string());

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.
	dialog->set_current_name(filename.filename().u8string());

	return dialog;
}

bool
App::dialog_save_file(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	// info("App::dialog_save_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());

#ifdef USE_WIN32_FILE_DIALOGS
	static TCHAR szFilter[] = TEXT (_("All Files (*.*)\0*.*\0\0")) ;

	GdkWindow *gdkWinPtr=toolbox->get_window()->gobj();
	HINSTANCE hInstance=static_cast<HINSTANCE>(GetModuleHandle(nullptr));
	HWND hWnd=static_cast<HWND>(GDK_WINDOW_HWND(gdkWinPtr));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = szFilter;
//	ofn.lpstrCustomFilter=nullptr;
//	ofn.nMaxCustFilter=0;
//	ofn.nFilterIndex=0;
//	ofn.lpstrFile=nullptr;
	ofn.nMaxFile=MAX_PATH;
//	ofn.lpstrFileTitle=nullptr;
//	ofn.lpstrInitialDir=nullptr;
//	ofn.lpstrTitle=nullptr;
	ofn.Flags=OFN_OVERWRITEPROMPT;
//	ofn.nFileOffset=0;
//	ofn.nFileExtension=0;
	ofn.lpstrDefExt=TEXT("sif");
//	ofn.lCustData = 0l;
	ofn.lpfnHook=nullptr;
//	ofn.lpTemplateName=nullptr;

	CHAR szFilename[MAX_PATH];
	CHAR szTitle[500];
	strcpy(szFilename,filename.c_str());
	strcpy(szTitle,title.c_str());

	ofn.lpstrFile=szFilename;
	ofn.lpstrFileTitle=szTitle;

	if(GetSaveFileName(&ofn))
	{
		filename=szFilename;
		_preferences.set_value(preference,filesystem::Path::dirname(filename));
		return true;
	}
	return false;
#else
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// file type filters
	Glib::RefPtr<Gtk::FileFilter> filter_sif = Gtk::FileFilter::create();
	filter_sif->set_name(_("Uncompressed Synfig file (*.sif)"));

	// sif share same mime type "application/x-sif" with sifz, so it will mixed .sif and .sifz files. Use only
	// pattern ("*.sif") for sif file format should be oK.
	//filter_sif->add_mime_type("application/x-sif");
	filter_sif->add_pattern("*.sif");

	Glib::RefPtr<Gtk::FileFilter> filter_sifz = Gtk::FileFilter::create();
	filter_sifz->set_name(_("Compressed Synfig file (*.sifz)"));
	filter_sifz->add_pattern("*.sifz");

	Glib::RefPtr<Gtk::FileFilter> filter_sfg = Gtk::FileFilter::create();
	filter_sfg->set_name(_("Container format file (*.sfg)"));
	filter_sfg->add_pattern("*.sfg");

	auto dialog = create_dialog_save_file(title, filename, prev_path, {filter_sifz, filter_sif, filter_sfg});

	Widget_Enum *file_type_enum = nullptr;
	if (preference == ANIMATION_DIR_PREFERENCE)
	{
		file_type_enum = manage(new Widget_Enum());
		file_type_enum->set_param_desc(ParamDesc().set_hint("enum")
				.add_enum_value(synfig::RELEASE_VERSION_CURRENT, "Current", _("Current"))
				.add_enum_value(synfig::RELEASE_VERSION_1_4_0, "1.4.0", "1.4.0")
				.add_enum_value(synfig::RELEASE_VERSION_1_2_0, "1.2.0", "1.2.0")
				.add_enum_value(synfig::RELEASE_VERSION_1_0_2, "1.0.2", "1.0.2")
				.add_enum_value(synfig::RELEASE_VERSION_1_0, "1.0", "1.0")
				.add_enum_value(synfig::RELEASE_VERSION_0_64_3, "0.64.3", "0.64.3")
				.add_enum_value(synfig::RELEASE_VERSION_0_64_2, "0.64.2", "0.64.2")
				.add_enum_value(synfig::RELEASE_VERSION_0_64_1, "0.64.1", "0.64.1")
				.add_enum_value(synfig::RELEASE_VERSION_0_64_0, "0.64.0", "0.64.0")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_04, "0.63.05", "0.63.05")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_04, "0.63.04", "0.63.04")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_03, "0.63.03", "0.63.03")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_02, "0.63.02", "0.63.02")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_01, "0.63.01", "0.63.01")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_00, "0.63.00", "0.63.00")
				.add_enum_value(synfig::RELEASE_VERSION_0_62_02, "0.62.02", "0.62.02")
				.add_enum_value(synfig::RELEASE_VERSION_0_62_01, "0.62.01", "0.62.01")
				.add_enum_value(synfig::RELEASE_VERSION_0_62_00, "0.62.00", "0.61.00")
				.add_enum_value(synfig::RELEASE_VERSION_0_61_09, "0.61.09", "0.61.09")
				.add_enum_value(synfig::RELEASE_VERSION_0_61_08, "0.61.08", "0.61.08")
				.add_enum_value(synfig::RELEASE_VERSION_0_61_07, "0.61.07", "0.61.07")
				.add_enum_value(synfig::RELEASE_VERSION_0_61_06, "0.61.06", strprintf("0.61.06 %s", _("and older"))));
		file_type_enum->set_value(RELEASE_VERSION_CURRENT); // default to the most recent version
		file_type_enum->set_hexpand(false);

		Gtk::Grid *grid = manage(new Gtk::Grid);
		grid->attach(*manage(new Gtk::Label(_("File Format Version: "))),0,0,1,1);
		grid->attach(*file_type_enum,1,0,1,1);
		grid->show_all();

		dialog->set_extra_widget(*grid);
	}

	// set file filter according to previous file format
	const std::map<std::string, Glib::RefPtr<Gtk::FileFilter>> filter_map = {
		{".sif", filter_sif},
		{".sifz", filter_sifz},
		{".sfg", filter_sfg}
	};

	{
		auto filter_iter = filter_map.find(filename.extension().u8string());
		if (filter_iter != filter_map.end())
			dialog->set_filter(filter_iter->second);
	}

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.
	dialog->set_current_name(filename.filename().u8string());

	if(dialog->run() == Gtk::RESPONSE_ACCEPT) {

		if (preference == ANIMATION_DIR_PREFERENCE)
			set_file_version(synfig::ReleaseVersion(file_type_enum->get_value()));

		// add file extension according to file filter selected by user if he doesn't type file extension in
		// file name entry. Right now it still detects file extension from file name entry, if extension is one
		// of .sif, sifz and sfg, it will be used otherwise, saved file format will depend on selected file filter.
		// It should be improved by changing file extension according to set file type filter, such as:
		// dialog->property_filter().signal_changed().connect(sigc::mem_fun(*this, &App::on_save_dialog_filter_changed));
		filename = dialog->get_filename();

		auto filter_iter = filter_map.find(filename.extension().u8string());

		// No known extension; get it from the selected filter
		if (filter_iter == filter_map.end()) {
			for (const auto& filter_item : filter_map) {
				if (filter_item.second == dialog->get_filter()) {
					filename = filename + filter_item.first;
					break;
				}
			}
		}

		_preferences.set_value(preference, filename.parent_path());
		return true;
	}

	return false;
#endif
}


std::string
App::dialog_export_file(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	filesystem::Path prev_path = _preferences.get_value(preference, Glib::get_home_dir());
	prev_path = filesystem::absolute(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_SAVE);

	for ( const ImportExport& exp : App::plugin_manager.exporters() )
	{
		Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
		filter->set_name(exp.description.get());
		for ( const std::string& extension : exp.extensions )
			filter->add_pattern("*" + extension);
		dialog->add_filter(filter);
	}

	dialog->set_current_folder(prev_path.u8string());
	dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_ACCEPT);

	if (filename.empty()) {
		dialog->set_filename(prev_path.u8string());
	} else {
		dialog->set_current_name(filename.stem().u8string());
	}

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.

	if(dialog->run() == Gtk::RESPONSE_ACCEPT) {

		filename = dialog->get_filename();

		_preferences.set_value(preference, filename.parent_path());

		auto filter = dialog->get_filter();
		for ( const auto& exporter : App::plugin_manager.exporters() )
		{
			if ( filter->get_name() == exporter.description.get() )
			{
				if ( !exporter.has_extension(filename.extension().u8string()) )
					filename += exporter.extensions[0];

				delete dialog;
				return exporter.id;
			}
		}
	}

	delete dialog;
	return {};
}

bool
App::dialog_save_file_spal(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// file type filters
	Glib::RefPtr<Gtk::FileFilter> filter_spal = Gtk::FileFilter::create();
	filter_spal->set_name(_("Synfig palette files (*.spal)"));
	filter_spal->add_pattern("*.spal");

	auto dialog = create_dialog_save_file(title, filename, prev_path, {filter_spal});

	dialog->set_filter(filter_spal);

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.
	dialog->set_current_name(filename.filename().u8string());

	if (dialog->run() == Gtk::RESPONSE_ACCEPT) {

		// add file extension according to file filter selected by user
		filename = dialog->get_filename();
		if (filename.extension().u8string() != ".spal")
			filename = dialog->get_filename() + ".spal";

		return true;
	}

	return false;
}

bool
App::dialog_save_file_sketch(const std::string& title, synfig::filesystem::Path& filename, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	// file type filters
	Glib::RefPtr<Gtk::FileFilter> filter_sketch = Gtk::FileFilter::create();
	filter_sketch->set_name(_("Synfig sketch files (*.sketch)"));
	filter_sketch->add_pattern("*.sketch");

	auto dialog = create_dialog_save_file(title, filename, prev_path, {filter_sketch});

	dialog->set_filter(filter_sketch);

	if (dialog->run() == Gtk::RESPONSE_ACCEPT) {

		// add file extension according to file filter selected by user
		filename = dialog->get_filename();
		if (filename.extension().u8string() != ".sketch")
			filename = dialog->get_filename() + ".sketch";

		return true;
	}

	return false;
}


bool
App::dialog_save_file_render(const std::string& title, filesystem::Path& filename, const std::string& preference)
{
	synfig::String prev_path = _preferences.get_value(preference, Glib::get_home_dir());

	auto dialog = create_dialog_save_file(title, filename, prev_path, {});

	if(dialog->run() == Gtk::RESPONSE_ACCEPT)
	{
		filename = dialog->get_filename();

		return true;
	}

	return false;
}


bool
App::dialog_select_list_item(const std::string &title, const std::string &message, const std::list<std::string> &list, int &item_index)
{
	Gtk::Dialog dialog(title, *App::main_window, true);

	Gtk::Label* label = manage (new Gtk::Label(message, 0, 0));
	label->set_line_wrap();

	class ModelColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<int> column_index;
		Gtk::TreeModelColumn<Glib::ustring> column_main;
		ModelColumns() { add(column_index); add(column_main); }
	} model_columns;

	Glib::RefPtr<Gtk::ListStore> list_store = Gtk::ListStore::create(model_columns);

	int k = 0;
	for(std::list<std::string>::const_iterator i = list.begin(); i != list.end(); ++i) {
		Gtk::ListStore::iterator j = list_store->append();
		j->set_value(model_columns.column_index, k++);
		j->set_value(model_columns.column_main, Glib::ustring(*i));
	}

	Gtk::TreeView * tree = manage (new Gtk::TreeView(list_store));
	Gtk::TreeViewColumn column_index("", model_columns.column_index);
	Gtk::TreeViewColumn column_main("", model_columns.column_main);
	column_index.set_visible(false);
	tree->append_column(column_index);
	tree->append_column(column_main);
	tree->set_hexpand(TRUE);
	tree->set_halign(Gtk::ALIGN_FILL);
	tree->set_vexpand(TRUE);
	tree->set_valign(Gtk::ALIGN_FILL);

	Gtk::TreeModel::Row selected_row = list_store->children()[item_index];
	if (selected_row)
		tree->get_selection()->select(selected_row);

	Gtk::Grid* grid = manage(new Gtk::Grid());
	grid->attach(*label, 0, 0, 1, 1);
	grid->attach(*tree, 0, 1, 1, 2);

	dialog.get_content_area()->pack_start(*grid);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_ACCEPT);
	dialog.set_default_size(300, 450);
	dialog.show_all();

	if (dialog.run() != Gtk::RESPONSE_ACCEPT)
		return false;

	item_index = tree->get_selection()->get_selected()->get_value(model_columns.column_index);
	return true;
}


void
App::dialog_not_implemented()
{
	Gtk::MessageDialog dialog(*App::main_window, _("Feature not available"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
	dialog.set_secondary_text(_("Sorry, this feature has not yet been implemented."));
	dialog.run();
}


// message dialog with 1 button.
void
App::dialog_message_1b(
	const std::string &type,
		//INFO:		Gtk::MESSAGE_INFO - Informational message.
		//WARNING:	Gtk::MESSAGE_WARNING - Non-fatal warning message.
		//QUESTION:	Gtk::MESSAGE_QUESTION - Question requiring a choice.
		//ERROR:	Gtk::MESSAGE_ERROR - Fatal error message.
		//OTHER:	Gtk::MESSAGE_OTHER - None of the above, doesn’t get an icon.
	const std::string &message,
	const std::string &details,
	const std::string &button1,
	const std::string &long_details)
{
	Gtk::MessageType _type = Gtk::MESSAGE_OTHER; // default
	if (type == "INFO")
		_type = Gtk::MESSAGE_INFO;
	if (type == "WARNING")
		_type = Gtk::MESSAGE_WARNING;
	if (type == "QUESTION")
		_type = Gtk::MESSAGE_QUESTION;
	if (type == "ERROR")
		_type = Gtk::MESSAGE_ERROR;
	if (type == "OTHER")
		_type = Gtk::MESSAGE_OTHER;

	Gtk::MessageDialog dialog(*App::main_window, message, false, _type, Gtk::BUTTONS_NONE, true);

	if (details != "details")
		dialog.set_secondary_text(details);

	Gtk::Label label;
	Gtk::ScrolledWindow sw;
	if (long_details != "long_details")
	{
		label.set_text(long_details);
		label.set_margin_start(5);
		label.set_margin_end(5);
		label.set_line_wrap();
		label.show();
		sw.set_size_request(400,100);
		sw.set_min_content_height(80);
		sw.add(label);
		sw.show();
		dialog.get_content_area()->pack_end(sw);
		dialog.set_resizable();
	}

	dialog.add_button(button1, 0);

	dialog.run();
}


// message dialog with 2 buttons.
bool
App::dialog_message_2b(const std::string &message,
	const std::string &details,
	const Gtk::MessageType &type,
		//MESSAGE_INFO - Informational message.
		//MESSAGE_WARNING - Non-fatal warning message.
		//MESSAGE_QUESTION - Question requiring a choice.
		//MESSAGE_ERROR - Fatal error message.
		//MESSAGE_OTHER - None of the above, doesn’t get an icon.
	const std::string &button1,
	const std::string &button2)
{
	Gtk::MessageDialog dialog(*App::main_window, message, false, type, Gtk::BUTTONS_NONE, true);
	dialog.set_secondary_text(details);
	dialog.add_button(button1, 0);
	dialog.add_button(button2, 1);

	if(dialog.run() == 1)
		return true;

	return false;
}


// message dialog with 3 buttons.
int
App::dialog_message_3b(const std::string &message,
	const std::string &details,
	const Gtk::MessageType &type,
		//MESSAGE_INFO - Informational message.
		//MESSAGE_WARNING - Non-fatal warning message.
		//MESSAGE_QUESTION - Question requiring a choice.
		//MESSAGE_ERROR - Fatal error message.
		//MESSAGE_OTHER - None of the above, doesn’t get an icon.
	const std::string &button1,
	const std::string &button2,
	const std::string &button3)
{
	Gtk::MessageDialog dialog(*App::main_window, message, false, type, Gtk::BUTTONS_NONE, true);
	dialog.set_secondary_text(details);
	dialog.add_button(button1, 0);
	dialog.add_button(button2, 1);
	dialog.add_button(button3, 2);

	return dialog.run();
}

static bool
try_open_uri(const std::string &uri)
{
#if GTK_CHECK_VERSION(3, 22, 0)
	return gtk_show_uri_on_window(
		App::main_window ? GTK_WINDOW(App::main_window->gobj()) : nullptr,
		uri.c_str(), GDK_CURRENT_TIME, nullptr );
#else
	return gtk_show_uri(nullptr, uri.c_str(), GDK_CURRENT_TIME, nullptr);
#endif
}


void
App::dialog_help()
{
	if (!try_open_uri("https://synfig.readthedocs.io/en/latest/index.html"))
	{
		Gtk::MessageDialog dialog(*App::main_window, _("Documentation"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true);
		dialog.set_secondary_text(_("Documentation for Synfig Studio is available on the website:\n\nhttps://synfig.readthedocs.io/en/latest/index.html"));
		dialog.set_title(_("Help"));
		dialog.run();
	}
}

#if GTK_CHECK_VERSION(3, 20, 0)
void
App::window_shortcuts()
{
	auto shortcuts_resource = Gtk::Builder::create_from_file(
				ResourceHelper::get_ui_path("shortcuts_window.glade"));

	Gtk::ShortcutsWindow *shortcuts_window = nullptr;
	shortcuts_resource->get_widget("shortcuts_window", shortcuts_window);

	shortcuts_window->set_transient_for(*App::main_window);
	shortcuts_window->set_modal();

	shortcuts_window->present();
}
#endif

void App::open_img_in_external(const std::string &uri)
{
	// Check if external editor is set
	if (App::image_editor_path.empty()) {
		ui_interface_->error(_("Please set preferred editing tool in \nEdit->Preferences->Editing"));
		return;
	}

	const std::string filename = Glib::filename_from_uri(uri);
	synfig::info("Opening with external tool: " + filename);

	std::vector<std::string> args;
	args.emplace_back(App::image_editor_path);
	args.emplace_back(filename);

	try {
		// Glib::spawn_* needs `gspawn-win??-helper.exe` for Windows
		Glib::spawn_async("", args);
	} catch (const Glib::SpawnError& e) {
		ui_interface_->error(synfig::strprintf(_("Failed to run command: %s (%s)"), App::image_editor_path.c_str(), e.what().c_str()));
	}
}

static std::unordered_map<std::string, int> vectorizer_configmap({ { "threshold", 8 },{ "accuracy", 9 },{ "despeckling", 5 },{ "maxthickness", 200 }});

void App::open_vectorizerpopup(const synfig::Layer_Bitmap::Handle my_layer_bitmap, const synfig::Layer::Handle reference_layer)
{
	String desc = my_layer_bitmap->get_description();
	synfig::info("Opening Vectorizerpopup for :"+desc);
	App::vectorizerpopup = studio::VectorizerSettings::create(*App::main_window,my_layer_bitmap,selected_instance,vectorizer_configmap,reference_layer);
	if(!vectorizerpopup){
		App::dialog_message_1b(
			"ERROR",
			_("Glade file could not be found!"),
			"details",
			_("Ok"),
			"long_details"
		);
	}
	else
		App::vectorizerpopup->show();
}

void App::open_uri(const std::string &uri)
{
	synfig::info("Opening URI: " + uri);
	if(!try_open_uri(uri))
	{
		Gtk::MessageDialog dialog(*App::main_window, _("No compatible application was found. Please load open file manually:"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		dialog.set_secondary_text(uri);
		dialog.set_title(_("Error"));
		dialog.run();
	}
}

bool
App::dialog_entry(const std::string &action, const std::string &content, std::string &text, const std::string &button1, const std::string &button2)
{
	Gtk::MessageDialog dialog(
		*App::main_window,
		action,
		false,
		Gtk::MESSAGE_INFO,
		Gtk::BUTTONS_NONE,
		true
	);

	// TODO Group All HARDCODED user interface information somewhere "global"
	// TODO All UI info from .rc
#define DIALOG_ENTRY_MARGIN 18
	Gtk::Label* label = manage (new Gtk::Label(content));
	label->set_margin_start(DIALOG_ENTRY_MARGIN);

	Gtk::Entry* entry = manage(new Gtk::Entry());
	entry->set_text(text);
	entry->set_margin_end(DIALOG_ENTRY_MARGIN);
	entry->set_activates_default(true);
	entry->set_hexpand(TRUE);
	entry->set_halign(Gtk::ALIGN_FILL);
#undef DIALOG_ENTRY_MARGIN

	Gtk::Grid* grid = manage (new Gtk::Grid());
	grid->add(*label);
	grid->add(*entry);

	grid->show_all();

	dialog.get_content_area()->pack_start(*grid);
	dialog.add_button(button1, Gtk::RESPONSE_CANCEL);
	dialog.add_button(button2, Gtk::RESPONSE_OK);

	dialog.set_default_response(Gtk::RESPONSE_OK);
	entry->signal_activate().connect(sigc::bind(sigc::mem_fun(dialog,&Gtk::Dialog::response),Gtk::RESPONSE_OK));
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_OK)
		return false;

	text = entry->get_text();

	return true;
}

bool
App::dialog_sets_entry(const std::string &action, const std::string &content, std::string &text, const std::string &button1, const std::string &button2)
{
	Gtk::MessageDialog dialog(
		*App::main_window,
		action,
		false,
		Gtk::MESSAGE_INFO,
		Gtk::BUTTONS_NONE,
		true
	);

#define DIALOG_ENTRY_MARGIN 18
	Gtk::Label* label = manage (new Gtk::Label(content));
	label->set_margin_start(DIALOG_ENTRY_MARGIN);

	Gtk::ComboBoxText* combo_entry = manage(new Gtk::ComboBoxText(true));
	combo_entry->set_hexpand(true);
	combo_entry->set_margin_end(DIALOG_ENTRY_MARGIN);
	combo_entry->set_halign(Gtk::ALIGN_FILL);
#undef DIALOG_ENTRY_MARGIN

	std::set<std::string> available_sets = App::get_selected_canvas_view()->canvas_interface()->get_canvas()->get_groups();

	SetsModel m_columns;

	combo_entry->set_entry_text_column(m_columns.entry_set_name);

	Glib::RefPtr<Gtk::TreeStore> m_refTreeModel;

	m_refTreeModel = Gtk::TreeStore::create(m_columns);
	combo_entry->set_model(m_refTreeModel);

	for (std::set<std::string>::iterator i = available_sets.begin(); i != available_sets.end(); ++i) {
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_columns.entry_set_name] = *i;
	}

	Gtk::Grid* grid = manage (new Gtk::Grid());
	grid->add(*label);
	grid->add(*combo_entry);

	grid->show_all();

	dialog.get_content_area()->pack_start(*grid);
	dialog.add_button(button1, Gtk::RESPONSE_CANCEL);
	dialog.add_button(button2, Gtk::RESPONSE_OK);

	dialog.set_default_response(Gtk::RESPONSE_OK);
	combo_entry->get_entry()->signal_activate().connect(sigc::bind(sigc::mem_fun(dialog, &Gtk::Dialog::response), Gtk::RESPONSE_OK));
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_OK)
		return false;

	text = combo_entry->get_entry_text();

	return true;
}

bool
App::dialog_paragraph(const std::string &title, const std::string &message,std::string &text)
{
	Gtk::Dialog dialog(
		title,			// Title
		*App::main_window,	// Parent
		true			// Modal
	);

	Gtk::Label* label = manage(new Gtk::Label(message));
	label->show();
	dialog.get_content_area()->pack_start(*label, Gtk::PACK_SHRINK);

	Glib::RefPtr<Gtk::TextBuffer> text_buffer(Gtk::TextBuffer::create());
	text_buffer->set_text(text);
	Gtk::TextView text_view(text_buffer);
	text_view.show();
	Gtk::ScrolledWindow scrolled_window;
	scrolled_window.add(text_view);
	scrolled_window.show();

	dialog.get_content_area()->pack_start(scrolled_window);

	dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog.add_button(_("_OK"),   Gtk::RESPONSE_OK)->set_image_from_icon_name("gtk-ok", Gtk::ICON_SIZE_BUTTON);
	dialog.set_default_response(Gtk::RESPONSE_OK);

	text_view.signal_key_press_event().connect(( [&dialog](GdkEventKey *ev) {

		SYNFIG_EXCEPTION_GUARD_BEGIN()
		if ((ev->type == GDK_KEY_PRESS) &&
			(ev->keyval == (GDK_KEY_Return) || ev->keyval == (GDK_KEY_KP_Enter)) &&
			(ev->state == GDK_CONTROL_MASK)){
				dialog.response(Gtk::RESPONSE_OK);
				return true;
		}
		return false;
		SYNFIG_EXCEPTION_GUARD_END_BOOL(true)

	}), false );
	//text_entry.signal_activate().connect(sigc::bind(sigc::mem_fun(dialog,&Gtk::Dialog::response),Gtk::RESPONSE_OK));
	dialog.set_default_size(400, 300);
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_OK)
		return false;

	text=text_buffer->get_text();

	return true;
}

synfig::filesystem::Path
App::get_temporary_directory()
{
	return synfigapp::Main::get_user_app_directory().append("tmp");
}

synfig::FileSystemTemporary::Handle
App::wrap_into_temporary_filesystem(
	synfig::FileSystem::Handle canvas_file_system,
	std::string filename,
	std::string as,
	synfig::FileContainerZip::file_size_t truncate_storage_size )
{
	FileSystemTemporary::Handle temporary_file_system = new FileSystemTemporary("instance", get_temporary_directory(), canvas_file_system);
	temporary_file_system->set_meta("filename", filename);
	temporary_file_system->set_meta("as", as);
	temporary_file_system->set_meta("truncate", synfig::strprintf("%lld", truncate_storage_size));
	return temporary_file_system;
}

bool
App::open(filesystem::Path filename, /* std::string as, */ synfig::FileContainerZip::file_size_t truncate_storage_size)
{
#ifdef _WIN32
	{

	size_t buf_size = MAX_PATH - 1;
	std::vector<wchar_t> long_name;
	long_name.resize(buf_size);
	long_name[0] = 0;
	if (GetLongPathNameW(filename.c_str(), long_name.data(), long_name.size()) == 0)
		;
	// when called from autorecover.cpp, filename doesn't exist, and so long_name is empty
	// don't use it if that's the case
	if (long_name[0] != '\0')
		filename = filesystem::Path::from_native(long_name.data());

	}
#endif

	try
	{
		OneMoment one_moment;
		String errors, warnings;

		// try open container
		FileSystem::Handle container = CanvasFileNaming::make_filesystem_container(filename.u8string(), truncate_storage_size);
		if (!container)
			throw (String)strprintf(_("Unable to open container \"%s\"\n\n"), filename.u8_str());

		// make canvas file system
		FileSystem::Handle canvas_file_system = CanvasFileNaming::make_filesystem(container);

		// wrap into temporary file system
		canvas_file_system = wrap_into_temporary_filesystem(canvas_file_system, filename.u8string(), filename.u8string(), truncate_storage_size);

		// file to open inside canvas file-system
		String canvas_filename = CanvasFileNaming::project_file(filename.u8string());

		Canvas::Handle canvas = open_canvas_as(canvas_file_system ->get_identifier(canvas_filename), filename.u8string(), errors, warnings);
		if(canvas && get_instance(canvas))
		{
			get_instance(canvas)->find_canvas_view(canvas)->present();
			info("%s is already open", canvas_filename.c_str());
			// throw (String)strprintf(_("\"%s\" appears to already be open!"),filename.c_str());
		}
		else
		{
			if(!canvas)
				throw (String)strprintf(_("Unable to load \"%s\":\n\n"), filename.u8_str()) + errors;

			// Set new pixel ratio
			canvas->rend_desc().set_pixel_ratio(canvas->rend_desc().get_w(), canvas->rend_desc().get_h());

			if (!warnings.empty())
				dialog_message_1b(
					"WARNING",
					_("Warning"),
					"details",
					_("Close"),
					warnings);

			if (filename.u8string().find(custom_filename_prefix) != 0)
				add_recent_file(filename);

			etl::handle<Instance> instance(Instance::create(canvas, container));

			if(!instance)
				throw (String)strprintf(_("Unable to create instance for \"%s\""), filename.u8_str());

			one_moment.hide();
		}
	}
	catch(String &x)
	{
		dialog_message_1b(
			"ERROR",
			x,
			"details",
			_("Close"));

		return false;
	}
	catch(std::runtime_error &x)
	{
		dialog_message_1b(
			"ERROR",
			x.what(),
			"details",
			_("Close"));

		return false;
	}
	catch(...)
	{
		dialog_message_1b(
			"ERROR",
			_("Uncaught error on file open (BUG)"),
			"details",
			_("Close"));

		return false;
	}

	return true;
}

// this is called from autorecover.cpp
bool
App::open_from_temporary_filesystem(const filesystem::Path& temporary_filename)
{
	try
	{
		OneMoment one_moment;
		String errors, warnings;

		// try open temporary container
		FileSystemTemporary::Handle file_system_temporary(new FileSystemTemporary(""));
		if (!file_system_temporary->open_temporary(temporary_filename))
			throw (String)strprintf(_("Unable to open temporary container \"%s\"\n\n"), temporary_filename.u8_str());

		// get original filename
		String filename = file_system_temporary->get_meta("filename");
		String as = file_system_temporary->get_meta("as");
		String truncate = file_system_temporary->get_meta("truncate");
		if (filename.empty() || as.empty() || truncate.empty())
			throw (String)strprintf(_("Original filename was not set in temporary container \"%s\"\n\n"), temporary_filename.u8_str());
		FileContainerZip::file_size_t truncate_storage_size = stoll(truncate);

		// make canvas file-system
		FileSystem::Handle canvas_container = CanvasFileNaming::make_filesystem_container(filename, truncate_storage_size);
		FileSystem::Handle canvas_file_system = CanvasFileNaming::make_filesystem(canvas_container);

		// wrap into temporary
		file_system_temporary->set_sub_file_system(canvas_file_system);
		canvas_file_system = file_system_temporary;

		// file to open inside canvas file system
		String canvas_filename = CanvasFileNaming::project_file(canvas_file_system);

		Canvas::Handle canvas(open_canvas_as(canvas_file_system->get_identifier(canvas_filename), as, errors, warnings));
		if(canvas && get_instance(canvas))
		{
			get_instance(canvas)->find_canvas_view(canvas)->present();
			info("%s is already open", as.c_str());
			// throw (String)strprintf(_("\"%s\" appears to already be open!"),filename.c_str());
		}
		else
		{
			if(!canvas)
				throw (String)strprintf(_("Unable to load \"%s\":\n\n"), temporary_filename.u8_str()) + errors;

			if (warnings != "")
				dialog_message_1b(
						"WARNING",
						strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
						"details",
						_("Close"));

			if (as.find(custom_filename_prefix.c_str()) != 0)
				add_recent_file(as);

			etl::handle<Instance> instance(Instance::create(canvas, canvas_container));

			if(!instance)
				throw (String)strprintf(_("Unable to create instance for \"%s\""), temporary_filename.u8_str());

			one_moment.hide();

			// This file isn't saved! mark it as such
			instance->inc_action_count();
		}
	}
	catch(String &x)
	{
		dialog_message_1b(
				"ERROR",
				 x,
				"details",
				_("Close"));

		return false;
	}
	catch(std::runtime_error &x)
	{
		dialog_message_1b(
				"ERROR",
				x.what(),
				"details",
				_("Close"));

		return false;
	}
	catch(...)
	{
		dialog_message_1b(
				"ERROR",
				_("Uncaught error on file open (BUG)"),
				"details",
				_("Close"));

		return false;
	}

	return true;
}


void
App::new_instance()
{
	Canvas::Handle canvas = Canvas::create();

	String filename(strprintf("%s%d", App::custom_filename_prefix.c_str(), Instance::get_count()+1));
	canvas->set_name(filename);

	canvas->rend_desc().set_frame_rate(preferred_fps);
	canvas->rend_desc().set_time_start(0.0);
	canvas->rend_desc().set_time_end(5.0);
	canvas->rend_desc().set_x_res(DPI2DPM(72.0));
	canvas->rend_desc().set_y_res(DPI2DPM(72.0));
	// The top left and bottom right positions are expressed in units
	// Original convention is that 1 unit = 60 pixels
	canvas->rend_desc().set_tl(Vector(-(preferred_x_size/60.0)/2.0,  (preferred_y_size/60.0)/2.0));
	canvas->rend_desc().set_br(Vector( (preferred_x_size/60.0)/2.0, -(preferred_y_size/60.0)/2.0));
	canvas->rend_desc().set_w(preferred_x_size);
	canvas->rend_desc().set_h(preferred_y_size);
	canvas->rend_desc().set_antialias(1);
	canvas->rend_desc().set_flags(RendDesc::PX_ASPECT|RendDesc::IM_SPAN);
	canvas->set_file_name(filename);
	canvas->keyframe_list().add(synfig::Keyframe());

	FileSystem::Handle container = new FileSystemEmpty();
	FileSystem::Handle file_system = CanvasFileNaming::make_filesystem(container);
	file_system = wrap_into_temporary_filesystem(file_system, filename, filename);

	// file name inside canvas file-system
	String canvas_filename = CanvasFileNaming::project_file(filename);
	canvas->set_identifier(file_system->get_identifier(canvas_filename));

	etl::handle<Instance> instance = Instance::create(canvas, container);

	if (App::default_background_layer_type == "solid_color")
	{
		//Create a SolidColor layer
		synfig::Layer::Handle layer(instance->find_canvas_interface(canvas)->add_layer_to("solid_color",
			                        canvas,
			                        0)); //target_depth

		//Rename it as Background
		synfigapp::Action::Handle action_LayerSetDesc(synfigapp::Action::create("LayerSetDesc"));
		if (action_LayerSetDesc) {
			action_LayerSetDesc->set_param("canvas",           canvas);
			action_LayerSetDesc->set_param("canvas_interface", instance->find_canvas_interface(canvas));
			action_LayerSetDesc->set_param("layer",            layer);
			action_LayerSetDesc->set_param("new_description",  _("Background"));
			App::get_selected_canvas_view()->canvas_interface()->get_instance()->perform_action(action_LayerSetDesc);
		}

		//Change its color to the selected one
		synfigapp::Action::Handle action_LayerParamSet(synfigapp::Action::create("LayerParamSet"));
		if (action_LayerParamSet) {
			action_LayerParamSet->set_param("canvas",           canvas);
			action_LayerParamSet->set_param("canvas_interface", instance->find_canvas_interface(canvas));
			action_LayerParamSet->set_param("layer",            layer);
			action_LayerParamSet->set_param("param",            "color");
			action_LayerParamSet->set_param("new_value",        ValueBase(App::default_background_layer_color));
			App::get_selected_canvas_view()->canvas_interface()->get_instance()->perform_action(action_LayerParamSet);
		}

	}
	else if (App::default_background_layer_type == "image")
	{
		String errors, warnings;
		instance->find_canvas_interface(canvas)->import(App::default_background_layer_image,
		                                                errors,
		                                                warnings,
		                                                App::resize_imported_images);

		synfig::Layer::Handle layer = instance->find_canvas_interface(canvas)->get_selection_manager()->get_selected_layer();

		//Rename it as Background
		synfigapp::Action::Handle action_LayerSetDesc(synfigapp::Action::create("LayerSetDesc"));
		if (action_LayerSetDesc) {
			action_LayerSetDesc->set_param("canvas",           canvas);
			action_LayerSetDesc->set_param("canvas_interface", instance->find_canvas_interface(canvas));
			action_LayerSetDesc->set_param("layer",            layer);
			action_LayerSetDesc->set_param("new_description",  _("Background"));
			App::get_selected_canvas_view()->canvas_interface()->get_instance()->perform_action(action_LayerSetDesc);
		}

		if (warnings != "")
			App::dialog_message_1b(
				"WARNING",
				synfig::strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
				"details",
				_("Close"));
		if (errors != "")
			App::dialog_message_1b(
				"ERROR",
				synfig::strprintf("%s:\n\n%s", _("Error"), errors.c_str()),
				"details",
				_("Close"));
	}

	if (getenv("SYNFIG_ENABLE_NEW_CANVAS_EDIT_PROPERTIES"))
		instance->find_canvas_view(canvas)->canvas_properties.present();
}

void
App::open_from_plugin(const filesystem::Path& filename, const std::string& importer_id)
{
	auto temp_lock = FileSystemTemporary::reserve_temporary_filename(get_temporary_directory(), "synfig", ".sif");
	if (!temp_lock.second) {
		dialog_message_1b("ERROR", _("Couldn't create a temporary file"), "details", _("Close"));
		return;
	}

	filesystem::Path tmp_filename = temp_lock.first;

	bool result = plugin_manager.run(importer_id, {filename.u8string(), tmp_filename.u8string()});

	if ( result ) {
		OneMoment one_moment;
		String errors, warnings;

		// try open container
		FileSystem::Handle container = CanvasFileNaming::make_filesystem_container(tmp_filename.u8string(), 0);
		if ( !container ) {
			errors += strprintf(_("Unable to open container \"%s\"\n\n"), tmp_filename.u8_str());
		} else {
			FileSystem::Handle canvas_file_system = CanvasFileNaming::make_filesystem(container);
			canvas_file_system = wrap_into_temporary_filesystem(canvas_file_system, tmp_filename.u8string(), filename.u8string(), 0);
			String canvas_filename = CanvasFileNaming::project_file(tmp_filename.u8string());
			Canvas::Handle canvas = open_canvas_as(canvas_file_system->get_identifier(canvas_filename), filename.u8string(), errors, warnings);
			if ( !canvas )
			{
				errors += strprintf(_("Unable to load \"%s\":\n\n"), filename.u8_str());
			}
			else
			{
				if ( !get_instance(canvas) )
				{
					if (warnings != "")
						dialog_message_1b("WARNING", _("Warning"), "details", _("Close"), warnings);

					etl::handle<Instance> instance(Instance::create(canvas, container));

					if ( !instance ) {
						errors += strprintf(_("Unable to create instance for \"%s\""), filename.u8_str());
					}
					one_moment.hide();
				}

				canvas->set_file_name(App::custom_filename_prefix);
				add_recent_file(filename);
			}
		}

		if ( !errors.empty() )
			dialog_message_1b("ERROR", errors, "details", _("Close"));
	}

	FileSystemNative::instance()->remove_recursive(tmp_filename);
	// lock file (temp_lock.second) is auto-deleted
}

void
App::open_recent(const filesystem::Path& filename)
{
	std::string importer;
	if ( !dialog_select_importer(filename, importer) )
		return;

	if ( importer.empty() )
		open(filename);
	else
		open_from_plugin(filename, importer);
}

void
App::dialog_open(filesystem::Path filename)
{
	if (filename.empty()) {
		filename = selected_instance ? selected_instance->get_file_name() : "*.sif";
	}

	bool show_history = false;
	std::string plugin_importer;
	while(dialog_open_file_with_history_button(_("Please select a file"), filename, show_history, ANIMATION_DIR_PREFERENCE, plugin_importer))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if (filename.u8string().find('*') != std::string::npos)
			continue;

		if ( !plugin_importer.empty() )
		{
			open_from_plugin(filename, plugin_importer);
			return;
		}

		FileContainerZip::file_size_t truncate_storage_size = 0;

		// TODO: ".sfg" literal
		if (show_history && filename.extension().u8string() == ".sfg")
		{
			// read history
			std::list<FileContainerZip::HistoryRecord> history
				= FileContainerZip::read_history(filename.u8string());

			// build list of history entries for dialog (descending)
			std::list<std::string> list;
			int index = 0;
			for (std::list<FileContainerZip::HistoryRecord>::const_iterator i = history.begin(); i != history.end(); ++i)
				list.push_front(strprintf("%s%d", _("History entry #"), ++index));

			// show dialog
			index=0;
			if (!dialog_select_list_item(_("Please select a file"), _("Select one of previous versions of file"), list, index))
				continue;

			// find selected entry in list (descending)
			for (std::list<FileContainerZip::HistoryRecord>::const_reverse_iterator i = history.rbegin(); i != history.rend(); ++i)
				if (0 == index--)
					truncate_storage_size = i->storage_size;
		}

		if (open(filename, truncate_storage_size))
			break;

		get_ui_interface()->error(_("Unable to open file"));
	}
}

void
App::set_selected_instance(etl::loose_handle<Instance> instance)
{
	if (selected_instance == instance)
		return;

	if (get_selected_canvas_view() && get_selected_canvas_view()->get_instance() != instance) {
		if (instance) {
			instance->focus( instance->get_canvas() );
		} else {
			set_selected_canvas_view(nullptr);
		}
	} else {
		selected_instance = instance;
		signal_instance_selected()(selected_instance);
	}
}

void
App::set_selected_canvas_view(CanvasView::LooseHandle canvas_view)
{
	if (selected_canvas_view == canvas_view)
		return;

	CanvasView::LooseHandle prev = selected_canvas_view;
	etl::loose_handle<Instance> prev_instance = selected_instance;

	selected_canvas_view.reset();
	if (prev)
		prev->deactivate();

	selected_canvas_view = canvas_view;
	if (selected_canvas_view) {
		selected_instance = canvas_view->get_instance();
		selected_canvas_view->activate();
	} else {
		selected_instance.reset();
	}

	signal_canvas_view_focus()(selected_canvas_view);
	if (selected_instance != prev_instance)
		signal_instance_selected()(selected_instance);
}

etl::loose_handle<Instance>
App::get_instance(Canvas::Handle canvas)
{
	if(!canvas) return nullptr;
	canvas=canvas->get_root();

	for (const auto& instance : instance_list) {
		if(instance->get_canvas()==canvas)
			return instance;
	}
	return nullptr;
}

Gamma
App::get_selected_canvas_gamma()
{
	if (CanvasView::LooseHandle canvas_view = App::get_selected_canvas_view())
		return canvas_view->get_canvas()->rend_desc().get_gamma();
	return Gamma();
}

void
App::dialog_about()
{
	if(about)
		about->show();
}

void
studio::App::undo()
{
	if(selected_instance)
		selected_instance->undo();
}

void
studio::App::redo()
{
	if(selected_instance)
		selected_instance->redo();
}

Gtk::Box*
studio::App::scale_imported_box()
{
	Gtk::Box *box = manage(new Gtk::Box);
	Gtk::Label *label_resize = manage(new Gtk::Label(_("Scale to fit Canvas")));
	Gtk::Switch *toggle_resize = manage(new Gtk::Switch);

	label_resize->set_margin_end(5);
	toggle_resize->set_valign(Gtk::ALIGN_CENTER);
	toggle_resize->set_active(App::resize_imported_images);

	toggle_resize->property_active().signal_changed().connect(
		sigc::mem_fun(*App::dialog_setup, &studio::Dialog_Setup::on_resize_imported_changed));

	box->pack_start(*label_resize, false, false);
	box->pack_end(*toggle_resize, false, false);
	box->set_tooltip_text(_("Check this to scale imported images to Canvas size"));
	box->show_all();

	return box;
}

synfig::String
studio::App::get_base_path()
{
	return FileSystem::fix_slashes(app_base_path_);
}

void
studio::App::setup_changed()
{
	for (const auto& instance : instance_list) {
		for (const auto& canvas_interface : instance->canvas_interface_list()) {
			canvas_interface->signal_rend_desc_changed()();
		}
	}
}

void
studio::App::process_all_events()
{
	const auto& ctx = Glib::MainContext::get_default();
	while(ctx->pending()) {
		ctx->iteration(false);
	}
}

bool
studio::App::check_python_version(const std::string& path)
{
	std::string working_directory, std_out, std_err;
	int exit_status;
	const char* err_msg = _("Failed to detect Python 3 executable:\n Error: %s");
	const std::vector<std::string> argv = {path, "--version"};
	try {
		Glib::spawn_sync(working_directory, argv, Glib::SPAWN_SEARCH_PATH, Glib::SlotSpawnChildSetup(), &std_out, &std_err, &exit_status);
	} catch (const Glib::SpawnError& e) {
		synfig::error(err_msg, e.what().c_str());
		return false;
	} catch (const Glib::ShellError& e) {
		synfig::error(err_msg, e.what().c_str());
		return false;
	}

	// Output is like: "Python 3.3.0"
	if (!exit_status && std_out.find("Python 3") != std::string::npos) {
		return true;
	}
	synfig::warning(err_msg, (std_out + '\n' + std_err).c_str());
	return false;
}
