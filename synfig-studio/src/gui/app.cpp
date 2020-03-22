/* === S Y N F I G ========================================================= */
/*!	\file app.cpp
**	\brief writeme
**
**	$Id$
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

#include <fstream>
#include <iostream>
#include <locale>
#include <cstring>

#ifdef __OpenBSD__
#include <errno.h>
#elif defined(HAVE_SYS_ERRNO_H)
#include <sys/errno.h>
#endif
#include <gtkmm/accelmap.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/iconsource.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/stockitem.h>
#include <gtkmm/textview.h>
#include <gtkmm/uimanager.h>

#include <glibmm/main.h>
#include <glibmm/miscutils.h>
#include <glibmm/spawn.h>
#include <glibmm/thread.h>
#include <glibmm/timer.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gdkmm/general.h>

#ifdef _WIN32
#define WINVER 0x0500
#include <windows.h>
#endif

#include <synfig/general.h>
#include <synfig/color.h>
#include <synfig/canvasfilenaming.h>
#include <synfig/filesystemnative.h>
#include <synfig/filesystemgroup.h>
#include <synfig/filesystemtemporary.h>
#include <synfig/importer.h>
#include <synfig/loadcanvas.h>
#include <synfig/savecanvas.h>
#include <synfig/layer.h>
#include <synfig/soundprocessor.h>

#include "app.h"
#include "splash.h"
#include "instance.h"
#include "canvasview.h"
#include "dialogs/about.h"
#include "dialogs/dialog_color.h"
#include "dialogs/dialog_gradient.h"
#include "dialogs/dialog_input.h"
#include "dialogs/dialog_setup.h"
#include "dialogs/dialog_workspaces.h"
#include "dialogs/vectorizersettings.h"
#include "onemoment.h"
#include "devicetracker.h"
#include "widgets/widget_enum.h"

#include <synfigapp/canvasinterface.h>
#include <synfigapp/actions/layersetdesc.h>

#include "statemanager.h"

#include "states/state_bline.h"
#include "states/state_brush.h"
#include "states/state_circle.h"
#include "states/state_draw.h"
#include "states/state_eyedrop.h"
#include "states/state_fill.h"
#include "states/state_gradient.h"
#include "states/state_lasso.h"
#include "states/state_mirror.h"
#include "states/state_normal.h"
#include "states/state_polygon.h"
#include "states/state_rectangle.h"
#include "states/state_rotate.h"
#include "states/state_scale.h"
#include "states/state_sketch.h"
#include "states/state_smoothmove.h"
#include "states/state_star.h"
#include "states/state_text.h"
#include "states/state_width.h"
#include "states/state_zoom.h"

#include "autorecover.h"

#include <synfigapp/settings.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/action.h>

#include "docks/dockmanager.h"
#include "docks/dialog_tooloptions.h"
#include "docks/dock_canvases.h"
#include "docks/dock_children.h"
#include "docks/dock_curves.h"
#include "docks/dock_history.h"
#include "docks/dock_info.h"
#include "docks/dock_keyframes.h"
#include "docks/dock_layers.h"
#include "docks/dock_layergroups.h"
#include "docks/dock_params.h"
#include "docks/dock_metadata.h"
#include "docks/dock_navigator.h"
#include "docks/dock_soundwave.h"
#include "docks/dock_timetrack.h"
#include "docks/dock_timetrack2.h"
#include "docks/dock_toolbox.h"

#include "modules/module.h"
#include "modules/mod_palette/mod_palette.h"

#include "ipc.h"

#include <gui/localization.h>

#include "gui/resourcehelper.h"
#include "gui/workspacehandler.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef DPM2DPI
#define DPM2DPI(x)	(float(x)/39.3700787402f)
#define DPI2DPM(x)	(float(x)*39.3700787402f)
#endif

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"tif"
#endif

#ifdef _WIN32
#	ifdef PLUGIN_DIR
#		undef PLUGIN_DIR
#		define PLUGIN_DIR "share\\synfig\\plugins"
#	endif
#endif

#ifndef PLUGIN_DIR
#	define PLUGIN_DIR "/usr/local/share/synfig/plugins"
#endif

#ifdef _WIN32
#	ifdef SOUND_DIR
#		undef SOUND_DIR
#		define SOUND_DIR "share\\synfig\\sounds"
#	endif
#endif

#ifndef SOUND_DIR
#	define SOUND_DIR "/usr/local/share/synfig/sounds"
#endif

#include <synfigapp/main.h>

/* === S I G N A L S ======================================================= */

static sigc::signal<void> signal_present_all_;
sigc::signal<void>&
App::signal_present_all() { return signal_present_all_; }

static sigc::signal<void> signal_recent_files_changed_;
sigc::signal<void>&
App::signal_recent_files_changed() { return signal_recent_files_changed_; }

static sigc::signal<void> signal_custom_workspaces_changed_;
sigc::signal<void>&
App::signal_custom_workspaces_changed()
{
	return signal_custom_workspaces_changed_;
}

static sigc::signal<void,etl::loose_handle<CanvasView> > signal_canvas_view_focus_;
sigc::signal<void,etl::loose_handle<CanvasView> >&
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

static std::list<std::string>           recent_files;
const  std::list<std::string>& App::get_recent_files() { return recent_files; }

const std::vector<std::string>
App::get_workspaces()
{
	std::vector<std::string> list;
	if (workspaces)
		workspaces->get_name_list(list);
	return list;
}

int	 App::Busy::count;
bool App::shutdown_in_progress;

Glib::RefPtr<studio::UIManager>	App::ui_manager_;

int        App::jack_locks_ = 0;
Dock_Info* App::dock_info_  = 0;

synfig::Distance::System  App::distance_system;

studio::Dialog_Setup     *App::dialog_setup;

etl::handle< studio::ModPalette > mod_palette_;
//studio::Dialog_Palette* App::dialog_palette;

std::list<etl::handle<Instance> > App::instance_list;

static etl::handle<synfigapp::UIInterface>           ui_interface_;
const  etl::handle<synfigapp::UIInterface>& App::get_ui_interface() { return ui_interface_; }

etl::handle<Instance>   App::selected_instance;
etl::handle<CanvasView> App::selected_canvas_view;

studio::About              *studio::App::about          = NULL;
studio::MainWindow         *studio::App::main_window    = NULL;
studio::Dock_Toolbox       *studio::App::dock_toolbox   = NULL;
studio::AutoRecover        *studio::App::auto_recover   = NULL;
studio::IPC                *ipc                         = NULL;
studio::DockManager        *studio::App::dock_manager   = 0;
studio::DeviceTracker      *studio::App::device_tracker = 0;

studio::Dialog_Gradient    *studio::App::dialog_gradient;
studio::Dialog_Color       *studio::App::dialog_color;
studio::Dialog_Input       *studio::App::dialog_input;
studio::Dialog_ToolOptions *studio::App::dialog_tool_options;
studio::VectorizerSettings *studio::App::vectorizerpopup;

studio::Dock_History       *dock_history;
studio::Dock_Canvases      *dock_canvases;
studio::Dock_Keyframes     *dock_keyframes;
studio::Dock_Layers        *dock_layers;
studio::Dock_Params        *dock_params;
studio::Dock_MetaData      *dock_meta_data;
studio::Dock_Children      *dock_children;
studio::Dock_Info          *dock_info;
studio::Dock_LayerGroups   *dock_layer_groups;
studio::Dock_Navigator     *dock_navigator;
studio::Dock_SoundWave     *dock_soundwave;
studio::Dock_Timetrack_Old     *dock_timetrack_old;
studio::Dock_Timetrack2    *dock_timetrack;
studio::Dock_Curves        *dock_curves;

std::list< etl::handle< studio::Module > > module_list_;

bool   studio::App::restrict_radius_ducks        = true;
bool   studio::App::resize_imported_images       = false;
bool   studio::App::enable_experimental_features = false;
bool   studio::App::use_dark_theme               = false;
bool   studio::App::show_file_toolbar            = true;
String studio::App::custom_filename_prefix       (DEFAULT_FILENAME_PREFIX);
int    studio::App::preferred_x_size             = 480;
int    studio::App::preferred_y_size             = 270;
String studio::App::predefined_size              (DEFAULT_PREDEFINED_SIZE);
String studio::App::predefined_fps               (DEFAULT_PREDEFINED_FPS);
float  studio::App::preferred_fps                = 24.0;
PluginManager studio::App::plugin_manager;
std::set< String >       studio::App::brushes_path;
String studio::App::image_editor_path;

String studio::App::sequence_separator(".");
String studio::App::navigator_renderer;
String studio::App::workarea_renderer;

String        studio::App::default_background_layer_type  = "none";
synfig::Color studio::App::default_background_layer_color =
	synfig::Color(1.000000, 1.000000, 1.000000, 1.000000);  //White
String        studio::App::default_background_layer_image = "undefined";
synfig::Color studio::App::preview_background_color =
	synfig::Color(0.742187, 0.742187, 0.742187, 1.000000);  //X11 Gray

bool   studio::App::enable_mainwin_menubar = true;
String studio::App::ui_language ("os_LANG");
long   studio::App::ui_handle_tooltip_flag(Duck::STRUCT_DEFAULT);

static int max_recent_files_=25;
int    studio::App::get_max_recent_files()      { return max_recent_files_; }
void   studio::App::set_max_recent_files(int x) {        max_recent_files_ = x; }

static synfig::String app_base_path_;

studio::WorkspaceHandler *studio::App::workspaces = nullptr;

namespace studio {

bool
really_delete_widget(Gtk::Widget *widget)
{
	// synfig::info("really delete %p", (void*)widget);
	delete widget;
	return false;
}

// nasty workaround - when we've finished with a popup menu, we want to delete it
// attaching to the signal_hide() signal gets us here before the action on the menu has run,
// so schedule the real delete to happen in 50ms, giving the action a chance to run
void
delete_widget(Gtk::Widget *widget)
{
	// synfig::info("delete %p", (void*)widget);
	Glib::signal_timeout().connect(sigc::bind(sigc::ptr_fun(&really_delete_widget), widget), 50);
}

//Static members need to be initialized outside of class declaration
SoundProcessor *App::sound_render_done = NULL;
bool App::use_render_done_sound = true;

}; // END of namespace studio
studio::StateManager* state_manager;




class GlobalUIInterface : public synfigapp::UIInterface
{
public:

	virtual Response confirmation(
			const std::string &message,
			const std::string &details,
			const std::string &cancel,
			const std::string &confirm,
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
		return (Response) dialog.run();
	}


	virtual Response yes_no_cancel(
				const std::string &message,
				const std::string &details,
				const std::string &button1,
				const std::string &button2,
				const std::string &button3,
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
		return (Response)dialog.run();
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

/* === P R O C E D U R E S ================================================= */

/*
void
studio::UIManager::insert_action_group (const Glib::RefPtr<Gtk::ActionGroup>& action_group, int pos)
{
	action_group_list.push_back(action_group);
	Gtk::UIManager::insert_action_group(action_group, pos);
}

void
studio::UIManager::remove_action_group (const Glib::RefPtr<Gtk::ActionGroup>& action_group)
{
	std::list<Glib::RefPtr<Gtk::ActionGroup> >::iterator iter;
	for(iter=action_group_list.begin();iter!=action_group_list.end();++iter)
		if(*iter==action_group)
		{
			action_group_list.erase(iter);
			Gtk::UIManager::remove_action_group(action_group);
			return;
		}
	synfig::error("Unable to find action group");
}

void
studio::add_action_group_to_top(Glib::RefPtr<studio::UIManager> ui_manager, Glib::RefPtr<Gtk::ActionGroup> group)
{
	ui_manager->insert_action_group(group,0);
	return;
	std::list<Glib::RefPtr<Gtk::ActionGroup> > prev_groups(ui_manager->get_action_groups());
	std::list<Glib::RefPtr<Gtk::ActionGroup> >::reverse_iterator iter;

	for(iter=prev_groups.rbegin();iter!=prev_groups.rend();++iter)
	{
		if(*iter && (*iter)->get_name()!="menus")
		{
			synfig::info("Removing action group "+(*iter)->get_name());
			ui_manager->remove_action_group(*iter);
		}
	}
	ui_manager->insert_action_group(group,0);

	for(;!prev_groups.empty();prev_groups.pop_front())
	{
		if(prev_groups.front() && prev_groups.front()!=group && prev_groups.front()->get_name()!="menus")
			ui_manager->insert_action_group(prev_groups.front(),1);
	}
}
*/
class Preferences : public synfigapp::Settings
{
public:
	virtual bool get_value(const synfig::String& key, synfig::String& value)const
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
				if(!App::brushes_path.empty())
					value=*(App::brushes_path.begin());
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
		return synfigapp::Settings::get_value(key,value);
	}

	virtual bool set_value(const synfig::String& key,const synfig::String& value)
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
		}
		catch(...)
		{
			synfig::warning("Preferences: Caught exception when attempting to set value.");
		}
		return synfigapp::Settings::set_value(key,value);
	}

	virtual KeyList get_key_list()const
	{
		KeyList ret(synfigapp::Settings::get_key_list());
		ret.push_back("time_format");
		ret.push_back("distance_system");
		ret.push_back("file_history.size");
		ret.push_back("autosave_backup");
		ret.push_back("autosave_backup_interval");
		ret.push_back("restrict_radius_ducks");
		ret.push_back("resize_imported_images");
		ret.push_back("enable_experimental_features");
		ret.push_back("use_dark_theme");
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
};

