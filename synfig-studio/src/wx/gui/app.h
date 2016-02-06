/* === S Y N F I G ========================================================= */
/*!	\file app.h
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008, 2013 Carlos López
**	Copyright (c) 2012 Konstantin Dmitriev
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

#ifndef __SYNFIG_STUDIO_APP_H
#define __SYNFIG_STUDIO_APP_H

/* === H E A D E R S ======================================================= */

#include <sigc++/bind.h>

#include <gtkmm/main.h>
#include <string>
#include <list>

#include <ETL/smart_ptr>

#include <synfig/distance.h>
#include <synfig/string.h>
#include <synfig/time.h>

#include <synfig/filecontainerzip.h>

#include <gtkmm/uimanager.h>

#include <synfigapp/instance.h>
#include <synfigapp/pluginmanager.h>
#include "iconcontroller.h"

/* === M A C R O S ========================================================= */

#define MISC_DIR_PREFERENCE			"misc_dir"
#define ANIMATION_DIR_PREFERENCE	"animation_dir"
#define IMAGE_DIR_PREFERENCE		"image_dir"
#define SKETCH_DIR_PREFERENCE		"sketch_dir"
#define RENDER_DIR_PREFERENCE		"render_dir"

// uncomment define SINGLE_THREADED to use a single thread, and hopefully get more stability (dooglus) - changed to be default if using windows (Nov 2009 pixelgeek)
#ifdef WIN32
#define SINGLE_THREADED
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk
{
	class UIManager;
	class ActionGroup;
};

namespace synfigapp
{
	class UIInterface;
	class Main;
};

class Preferences;

namespace studio {

typedef Gtk::UIManager UIManager;

class About;
class MainWindow;
class Instance;
class CanvasView;
class Dialog_Setup;
class Dialog_Gradient;
class Dialog_Input;
class Dialog_Color;
class Dialog_ToolOptions;
class DeviceTracker;
class AutoRecover;

class DockManager;

class Dock_Toolbox;
class Dock_History;
class Dock_Canvases;

class Dock_Keyframes;
class Dock_Params;
class Dock_Layers;
class Dock_MetaData;
class Dock_Children;
class Dock_Info;
class Dock_Navigator;
class Dock_LayerGroups;
class IPC;

class Module;

class StateManager;
class IconController;

class App : public Gtk::Main, private IconController
{
	friend class Preferences;
	friend class Dialog_Setup;

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	struct Busy
	{
		static int count;
		Busy(){count++;}
		~Busy(){count--;}
	};

	enum Response
	{
		RESPONSE_ACCEPT_WITH_HISTORY = 1
	};


	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:
	//static etl::handle<synfigapp::UIInterface> ui_interface_;
	//static int max_recent_files;

/*      //declated as globals in app.cpp
	static Dock_Keyframes *dock_keyframes;
	static Dock_Layers *dock_layers;
	static Dock_Params *dock_params;
	static Dock_MetaData *dock_meta_data;
	static Dock_Children *dock_children;
	static Dock_Info *dock_info;
	static Dock_Navigator *dock_navigator;
	static Dock_History *dock_history;
	static Dock_Canvases *dock_canvases;
	static Dock_LayerGroups *dock_layer_groups;

	static IPC *ipc;
*/

	etl::smart_ptr<synfigapp::Main> synfigapp_main;


	static etl::handle<Instance> selected_instance;
	static etl::handle<CanvasView> selected_canvas_view;

	static Glib::RefPtr<UIManager>	ui_manager_;

	static int jack_locks_;

//	static std::list< etl::handle< Module > > module_list_;

	/*
 -- ** -- P U B L I C   D A T A -----------------------------------------------
	*/

public:
	static Dialog_Input* dialog_input;

	static DeviceTracker*	device_tracker;
	static AutoRecover*	auto_recover;
	static DockManager* dock_manager;

	static DockManager* get_dock_manager() { return dock_manager; }

	static Dialog_Setup* dialog_setup;
	static Dialog_Gradient* dialog_gradient;
	static Dialog_Color* dialog_color;
//	static Dialog_Palette* dialog_palette;
	static Dialog_ToolOptions *dialog_tool_options;

	static synfig::Distance::System distance_system;

	static synfig::Gamma gamma;

	static About *about;
	static MainWindow *main_window;
	static Dock_Toolbox *dock_toolbox;

	static std::list<etl::handle<Instance> > instance_list;

	static bool shutdown_in_progress;

	static bool use_colorspace_gamma;
	
#ifdef SINGLE_THREADED
	static bool single_threaded;
#endif

	static bool restrict_radius_ducks;
	static bool resize_imported_images;
	static bool enable_experimental_features;
	static bool use_dark_theme;

	static synfigapp::PluginManager plugin_manager;
	
