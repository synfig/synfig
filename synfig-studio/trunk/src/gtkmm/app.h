/* === S I N F G =========================================================== */
/*!	\file app.h
**	\brief writeme
**
**	$Id: app.h,v 1.2 2005/01/13 21:11:16 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_APP_H
#define __SINFG_STUDIO_APP_H

/* === H E A D E R S ======================================================= */

#include <sigc++/compatibility.h>
#include <sigc++/bind.h>

#include <gtkmm/main.h>
#include <string>
#include <list>

#include <ETL/smart_ptr>

#include <sinfg/distance.h>
#include <sinfg/string.h>
#include <sinfg/time.h>

#include <gtkmm/uimanager.h>

#include <sinfgapp/instance.h>
#include "iconcontroler.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk
{
	class InputDialog;
	class UIManager;
	class ActionGroup;
};

namespace sinfgapp
{
	class UIInterface;
	class Main;
};

class Preferences;
	
namespace studio {

typedef Gtk::UIManager UIManager;

class Toolbox;
class Instance;
class CanvasView;
class Dialog_Setup;
class Dialog_Gradient;
class Dialog_Color;
class Dialog_ToolOptions;
class DeviceTracker;
class AutoRecover;

class DockManager;

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
class IconControler;
	
class App : public Gtk::Main, private IconControler
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


	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:
	//static etl::handle<sinfgapp::UIInterface> ui_interface_;
	//static int max_recent_files;

/*
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

	etl::smart_ptr<sinfgapp::Main> sinfgapp_main;


	static etl::handle<Instance> selected_instance;
	static etl::handle<CanvasView> selected_canvas_view;

	static Glib::RefPtr<UIManager>	ui_manager_;
	
//	static std::list< etl::handle< Module > > module_list_;

	/*
 -- ** -- P U B L I C   D A T A -----------------------------------------------
	*/

public:
	static Gtk::InputDialog* dialog_input;

	static DeviceTracker*	device_tracker;
	static AutoRecover*	auto_recover;
	static DockManager* dock_manager;

	static DockManager* get_dock_manager() { return dock_manager; }

	static Dialog_Setup* dialog_setup;
	static Dialog_Gradient* dialog_gradient;
	static Dialog_Color* dialog_color;
//	static Dialog_Palette* dialog_palette;
	static Dialog_ToolOptions *dialog_tool_options;

	static sinfg::Distance::System distance_system;

	static sinfg::Gamma gamma;

	static Toolbox *toolbox;

	static std::list<etl::handle<Instance> > instance_list;

	static bool shutdown_in_progress;	

	static bool use_colorspace_gamma;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/
/*
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

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	App(int *argc, char ***argv);
	virtual ~App();

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	static StateManager* get_state_manager();

	static Glib::RefPtr<UIManager>& ui_manager() { return ui_manager_; }

	static void add_recent_file(const std::string &filename);

	static sinfg::String get_base_path();
	static void save_settings();
	static void load_settings();

	static const std::list<std::string>& get_recent_files();

	static const etl::handle<sinfgapp::UIInterface>& get_ui_interface();


	static void set_selected_instance(etl::loose_handle<Instance> instance);
	static void set_selected_canvas_view(etl::loose_handle<CanvasView>);

	static etl::loose_handle<Instance> get_instance(etl::handle<sinfg::Canvas> canvas);

	static etl::loose_handle<Instance> get_selected_instance() { return selected_instance; }
	static etl::loose_handle<CanvasView> get_selected_canvas_view() { return selected_canvas_view; }

	static bool open(std::string filename);

	static bool open_as(std::string filename,std::string as);

	static void new_instance();

	static void dialog_open();

	static void dialog_about();
	
	static void quit();
	
	static void show_setup();

	static void undo();
	static void redo();
	
	static int get_max_recent_files();
	static void set_max_recent_files(int x);


	static sinfg::Time::Format get_time_format();
	static void set_time_format(sinfg::Time::Format x);

	static bool shutdown_request(GdkEventAny*bleh=NULL);
	
//	static bool dialog_file(const std::string &title, std::string &filename);

	static bool dialog_open_file(const std::string &title, std::string &filename);
	static bool dialog_save_file(const std::string &title, std::string &filename);
	static bool dialog_saveas_file(const std::string &title, std::string &filename);

	static void dialog_error_blocking(const std::string &title, const std::string &message);

	static void dialog_warning_blocking(const std::string &title, const std::string &message);

	static bool dialog_entry(const std::string &title, const std::string &message,std::string &text);

	static bool dialog_yes_no(const std::string &title, const std::string &message);

	static int dialog_yes_no_cancel(const std::string &title, const std::string &message);
	
	static void dialog_not_implemented();

	static sinfg::String get_user_app_directory();
	static sinfg::String get_config_file(const sinfg::String& file);
}; // END of class App

}; // END namespace studio

/* === E N D =============================================================== */

#endif