static ::Preferences _preferences;

void
init_ui_manager()
{
	Glib::RefPtr<Gtk::ActionGroup> menus_action_group = Gtk::ActionGroup::create("menus");

	Glib::RefPtr<Gtk::ActionGroup> actions_action_group = Gtk::ActionGroup::create("actions");

	menus_action_group->add( Gtk::Action::create("menu-file",            _("_File")));
	menus_action_group->add( Gtk::Action::create("menu-open-recent",     _("Open Recent")));

	menus_action_group->add( Gtk::Action::create("menu-edit",            _("_Edit")));

	menus_action_group->add( Gtk::Action::create("menu-view",            _("_View")));
	menus_action_group->add( Gtk::Action::create("menu-navigation",            _("_Navigation")));
	menus_action_group->add( Gtk::Action::create("menu-duck-mask",       _("Show/Hide Handles")));
	menus_action_group->add( Gtk::Action::create("menu-preview-quality", _("Preview Quality")));
	menus_action_group->add( Gtk::Action::create("menu-lowres-pixel",    _("Low-Res Pixel Size")));

	menus_action_group->add( Gtk::Action::create("menu-canvas",          _("_Canvas")));

	menus_action_group->add( Gtk::Action::create("menu-layer",           _("_Layer")));
	menus_action_group->add( Gtk::Action::create("menu-layer-new",       _("New Layer")));
	menus_action_group->add( Gtk::Action::create("menu-toolbox",         _("Toolbox")));
	menus_action_group->add( Gtk::Action::create("menu-plugins",         _("Plug-Ins")));

	menus_action_group->add( Gtk::Action::create("menu-window",          _("_Window")));
	menus_action_group->add( Gtk::Action::create("menu-arrange",         _("_Arrange")));
	menus_action_group->add( Gtk::Action::create("menu-workspace",       _("Work_space")));

	menus_action_group->add( Gtk::Action::create("menu-help",            _("_Help")));

	menus_action_group->add(Gtk::Action::create("menu-keyframe",          _("Keyframe")));

	// Add the synfigapp actions (layer panel toolbar items, etc...)
	synfigapp::Action::Book::iterator iter;
	for(iter=synfigapp::Action::book().begin();iter!=synfigapp::Action::book().end();++iter)
	{
		actions_action_group->add(Gtk::Action::create(
			"action-"+iter->second.name,
			get_action_stock_id(iter->second),
			iter->second.local_name,iter->second.local_name
		));
	}

// predefined actions to initial menu items, so that there is all menu items listing
// even there is no any canvas instance existed, for example, when app just opened.
// the menu items (action names) should be named consistently with those in canvasview.cpp and others.
#define DEFINE_ACTION(x,stock) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock) ); actions_action_group->add(action); }

// actions in File menu
DEFINE_ACTION("new",            Gtk::StockID("synfig-new"));
DEFINE_ACTION("open",           Gtk::StockID("synfig-open"));
DEFINE_ACTION("save",           Gtk::StockID("synfig-save"));
DEFINE_ACTION("save-as",        Gtk::StockID("synfig-save_as"));
DEFINE_ACTION("save-all",       Gtk::StockID("synfig-save_all"));
DEFINE_ACTION("revert",         Gtk::Stock::REVERT_TO_SAVED);
DEFINE_ACTION("import",         _("Import..."));
DEFINE_ACTION("import-sequence",_("Import Sequence..."));
DEFINE_ACTION("render",         _("Render..."));
DEFINE_ACTION("preview",        _("Preview..."));
DEFINE_ACTION("close-document", _("Close Document"));
DEFINE_ACTION("quit",           Gtk::Stock::QUIT);

// actions in Edit menu
DEFINE_ACTION("undo",                     Gtk::StockID("synfig-undo"));
DEFINE_ACTION("redo",                     Gtk::StockID("synfig-redo"));
DEFINE_ACTION("copy",                     Gtk::Stock::COPY);
DEFINE_ACTION("cut",                      Gtk::Stock::CUT);
DEFINE_ACTION("paste",                    Gtk::Stock::PASTE);
DEFINE_ACTION("select-all-ducks",         _("Select All Handles"));
DEFINE_ACTION("unselect-all-ducks",       _("Unselect All Handles"));
DEFINE_ACTION("select-all-layers",        _("Select All Layers"));
DEFINE_ACTION("unselect-all-layers",      _("Unselect All Layers"));
DEFINE_ACTION("input-devices",            _("Input Devices..."));
DEFINE_ACTION("setup",                    _("Preferences..."));

// actions in View menu
DEFINE_ACTION("toggle-mainwin-menubar",   _("Menubar"));
DEFINE_ACTION("toggle-mainwin-toolbar",   _("Toolbar"));

DEFINE_ACTION("mask-none-ducks",                _("Toggle None/Last visible Handles"));
DEFINE_ACTION("mask-position-ducks",            _("Show Position Handles"));
DEFINE_ACTION("mask-vertex-ducks",              _("Show Vertex Handles"));
DEFINE_ACTION("mask-tangent-ducks",             _("Show Tangent Handles"));
DEFINE_ACTION("mask-radius-ducks",              _("Show Radius Handles"));
DEFINE_ACTION("mask-width-ducks",               _("Show Width Handles"));
DEFINE_ACTION("mask-widthpoint-position-ducks", _("Show WidthPoints Position Handles"));
DEFINE_ACTION("mask-angle-ducks",               _("Show Angle Handles"));
DEFINE_ACTION("mask-bone-setup-ducks",          _("Show Bone Setup Handles"));
DEFINE_ACTION("mask-bone-recursive-ducks",      _("Show Recursive Scale Bone Handles"));
DEFINE_ACTION("mask-bone-ducks",                _("Next Bone Handles"));

DEFINE_ACTION("quality-00", _("Use Parametric Renderer"));
DEFINE_ACTION("quality-01", _("Use Quality Level 1"));
DEFINE_ACTION("quality-02", _("Use Quality Level 2"));
DEFINE_ACTION("quality-03", _("Use Quality Level 3"));
DEFINE_ACTION("quality-04", _("Use Quality Level 4"));
DEFINE_ACTION("quality-05", _("Use Quality Level 5"));
DEFINE_ACTION("quality-06", _("Use Quality Level 6"));
DEFINE_ACTION("quality-07", _("Use Quality Level 7"));
DEFINE_ACTION("quality-08", _("Use Quality Level 8"));
DEFINE_ACTION("quality-09", _("Use Quality Level 9"));
DEFINE_ACTION("quality-10", _("Use Quality Level 10"));

for(list<int>::iterator iter = CanvasView::get_pixel_sizes().begin(); iter != CanvasView::get_pixel_sizes().end(); iter++)
  DEFINE_ACTION(strprintf("lowres-pixel-%d", *iter), strprintf(_("Set Low-Res pixel size to %d"), *iter));

DEFINE_ACTION("toggle-grid-show",  _("Toggle Grid Show"));
DEFINE_ACTION("toggle-grid-snap",  _("Toggle Grid Snap"));
DEFINE_ACTION("toggle-guide-show", _("Toggle Guide Show"));
DEFINE_ACTION("toggle-guide-snap", _("Toggle Guide Snap"));
DEFINE_ACTION("toggle-low-res",    _("Toggle Low-Res"));

DEFINE_ACTION("decrease-low-res-pixel-size", _("Decrease Low-Res Pixel Size"));
DEFINE_ACTION("increase-low-res-pixel-size", _("Increase Low-Res Pixel Size"));
DEFINE_ACTION("toggle-background-rendering", _("Toggle Background Rendering"));
DEFINE_ACTION("toggle-onion-skin",           _("Toggle Onion Skin"));
DEFINE_ACTION("canvas-zoom-in",              Gtk::StockID("gtk-zoom-in"));
DEFINE_ACTION("canvas-zoom-out",             Gtk::StockID("gtk-zoom-out"));
DEFINE_ACTION("canvas-zoom-fit",             Gtk::StockID("gtk-zoom-fit"));
DEFINE_ACTION("canvas-zoom-100",             Gtk::StockID("gtk-zoom-100"));
DEFINE_ACTION("time-zoom-in",                Gtk::StockID("gtk-zoom-in"));
DEFINE_ACTION("time-zoom-out",               Gtk::StockID("gtk-zoom-out"));

//actions in Navigation menu
DEFINE_ACTION("play", _("Play"));
// the stop is not a normal stop but a pause. So use "Pause" in UI, including TEXT and
// icon. the internal code is still using stop.
DEFINE_ACTION("stop", _("Pause"));

DEFINE_ACTION("jump-next-keyframe",          _("Seek to Next Keyframe"));
DEFINE_ACTION("jump-prev-keyframe",          _("Seek to previous Keyframe"));
DEFINE_ACTION("seek-next-frame",             _("Seek to Next Frame"));
DEFINE_ACTION("seek-prev-frame",             _("Seek to Previous Frame"));
DEFINE_ACTION("seek-next-second",            _("Seek Forward"));
DEFINE_ACTION("seek-prev-second",            _("Seek Backward"));
DEFINE_ACTION("seek-begin",                  _("Seek to Begin"));
DEFINE_ACTION("seek-end",                    _("Seek to End"));


// actions in Canvas menu
DEFINE_ACTION("properties", _("Properties..."));
DEFINE_ACTION("options",    _("Options..."));

// actions in Layer menu
DEFINE_ACTION("amount-inc", _("Increase Layer Amount"))
DEFINE_ACTION("amount-dec", _("Decrease Layer Amount"))

// actions in Window menu
DEFINE_ACTION("workspace-compositing", _("Compositing"));
DEFINE_ACTION("workspace-default",     _("Default"));
DEFINE_ACTION("workspace-animating",   _("Animating"));
DEFINE_ACTION("save-workspace",        _("Save workspace..."));
DEFINE_ACTION("dialog-flipbook",       _("Preview Dialog"));
DEFINE_ACTION("panel-toolbox",         _("Toolbox"));
DEFINE_ACTION("panel-tool_options",    _("Tool Options"));
DEFINE_ACTION("panel-history",         _("History"));
DEFINE_ACTION("panel-canvases",        _("Canvas Browser"));
DEFINE_ACTION("panel-keyframes",       _("Keyframes"));
DEFINE_ACTION("panel-layers",          _("Layers"));
DEFINE_ACTION("panel-params",          _("Parameters"));
DEFINE_ACTION("panel-meta_data",       _("Canvas MetaData"));
DEFINE_ACTION("panel-children",        _("Library"));
DEFINE_ACTION("panel-info",            _("Info"));
DEFINE_ACTION("panel-navigator",       _("Navigator"));
DEFINE_ACTION("panel-timetrack-old",   _("Timetrack (old)"));
DEFINE_ACTION("panel-curves",          _("Graphs"));
DEFINE_ACTION("panel-groups",          _("Sets"));
DEFINE_ACTION("panel-pal_edit",        _("Palette Editor"));
DEFINE_ACTION("panel-soundwave",       _("Sound"));
DEFINE_ACTION("panel-timetrack",      _("Timetrack"));

// actions in Help menu
DEFINE_ACTION("help",           Gtk::Stock::HELP);
DEFINE_ACTION("help-tutorials", Gtk::Stock::HELP);
DEFINE_ACTION("help-reference", Gtk::Stock::HELP);
DEFINE_ACTION("help-faq",       Gtk::Stock::HELP);
DEFINE_ACTION("help-support",   Gtk::Stock::HELP);
DEFINE_ACTION("help-about",     Gtk::StockID("synfig-about"));

// actions: Keyframe
DEFINE_ACTION("keyframe-properties", _("Properties"));


//Layout the actions in the main menu (caret menu, right click on canvas menu) and toolbar:
	Glib::ustring ui_info_menu =
"	<menu action='menu-file'>"
"		<menuitem action='new' />"
"		<menuitem action='open' />"
"		<menu action='menu-open-recent' />"
"		<separator name='sep-file1'/>"
"		<menuitem action='save' />"
"		<menuitem action='save-as' />"
"		<menuitem action='save-all' />"
"		<menuitem action='revert' />"
"		<separator name='sep-file2'/>"
"		<menuitem action='import' />"
"		<menuitem action='import-sequence' />"
"		<separator name='sep-file4'/>"
"		<menuitem action='preview' />"
"		<menuitem action='render' />"
"		<separator name='sep-file5'/>"
"		<menuitem action='close-document' />"
"		<separator name='sep-file6'/>"
"		<menuitem action='quit' />"
"	</menu>"
"	<menu action='menu-edit'>"
"		<menuitem action='undo'/>"
"		<menuitem action='redo'/>"
"		<separator name='sep-edit1'/>"
"		<menuitem action='cut'/>"
"		<menuitem action='copy'/>"
"		<menuitem action='paste'/>"
"		<separator name='sep-edit2'/>"
"		<menuitem action='select-all-layers'/>"
"		<menuitem action='unselect-all-layers'/>"
"		<menuitem action='select-all-ducks'/>"
"		<menuitem action='unselect-all-ducks'/>"
"		<separator name='sep-edit3'/>"
"		<menuitem action='input-devices' />"
"		<menuitem action='setup' />"
"	</menu>"
"	<menu action='menu-view'>"
"		<menuitem action='toggle-mainwin-menubar' />"
"		<menuitem action='toggle-mainwin-toolbar' />"
"		<separator />"
"		<menu action='menu-duck-mask'>"
"			<menuitem action='mask-none-ducks' />"
"			<menuitem action='mask-position-ducks' />"
"			<menuitem action='mask-vertex-ducks' />"
"			<menuitem action='mask-tangent-ducks' />"
"			<menuitem action='mask-radius-ducks' />"
"			<menuitem action='mask-width-ducks' />"
"			<menuitem action='mask-widthpoint-position-ducks' />"
"			<menuitem action='mask-angle-ducks' />"
"			<menuitem action='mask-bone-setup-ducks' />"
"			<menuitem action='mask-bone-recursive-ducks' />"
"			<menuitem action='mask-bone-ducks' />"
"		</menu>"
"		<menu action='menu-preview-quality'>"
"			<menuitem action='quality-00' />"
"			<menuitem action='quality-01' />"
"			<menuitem action='quality-02' />"
"			<menuitem action='quality-03' />"
"			<menuitem action='quality-04' />"
"			<menuitem action='quality-05' />"
"			<menuitem action='quality-06' />"
"			<menuitem action='quality-07' />"
"			<menuitem action='quality-08' />"
"			<menuitem action='quality-09' />"
"			<menuitem action='quality-10' />"
"		</menu>"
"		<menu action='menu-lowres-pixel'>"
"			<menuitem action='decrease-low-res-pixel-size'/>"
"			<menuitem action='increase-low-res-pixel-size'/>"
"			<separator name='pixel-size-separator'/>"
;

	for(list<int>::iterator iter = CanvasView::get_pixel_sizes().begin(); iter != CanvasView::get_pixel_sizes().end(); iter++)
		ui_info_menu += strprintf("			<menuitem action='lowres-pixel-%d' />", *iter);

	ui_info_menu +=
"		</menu>"
"		<separator name='sep-view1'/>"
"		<menuitem action='toggle-grid-show'/>"
"		<menuitem action='toggle-grid-snap'/>"
"		<menuitem action='toggle-guide-show'/>"
"		<menuitem action='toggle-guide-snap'/>"
"		<menuitem action='toggle-low-res'/>"
"		<menuitem action='toggle-background-rendering'/>"
"		<menuitem action='toggle-onion-skin'/>"
"		<separator name='sep-view2'/>"
"		<menuitem action='canvas-zoom-in'/>"
"		<menuitem action='canvas-zoom-out'/>"
"		<menuitem action='canvas-zoom-fit'/>"
"		<menuitem action='canvas-zoom-100'/>"
"		<separator name='sep-view3'/>"
"		<menuitem action='time-zoom-in'/>"
"		<menuitem action='time-zoom-out'/>"
"	</menu>"
"    <menu action='menu-navigation'>"
"		<menuitem action='play'/>"
"		<menuitem action='stop'/>"
"		<separator name='sep-view1'/>"
"		<menuitem action='jump-prev-keyframe'/>"
"		<menuitem action='jump-next-keyframe'/>"
"		<menuitem action='seek-prev-frame'/>"
"		<menuitem action='seek-next-frame'/>"
"		<menuitem action='seek-prev-second'/>"
"		<menuitem action='seek-next-second'/>"
"		<menuitem action='seek-begin'/>"
"		<menuitem action='seek-end'/>"
"	</menu>"
"	<menu action='menu-canvas'>"
"		<menuitem action='properties'/>"
"		<menuitem action='options'/>"
"	</menu>"
"	<menu action='menu-toolbox'>"
"	</menu>"
"	<menu action='menu-layer'>"
"		<menu action='menu-layer-new'></menu>"
"		<menuitem action='amount-inc'/>"
"		<menuitem action='amount-dec'/>"
"	</menu>"
"	<menu action='menu-plugins'>"
;

	list<PluginManager::plugin> plugin_list = studio::App::plugin_manager.get_list();
	for(list<PluginManager::plugin>::const_iterator p=plugin_list.begin();p!=plugin_list.end();++p) {

		// TODO: (Plugins) Arrange menu items into groups

		PluginManager::plugin plugin = *p;

		DEFINE_ACTION(plugin.id, plugin.name);
		ui_info_menu += strprintf("	<menuitem action='%s'/>", plugin.id.c_str());
	}

	ui_info_menu +=
"	</menu>"
"	<menu action='menu-window'>"
"		<menu action='menu-arrange'> </menu>"
"		<menu action='menu-workspace'>"
"			<menuitem action='workspace-default' />"
"			<menuitem action='workspace-compositing' />"
"			<menuitem action='workspace-animating' />"
"		</menu>"
"		<separator />"
"		<menuitem action='dialog-flipbook'/>"
"		<menuitem action='panel-toolbox' />"
"		<menuitem action='panel-tool_options' />"
"		<menuitem action='panel-history' />"
"		<menuitem action='panel-canvases' />"
"		<menuitem action='panel-keyframes' />"
"		<menuitem action='panel-layers' />"
"		<menuitem action='panel-params' />"
"		<menuitem action='panel-meta_data' />"
"		<menuitem action='panel-children' />"
"		<menuitem action='panel-info' />"
"		<menuitem action='panel-navigator' />"
"		<menuitem action='panel-timetrack-old' />"
"		<menuitem action='panel-timetrack' />"
"		<menuitem action='panel-curves' />"
"		<menuitem action='panel-groups' />"
"		<menuitem action='panel-pal_edit' />"
"		<menuitem action='panel-soundwave' />"
"		<separator />"
// opened documents will be listed here below the above separator.
"	</menu>"
"	<menu action='menu-help'>"
"		<menuitem action='help'/>"
"		<separator name='sep-help1'/>"
"		<menuitem action='help-tutorials'/>"
"		<menuitem action='help-reference'/>"
"		<menuitem action='help-faq'/>"
"		<separator name='sep-help2'/>"
"		<menuitem action='help-support'/>"
"		<separator name='sep-help3'/>"
"		<menuitem action='help-about'/>"
"	</menu>";

	Glib::ustring ui_info_main_tool =
"		<toolitem action='new'/>"
"		<toolitem action='open'/>"
"		<toolitem action='save'/>"
"		<toolitem action='save-as'/>"
"		<toolitem action='save-all'/>"
"		<separator />"
"		<toolitem action='undo'/>"
"		<toolitem action='redo'/>"
"		<separator />"
"		<toolitem action='render'/>"
"		<toolitem action='preview'/>";

	Glib::ustring ui_info =
"<ui>"
"   <popup name='menu-toolbox' action='menu-toolbox'>"
"	<menu action='menu-file'>"
"	</menu>"
"	</popup>"
"	<popup name='menu-main' action='menu-main'>" + ui_info_menu + "</popup>"
"	<menubar name='menubar-main' action='menubar-main'>" + ui_info_menu + "</menubar>"
"	<toolbar name='toolbar-main'>" + ui_info_main_tool + "</toolbar>"
"</ui>";

	#undef DEFINE_ACTION

	try
	{
		actions_action_group->set_sensitive(false);
		App::ui_manager()->set_add_tearoffs(false);
		App::ui_manager()->insert_action_group(menus_action_group,1);
		App::ui_manager()->insert_action_group(actions_action_group,1);
		App::ui_manager()->add_ui_from_string(ui_info);

		//App::ui_manager()->get_accel_group()->unlock();
	}
	catch(const Glib::Error& ex)
	{
		synfig::error("building menus and toolbars failed: " + ex.what());
	}

	// Add default keyboard accelerators
#define ACCEL(accel,path)						\
	{											\
		Gtk::AccelKey accel_key(accel,path);	\
		Gtk::AccelMap::add_entry(accel_key.get_path(), accel_key.get_key(), accel_key.get_mod());	\
	}

#define ACCEL2(accel)							\
	{											\
		Gtk::AccelKey accel_key(accel);			\
		Gtk::AccelMap::add_entry(accel_key.get_path(), accel_key.get_key(), accel_key.get_mod());	\
	}

	// the toolbox
	ACCEL("<Mod1>a",								"<Actions>/action_group_state_manager/state-normal"			);
	ACCEL("<Mod1>v",								"<Actions>/action_group_state_manager/state-smooth_move"		);
	ACCEL("<Mod1>s",								"<Actions>/action_group_state_manager/state-scale"			);
	ACCEL("<Mod1>t",								"<Actions>/action_group_state_manager/state-rotate"			);
	ACCEL("<Mod1>m",								"<Actions>/action_group_state_manager/state-mirror"			);
	ACCEL("<Mod1>c",								"<Actions>/action_group_state_manager/state-circle"			);
	ACCEL("<Mod1>r",								"<Actions>/action_group_state_manager/state-rectangle"			);
	ACCEL("<Mod1>q",								"<Actions>/action_group_state_manager/state-star"			);
	ACCEL("<Mod1>g",								"<Actions>/action_group_state_manager/state-gradient"			);
	ACCEL("<Mod1>p",								"<Actions>/action_group_state_manager/state-polygon"			);
	ACCEL("<Mod1>b",								"<Actions>/action_group_state_manager/state-bline"			);
	ACCEL("<Mod1>x",								"<Actions>/action_group_state_manager/state-text"			);
	ACCEL("<Mod1>f",								"<Actions>/action_group_state_manager/state-fill"			);
	ACCEL("<Mod1>e",								"<Actions>/action_group_state_manager/state-eyedrop"			);
	ACCEL("<Mod1>z",								"<Actions>/action_group_state_manager/state-zoom"			);
	ACCEL("<Mod1>d",								"<Actions>/action_group_state_manager/state-draw"			);
	ACCEL("<Mod1>k",								"<Actions>/action_group_state_manager/state-sketch"			);
	ACCEL("<Mod1>w",								"<Actions>/action_group_state_manager/state-width"			);

	// everything else
	ACCEL("<Control>a",								"<Actions>/canvasview/select-all-ducks"					);
	ACCEL("<Control>d",								"<Actions>/canvasview/unselect-all-ducks"				);
	ACCEL("<Control><Shift>a",							"<Actions>/canvasview/select-all-layers"				);
	ACCEL("<Control><Shift>d",							"<Actions>/canvasview/unselect-all-layers"				);
	ACCEL("F9",									"<Actions>/canvasview/render"						);
	ACCEL("F11",									"<Actions>/canvasview/preview"						);
	ACCEL("F8",									"<Actions>/canvasview/properties"					);
	ACCEL("F12",									"<Actions>/canvasview/options"						);
	ACCEL("<control>i",								"<Actions>/canvasview/import"						);
	//ACCEL2(Gtk::AccelKey(GDK_KEY_Escape,static_cast<Gdk::ModifierType>(0), 		"<Actions>/canvasview/stop"						));
	ACCEL2(Gtk::AccelKey(GDK_KEY_Escape, Gdk::ModifierType(), 		"<Actions>/canvasview/stop"						));
	ACCEL("<Control>g",								"<Actions>/canvasview/toggle-grid-show"					);
	ACCEL("<Control>l",								"<Actions>/canvasview/toggle-grid-snap"					);
	ACCEL("<Control>n",								"<Actions>/mainwindow/new"						);
	ACCEL("<Control>o",								"<Actions>/mainwindow/open"						);
	ACCEL("<Control>s",								"<Actions>/canvasview/save"						);
	ACCEL("<Control><Shift>s",							"<Actions>/canvasview/save-as"						);
	ACCEL2(Gtk::AccelKey('`',Gdk::CONTROL_MASK,					"<Actions>/canvasview/toggle-low-res"					));
	ACCEL("<Mod1>0",                                                    		"<Actions>/canvasview/mask-none-ducks"          			);
	ACCEL("<Mod1>1",								"<Actions>/canvasview/mask-position-ducks"				);
	ACCEL("<Mod1>2",								"<Actions>/canvasview/mask-vertex-ducks"				);
	ACCEL("<Mod1>3",								"<Actions>/canvasview/mask-tangent-ducks"				);
	ACCEL("<Mod1>4",								"<Actions>/canvasview/mask-radius-ducks"				);
	ACCEL("<Mod1>5",								"<Actions>/canvasview/mask-width-ducks"					);
	ACCEL("<Mod1>6",								"<Actions>/canvasview/mask-angle-ducks"					);
	ACCEL("<Mod1>7",								"<Actions>/canvasview/mask-bone-setup-ducks"				);
	ACCEL("<Mod1>8",								"<Actions>/canvasview/mask-bone-recursive-ducks"			);
	ACCEL("<Mod1>9",								"<Actions>/canvasview/mask-bone-ducks"					);
	ACCEL("<Mod1>5",								"<Actions>/canvasview/mask-widthpoint-position-ducks"			);
	ACCEL2(Gtk::AccelKey(GDK_KEY_Page_Up,Gdk::SHIFT_MASK,				"<Actions>/action_group_layer_action_manager/action-LayerRaise"		));
	ACCEL2(Gtk::AccelKey(GDK_KEY_Page_Down,Gdk::SHIFT_MASK,				"<Actions>/action_group_layer_action_manager/action-LayerLower"		));
	ACCEL("<Control>1",								"<Actions>/canvasview/quality-01"					);
	ACCEL("<Control>2",								"<Actions>/canvasview/quality-02"					);
	ACCEL("<Control>3",								"<Actions>/canvasview/quality-03"					);
	ACCEL("<Control>4",								"<Actions>/canvasview/quality-04"					);
	ACCEL("<Control>5",								"<Actions>/canvasview/quality-05"					);
	ACCEL("<Control>6",								"<Actions>/canvasview/quality-06"					);
	ACCEL("<Control>7",								"<Actions>/canvasview/quality-07"					);
	ACCEL("<Control>8",								"<Actions>/canvasview/quality-08"					);
	ACCEL("<Control>9",								"<Actions>/canvasview/quality-09"					);
	ACCEL("<Control>0",								"<Actions>/canvasview/quality-10"					);
	ACCEL("<Control>z",								"<Actions>/action_group_dock_history/undo"				);
	ACCEL("<Control>r",								"<Actions>/action_group_dock_history/redo"				);
	ACCEL2(Gtk::AccelKey(GDK_KEY_Delete,Gdk::ModifierType(),				"<Actions>/action_group_layer_action_manager/action-LayerRemove"	));
	ACCEL2(Gtk::AccelKey('(',Gdk::CONTROL_MASK,					"<Actions>/canvasview/decrease-low-res-pixel-size"			));
	ACCEL2(Gtk::AccelKey(')',Gdk::CONTROL_MASK,					"<Actions>/canvasview/increase-low-res-pixel-size"			));
	ACCEL2(Gtk::AccelKey('(',Gdk::MOD1_MASK|Gdk::CONTROL_MASK,			"<Actions>/action_group_layer_action_manager/amount-dec"		));
	ACCEL2(Gtk::AccelKey(')',Gdk::MOD1_MASK|Gdk::CONTROL_MASK,			"<Actions>/action_group_layer_action_manager/amount-inc"		));
	ACCEL2(Gtk::AccelKey(']',Gdk::CONTROL_MASK,					"<Actions>/canvasview/jump-next-keyframe"				));
	ACCEL2(Gtk::AccelKey('[',Gdk::CONTROL_MASK,					"<Actions>/canvasview/jump-prev-keyframe"				));
	ACCEL2(Gtk::AccelKey('=',Gdk::CONTROL_MASK,					"<Actions>/canvasview/canvas-zoom-in"					));
	ACCEL2(Gtk::AccelKey('-',Gdk::CONTROL_MASK,					"<Actions>/canvasview/canvas-zoom-out"					));
	ACCEL2(Gtk::AccelKey('+',Gdk::CONTROL_MASK,					"<Actions>/canvasview/time-zoom-in"					));
	ACCEL2(Gtk::AccelKey('_',Gdk::CONTROL_MASK,					"<Actions>/canvasview/time-zoom-out"					));
	ACCEL2(Gtk::AccelKey('.',Gdk::CONTROL_MASK,					"<Actions>/canvasview/seek-next-frame"					));
	ACCEL2(Gtk::AccelKey(',',Gdk::CONTROL_MASK,					"<Actions>/canvasview/seek-prev-frame"					));
	ACCEL2(Gtk::AccelKey('>',Gdk::CONTROL_MASK,					"<Actions>/canvasview/seek-next-second"					));
	ACCEL2(Gtk::AccelKey('<',Gdk::CONTROL_MASK,					"<Actions>/canvasview/seek-prev-second"					));
	ACCEL("<Mod1>o",								"<Actions>/canvasview/toggle-onion-skin"				);
	ACCEL("<Control><Shift>z",							"<Actions>/canvasview/canvas-zoom-fit"					);
	ACCEL("<Control>p",								"<Actions>/canvasview/play"						);


#undef ACCEL
#undef ACCEL2
}