	static synfig::String browser_command;
	static synfig::String brushes_path;
	static synfig::String custom_filename_prefix;
	static int preferred_x_size;
	static int preferred_y_size;
	static synfig::String predefined_size;
	static synfig::String predefined_fps;
	static float preferred_fps;
	static synfig::String sequence_separator;
	static bool navigator_uses_cairo;
	static bool workarea_uses_cairo;
	static bool enable_mainwin_menubar;
	static synfig::String ui_language;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/
/*      //declated as globals in app.cpp
	static sigc::signal<
		void,
		etl::loose_handle<CanvasView>
	> signal_canvas_view_focus_;
	static sigc::signal<
		void,
		etl::handle<Instance>
	> signal_instance_selected_;
	static sigc::signal<
		void,
		etl::handle<Instance>
	> signal_instance_created_;
	static sigc::signal<
		void,
		etl::handle<Instance>
	> signal_instance_deleted_;
	static sigc::signal<void> signal_recent_files_changed_;
	static sigc::signal<void> signal_present_all_;
*/
public:

	static sigc::signal<void> &signal_present_all();

	static sigc::signal<void> &signal_recent_files_changed();

	static sigc::signal<
		void,
		etl::loose_handle<CanvasView>
	>& signal_canvas_view_focus();

	static sigc::signal<
		void,
		etl::handle<Instance>
	> &signal_instance_selected();

	static sigc::signal<
		void,
		etl::handle<Instance>
	> &signal_instance_created();

	static sigc::signal<
		void,
		etl::handle<Instance>
	> &signal_instance_deleted();

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	static void add_recent_file(const std::string &filename);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	App(const synfig::String& basepath, int *argc, char ***argv);
	virtual ~App();

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	static StateManager* get_state_manager();

	static Glib::RefPtr<UIManager>& ui_manager() { return ui_manager_; }

	static void add_recent_file(const etl::handle<Instance> instance);

	static synfig::String get_base_path();
	static void save_settings();
	static bool load_settings(const synfig::String& key_filter = "");
	static void load_accel_map();
	static void load_file_window_size();
	static void load_language_settings();
	static void set_workspace_default();
	static void set_workspace_compositing();
	static void set_workspace_animating();
	static void restore_default_settings();
	static void apply_gtk_settings(bool);

	static const std::list<std::string>& get_recent_files();

	static const etl::handle<synfigapp::UIInterface>& get_ui_interface();


	static void set_selected_instance(etl::loose_handle<Instance> instance);
	static void set_selected_canvas_view(etl::loose_handle<CanvasView>);

	static etl::loose_handle<Instance> get_instance(etl::handle<synfig::Canvas> canvas);

	static etl::loose_handle<Instance> get_selected_instance() { return selected_instance; }
	static etl::loose_handle<CanvasView> get_selected_canvas_view() { return selected_canvas_view; }

	static bool open(std::string filename);

	static bool open_as(std::string filename,std::string as,synfig::FileContainerZip::file_size_t truncate_storage_size = 0);

	static bool open_from_temporary_container_as(std::string container_filename_base,std::string as);

	static void new_instance();

	static void dialog_open(std::string filename = "");

	static void dialog_about();

	static void quit();

	static void show_setup();

	static void undo();
	static void redo();

	static int get_max_recent_files();
	static void set_max_recent_files(int x);

	static bool jack_is_locked();
	static void jack_lock();
	static void jack_unlock();

	static synfig::Time::Format get_time_format();
	static void set_time_format(synfig::Time::Format x);

	static bool shutdown_request(GdkEventAny*bleh=NULL);

//	static bool dialog_file(const std::string &title, std::string &filename);

	static bool dialog_open_file(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_open_file_spal(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_open_file_sketch(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_open_file_image(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_open_file_audio(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_open_file_with_history_button(const std::string &title, std::string &filename, bool &show_history, std::string preference);
	static bool dialog_save_file(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_save_file_spal(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_save_file_sketch(const std::string &title, std::string &filename, std::string preference);
	static bool dialog_save_file_render(const std::string &title, std::string &filename, std::string preference);

	static bool dialog_select_list_item(const std::string &title, const std::string &message, const std::list<std::string> &list, int &item_index);

	static bool dialog_entry(const std::string &action, const std::string &content, std::string &text, const std::string &button1, const std::string &button2);

	static bool dialog_paragraph(const std::string &title, const std::string &message,std::string &text);

	static void dialog_not_implemented();

	static void dialog_help();

	static void dialog_message_1b(
			const std::string &type,
			const std::string &message,
			const std::string &detials,
			const std::string &button1,
			const std::string &long_details = "long_details");

	static bool dialog_message_2b(const std::string &message,
			const std::string &details,
			const Gtk::MessageType &type,
			const std::string &button1,
			const std::string &button2);

	static int dialog_message_3b(const std::string &message,
			const std::string &details,
			const Gtk::MessageType &type,
			const std::string &button1,
			const std::string &button2,
			const std::string &button3);

	static void open_url(const std::string &url);

	static synfig::String get_config_file(const synfig::String& file);
	// This will spread the changes made in preferences.
	// (By now it updates the System Units or Time Format for all the canvases).
	// This fixes bug 1890020
	static void setup_changed();
}; // END of class App

	void delete_widget(Gtk::Widget *widget);

}; // END namespace studio

/* === E N D =============================================================== */

#endif