#ifdef _WIN32
#define mkdir(x,y) mkdir(x)
#endif

/* === M E T H O D S ======================================================= */

App::App(const synfig::String& basepath, int *argc, char ***argv):
	Gtk::Main(argc,argv)
{

	Glib::init(); // need to use Gio functions before app is started
	app_base_path_=etl::dirname(basepath);

	// Set ui language
	load_language_settings();
	if (ui_language != "os_LANG")
	{
		Glib::setenv ("LANGUAGE",  App::ui_language.c_str(), 1);
	}

	// paths
#ifdef _WIN32
	String path_to_plugins = get_base_path()
		+ ETL_DIRECTORY_SEPARATOR + PLUGIN_DIR;
	String path_to_sounds = get_base_path()
		+ ETL_DIRECTORY_SEPARATOR + SOUND_DIR;
#else
	String path_to_plugins = PLUGIN_DIR;
	String path_to_sounds = SOUND_DIR;
#endif

	String path_to_icons = ResourceHelper::get_image_path();

	if (char* synfig_root = getenv("SYNFIG_ROOT")) {
		path_to_plugins = String(synfig_root)
			+ ETL_DIRECTORY_SEPARATOR + "share"
			+ ETL_DIRECTORY_SEPARATOR + "synfig"
			+ ETL_DIRECTORY_SEPARATOR + "plugins";
		path_to_sounds = String(synfig_root)
			+ ETL_DIRECTORY_SEPARATOR + "share"
			+ ETL_DIRECTORY_SEPARATOR + "synfig"
			+ ETL_DIRECTORY_SEPARATOR + "sounds";
	}

	String path_to_user_plugins = synfigapp::Main::get_user_app_directory()
		+ ETL_DIRECTORY_SEPARATOR + "plugins";
	
	// icons
	init_icons(path_to_icons + ETL_DIRECTORY_SEPARATOR);

	ui_interface_=new GlobalUIInterface();

	// don't call thread_init() if threads are already initialized
	// on some machines bonobo_init() initialized threads before we get here
	if (!g_thread_supported())
		Glib::thread_init();

	distance_system=Distance::SYSTEM_PIXELS;
	
#ifdef _WIN32
	// Do not show "No disc in drive" errors
	// - https://github.com/synfig/synfig/issues/489
	// - https://github.com/synfig/synfig/issues/724
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif

	if(mkdir(synfigapp::Main::get_user_app_directory().c_str(),ACCESSPERMS)<0)
	{
		if(errno!=EEXIST)
			synfig::error("UNABLE TO CREATE \"%s\"",synfigapp::Main::get_user_app_directory().c_str());
	}
	else
	{
		synfig::info("Created directory \"%s\"",synfigapp::Main::get_user_app_directory().c_str());
	}


	ipc=new IPC();

	if(!SYNFIG_CHECK_VERSION())
	{
		cerr<<"FATAL: Synfig Version Mismatch"<<endl;
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
	try { synfigapp_main=etl::smart_ptr<synfigapp::Main>(new synfigapp::Main(basepath,&synfig_init_cb)); }
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
		plugin_manager.load_dir(path_to_user_plugins);

		studio_init_cb.task(_("Init UI Manager..."));
		App::ui_manager_=studio::UIManager::create();
		init_ui_manager();

		studio_init_cb.task(_("Init Dock Manager..."));
		dock_manager=new studio::DockManager();

		studio_init_cb.task(_("Init State Manager..."));
		state_manager=new StateManager();

		studio_init_cb.task(_("Init Main Window..."));
		main_window=new studio::MainWindow();
		main_window->add_accel_group(App::ui_manager_->get_accel_group());

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
		workspaces = new WorkspaceHandler();
		workspaces->signal_list_changed().connect( sigc::mem_fun(signal_custom_workspaces_changed_, &sigc::signal<void>::emit) );
		load_custom_workspaces();

		studio_init_cb.task(_("Init auto recovery..."));
		auto_recover=new AutoRecover();

		studio_init_cb.amount_complete(9250,10000);
		studio_init_cb.task(_("Loading Settings..."));
		load_accel_map();
		if (!load_settings())
			set_workspace_default();
		if (!load_settings("workspace.layout"))
			set_workspace_default();
		load_file_window_size();

		// Init Tools must be done after load_accel_map() : accelerators keys
		// are displayed in toolbox labels
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
                state_manager->add_state(&state_lasso); // Enabled for now.  Let's see whether they're good enough yet.
		if(!getenv("SYNFIG_DISABLE_WIDTH"  )) state_manager->add_state(&state_width); // Enabled since 0.61.09
		state_manager->add_state(&state_fill);
		state_manager->add_state(&state_eyedrop);

		/* other */
		state_manager->add_state(&state_text);
		if(!getenv("SYNFIG_DISABLE_SKETCH" )) state_manager->add_state(&state_sketch);
		if(!getenv("SYNFIG_DISABLE_BRUSH"  ) && App::enable_experimental_features) state_manager->add_state(&state_brush);
		state_manager->add_state(&state_zoom);


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

		bool opened_any = false;
		if (!getenv("SYNFIG_DISABLE_AUTO_RECOVERY") && auto_recover->recovery_needed())
		{
			splash_screen.hide();
			if (get_ui_interface()->confirmation(
					_("Auto recovery file(s) found. Do you want to recover unsaved changes?"),
					_("Synfig Studio seems to have crashed before you could save all your files."),
					_("Ignore"),
					_("Recover")
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

				if (number_recovered)
					opened_any = true;
			}
			else
			{
				auto_recover->clear_backups();
			}
			splash_screen.show();
		}

		// Look for any files given on the command line,
		// and load them if found.
		for(;*argc>=1;(*argc)--)
			if((*argv)[*argc] && (*argv)[*argc][0]!='-')
			{
				studio_init_cb.task(_("Loading files..."));
				splash_screen.hide();
				open(Glib::locale_to_utf8((*argv)[*argc]));
				opened_any = true;
				splash_screen.show();
			}

		// if no file was specified to be opened, create a new document to help new users get started more easily
		if (!opened_any && !getenv("SYNFIG_DISABLE_AUTOMATIC_DOCUMENT_CREATION"))
			new_instance();

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
		dock_toolbox->present();

		splash_screen.hide();

		String message;
		String details;
		/*
		if (App::enable_experimental_features) {
			message = _("Following experimental features are enabled: ");
			message += ("Skeleton Layer");
			detials = _("The experimental features are NOT intended for production use. "
					"It is quite posiible their functionality will change in the "
					"future versions, which can break compatibility for your "
					"files. Use for testing purposes only. You can disable "
					"experimental features on the \"Misc\" tab of Setup dialog.");
		}
		*/
#ifdef _WIN32
		if (message!=""){
			message = _("There is a bug, which can cause computer to hang/freeze when "
					"resizing the canvas window.");
			details = _("If you got affected by this issue, consider pressing ALT+TAB "
					"to unfreeze your system and get it back to the working "
					"state. Please accept our apologies for inconvenience, we "
					"hope to get this issue resolved in the future versions.");
		}
#endif
		if (message!="")
			dialog_message_1b("WARNING",
					message,
					details,
					_("Got it"));
	}
	catch(String &x)
	{
		get_ui_interface()->error(_("Unknown exception caught when constructing App.\nThis software may be unstable.") + String("\n\n") + x);
	}
	catch(...)
	{
		get_ui_interface()->error(_("Unknown exception caught when constructing App.\nThis software may be unstable."));
	}

	// Load sound effects
	sound_render_done = new SoundProcessor();
	sound_render_done->addSound(
		SoundProcessor::PlayOptions(),
		SoundProcessor::Sound(path_to_sounds + ETL_DIRECTORY_SEPARATOR + "renderdone.wav") );
	
	App::dock_info_ = dock_info;
}

StateManager* App::get_state_manager() { return state_manager; }

App::~App()
{
	shutdown_in_progress=true;

	save_settings();

	synfigapp::Main::settings().remove_domain("pref");

	selected_instance=0;

	// Unload all of the modules
	for(;!module_list_.empty();module_list_.pop_back())
		;

	delete state_manager;

	delete ipc;

	delete auto_recover;

	delete about;

	main_window->hide();

	delete main_window;

	delete dialog_setup;

	delete dialog_gradient;

	delete dialog_color;

	delete dialog_input;

	delete dock_manager;

	delete workspaces;

	instance_list.clear();

	if (sound_render_done) delete sound_render_done;
	sound_render_done = NULL;
}

synfig::String
App::get_config_file(const synfig::String& file)
{
	return Glib::build_filename(synfigapp::Main::get_user_app_directory(),file);
}

void
App::add_recent_file(const etl::handle<Instance> instance)
{
	add_recent_file(absolute_path(instance->get_file_name()));
}

void
App::add_recent_file(const std::string &file_name)
{
	std::string filename(FileSystem::fix_slashes(file_name));

	assert(!filename.empty());

	if(filename.empty())
		return;

	// Toss out any "hidden" files
	if(basename(filename)[0]=='.')
		return;

	// If we aren't an absolute path, turn ourselves into one
	if(!is_absolute_path(filename))
		filename=absolute_path(filename);

	list<string>::iterator iter;
	// Check to see if the file is already on the list.
	// If it is, then remove it from the list
	for(iter=recent_files.begin();iter!=recent_files.end();iter++)
		if(*iter==filename)
		{
			recent_files.erase(iter);
			break;
		}


	// Push the filename to the front of the list
	recent_files.push_front(filename);

	// Clean out the files at the end of the list.
	while(recent_files.size()>(unsigned)get_max_recent_files())
	{
		recent_files.pop_back();
	}

	signal_recent_files_changed_();

	return;
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
		for(std::list< etl::handle<Instance> >::const_iterator i = instance_list.begin(); i != instance_list.end(); ++i)
		{
			const Instance::CanvasViewList &views = (*i)->canvas_view_list();
			for(Instance::CanvasViewList::const_iterator j = views.begin(); j != views.end(); ++j)
				(*j)->jack_lock();
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
		for(std::list< etl::handle<Instance> >::const_iterator i = instance_list.begin(); i != instance_list.end(); ++i)
		{
			const Instance::CanvasViewList &views = (*i)->canvas_view_list();
			for(Instance::CanvasViewList::const_iterator j = views.begin(); j != views.end(); ++j)
				(*j)->jack_unlock();
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
			std::string filename=get_config_file("accelrc");
			Gtk::AccelMap::save(filename);
		}
		{
			std::string filename=get_config_file("language");

			std::ofstream file(filename.c_str());

			if(!file)
			{
				synfig::warning("Unable to save %s",filename.c_str());
			} else {
				file<<App::ui_language.c_str()<<endl;
			}
		}
		do{
			std::string filename=get_config_file("recentfiles");

			std::ofstream file(filename.c_str());

			if(!file)
			{
				synfig::warning("Unable to save %s",filename.c_str());
				break;
			}

			list<string>::reverse_iterator iter;

			for(iter=recent_files.rbegin();iter!=recent_files.rend();iter++)
				file<<(*iter).c_str()<<endl;
		}while(0);
		std::string filename=get_config_file("settings-1.4");
		synfigapp::Main::settings().save_to_file(filename);

		{
			std::string filename = get_config_file("workspaces");
			workspaces->save(filename);
		}
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
		std::string filename=get_config_file("settings-1.4");
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
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		{
			std::string filename=get_config_file("accelrc");
			Gtk::AccelMap::load(filename);
		}
	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load accel map settings.");
	}
}

void
App::load_file_window_size()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		{
			std::string filename=get_config_file("recentfiles");
			std::ifstream file(filename.c_str());

			while(file)
			{
				std::string recent_file;
				std::string recent_file_window_size;
				getline(file,recent_file);
				if(!recent_file.empty() && FileSystemNative::instance()->is_file(recent_file))
					add_recent_file(recent_file);
			}
		}

	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load window settings.");
	}
}

void
App::load_language_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		{
			std::string filename=get_config_file("language");
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
		synfig::warning("Caught exception when attempting to loading language settings.");
	}
}

void
App::set_workspace_default()
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
App::set_workspace_compositing()
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
App::set_workspace_animating()
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

void App::set_workspace_from_template(const string& tpl)
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
	dock_manager->load_layout_from_string(layout);
	dock_manager->show_all_dock_dialogs();
}

void App::set_workspace_from_name(const string& name)
{
	std::string tpl;
	bool ok = workspaces->get_workspace(name, tpl);
	if (!ok)
		return;
	set_workspace_from_template(tpl);
}

void App::load_custom_workspaces()
{
	workspaces->clear();
	std::string filename = get_config_file("workspaces");
	workspaces->load(filename);
}

static void trim_string(std::string &text)
{
	text.erase(text.begin(),
			   std::find_if(text.begin(), text.end(),
							[](int chr) { return !std::isspace(chr);})
			   );
	text.erase(std::find_if(text.rbegin(), text.rend(),
							[](int chr) { return !std::isspace(chr);}).base(),
			   text.end()
			   );
}

void App::save_custom_workspace()
{
	Gtk::MessageDialog dialog(*App::main_window, _("Type a name for this custom workspace:"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);

	dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	Gtk::Button * ok_button = dialog.add_button(_("Ok"), Gtk::RESPONSE_OK);
	ok_button->set_sensitive(false);

	Gtk::Entry * name_entry = Gtk::manage(new Gtk::Entry());
	name_entry->set_margin_left(16);
	name_entry->set_margin_right(16);
	name_entry->signal_changed().connect([&](){
		std::string name = name_entry->get_text();
		trim_string(name);
		bool has_equal_sign = name.find("=") != std::string::npos;
		ok_button->set_sensitive(!name.empty() && !has_equal_sign);
		if (ok_button->is_sensitive())
			ok_button->grab_default();
	});
	name_entry->signal_activate().connect(sigc::mem_fun(*ok_button, &Gtk::Button::clicked));

	dialog.get_content_area()->set_spacing(12);
	dialog.get_content_area()->add(*name_entry);

	ok_button->set_can_default(true);

	dialog.show_all();

	int response = dialog.run();
	if (response == Gtk::RESPONSE_CANCEL)
		return;

	std::string name = name_entry->get_text();
	trim_string(name);

	std::string tpl = dock_manager->save_layout_to_string();
	if (!workspaces->has_workspace(name))
		workspaces->add_workspace(name, tpl);
	else {
		Gtk::MessageDialog confirm_dlg(dialog, _("Do you want to overwrite this workspace?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
		if (confirm_dlg.run() == Gtk::RESPONSE_CANCEL)
			return;
		workspaces->set_workspace(name, tpl);
	}
}

void App::edit_custom_workspace_list()
{
	Dialog_Workspaces * dlg = Dialog_Workspaces::create(*App::main_window);
	if (!dlg) {
		synfig::warning("Can't load Dialog_Workspaces");
		return;
	}
	dlg->run();
	delete dlg;
}

void
App::restore_default_settings()
{
	ostringstream temp_time_format;
	temp_time_format << Time::FORMAT_FRAMES;
	synfigapp::Main::settings().set_value("pref.time_format",                    temp_time_format.str());

	synfigapp::Main::settings().set_value("pref.distance_system",                "pt");
	synfigapp::Main::settings().set_value("pref.file_history.size",              "25");
	synfigapp::Main::settings().set_value("pref.autosave_backup",                "1");
	synfigapp::Main::settings().set_value("pref.autosave_backup_interval",       "15000");
	synfigapp::Main::settings().set_value("pref.restrict_radius_ducks",          "1");
	synfigapp::Main::settings().set_value("pref.resize_imported_images",         "0");
	synfigapp::Main::settings().set_value("pref.enable_experimental_features",   "0");
	synfigapp::Main::settings().set_value("pref.use_dark_theme",                 "0");
	synfigapp::Main::settings().set_value("pref.show_file_toolbar",              "1");
	synfigapp::Main::settings().set_value("pref.brushes_path",                   "");
	synfigapp::Main::settings().set_value("pref.custom_filename_prefix",         DEFAULT_FILENAME_PREFIX);
	synfigapp::Main::settings().set_value("pref.ui_language",                    "os_LANG");
	synfigapp::Main::settings().set_value("pref.preferred_x_size",               "480");
	synfigapp::Main::settings().set_value("pref.preferred_y_size",               "270");
	synfigapp::Main::settings().set_value("pref.predefined_size",                DEFAULT_PREDEFINED_SIZE);
	synfigapp::Main::settings().set_value("pref.preferred_fps",                  "24.0");
	synfigapp::Main::settings().set_value("pref.predefined_fps",                 DEFAULT_PREDEFINED_FPS);
	synfigapp::Main::settings().set_value("pref.sequence_separator",             ".");
	synfigapp::Main::settings().set_value("pref.navigator_renderer",             "");
	synfigapp::Main::settings().set_value("pref.workarea_renderer",              "");
	synfigapp::Main::settings().set_value("pref.default_background_layer_type",  "none");
	synfigapp::Main::settings().set_value("pref.default_background_layer_color", "1.000000 1.000000 1.000000 1.000000"); //White
	synfigapp::Main::settings().set_value("pref.default_background_layer_image", "");
	synfigapp::Main::settings().set_value("pref.preview_background_color",       "0.742187 0.742187 0.742187 1.000000"); //X11 Gray
	synfigapp::Main::settings().set_value("pref.use_render_done_sound",          "1");
	synfigapp::Main::settings().set_value("pref.enable_mainwin_menubar",         "1");

	ostringstream temp_ui_handle_tooltip_flag;
	temp_ui_handle_tooltip_flag << Duck::STRUCT_DEFAULT;
	synfigapp::Main::settings().set_value("pref.ui_handle_tooltip_flag",         temp_ui_handle_tooltip_flag.str());

	synfigapp::Main::settings().set_value("pref.image_editor_path",              "");
}

void
App::apply_gtk_settings()
{
	GtkSettings *gtk_settings;
	gtk_settings = gtk_settings_get_default ();

	gchar *theme_name=getenv("SYNFIG_GTK_THEME");
	if(theme_name) {
		g_object_set (G_OBJECT (gtk_settings), "gtk-theme-name", theme_name, NULL);
	}

	// dark theme
	g_object_set (G_OBJECT (gtk_settings), "gtk-application-prefer-dark-theme", App::use_dark_theme, NULL);

	// enable menu icons
	g_object_set (G_OBJECT (gtk_settings), "gtk-menu-images", TRUE, NULL);

	// fix CSS
	Glib::ustring data;
	// Fix GtkPaned (big margin makes it hard to grab first keyframe))
	data += "GtkPaned { margin: 2px; }\n";
	// Fix #348: Synfig's Interface went Too Thick
	// following css works in gtk since 3.14:
	data += ".button                            { padding-left: 4px; padding-right: 4px; }\n";
	data += ".button                            { padding-top: 0px; padding-bottom: 0px; }\n";
	data += ".button *                          { padding-top: 4px; padding-bottom: 4px; }\n";
	data += ".button > GtkBox                   { padding-top: 0px; padding-bottom: 0px; }\n";
	data += ".button > GtkBox > *               { padding-top: 4px; padding-bottom: 4px; }\n";
	data += ".button > GtkLabel                 { padding-top: 0px; padding-bottom: 0px; }\n";
	data += "GtkComboBox > .button > GtkBox > * { padding-top: 0px; padding-bottom: 0px; }\n";
	data += ".entry                             { padding-top: 0px; padding-bottom: 0px; }\n";
	data += "progress, trough 					{ min-height: 20px; }\n";
#if GTKMM_MAJOR_VERSION < 3 || (GTKMM_MAJOR_VERSION == 3 && GTKMM_MINOR_VERSION < 22)
	// following css works in old versions of gtk
	data += "button { padding: 0px; }\n";
#else
	// following css works for gtk 3.22:
	data += "entry, spinbutton { min-height: 16px; }\n";
	data += "button { min-height: 16px; min-width: 16px; padding: 0px; }\n";
#endif
	data += "button > box { padding: 5px; }\n";
	data += "button > image { padding: 5px; }\n";
	data += "combobox > box > button > box { padding-top: 0px; padding-bottom: 0px; }\n";
	// Fix #810: Insetsetive context menus on OSX
	g_object_get (G_OBJECT (gtk_settings), "gtk-theme-name", &theme_name, NULL);
	if ( String(theme_name) == "Adwaita" )
		data += ".window-frame, .window-frame:backdrop { box-shadow: none; margin: 0; }\n";
	g_free(theme_name);

	if (!data.empty()) {
		Glib::RefPtr<Gtk::CssProvider> css = Gtk::CssProvider::create();
		try {
			css->load_from_data(data);
		} catch (Gtk::CssProviderError &e) {
			synfig::warning("Failed to load css rules. %s", e.what().c_str());
		}
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
		Gtk::StyleContext::add_provider_for_screen(screen,css, GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
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

	Gtk::Main::quit();

	get_ui_interface()->task(_("Quit Request sent"));
}

void
App::show_setup()
{
	dialog_setup->refresh();
	dialog_setup->show();
}

gint Signal_Open_Ok    (GtkWidget */*widget*/, int *val){*val=1; return 0;}
gint Signal_Open_Cancel(GtkWidget */*widget*/, int *val){*val=2; return 0;}

//#ifdef _WIN32
//#define USE_WIN32_FILE_DIALOGS 1
//#endif

#ifdef USE_WIN32_FILE_DIALOGS
static OPENFILENAME ofn={};
#endif

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#endif

bool
App::dialog_open_file(const std::string &title, std::string &filename, std::string preference)
{
	// info("App::dialog_open_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());
	// TODO: Win32 native dialod not ready yet
#ifdef USE_WIN32_FILE_DIALOGS
	static TCHAR szFilter[] = TEXT (_("All Files (*.*)\0*.*\0\0")) ;

	GdkWindow *gdkWinPtr=toolbox->get_window()->gobj();
	HINSTANCE hInstance=static_cast<HINSTANCE>(GetModuleHandle(NULL));
	HWND hWnd=static_cast<HWND>(GDK_WINDOW_HWND(gdkWinPtr));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = szFilter;
//	ofn.lpstrCustomFilter=NULL;
//	ofn.nMaxCustFilter=0;
//	ofn.nFilterIndex=0;
//	ofn.lpstrFile=NULL;
	ofn.nMaxFile=MAX_PATH;
//	ofn.lpstrFileTitle=NULL;
//	ofn.lpstrInitialDir=NULL;
//	ofn.lpstrTitle=NULL;
	ofn.Flags=OFN_HIDEREADONLY;
//	ofn.nFileOffset=0;
//	ofn.nFileExtension=0;
	ofn.lpstrDefExt=TEXT("sif");
//	ofn.lCustData = 0l;
	ofn.lpfnHook=NULL;
//	ofn.lpTemplateName=NULL;

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
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();
	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Import"), Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open",   Gtk::ICON_SIZE_BUTTON);

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
	filter_supported->add_pattern("*.png");
	filter_supported->add_pattern("*.jpeg");
	filter_supported->add_pattern("*.jpg");
	filter_supported->add_pattern("*.bmp");
	filter_supported->add_pattern("*.svg");
	filter_supported->add_pattern("*.lst");
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

	// Sub fileters
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
	filter_lipsync->set_name(_("Lipsync (*.pgo)"));
	filter_lipsync->add_pattern("*.pgo");

	// 6 Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	dialog->add_filter(filter_supported);
	dialog->add_filter(filter_synfig);
	dialog->add_filter(filter_image);
	dialog->add_filter(filter_image_list);
	dialog->add_filter(filter_audio);
	dialog->add_filter(filter_video);
	dialog->add_filter(filter_lipsync);
	dialog->add_filter(filter_any);

	if (filename.empty())
		dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
		dialog->set_filename(filename);
	else
		dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		// info("Saving preference %s = '%s' in App::dialog_open_file()", preference.c_str(), dirname(filename).c_str());
		_preferences.set_value(preference, dirname(filename));
		delete dialog;
		return true;
	}

	delete dialog;
	return false;
#endif   // not USE_WIN32_FILE_DIALOGS
}

bool
App::dialog_open_file_spal(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();
	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Load"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

	Glib::RefPtr<Gtk::FileFilter> filter_supported = Gtk::FileFilter::create();
	filter_supported->set_name(_("Palette files (*.spal, *.gpl)"));
	filter_supported->add_pattern("*.spal");
	filter_supported->add_pattern("*.gpl");
	dialog->add_filter(filter_supported);

	// show only Synfig color palette file (*.spal)
	Glib::RefPtr<Gtk::FileFilter> filter_spal = Gtk::FileFilter::create();
	filter_spal->set_name(_("Synfig palette files (*.spal)"));
	filter_spal->add_pattern("*.spal");
	dialog->add_filter(filter_spal);

	// ...and add GIMP color palette file too (*.gpl)
	Glib::RefPtr<Gtk::FileFilter> filter_gpl = Gtk::FileFilter::create();
	filter_gpl->set_name(_("GIMP palette files (*.gpl)"));
	filter_gpl->add_pattern("*.gpl");
	dialog->add_filter(filter_gpl);

	if (filename.empty())
	dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
	dialog->set_filename(filename);
	else
	dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, dirname(filename));
		delete dialog;
		return true;
	}

	delete dialog;
	return false;
}

bool
App::dialog_open_file_sketch(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();
	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Load"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

	// show only Synfig sketch file (*.sketch)
	Glib::RefPtr<Gtk::FileFilter> filter_sketch = Gtk::FileFilter::create();
	filter_sketch->set_name(_("Synfig sketch files (*.sketch)"));
	filter_sketch->add_pattern("*.sketch");
	dialog->add_filter(filter_sketch);

	if (filename.empty())
	dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
	dialog->set_filename(filename);
	else
	dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, dirname(filename));
		delete dialog;
		return true;
	}

	delete dialog;
	return false;
}


bool
App::dialog_open_file_image(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();

	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Load"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

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
	dialog->add_filter(filter_image);

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");
	dialog->add_filter(filter_any);

	if (filename.empty())
		dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
		dialog->set_filename(filename);
	else
		dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, dirname(filename));
		delete dialog;
		return true;
	}

	delete dialog;
	return false;
}


bool
App::dialog_open_file_audio(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();

	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Load"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

	// Audio files
	Glib::RefPtr<Gtk::FileFilter> filter_audio = Gtk::FileFilter::create();
	filter_audio->set_name(_("Audio (*.ogg, *.mp3, *.wav)"));
	filter_audio->add_mime_type("audio/x-vorbis+ogg");
	filter_audio->add_mime_type("audio/mpeg");
	filter_audio->add_mime_type("audio/x-wav");
	filter_audio->add_pattern("*.ogg");
	filter_audio->add_pattern("*.mp3");
	filter_audio->add_pattern("*.wav");
	dialog->add_filter(filter_audio);

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");
	dialog->add_filter(filter_any);

	if (filename.empty())
	dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
	dialog->set_filename(filename);
	else
	dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		filename = dialog->get_filename();
		_preferences.set_value(preference, dirname(filename));
		delete dialog;
		return true;
	}

	delete dialog;
	return false;
}

bool
App::dialog_open_file_image_sequence(const std::string &title, std::set<synfig::String> &filenames, std::string preference)
{
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();

	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->set_select_multiple(true);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Load"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

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
	dialog->add_filter(filter_image);

	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");
	dialog->add_filter(filter_any);

	std::string filename = filenames.empty() ? std::string() : *filenames.begin();
	if (filename.empty())
		dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
		dialog->set_filename(filename);
	else
		dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	filenames.clear();
	if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		std::vector<std::string> files = dialog->get_filenames();
		filenames.insert(files.begin(), files.end());
		_preferences.set_value(preference, dirname(dialog->get_filename()));
		delete dialog;
		return true;
	}

	delete dialog;
	return false;
}

void
on_open_dialog_with_history_selection_changed(Gtk::FileChooserDialog *dialog, Gtk::Button* history_button)
{
	// activate the history button when something is selected
	history_button->set_sensitive(!dialog->get_filename().empty());
}

bool
App::dialog_open_file_with_history_button(const std::string &title, std::string &filename, bool &show_history, std::string preference)
{
	// info("App::dialog_open_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());

// TODO: Win32 native dialog not ready yet
//#ifdef USE_WIN32_FILE_DIALOGS
#if 0
	static TCHAR szFilter[] = TEXT (_("All Files (*.*)\0*.*\0\0")) ;

	GdkWindow *gdkWinPtr=toolbox->get_window()->gobj();
	HINSTANCE hInstance=static_cast<HINSTANCE>(GetModuleHandle(NULL));
	HWND hWnd=static_cast<HWND>(GDK_WINDOW_HWND(gdkWinPtr));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = szFilter;
//	ofn.lpstrCustomFilter=NULL;
//	ofn.nMaxCustFilter=0;
//	ofn.nFilterIndex=0;
//	ofn.lpstrFile=NULL;
	ofn.nMaxFile=MAX_PATH;
//	ofn.lpstrFileTitle=NULL;
//	ofn.lpstrInitialDir=NULL;
//	ofn.lpstrTitle=NULL;
	ofn.Flags=OFN_HIDEREADONLY;
//	ofn.nFileOffset=0;
//	ofn.nFileExtension=0;
	ofn.lpstrDefExt=TEXT("sif");
//	ofn.lCustData = 0l;
	ofn.lpfnHook=NULL;
//	ofn.lpTemplateName=NULL;

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
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();

	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
				title, Gtk::FILE_CHOOSER_ACTION_OPEN);

		dialog->set_transient_for(*App::main_window);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Open"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);
	Gtk::Button* history_button = dialog->add_button(_("Open history"), RESPONSE_ACCEPT_WITH_HISTORY);
	// TODO: the Open history button should be file type sensitive one.
	dialog->set_response_sensitive(RESPONSE_ACCEPT_WITH_HISTORY, true);

	// File filters
	// Synfig Documents
	Glib::RefPtr<Gtk::FileFilter> filter_supported = Gtk::FileFilter::create();
	filter_supported->set_name(_("Synfig files (*.sif, *.sifz, *.sfg)"));
	filter_supported->add_mime_type("application/x-sif");
	filter_supported->add_pattern("*.sif");
	filter_supported->add_pattern("*.sifz");
	filter_supported->add_pattern("*.sfg");
	// Any files
	Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
	filter_any->set_name(_("Any files"));
	filter_any->add_pattern("*");

	dialog->add_filter(filter_supported);
	dialog->add_filter(filter_any);

	if (filename.empty())
		dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
		dialog->set_filename(filename);
	else
		dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

	// this ptr is't available to a static member fnc, connect to global function.
	sigc::connection connection_sc = dialog->signal_selection_changed().connect(sigc::bind(sigc::ptr_fun(on_open_dialog_with_history_selection_changed), dialog, history_button));

	int response = dialog->run();
	if (response == Gtk::RESPONSE_ACCEPT || response == RESPONSE_ACCEPT_WITH_HISTORY) {
		filename = dialog->get_filename();
		show_history = response == RESPONSE_ACCEPT_WITH_HISTORY;
		// info("Saving preference %s = '%s' in App::dialog_open_file()", preference.c_str(), dirname(filename).c_str());
		_preferences.set_value(preference, dirname(filename));
		delete dialog;
		return true;
	}

	connection_sc.disconnect();
	delete dialog;
	return false;
#endif   // not USE_WIN32_FILE_DIALOGS
}

bool
App::dialog_open_folder(const std::string &title, std::string &foldername, std::string preference, Gtk::Window& transientwind)
{
	synfig::String prev_path;
	synfigapp::Settings settings;
	if(settings.get_value(preference, prev_path))
		prev_path = ".";

	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,
			title, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

	dialog->set_transient_for(transientwind);
	dialog->set_current_folder(prev_path);
	dialog->add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog->add_button(_("Open"),   Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("gtk-open", Gtk::ICON_SIZE_BUTTON);

	if(dialog->run() == GTK_RESPONSE_ACCEPT)
	{
		foldername = dialog->get_filename();
		delete dialog;
		return true;
	}
	delete dialog;
	return false;
}


bool
App::dialog_save_file(const std::string &title, std::string &filename, std::string preference)
{
	// info("App::dialog_save_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());

#if USE_WIN32_FILE_DIALOGS
	static TCHAR szFilter[] = TEXT (_("All Files (*.*)\0*.*\0\0")) ;

	GdkWindow *gdkWinPtr=toolbox->get_window()->gobj();
	HINSTANCE hInstance=static_cast<HINSTANCE>(GetModuleHandle(NULL));
	HWND hWnd=static_cast<HWND>(GDK_WINDOW_HWND(gdkWinPtr));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = szFilter;
//	ofn.lpstrCustomFilter=NULL;
//	ofn.nMaxCustFilter=0;
//	ofn.nFilterIndex=0;
//	ofn.lpstrFile=NULL;
	ofn.nMaxFile=MAX_PATH;
//	ofn.lpstrFileTitle=NULL;
//	ofn.lpstrInitialDir=NULL;
//	ofn.lpstrTitle=NULL;
	ofn.Flags=OFN_OVERWRITEPROMPT;
//	ofn.nFileOffset=0;
//	ofn.nFileExtension=0;
	ofn.lpstrDefExt=TEXT("sif");
//	ofn.lCustData = 0l;
	ofn.lpfnHook=NULL;
//	ofn.lpTemplateName=NULL;

	CHAR szFilename[MAX_PATH];
	CHAR szTitle[500];
	strcpy(szFilename,filename.c_str());
	strcpy(szTitle,title.c_str());

	ofn.lpstrFile=szFilename;
	ofn.lpstrFileTitle=szTitle;

	if(GetSaveFileName(&ofn))
	{
		filename=szFilename;
		_preferences.set_value(preference,dirname(filename));
		return true;
	}
	return false;
#else
	synfig::String prev_path;

	if(!_preferences.get_value(preference, prev_path))
		prev_path = Glib::get_home_dir();

	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_SAVE);

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

	dialog->set_current_folder(prev_path);
	dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_ACCEPT);

	dialog->add_filter(filter_sifz);
	dialog->add_filter(filter_sif);
	dialog->add_filter(filter_sfg);

	Widget_Enum *file_type_enum = 0;
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
		file_type_enum->set_value(RELEASE_VERSION_END-1); // default to the most recent version

		Gtk::Grid *grid = manage(new Gtk::Grid);
		grid->attach(*manage(new Gtk::Label(_("File Format Version: "))),0,0,1,1);
		grid->attach(*file_type_enum,1,0,1,1);
		grid->show_all();

		dialog->set_extra_widget(*grid);
	}

	if (filename.empty()) {
		dialog->set_filename(prev_path);

	}else{
		std::string full_path;
		if (is_absolute_path(filename))
			full_path = filename;
		else
			full_path = prev_path + ETL_DIRECTORY_SEPARATOR + filename;

		// select the file if it exists
		dialog->set_filename(full_path);

		// if the file doesn't exist, put its name into the filename box
		struct stat s;
		if(stat(full_path.c_str(),&s) == -1 && errno == ENOENT)
			dialog->set_current_name(basename(filename));

	}
	// set file filter according to previous file format
	if (filename_extension(filename) == ".sif" ) dialog->set_filter(filter_sif);
	if (filename_extension(filename)== ".sifz" ) dialog->set_filter(filter_sifz);
	if (filename_extension(filename) == ".sfg" ) dialog->set_filter(filter_sfg);

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.
	dialog->set_current_name(basename(filename));

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {

		if (preference == ANIMATION_DIR_PREFERENCE)
			set_file_version(synfig::ReleaseVersion(file_type_enum->get_value()));

		// add file extension according to file filter selected by user if he doesn't type file extension in
		// file name entry. Right now it still detects file extension from file name entry, if extension is one
		// of .sif, sifz and sfg, it will be used otherwise, saved file format will depend on selected file filter.
		// It should be improved by changing file extension according to set file type filter, such as:
		// dialog->property_filter().signal_changed().connect(sigc::mem_fun(*this, &App::on_save_dialog_filter_changed));
		filename = dialog->get_filename();

		if (filename_extension(filename) != ".sif" &&
			filename_extension(filename) != ".sifz" &&
			filename_extension(filename) != ".sfg")
		{
			if (dialog->get_filter() == filter_sif)
				filename = dialog->get_filename() + ".sif";
			else if (dialog->get_filter() == filter_sifz)
				filename = dialog->get_filename() + ".sifz";
			else if (dialog->get_filter() == filter_sfg)
				filename = dialog->get_filename() + ".sfg";
		}

	// info("Saving preference %s = '%s' in App::dialog_save_file()", preference.c_str(), dirname(filename).c_str());
	_preferences.set_value(preference, dirname(filename));
	delete dialog;
	return true;
    }

    delete dialog;
    return false;
#endif
}


bool
App::dialog_save_file_spal(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;
	if(!_preferences.get_value(preference, prev_path))
		prev_path=Glib::get_home_dir();
	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_SAVE);

	// file type filters
	Glib::RefPtr<Gtk::FileFilter> filter_spal = Gtk::FileFilter::create();
	filter_spal->set_name(_("Synfig palette files (*.spal)"));
	filter_spal->add_pattern("*.spal");

	dialog->set_current_folder(prev_path);
	dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_ACCEPT);

	dialog->add_filter(filter_spal);

	if (filename.empty()) {
		dialog->set_filename(prev_path);

	}else{
		std::string full_path;
		if (is_absolute_path(filename))
			full_path = filename;
		else
			full_path = prev_path + ETL_DIRECTORY_SEPARATOR + filename;

		// select the file if it exists
		dialog->set_filename(full_path);

		// if the file doesn't exist, put its name into the filename box
		struct stat s;
		if(stat(full_path.c_str(),&s) == -1 && errno == ENOENT)
			dialog->set_current_name(basename(filename));

	}

	dialog->set_filter(filter_spal);

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.
	dialog->set_current_name(basename(filename));

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {

		// add file extension according to file filter selected by user
		filename = dialog->get_filename();
		if (filename_extension(filename) != ".spal")
			filename = dialog->get_filename() + ".spal";

	delete dialog;
	return true;
	}

	delete dialog;
	return false;
}

bool
App::dialog_save_file_sketch(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;
	if(!_preferences.get_value(preference, prev_path))
		prev_path=Glib::get_home_dir();
	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_SAVE);

	// file type filters
	Glib::RefPtr<Gtk::FileFilter> filter_sketch = Gtk::FileFilter::create();
	filter_sketch->set_name(_("Synfig sketch files (*.sketch)"));
	filter_sketch->add_pattern("*.sketch");

	dialog->set_current_folder(prev_path);
	dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_ACCEPT);

	dialog->add_filter(filter_sketch);

	if (filename.empty()) {
		dialog->set_filename(prev_path);

	}else{
		std::string full_path;
		if (is_absolute_path(filename))
			full_path = filename;
		else
			full_path = prev_path + ETL_DIRECTORY_SEPARATOR + filename;

		// select the file if it exists
		dialog->set_filename(full_path);

		// if the file doesn't exist, put its name into the filename box
		struct stat s;
		if(stat(full_path.c_str(),&s) == -1 && errno == ENOENT)
			dialog->set_current_name(basename(filename));

	}

	dialog->set_filter(filter_sketch);

	// set focus to the file name entry(box) of dialog instead to avoid the name
	// we are going to save changes while changing file filter each time.
	dialog->set_current_name(basename(filename));

	if(dialog->run() == GTK_RESPONSE_ACCEPT) {

		// add file extension according to file filter selected by user
		filename = dialog->get_filename();
		if (filename_extension(filename) != ".sketch")
			filename = dialog->get_filename() + ".sketch";

	delete dialog;
	return true;
	}

	delete dialog;
	return false;
}


bool
App::dialog_save_file_render(const std::string &title, std::string &filename, std::string preference)
{
	synfig::String prev_path;
	if(!_preferences.get_value(preference, prev_path))
		prev_path=Glib::get_home_dir();
	prev_path = absolute_path(prev_path);

	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window, title, Gtk::FILE_CHOOSER_ACTION_SAVE);

	dialog->set_current_folder(prev_path);
	dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->add_button(Gtk::Stock::OK,   Gtk::RESPONSE_ACCEPT);

	if (filename.empty()) {
		dialog->set_filename(prev_path);

	}else{
		std::string full_path;
		if (is_absolute_path(filename))
			full_path = filename;
		else
			full_path = prev_path + ETL_DIRECTORY_SEPARATOR + filename;

		// select the file if it exists
		dialog->set_filename(full_path);

		// if the file doesn't exist, put its name into the filename box
		struct stat s;
		if(stat(full_path.c_str(),&s) == -1 && errno == ENOENT)
			dialog->set_current_name(basename(filename));

	}

	if(dialog->run() == GTK_RESPONSE_ACCEPT)
	{
		filename = dialog->get_filename();

		delete dialog;
		return true;
	}

	delete dialog;
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
	for(std::list<std::string>::const_iterator i = list.begin(); i != list.end(); i++) {
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

	if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
		item_index = tree->get_selection()->get_selected()->get_value(model_columns.column_index);
		return true;
	}

	return false;
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
		label.show();
		sw.add(label);
		sw.set_size_request(400,300);
		sw.show();
		dialog.get_content_area()->pack_end(sw);
		dialog.set_resizable(true);
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

	return	dialog.run();
}


// message dialog with 3 buttons.
int
App::dialog_message_3b(const std::string &message,
	const std::string &detials,
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
	dialog.set_secondary_text(detials);
	dialog.add_button(button1, 0);
	dialog.add_button(button2, 1);
	dialog.add_button(button3, 2);

	return dialog.run();
}

static bool
try_open_img_external(const std::string &uri)
{
	std::string new_uri=uri;
	std::string s = "file://";
	std::string::size_type i = new_uri.find(s);
	if (i != std::string::npos)
	{
		new_uri.erase(i, s.length());
	}
   	size_t start_pos = 0;
	std::string to = " ";
	std::string from = "%20";
    while((start_pos = new_uri.find(from, start_pos)) != std::string::npos) 
	{
        new_uri.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
	new_uri = "\"" + new_uri + "\"";
	if(App::image_editor_path!="")
	{
		#ifdef WIN32
			char buffer[512];
    		::snprintf(buffer, sizeof(buffer), "%s %s",App::image_editor_path.c_str(), new_uri.c_str());
    		Glib::spawn_command_line_async(buffer);
		#elif defined(__APPLE__)
    		char buffer[512];
    		::snprintf(buffer, sizeof(buffer), "open -a %s %s", App::image_editor_path.c_str(), new_uri.c_str());
    		Glib::spawn_command_line_async(buffer);
		#else
    		char buffer[512];
    		::snprintf(buffer, sizeof(buffer), "%s %s",App::image_editor_path.c_str(), new_uri.c_str());
			Glib::spawn_command_line_async(buffer);
		#endif
		return true;

	}
	else
	{
		return false;
	}
	
	
}
static bool
try_open_uri(const std::string &uri)
{
#if GTK_CHECK_VERSION(3, 22, 0)
	return gtk_show_uri_on_window(
		App::main_window ? App::main_window->gobj() : NULL,
		uri.c_str(), GDK_CURRENT_TIME, NULL );
#else
	return gtk_show_uri(NULL, uri.c_str(), GDK_CURRENT_TIME, NULL);
#endif
}


void
App::dialog_help()
{
	if (!try_open_uri("https://wiki.synfig.org/Category:Manual"))
	{
		Gtk::MessageDialog dialog(*App::main_window, _("Documentation"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true);
		dialog.set_secondary_text(_("Documentation for Synfig Studio is available on the website:\n\nhttps://wiki.synfig.org/Category:Manual"));
		dialog.set_title(_("Help"));
		dialog.run();
	}
}
void App::open_img_in_external(const std::string &uri)
{
	synfig::info("Opening with external tool: " + uri);
	if(!try_open_img_external(uri))
	{
		Gtk::MessageDialog dialog(*App::main_window, _("Make sure Preferred editing tool was set in \n Edit->Preferences->Editing:"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		dialog.set_secondary_text(uri);
		dialog.set_title(_("Error"));
		dialog.run();
	}

}
unordered_map<std::string, int> configmap({ { "threshold", 8 },{ "accuracy", 9 },{ "despeckling", 5 },{ "maxthickness", 200 }});
void App::open_vectorizerpopup(const etl::handle<synfig::Layer_Bitmap> my_layer_bitmap, const etl::handle<synfig::Layer> reference_layer)
{
	String desc = my_layer_bitmap->get_description();
	synfig::info("Opening Vectorizerpopup for :"+desc);
	App::vectorizerpopup = new studio::VectorizerSettings(*App::main_window,my_layer_bitmap,selected_instance,configmap,reference_layer);
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
App::dialog_paragraph(const std::string &title, const std::string &message,std::string &text)
{
	Gtk::Dialog dialog(
		title,			// Title
		*App::main_window,	// Parent
		true			// Modal
	);

	Gtk::Label* label = manage(new Gtk::Label(message));
	label->show();
	dialog.get_content_area()->pack_start(*label);

	Glib::RefPtr<Gtk::TextBuffer> text_buffer(Gtk::TextBuffer::create());
	text_buffer->set_text(text);
	Gtk::TextView text_view(text_buffer);
	text_view.show();

	dialog.get_content_area()->pack_start(text_view);

	dialog.add_button(_("OK"),   Gtk::RESPONSE_OK)->set_image_from_icon_name("gtk-ok", Gtk::ICON_SIZE_BUTTON);
	dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_BUTTON);
	dialog.set_default_response(Gtk::RESPONSE_OK);

	//text_entry.signal_activate().connect(sigc::bind(sigc::mem_fun(dialog,&Gtk::Dialog::response),Gtk::RESPONSE_OK));
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_OK)
		return false;

	text=text_buffer->get_text();

	return true;
}

std::string
App::get_temporary_directory()
{
	return synfigapp::Main::get_user_app_directory() + ETL_DIRECTORY_SEPARATOR + "tmp";
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
	temporary_file_system->set_meta("truncate", etl::strprintf("%lld", truncate_storage_size));
	return temporary_file_system;
}

bool
App::open(std::string filename, /* std::string as, */ synfig::FileContainerZip::file_size_t truncate_storage_size)
{
#ifdef _WIN32
    size_t buf_size = PATH_MAX - 1;
    char* long_name = (char*)malloc(buf_size);
    long_name[0] = '\0';
    if(GetLongPathName(filename.c_str(),long_name,sizeof(long_name)));
    // when called from autorecover.cpp, filename doesn't exist, and so long_name is empty
    // don't use it if that's the case
    if (long_name[0] != '\0')
        filename=String(long_name);
    free(long_name);
#endif

	try
	{
		OneMoment one_moment;
		String errors, warnings;

		// try open container
		FileSystem::Handle container = CanvasFileNaming::make_filesystem_container(filename, truncate_storage_size);
		if (!container)
			throw (String)strprintf(_("Unable to open container \"%s\"\n\n"),filename.c_str());

		// make canvas file system
		FileSystem::Handle canvas_file_system = CanvasFileNaming::make_filesystem(container);

		// wrap into temporary file system
		canvas_file_system = wrap_into_temporary_filesystem(canvas_file_system, filename, filename, truncate_storage_size);

		// file to open inside canvas file-system
		String canvas_filename = CanvasFileNaming::project_file(filename);

		etl::handle<synfig::Canvas> canvas = open_canvas_as(canvas_file_system ->get_identifier(canvas_filename), filename, errors, warnings);
		if(canvas && get_instance(canvas))
		{
			get_instance(canvas)->find_canvas_view(canvas)->present();
			info("%s is already open", canvas_filename.c_str());
			// throw (String)strprintf(_("\"%s\" appears to already be open!"),filename.c_str());
		}
		else
		{
			if(!canvas)
				throw (String)strprintf(_("Unable to load \"%s\":\n\n"),filename.c_str()) + errors;

			if (warnings != "")
				dialog_message_1b(
					"WARNING",
					_("Warning"),
					"details",
					_("Close"),
					warnings);

			if (filename.find(custom_filename_prefix.c_str()) != 0)
				add_recent_file(filename);

			handle<Instance> instance(Instance::create(canvas, container));

			if(!instance)
				throw (String)strprintf(_("Unable to create instance for \"%s\""),filename.c_str());

			one_moment.hide();

			if(instance->is_updated() && App::dialog_message_2b(
				_("Newer version of this file available on the CVS repository!"),
				_("repository. Would you like to update now? (It would probably be a good idea)"),
				Gtk::MESSAGE_QUESTION,
				_("Cancel"),
				_("Update Anyway"))
			)
				instance->dialog_cvs_update();
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
	catch(runtime_error &x)
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
App::open_from_temporary_filesystem(std::string temporary_filename)
{
	try
	{
		OneMoment one_moment;
		String errors, warnings;

		// try open temporary container
		FileSystemTemporary::Handle file_system_temporary(new FileSystemTemporary(""));
		if (!file_system_temporary->open_temporary(temporary_filename))
			throw (String)strprintf(_("Unable to open temporary container \"%s\"\n\n"), temporary_filename.c_str());

		// get original filename
		String filename = file_system_temporary->get_meta("filename");
		String as = file_system_temporary->get_meta("as");
		String truncate = file_system_temporary->get_meta("truncate");
		if (filename.empty() || as.empty() || truncate.empty())
			throw (String)strprintf(_("Original filename was not set in temporary container \"%s\"\n\n"), temporary_filename.c_str());
#ifdef __APPLE__
		FileContainerZip::file_size_t truncate_storage_size = atoll(truncate.c_str());
#else
		FileContainerZip::file_size_t truncate_storage_size = stoll(truncate);
#endif

		// make canvas file-system
		FileSystem::Handle canvas_container = CanvasFileNaming::make_filesystem_container(filename, truncate_storage_size);
		FileSystem::Handle canvas_file_system = CanvasFileNaming::make_filesystem(canvas_container);

		// wrap into temporary
		file_system_temporary->set_sub_file_system(canvas_file_system);
		canvas_file_system = file_system_temporary;

		// file to open inside canvas file system
		String canvas_filename = CanvasFileNaming::project_file(canvas_file_system);

		etl::handle<synfig::Canvas> canvas(open_canvas_as(canvas_file_system->get_identifier(canvas_filename), as, errors, warnings));
		if(canvas && get_instance(canvas))
		{
			get_instance(canvas)->find_canvas_view(canvas)->present();
			info("%s is already open", as.c_str());
			// throw (String)strprintf(_("\"%s\" appears to already be open!"),filename.c_str());
		}
		else
		{
			if(!canvas)
				throw (String)strprintf(_("Unable to load \"%s\":\n\n"), temporary_filename.c_str()) + errors;

			if (warnings != "")
				dialog_message_1b(
						"WARNING",
						strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
						"details",
						_("Close"));

			if (as.find(custom_filename_prefix.c_str()) != 0)
				add_recent_file(as);

			handle<Instance> instance(Instance::create(canvas, canvas_container));

			if(!instance)
				throw (String)strprintf(_("Unable to create instance for \"%s\""), temporary_filename.c_str());

			one_moment.hide();

			if(instance->is_updated() && App::dialog_message_2b(
				_("Newer version of this file available on the CVS repository!"),
				_("Would you like to update now? (It would probably be a good idea)"),
				Gtk::MESSAGE_QUESTION,
				_("Cancel"),
				_("Update Anyway"))
			)
				instance->dialog_cvs_update();

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
	catch(runtime_error &x)
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
	handle<synfig::Canvas> canvas=synfig::Canvas::create();

	String filename(strprintf("%s%d", App::custom_filename_prefix.c_str(), Instance::get_count()+1));
	canvas->set_name(filename);

	canvas->rend_desc().set_frame_rate(preferred_fps);
	canvas->rend_desc().set_time_start(0.0);
	canvas->rend_desc().set_time_end(5.0);
	canvas->rend_desc().set_x_res(DPI2DPM(72.0f));
	canvas->rend_desc().set_y_res(DPI2DPM(72.0f));
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

	handle<Instance> instance = Instance::create(canvas, container);

    if (App::default_background_layer_type == "solid_color")
    {
		//Create a SolidColor layer
		synfig::Layer::Handle layer(instance->find_canvas_interface(canvas)->add_layer_to("SolidColor",
			                        canvas,
			                        0)); //target_depth

		//Rename it as Background
		synfigapp::Action::Handle action_LayerSetDesc(synfigapp::Action::create("LayerSetDesc"));
		if (action_LayerSetDesc) {
			action_LayerSetDesc->set_param("canvas",           canvas);
			action_LayerSetDesc->set_param("canvas_interface", instance->find_canvas_interface(canvas));
			action_LayerSetDesc->set_param("layer",            layer);
			action_LayerSetDesc->set_param("new_description",  "Background");
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
			action_LayerSetDesc->set_param("new_description",  "Background");
			App::get_selected_canvas_view()->canvas_interface()->get_instance()->perform_action(action_LayerSetDesc);
		}

		if (warnings != "")
			App::dialog_message_1b(
				"WARNING",
				etl::strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
				"details",
				_("Close"));
		if (errors != "")
			App::dialog_message_1b(
				"ERROR",
				etl::strprintf("%s:\n\n%s", _("Error"), errors.c_str()),
				"details",
				_("Close"));
	}

	if (getenv("SYNFIG_AUTO_ADD_SKELETON_LAYER"))
		instance->find_canvas_view(canvas)->add_layer("skeleton");

	if (getenv("SYNFIG_AUTO_ADD_MOTIONBLUR_LAYER"))
		instance->find_canvas_view(canvas)->add_layer("MotionBlur");

	if (getenv("SYNFIG_ENABLE_NEW_CANVAS_EDIT_PROPERTIES"))
		instance->find_canvas_view(canvas)->canvas_properties.present();
}

void
App::dialog_open(string filename)
{
	if (filename.empty() && selected_instance)
		filename = selected_instance->get_file_name();
	if (filename.empty())
		filename="*.sif";

	bool show_history = false;
	while(dialog_open_file_with_history_button(_("Please select a file"), filename, show_history, ANIMATION_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if(find(filename.begin(),filename.end(),'*')!=filename.end())
			continue;

		FileContainerZip::file_size_t truncate_storage_size = 0;

		// TODO: ".sfg" literal
		if (show_history && filename_extension(filename) == ".sfg")
		{
			// read history
			std::list<FileContainerZip::HistoryRecord> history
				= FileContainerZip::read_history(filename);

			// build list of history entries for dialog (descending)
			std::list<std::string> list;
			int index = 0;
			for(std::list<FileContainerZip::HistoryRecord>::const_iterator i = history.begin(); i != history.end(); i++)
				list.push_front(strprintf("%s%d", _("History entry #"), ++index));

			// show dialog
			index=0;
			if (!dialog_select_list_item(_("Please select a file"), _("Select one of previous versions of file"), list, index))
				continue;

			// find selected entry in list (descending)
			for(std::list<FileContainerZip::HistoryRecord>::const_reverse_iterator i = history.rbegin(); i != history.rend(); i++)
				if (0 == index--)
					truncate_storage_size = i->storage_size;
		}

		if(open(filename,truncate_storage_size))
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
			set_selected_canvas_view(0);
		}
	} else {
		selected_instance = instance;
		signal_instance_selected()(selected_instance);
	}
}

void
App::set_selected_canvas_view(etl::loose_handle<CanvasView> canvas_view)
{
	if (selected_canvas_view == canvas_view)
		return;
	
	etl::loose_handle<CanvasView> prev = selected_canvas_view;
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
App::get_instance(etl::handle<synfig::Canvas> canvas)
{
	if(!canvas) return 0;
	canvas=canvas->get_root();

	std::list<etl::handle<Instance> >::iterator iter;
	for(iter=instance_list.begin();iter!=instance_list.end();++iter)
	{
		if((*iter)->get_canvas()==canvas)
			return *iter;
	}
	return 0;
}

Gamma
App::get_selected_canvas_gamma()
{
	if (etl::loose_handle<CanvasView> canvas_view = App::get_selected_canvas_view())
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

synfig::String
studio::App::get_base_path()
{
	return FileSystem::fix_slashes(app_base_path_);
}

void
studio::App::setup_changed()
{
	std::list<etl::handle<Instance> >::iterator iter;
	for(iter=instance_list.begin();iter!=instance_list.end();++iter)
	{
		std::list< etl::handle<synfigapp::CanvasInterface> >::iterator citer;
		std::list< etl::handle<synfigapp::CanvasInterface> >& cilist((*iter)->canvas_interface_list());
		for(citer=cilist.begin();citer!=cilist.end();++citer)
			{
				(*citer)->signal_rend_desc_changed()();
			}
	}
}

void
studio::App::process_all_events(long unsigned int us)
{
	Glib::usleep(us);
	while(studio::App::events_pending()) {
		while(studio::App::events_pending())
			studio::App::iteration(false);
		Glib::usleep(us);
	}
}

bool
studio::App::check_python_version(String path)
{
	String command;
	String result;
	command = path + " --version 2>&1";
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		return false;
	}
	char buffer[128];
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
				result += buffer;
	}
	pclose(pipe);
	// Output is like: "Python 3.3.0"
	if (result.substr(7,1) != "3"){
		return false;
	}
	return true;
}
