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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef WIN32
#define WINVER 0x0500
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <locale>
#include <cstring>

#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif
#include <gtkmm/fileselection.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/stockitem.h>
#include <gtkmm/iconsource.h>
#include <gtkmm/inputdialog.h>
#include <gtkmm/accelmap.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/textview.h>

#include <gtk/gtk.h>

#include <gdkmm/general.h>

#include <synfig/loadcanvas.h>
#include <synfig/savecanvas.h>
#include <synfig/importer.h>
#include <synfig/filesystemnative.h>
#include <synfig/filesystemgroup.h>
#include <synfig/filecontainertemporary.h>

#include "app.h"
#include "dialogs/about.h"
#include "splash.h"
#include "instance.h"
#include "canvasview.h"
#include "dialogs/dialog_setup.h"
#include "dialogs/dialog_gradient.h"
#include "dialogs/dialog_color.h"
#include "toolbox.h"
#include "onemoment.h"

#include "docks/dockmanager.h"

#include "states/state_eyedrop.h"
#include "states/state_normal.h"
#include "states/state_mirror.h"
#include "states/state_draw.h"
#include "states/state_fill.h"
#include "states/state_bline.h"
#include "states/state_polygon.h"
#include "states/state_sketch.h"
#include "states/state_gradient.h"
#include "states/state_circle.h"
#include "states/state_rectangle.h"
#include "states/state_smoothmove.h"
#include "states/state_scale.h"
#include "states/state_star.h"
#include "states/state_text.h"
#include "states/state_width.h"
#include "states/state_rotate.h"
#include "states/state_zoom.h"

#include "devicetracker.h"
#include "docks/dialog_tooloptions.h"
#include "widgets/widget_enum.h"

#include "autorecover.h"

#include <synfigapp/settings.h>
#include "docks/dock_history.h"
#include "docks/dock_canvases.h"
#include "docks/dock_keyframes.h"
#include "docks/dock_layers.h"
#include "docks/dock_params.h"
#include "docks/dock_metadata.h"
#include "docks/dock_children.h"
#include "docks/dock_info.h"
#include "docks/dock_navigator.h"
#include "docks/dock_layergroups.h"
#include "docks/dock_timetrack.h"
#include "docks/dock_curves.h"

#include "modules/module.h"
#include "modules/mod_palette/mod_palette.h"

#include "ipc.h"

#include "statemanager.h"

#ifdef WITH_FMOD
#include <fmod.h>
#endif

#include <gtkmm/accelmap.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/filechooserdialog.h>

#include "general.h"

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

#ifdef WIN32
#	ifdef IMAGE_DIR
#		undef IMAGE_DIR
#		define IMAGE_DIR "share\\pixmaps"
#	endif
#endif

#ifndef IMAGE_DIR
#	define IMAGE_DIR "/usr/local/share/pixmaps"
#endif

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"tif"
#endif

#ifdef WIN32
#	ifdef PLUGIN_DIR
#		undef PLUGIN_DIR
#		define PLUGIN_DIR "share\\synfig\\plugins"
#	endif
#endif

#ifndef PLUGIN_DIR
#	define PLUGIN_DIR "/usr/local/share/synfig/plugins"
#endif

#include <synfigapp/main.h>

/* === S I G N A L S ======================================================= */

static sigc::signal<void> signal_present_all_;
sigc::signal<void>&
App::signal_present_all() { return signal_present_all_; }

static sigc::signal<void> signal_recent_files_changed_;
sigc::signal<void>&
App::signal_recent_files_changed() { return signal_recent_files_changed_; }

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

static std::list<std::string> recent_files;
const std::list<std::string>& App::get_recent_files() { return recent_files; }

static std::list<std::string> recent_files_window_size;

int	App::Busy::count;
bool App::shutdown_in_progress;

synfig::Gamma App::gamma;

Glib::RefPtr<studio::UIManager>	App::ui_manager_;

synfig::Distance::System App::distance_system;

studio::Dialog_Setup* App::dialog_setup;

etl::handle< studio::ModPalette > mod_palette_;
//studio::Dialog_Palette* App::dialog_palette;

std::list<etl::handle<Instance> > App::instance_list;

static etl::handle<synfigapp::UIInterface> ui_interface_;
const etl::handle<synfigapp::UIInterface>& App::get_ui_interface() { return ui_interface_; }

etl::handle<Instance> App::selected_instance;
etl::handle<CanvasView> App::selected_canvas_view;

studio::About *studio::App::about=NULL;

studio::Toolbox *studio::App::toolbox=NULL;

studio::AutoRecover *studio::App::auto_recover=NULL;

studio::IPC *ipc=NULL;

studio::DockManager* studio::App::dock_manager=0;

studio::DeviceTracker* studio::App::device_tracker=0;

studio::Dialog_Gradient* studio::App::dialog_gradient;

studio::Dialog_Color* studio::App::dialog_color;

Gtk::InputDialog* studio::App::dialog_input;

studio::Dialog_ToolOptions* studio::App::dialog_tool_options;

studio::Dock_History* dock_history;
studio::Dock_Canvases* dock_canvases;
studio::Dock_Keyframes* dock_keyframes;
studio::Dock_Layers* dock_layers;
studio::Dock_Params* dock_params;
studio::Dock_MetaData* dock_meta_data;
studio::Dock_Children* dock_children;
studio::Dock_Info* dock_info;
studio::Dock_LayerGroups* dock_layer_groups;
studio::Dock_Navigator* dock_navigator;
studio::Dock_Timetrack* dock_timetrack;
studio::Dock_Curves* dock_curves;

std::list< etl::handle< studio::Module > > module_list_;

bool studio::App::use_colorspace_gamma=true;
#ifdef SINGLE_THREADED
	#ifdef	WIN32
	bool studio::App::single_threaded=true;
	#else
	bool studio::App::single_threaded=false;
	#endif // WIN32
#endif  // SINGLE THREADED
bool studio::App::restrict_radius_ducks=true;
bool studio::App::resize_imported_images=false;
String studio::App::custom_filename_prefix(DEFAULT_FILENAME_PREFIX);
int studio::App::preferred_x_size=480;
int studio::App::preferred_y_size=270;
String studio::App::predefined_size(DEFAULT_PREDEFINED_SIZE);
String studio::App::predefined_fps(DEFAULT_PREDEFINED_FPS);
float studio::App::preferred_fps=24.0;
synfigapp::PluginManager studio::App::plugin_manager;
#ifdef USE_OPEN_FOR_URLS
String studio::App::browser_command("open"); // MacOS only
#else
String studio::App::browser_command("xdg-open"); // Linux XDG standard
#endif
String studio::App::sequence_separator(".");
bool studio::App::navigator_uses_cairo=false;
bool studio::App::workarea_uses_cairo=false;

static int max_recent_files_=25;
int studio::App::get_max_recent_files() { return max_recent_files_; }
void studio::App::set_max_recent_files(int x) { max_recent_files_=x; }

static synfig::String app_base_path_;

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

}; // END of namespace studio
studio::StateManager* state_manager;




class GlobalUIInterface : public synfigapp::UIInterface
{
public:

	virtual Response confirmation(const std::string &title,
			const std::string &primaryText,
			const std::string &secondaryText,
			const std::string &confirmPhrase,
			const std::string &cancelPhrase,
			Response defaultResponse)
	{
		Gtk::MessageDialog dialog(
			primaryText,		// Message
			false,			// Markup
			Gtk::MESSAGE_WARNING,	// Type
			Gtk::BUTTONS_NONE,	// Buttons
			true			// Modal
		);

		if (! title.empty())
			dialog.set_title(title);
		if (! secondaryText.empty())
			dialog.set_secondary_text(secondaryText);

		dialog.add_button(cancelPhrase, RESPONSE_CANCEL);
		dialog.add_button(confirmPhrase, RESPONSE_OK);
		dialog.set_default_response(defaultResponse);

		dialog.show_all();
		return (Response) dialog.run();
	}

	virtual Response yes_no(const std::string &title, const std::string &message,Response dflt=RESPONSE_YES)
	{
		Gtk::Dialog dialog(
			title,		// Title
			true,		// Modal
			true		// use_separator
		);
		Gtk::Label label(message);
		label.show();

		dialog.get_vbox()->pack_start(label);
		dialog.add_button(Gtk::StockID("gtk-yes"),RESPONSE_YES);
		dialog.add_button(Gtk::StockID("gtk-no"),RESPONSE_NO);

		dialog.set_default_response(dflt);
		dialog.show();
		return (Response)dialog.run();
	}
	virtual Response yes_no_cancel(const std::string &title, const std::string &message,Response dflt=RESPONSE_YES)
	{
		Gtk::Dialog dialog(
			title,		// Title
			true,		// Modal
			true		// use_separator
		);
		Gtk::Label label(message);
		label.show();

		dialog.get_vbox()->pack_start(label);
		dialog.add_button(Gtk::StockID("gtk-yes"),RESPONSE_YES);
		dialog.add_button(Gtk::StockID("gtk-no"),RESPONSE_NO);
		dialog.add_button(Gtk::StockID("gtk-cancel"),RESPONSE_CANCEL);

		dialog.set_default_response(dflt);
		dialog.show();
		return (Response)dialog.run();
	}
	virtual Response ok_cancel(const std::string &title, const std::string &message,Response dflt=RESPONSE_OK)
	{
		Gtk::Dialog dialog(
			title,		// Title
			true,		// Modal
			true		// use_separator
		);
		Gtk::Label label(message);
		label.show();

		dialog.get_vbox()->pack_start(label);
		dialog.add_button(Gtk::StockID("gtk-ok"),RESPONSE_OK);
		dialog.add_button(Gtk::StockID("gtk-cancel"),RESPONSE_CANCEL);

		dialog.set_default_response(dflt);
		dialog.show();
		return (Response)dialog.run();
	}

	virtual bool
	task(const std::string &task)
	{
		std::cerr<<task.c_str()<<std::endl;
		while(studio::App::events_pending())studio::App::iteration(false);
		return true;
	}

	virtual bool
	error(const std::string &err)
	{
		Gtk::MessageDialog dialog(err, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		dialog.show();
		dialog.run();
		return true;
	}

	virtual bool
	warning(const std::string &err)
	{
		std::cerr<<"warning: "<<err.c_str()<<std::endl;
		while(studio::App::events_pending())studio::App::iteration(false);
		return true;
	}

	virtual bool
	amount_complete(int /*current*/, int /*total*/)
	{
		while(studio::App::events_pending())studio::App::iteration(false);
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
			if(key=="gamma")
			{
				value=strprintf("%f %f %f %f",
					App::gamma.get_gamma_r(),
					App::gamma.get_gamma_g(),
					App::gamma.get_gamma_b(),
					App::gamma.get_black_level()
				);
				return true;
			}
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
			if(key=="use_colorspace_gamma")
			{
				value=strprintf("%i",(int)App::use_colorspace_gamma);
				return true;
			}
			if(key=="distance_system")
			{
				value=strprintf("%s",Distance::system_name(App::distance_system).c_str());
				return true;
			}
#ifdef SINGLE_THREADED
			if(key=="single_threaded")
			{
				value=strprintf("%i",(int)App::single_threaded);
				return true;
			}
#endif
			if(key=="auto_recover_backup_interval")
			{
				value=strprintf("%i",App::auto_recover->get_timeout());
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
			if(key=="browser_command")
			{
				value=App::browser_command;
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
			if(key=="navigator_uses_cairo")
			{
				value=strprintf("%i",(int)App::navigator_uses_cairo);
				return true;
			}
			if(key=="workarea_uses_cairo")
			{
				value=strprintf("%i",(int)App::workarea_uses_cairo);
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
			if(key=="gamma")
			{
				float r,g,b,blk;

				strscanf(value,"%f %f %f %f",
					&r,
					&g,
					&b,
					&blk
				);

				App::gamma.set_all(r,g,b,blk);

				return true;
			}
			if(key=="time_format")
			{
				int i(atoi(value.c_str()));
				App::set_time_format(static_cast<synfig::Time::Format>(i));
				return true;
			}
			if(key=="auto_recover_backup_interval")
			{
				int i(atoi(value.c_str()));
				App::auto_recover->set_timeout(i);
				return true;
			}
			if(key=="file_history.size")
			{
				int i(atoi(value.c_str()));
				App::set_max_recent_files(i);
				return true;
			}
			if(key=="use_colorspace_gamma")
			{
				int i(atoi(value.c_str()));
				App::use_colorspace_gamma=i;
				return true;
			}
			if(key=="distance_system")
			{
				App::distance_system=Distance::ident_system(value);;
				return true;
			}
#ifdef SINGLE_THREADED
			if(key=="single_threaded")
			{
				int i(atoi(value.c_str()));
				App::single_threaded=i;
				return true;
			}
#endif
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
			if(key=="browser_command")
			{
				App::browser_command=value;
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
			if(key=="navigator_uses_cairo")
			{
				int i(atoi(value.c_str()));
				App::navigator_uses_cairo=i;
				return true;
			}
			if(key=="workarea_uses_cairo")
			{
				int i(atoi(value.c_str()));
				App::workarea_uses_cairo=i;
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
		ret.push_back("gamma");
		ret.push_back("time_format");
		ret.push_back("distance_system");
		ret.push_back("file_history.size");
		ret.push_back("use_colorspace_gamma");
#ifdef SINGLE_THREADED
		ret.push_back("single_threaded");
#endif
		ret.push_back("auto_recover_backup_interval");
		ret.push_back("restrict_radius_ducks");
		ret.push_back("resize_imported_images");
		ret.push_back("browser_command");
		ret.push_back("custom_filename_prefix");
		ret.push_back("preferred_x_size");
		ret.push_back("preferred_y_size");
		ret.push_back("predefined_size");
		ret.push_back("preferred_fps");
		ret.push_back("predefined_fps");
		ret.push_back("sequence_separator");
		ret.push_back("navigator_uses_cairo");
		ret.push_back("workarea_uses_cairo");
		return ret;
	}
};

static ::Preferences _preferences;

void
init_ui_manager()
{
	Glib::RefPtr<Gtk::ActionGroup> menus_action_group = Gtk::ActionGroup::create("menus");

	Glib::RefPtr<Gtk::ActionGroup> toolbox_action_group = Gtk::ActionGroup::create("toolbox");

	Glib::RefPtr<Gtk::ActionGroup> actions_action_group = Gtk::ActionGroup::create("actions");

	menus_action_group->add( Gtk::Action::create("menu-file", _("_File")) );
	menus_action_group->add( Gtk::Action::create("menu-edit", _("_Edit")) );
	menus_action_group->add( Gtk::Action::create("menu-view", _("_View")) );
	menus_action_group->add( Gtk::Action::create("menu-canvas", _("_Canvas")) );
	menus_action_group->add( Gtk::Action::create("menu-layer", _("_Layer")) );
	menus_action_group->add( Gtk::Action::create("menu-duck-mask", _("Show/Hide Handles")) );
	menus_action_group->add( Gtk::Action::create("menu-preview-quality", _("Preview Quality")) );
	menus_action_group->add( Gtk::Action::create("menu-lowres-pixel", _("Low-Res Pixel Size")) );
	menus_action_group->add( Gtk::Action::create("menu-layer-new", _("New Layer")) );
	menus_action_group->add( Gtk::Action::create("menu-keyframe", _("Keyframe")) );
	menus_action_group->add( Gtk::Action::create("menu-group", _("Set")) );
	menus_action_group->add( Gtk::Action::create("menu-state", _("Tool")) );
	menus_action_group->add( Gtk::Action::create("menu-toolbox", _("Toolbox")) );
	menus_action_group->add( Gtk::Action::create("menu-plugins", _("Plug-Ins")) );

	// Add the synfigapp actions...
	synfigapp::Action::Book::iterator iter;
	for(iter=synfigapp::Action::book().begin();iter!=synfigapp::Action::book().end();++iter)
	{
		actions_action_group->add(Gtk::Action::create(
			"action-"+iter->second.name,
			get_action_stock_id(iter->second),
			iter->second.local_name,iter->second.local_name
		));
	}

#define DEFINE_ACTION(x,stock) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock) ); actions_action_group->add(action); }
#define DEFINE_ACTION2(x,stock,label) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock,label,label) ); actions_action_group->add(action); }
#define DEFINE_ACTION_SIG(group,x,stock,sig) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock) ); group->add(action,sig); }

	DEFINE_ACTION2("keyframe-properties", Gtk::StockID("gtk-properties"), _("Keyframe Properties"));
	DEFINE_ACTION("about", Gtk::StockID("synfig-about"));
	DEFINE_ACTION("new", Gtk::Stock::NEW);
	DEFINE_ACTION("open", Gtk::Stock::OPEN);
	DEFINE_ACTION("save", Gtk::Stock::SAVE);
	DEFINE_ACTION("save-as", Gtk::Stock::SAVE_AS);
	DEFINE_ACTION("revert", Gtk::Stock::REVERT_TO_SAVED);
	DEFINE_ACTION("cvs-add", Gtk::StockID("synfig-cvs_add"));
	DEFINE_ACTION("cvs-update", Gtk::StockID("synfig-cvs_update"));
	DEFINE_ACTION("cvs-commit", Gtk::StockID("synfig-cvs_commit"));
	DEFINE_ACTION("cvs-revert", Gtk::StockID("synfig-cvs_revert"));
	DEFINE_ACTION("import", _("Import"));
	DEFINE_ACTION("render", _("Render"));
	DEFINE_ACTION("preview", _("Preview"));
	DEFINE_ACTION("dialog-flipbook", _("Preview Dialog"));
	DEFINE_ACTION("sound", _("Sound File"));
	DEFINE_ACTION("options", _("Options"));
	DEFINE_ACTION("close", _("Close View"));
	DEFINE_ACTION("close-document", _("Close Document"));
	DEFINE_ACTION("quit", Gtk::Stock::QUIT);


	DEFINE_ACTION("undo", Gtk::StockID("gtk-undo"));
	DEFINE_ACTION("redo", Gtk::StockID("gtk-redo"));
	DEFINE_ACTION("cut", Gtk::StockID("gtk-cut"));
	DEFINE_ACTION("copy", Gtk::StockID("gtk-copy"));
	DEFINE_ACTION("paste", Gtk::StockID("gtk-paste"));
	DEFINE_ACTION("select-all-ducks", _("Select All Handles"));
	DEFINE_ACTION("unselect-all-ducks", _("Unselect All Handles"));
	DEFINE_ACTION("select-all-layers", _("Select All Layers"));
	DEFINE_ACTION("unselect-all-layers", _("Unselect All Layers"));
	DEFINE_ACTION("properties", _("Properties"));

	DEFINE_ACTION("mask-position-ducks", _("Show Position Handles"));
	DEFINE_ACTION("mask-vertex-ducks", _("Show Vertex Handles"));
	DEFINE_ACTION("mask-tangent-ducks", _("Show Tangent Handles"));
	DEFINE_ACTION("mask-radius-ducks", _("Show Radius Handles"));
	DEFINE_ACTION("mask-width-ducks", _("Show Width Handles"));
	DEFINE_ACTION("mask-angle-ducks", _("Show Angle Handles"));
	DEFINE_ACTION("mask-bone-setup-ducks", _("Show Bone Setup Handles"));
	DEFINE_ACTION("mask-bone-recursive-ducks", _("Show Recursive Scale Bone Handles"));
	DEFINE_ACTION("mask-bone-ducks", _("Next Bone Handles"));
	DEFINE_ACTION("mask-widthpoint-position-ducks", _("Show WidthPoints Position Handles"));
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
	DEFINE_ACTION("play", _("Play"));
	// DEFINE_ACTION("pause", _("Pause"));
	DEFINE_ACTION("stop", _("Stop"));
	DEFINE_ACTION("toggle-grid-show", _("Toggle Grid Show"));
	DEFINE_ACTION("toggle-grid-snap", _("Toggle Grid Snap"));
	DEFINE_ACTION("toggle-guide-show", _("Toggle Guide Show"));
	DEFINE_ACTION("toggle-guide-snap", _("Toggle Guide Snap"));
	DEFINE_ACTION("toggle-low-res", _("Toggle Low-Res"));
	DEFINE_ACTION("decrease-low-res-pixel-size", _("Decrease Low-Res Pixel Size"));
	DEFINE_ACTION("increase-low-res-pixel-size", _("Increase Low-Res Pixel Size"));
	DEFINE_ACTION("toggle-onion-skin", _("Toggle Onion Skin"));
	DEFINE_ACTION("canvas-zoom-in", Gtk::StockID("gtk-zoom-in"));
	DEFINE_ACTION("canvas-zoom-out", Gtk::StockID("gtk-zoom-out"));
	DEFINE_ACTION("canvas-zoom-fit", Gtk::StockID("gtk-zoom-fit"));
	DEFINE_ACTION("canvas-zoom-100", Gtk::StockID("gtk-zoom-100"));
	DEFINE_ACTION("time-zoom-in", Gtk::StockID("gtk-zoom-in"));
	DEFINE_ACTION("time-zoom-out", Gtk::StockID("gtk-zoom-out"));
	DEFINE_ACTION("jump-next-keyframe", _("Jump to Next Keyframe"));
	DEFINE_ACTION("jump-prev-keyframe", _("Jump to Prev Keyframe"));
	DEFINE_ACTION("seek-next-frame", _("Next Frame"));
	DEFINE_ACTION("seek-prev-frame", _("Prev Frame"));
	DEFINE_ACTION("seek-next-second", _("Seek Forward"));
	DEFINE_ACTION("seek-prev-second", _("Seek Backward"));
	DEFINE_ACTION("seek-begin", _("Seek to Begin"));
	DEFINE_ACTION("seek-end", _("Seek to End"));

	DEFINE_ACTION("action-group_add", _("Add set"));

	DEFINE_ACTION("canvas-new", _("New Canvas"));

	DEFINE_ACTION("amount-inc", _("Increase Amount"));
	DEFINE_ACTION("amount-dec", _("Decrease Amount"));

  //Layout the actions in the main menu (caret menu, right click on canvas menu) and toolbar:
    Glib::ustring ui_info =
"<ui>"
"	<popup name='menu-toolbox' action='menu-toolbox'>"
"	<menu action='menu-file'>"
"	</menu>"
"	</popup>"
"	<popup name='menu-main' action='menu-main'>"
"	<menu action='menu-file'>"
"		<menuitem action='new' />"
"		<menuitem action='open' />"
"		<menuitem action='save' />"
"		<menuitem action='save-as' />"
"		<menuitem action='revert' />"
"		<separator name='bleh01'/>"
"		<menuitem action='cvs-add' />"
"		<menuitem action='cvs-update' />"
"		<menuitem action='cvs-commit' />"
"		<menuitem action='cvs-revert' />"
"		<separator name='bleh02'/>"
"		<menuitem action='import' />"
"		<separator name='bleh03'/>"
"		<menuitem action='render' />"
"		<menuitem action='preview' />"
"		<menuitem action='sound' />"
"		<separator name='bleh04'/>"
"		<menuitem action='options' />"
"		<menuitem action='close' />"
"		<menuitem action='close-document' />"
"		<menuitem action='quit' />"
"	</menu>"
"	<menu action='menu-edit'>"
"		<menuitem action='undo'/>"
"		<menuitem action='redo'/>"
"		<separator name='bleh05'/>"
"		<menuitem action='cut'/>"
"		<menuitem action='copy'/>"
"		<menuitem action='paste'/>"
"		<separator name='bleh06'/>"
"		<menuitem action='select-all-layers'/>"
"		<menuitem action='unselect-all-layers'/>"
"		<menuitem action='select-all-ducks'/>"
"		<menuitem action='unselect-all-ducks'/>"
"		<separator name='bleh07'/>"
"		<menuitem action='properties'/>"
"	</menu>"
"	<menu action='menu-view'>"
"		<menu action='menu-duck-mask'>"
"			<menuitem action='mask-position-ducks' />"
"			<menuitem action='mask-vertex-ducks' />"
"			<menuitem action='mask-tangent-ducks' />"
"			<menuitem action='mask-radius-ducks' />"
"			<menuitem action='mask-width-ducks' />"
"			<menuitem action='mask-angle-ducks' />"
"			<menuitem action='mask-bone-setup-ducks' />"
"			<menuitem action='mask-bone-recursive-ducks' />"
"			<menuitem action='mask-bone-ducks' />"
"			<menuitem action='mask-widthpoint-position-ducks' />"
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
"		<menuitem action='decrease-low-res-pixel-size'/>"
"		<menuitem action='increase-low-res-pixel-size'/>"
"		<separator name='pixel-size-separator'/>"
;

	for(list<int>::iterator iter = CanvasView::get_pixel_sizes().begin(); iter != CanvasView::get_pixel_sizes().end(); iter++)
		ui_info += strprintf("			<menuitem action='lowres-pixel-%d' />", *iter);

	ui_info +=
"		</menu>"
"		<separator name='bleh08'/>"
"		<menuitem action='play'/>"
//"		<menuitem action='pause'/>"
"		<menuitem action='stop'/>"
"		<menuitem action='dialog-flipbook'/>"
"		<separator name='bleh09'/>"
"		<menuitem action='toggle-grid-show'/>"
"		<menuitem action='toggle-grid-snap'/>"
"		<menuitem action='toggle-guide-show'/>"
"		<menuitem action='toggle-guide-snap'/>"
"		<menuitem action='toggle-low-res'/>"
"		<menuitem action='toggle-onion-skin'/>"
"		<separator name='bleh10'/>"
"		<menuitem action='canvas-zoom-in'/>"
"		<menuitem action='canvas-zoom-out'/>"
"		<menuitem action='canvas-zoom-fit'/>"
"		<menuitem action='canvas-zoom-100'/>"
"		<separator name='bleh11'/>"
"		<menuitem action='time-zoom-in'/>"
"		<menuitem action='time-zoom-out'/>"
"		<separator name='bleh12'/>"
"		<menuitem action='jump-next-keyframe'/>"
"		<menuitem action='jump-prev-keyframe'/>"
"		<menuitem action='seek-next-frame'/>"
"		<menuitem action='seek-prev-frame'/>"
"		<menuitem action='seek-next-second'/>"
"		<menuitem action='seek-prev-second'/>"
"		<menuitem action='seek-begin'/>"
"		<menuitem action='seek-end'/>"
"	</menu>"
"	<menu action='menu-canvas'>"
"		<menuitem action='canvas-new'/>"
"	</menu>"
"	<menu name='menu-state' action='menu-state'>"
"	</menu>"
"	<menu action='menu-group'>"
"		<menuitem action='action-group_add'/>"
"	</menu>"
"	<menu action='menu-layer'>"
//"		<menuitem action='cut'/>"
//"		<menuitem action='copy'/>"
//"		<menuitem action='paste'/>"
//"		<separator name='bleh06'/>"
"		<menu action='menu-layer-new'></menu>"
"		<menuitem action='amount-inc'/>"
"		<menuitem action='amount-dec'/>"
"	</menu>"
"	<menu action='menu-keyframe'>"
"		<menuitem action='action-KeyframeAdd'/>"
"		<menuitem action='action-KeyframeDuplicate'/>"
"		<menuitem action='action-KeyframeRemove'/>"
"		<menuitem action='keyframe-properties'/>"
"	</menu>"
"	<menu action='menu-plugins'>"
;

	list<synfigapp::PluginManager::plugin> plugin_list = studio::App::plugin_manager.get_list();
	for(list<synfigapp::PluginManager::plugin>::const_iterator p=plugin_list.begin();p!=plugin_list.end();++p) {

		// TODO: (Plugins) Arrange menu items into groups

		synfigapp::PluginManager::plugin plugin = *p;
		
		DEFINE_ACTION(plugin.id, plugin.name);
		ui_info += strprintf("		<menuitem action='%s'/>", plugin.id.c_str());
	}

	ui_info +=
"	</menu>"
"	</popup>"

"</ui>"
;

	#undef DEFINE_ACTION
	#undef DEFINE_ACTION_2
	#undef DEFINE_ACTION_SIG

/*		"<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='New'/>"
        "      <menuitem action='Open'/>"
        "      <separator/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu action='MenuEdit'>"
        "      <menuitem action='Cut'/>"
        "      <menuitem action='Copy'/>"
        "      <menuitem action='Paste'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
        "    <toolitem action='Open'/>"
        "    <toolitem action='Quit'/>"
        "  </toolbar>"
        "</ui>";
*/
	try
	{
		actions_action_group->set_sensitive(false);
		App::ui_manager()->set_add_tearoffs(true);
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
	ACCEL("<Mod1>a",													"<Actions>/action_group_state_manager/state-normal"					);
	ACCEL("<Mod1>v",													"<Actions>/action_group_state_manager/state-smooth_move"				);
	ACCEL("<Mod1>s",													"<Actions>/action_group_state_manager/state-scale"					);
	ACCEL("<Mod1>t",													"<Actions>/action_group_state_manager/state-rotate"					);
	ACCEL("<Mod1>m",													"<Actions>/action_group_state_manager/state-mirror"					);
	ACCEL("<Mod1>c",													"<Actions>/action_group_state_manager/state-circle"					);
	ACCEL("<Mod1>r",													"<Actions>/action_group_state_manager/state-rectangle"				);
	ACCEL("<Mod1>q",													"<Actions>/action_group_state_manager/state-star"						);
	ACCEL("<Mod1>g",													"<Actions>/action_group_state_manager/state-gradient"					);
	ACCEL("<Mod1>p",													"<Actions>/action_group_state_manager/state-polygon"					);
	ACCEL("<Mod1>b",													"<Actions>/action_group_state_manager/state-bline"					);
	ACCEL("<Mod1>x",													"<Actions>/action_group_state_manager/state-text"						);
	ACCEL("<Mod1>f",													"<Actions>/action_group_state_manager/state-fill"						);
	ACCEL("<Mod1>e",													"<Actions>/action_group_state_manager/state-eyedrop"					);
	ACCEL("<Mod1>z",													"<Actions>/action_group_state_manager/state-zoom"						);
	ACCEL("<Mod1>d",													"<Actions>/action_group_state_manager/state-draw"						);
	ACCEL("<Mod1>k",													"<Actions>/action_group_state_manager/state-sketch"					);
	ACCEL("<Mod1>w",													"<Actions>/action_group_state_manager/state-width"					);

	// everything else
	ACCEL("<Control>a",													"<Actions>/canvasview/select-all-ducks"				);
	ACCEL("<Control>d",													"<Actions>/canvasview/unselect-all-ducks"				);
	ACCEL("<Control><Shift>a",											"<Actions>/canvasview/select-all-layers"				);
	ACCEL("<Control><Shift>d",											"<Actions>/canvasview/unselect-all-layers"			);
	ACCEL("F9",															"<Actions>/canvasview/render"							);
	ACCEL("F11",														"<Actions>/canvasview/preview"						);
	ACCEL("F8",															"<Actions>/canvasview/properties"						);
	ACCEL("F12",														"<Actions>/canvasview/options"						);
	ACCEL("<control>i",													"<Actions>/canvasview/import"							);
	ACCEL2(Gtk::AccelKey(GDK_Escape,static_cast<Gdk::ModifierType>(0),	"<Actions>/canvasview/stop"							));
	ACCEL("<Control>g",													"<Actions>/canvasview/toggle-grid-show"				);
	ACCEL("<Control>l",													"<Actions>/canvasview/toggle-grid-snap"				);
	ACCEL2(Gtk::AccelKey('`',Gdk::CONTROL_MASK,							"<Actions>/canvasview/toggle-low-res"					));
	ACCEL("<Mod1>1",													"<Actions>/canvasview/mask-position-ducks"			);
	ACCEL("<Mod1>2",													"<Actions>/canvasview/mask-vertex-ducks"				);
	ACCEL("<Mod1>3",													"<Actions>/canvasview/mask-tangent-ducks"				);
	ACCEL("<Mod1>4",													"<Actions>/canvasview/mask-radius-ducks"				);
	ACCEL("<Mod1>5",													"<Actions>/canvasview/mask-width-ducks"				);
	ACCEL("<Mod1>6",													"<Actions>/canvasview/mask-angle-ducks"				);
	ACCEL("<Mod1>7",													"<Actions>/canvasview/mask-bone-setup-ducks"			);
	ACCEL("<Mod1>8",													"<Actions>/canvasview/mask-bone-recursive-ducks"		);
	ACCEL("<Mod1>9",													"<Actions>/canvasview/mask-bone-ducks"				);
	ACCEL("<Mod1>5",													"<Actions>/canvasview/mask-widthpoint-position-ducks"				);
	ACCEL2(Gtk::AccelKey(GDK_Page_Up,Gdk::SHIFT_MASK,					"<Actions>/action_group_layer_action_manager/action-LayerRaise"				));
	ACCEL2(Gtk::AccelKey(GDK_Page_Down,Gdk::SHIFT_MASK,					"<Actions>/action_group_layer_action_manager/action-LayerLower"				));
	ACCEL("<Control>1",													"<Actions>/canvasview/quality-01"						);
	ACCEL("<Control>2",													"<Actions>/canvasview/quality-02"						);
	ACCEL("<Control>3",													"<Actions>/canvasview/quality-03"						);
	ACCEL("<Control>4",													"<Actions>/canvasview/quality-04"						);
	ACCEL("<Control>5",													"<Actions>/canvasview/quality-05"						);
	ACCEL("<Control>6",													"<Actions>/canvasview/quality-06"						);
	ACCEL("<Control>7",													"<Actions>/canvasview/quality-07"						);
	ACCEL("<Control>8",													"<Actions>/canvasview/quality-08"						);
	ACCEL("<Control>9",													"<Actions>/canvasview/quality-09"						);
	ACCEL("<Control>0",													"<Actions>/canvasview/quality-10"						);
	ACCEL("<Control>z",													"<Actions>/action_group_dock_history/undo"							);
	ACCEL("<Control>r",													"<Actions>/action_group_dock_history/redo"							);
	ACCEL2(Gtk::AccelKey(GDK_Delete,Gdk::CONTROL_MASK,					"<Actions>/action_group_layer_action_manager/action-LayerRemove"				));
	ACCEL2(Gtk::AccelKey('(',Gdk::CONTROL_MASK,							"<Actions>/canvasview/decrease-low-res-pixel-size"	));
	ACCEL2(Gtk::AccelKey(')',Gdk::CONTROL_MASK,							"<Actions>/canvasview/increase-low-res-pixel-size"	));
	ACCEL2(Gtk::AccelKey('(',Gdk::MOD1_MASK|Gdk::CONTROL_MASK,			"<Actions>/action_group_layer_action_manager/amount-dec"						));
	ACCEL2(Gtk::AccelKey(')',Gdk::MOD1_MASK|Gdk::CONTROL_MASK,			"<Actions>/action_group_layer_action_manager/amount-inc"						));
	ACCEL2(Gtk::AccelKey(']',Gdk::CONTROL_MASK,							"<Actions>/canvasview/jump-next-keyframe"				));
	ACCEL2(Gtk::AccelKey('[',Gdk::CONTROL_MASK,							"<Actions>/canvasview/jump-prev-keyframe"				));
	ACCEL2(Gtk::AccelKey('=',Gdk::CONTROL_MASK,							"<Actions>/canvasview/canvas-zoom-in"					));
	ACCEL2(Gtk::AccelKey('-',Gdk::CONTROL_MASK,							"<Actions>/canvasview/canvas-zoom-out"				));
	ACCEL2(Gtk::AccelKey('+',Gdk::CONTROL_MASK,							"<Actions>/canvasview/time-zoom-in"					));
	ACCEL2(Gtk::AccelKey('_',Gdk::CONTROL_MASK,							"<Actions>/canvasview/time-zoom-out"					));
	ACCEL2(Gtk::AccelKey('.',Gdk::CONTROL_MASK,							"<Actions>/canvasview/seek-next-frame"				));
	ACCEL2(Gtk::AccelKey(',',Gdk::CONTROL_MASK,							"<Actions>/canvasview/seek-prev-frame"				));
	ACCEL2(Gtk::AccelKey('>',Gdk::CONTROL_MASK,							"<Actions>/canvasview/seek-next-second"				));
	ACCEL2(Gtk::AccelKey('<',Gdk::CONTROL_MASK,							"<Actions>/canvasview/seek-prev-second"				));
	ACCEL("<Mod1>o",													"<Actions>/canvasview/toggle-onion-skin"				);
	ACCEL("<Control><Shift>z",											"<Actions>/canvasview/canvas-zoom-fit"				);
	ACCEL("<Control>p",													"<Actions>/canvasview/play"							);
	ACCEL("Home",														"<Actions>/canvasview/seek-begin"						);
	ACCEL("End",														"<Actions>/canvasview/seek-end"						);


#undef ACCEL
#undef ACCEL2
}

#ifdef _WIN32
#define mkdir(x,y) mkdir(x)
#endif

/* === M E T H O D S ======================================================= */

App::App(const synfig::String& basepath, int *argc, char ***argv):
	Gtk::Main(argc,argv),
	IconController(basepath)
{

	app_base_path_=etl::dirname(basepath);

	ui_interface_=new GlobalUIInterface();

	gdk_rgb_init();

	// don't call thread_init() if threads are already initialized
	// on some machines bonobo_init() initialized threads before we get here
	if (!g_thread_supported())
		Glib::thread_init();

	distance_system=Distance::SYSTEM_UNITS;

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
		dialog_error_blocking("Synfig Studio",
			"This copy of Synfig Studio was compiled against a\n"
			"different version of libsynfig than what is currently\n"
			"installed. Synfig Studio will now abort. Try downloading\n"
			"the latest version from the Synfig website at\n"
			"http://synfig.org/en/current-release"
		);
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
	catch(std::runtime_error x)
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
		studio_init_cb.task(_("Loading Plugins..."));
		
		std::string pluginsprefix;
	
		// system plugins path
#ifdef WIN32
		pluginsprefix=App::get_base_path()+ETL_DIRECTORY_SEPARATOR+PLUGIN_DIR;
#else
		pluginsprefix=PLUGIN_DIR;
#endif
		char* synfig_root=getenv("SYNFIG_ROOT");
		if(synfig_root) {
			pluginsprefix=std::string(synfig_root)
				+ETL_DIRECTORY_SEPARATOR+"share"
				+ETL_DIRECTORY_SEPARATOR+"synfig"
				+ETL_DIRECTORY_SEPARATOR+"plugins";
		}
		plugin_manager.load_dir(pluginsprefix);
		
		// user plugins path
		pluginsprefix=Glib::build_filename(synfigapp::Main::get_user_app_directory(),"plugins");
		plugin_manager.load_dir(pluginsprefix);
		
		
		
		studio_init_cb.task(_("Init UI Manager..."));
		App::ui_manager_=studio::UIManager::create();
		init_ui_manager();

		studio_init_cb.task(_("Init Dock Manager..."));
		dock_manager=new studio::DockManager();

		studio_init_cb.task(_("Init State Manager..."));
		state_manager=new StateManager();

		studio_init_cb.task(_("Init Toolbox..."));
		toolbox=new studio::Toolbox();

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

		studio_init_cb.task(_("Init Timetrack..."));
		dock_timetrack = new studio::Dock_Timetrack();
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
		dialog_setup=new studio::Dialog_Setup();

		studio_init_cb.task(_("Init Input Dialog..."));
		dialog_input=new Gtk::InputDialog();
		dialog_input->get_close_button()->signal_clicked().connect( sigc::mem_fun( *dialog_input, &Gtk::InputDialog::hide ) );
		dialog_input->get_save_button()->signal_clicked().connect( sigc::mem_fun( *device_tracker, &DeviceTracker::save_preferences) );

		studio_init_cb.task(_("Init auto recovery..."));
		auto_recover=new AutoRecover();

		studio_init_cb.amount_complete(9250,10000);
		studio_init_cb.task(_("Loading Settings..."));
		load_settings();

		// Init Tools..must be done after load_settings() : accelerators keys
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
		if(!getenv("SYNFIG_DISABLE_DRAW"   )) state_manager->add_state(&state_draw); // Enabled for now.  Let's see whether they're good enough yet.
		if(!getenv("SYNFIG_DISABLE_WIDTH"  )) state_manager->add_state(&state_width); // Enabled since 0.61.09
		state_manager->add_state(&state_fill);
		state_manager->add_state(&state_eyedrop);

		/* other */
		state_manager->add_state(&state_text);
		if(!getenv("SYNFIG_DISABLE_SKETCH" )) state_manager->add_state(&state_sketch);
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
			if (get_ui_interface()->confirmation(_("Crash Recovery"),
					_("Auto recovery file found"),
					_("Synfig Studio seems to have crashed before you could save all your files. "
					  "Recover unsaved changes?"),
					_("Recover"), _("Ignore"))
				== synfigapp::UIInterface::RESPONSE_OK)
			{
				int number_recovered;
				if(!auto_recover->recover(number_recovered))
					if (number_recovered)
						get_ui_interface()->error(_("Unable to fully recover from previous crash"));
					else
						get_ui_interface()->error(_("Unable to recover from previous crash"));
				else
					dialog_warning_blocking(_("Warning"),
						_("Synfig Studio has attempted to recover from a previous crash. "
						"The files that it has recovered are NOT YET SAVED. It would be a "
						"good idea to review them and save them now."));

				if (number_recovered)
					opened_any = true;
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
				open((*argv)[*argc]);
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
		// * http://synfig.org/forums/viewtopic.php?f=1&t=1131&st=0&sk=t&sd=a&start=30
		// * http://synfig.org/forums/viewtopic.php?f=15&t=1062
		dock_manager->show_all_dock_dialogs();

		toolbox->present();

		splash_screen.hide();

#ifdef WIN32
		dialog_warning_blocking(_("Warning"), _("WARNING:\n\nThis version of Synfig Studio have a bug, which can cause computer to hang/freeze when you resize the canvas window.\n\nIf you got affected by this issue, consider pressing ALT+TAB to unfreeze your system and get it back to the working state.\n\nPlease accept our apologies for inconvenience, we hope to get this issue resolved in the future versions."));
#endif
	}
	catch(String x)
	{
		get_ui_interface()->error(_("Unknown exception caught when constructing App.\nThis software may be unstable.") + String("\n\n") + x);
	}
	catch(...)
	{
		get_ui_interface()->error(_("Unknown exception caught when constructing App.\nThis software may be unstable."));
	}
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

	toolbox->hide();

	delete toolbox;

	delete dialog_setup;

	delete dialog_gradient;

	delete dialog_color;

	delete dialog_input;

	delete dock_manager;

	instance_list.clear();
}

synfig::String
App::get_config_file(const synfig::String& file)
{
	return Glib::build_filename(synfigapp::Main::get_user_app_directory(),file);
}

//! set the \a instance's canvas(es) position and size to be those specified in the first entry of recent_files_window_size
void
App::set_recent_file_window_size(etl::handle<Instance> instance)
{

	const std::string &canvas_window_size = *recent_files_window_size.begin();

	if(canvas_window_size.empty())
		return;

	synfig::String::size_type current=0;
	bool seen_root(false), shown_non_root(false);

	while(current != synfig::String::npos)
	{
		// find end of first field (canvas) or return
		synfig::String::size_type separator = canvas_window_size.find_first_of(' ', current);
		if(separator == synfig::String::npos) break;

		// find the canvas
		synfig::Canvas::Handle canvas;
		try {
			String warnings;
			canvas = instance->get_canvas()->find_canvas(String(canvas_window_size, current, separator-current), warnings);
		}
		catch(Exception::IDNotFound) {
			// can't find the canvas; skip to the next canvas or return
			separator = canvas_window_size.find_first_of('\t', current);
			if(separator == synfig::String::npos) return;
			current = separator+1;
			continue;
		}

		if (canvas->is_root())
			seen_root = true;
		else
			shown_non_root = true;

		// check that we have the tab character the ends this canvas' data or return
		current = separator+1;
		separator = canvas_window_size.find_first_of('\t', current);
		if(separator == synfig::String::npos) return;

		int x,y,w,h;
		if(!strscanf(String(canvas_window_size, current, separator-current),"%d %d %d %d",&x, &y, &w, &h))
		{
			current = separator+1;
			continue;
		}
		CanvasView::Handle canvasview = instance->find_canvas_view(canvas);
		canvasview->move(x,y);
		canvasview->resize(w,h);
		canvasview->present();

		current = separator+1;
	}

	if (shown_non_root && !seen_root)
		instance->find_canvas_view(instance->get_canvas())->hide();
}

void
App::add_recent_file(const etl::handle<Instance> instance)
{

	std::string canvas_window_size;

	const Instance::CanvasViewList& cview_list = instance->canvas_view_list();
	Instance::CanvasViewList::const_iterator iter;

	for(iter=cview_list.begin();iter!=cview_list.end();iter++)
	{
		if( !((*iter)->is_visible()) )
			continue;

		etl::handle<synfig::Canvas> canvas = (*iter)->get_canvas();
		int x_pos, y_pos, x_size, y_size;
		(*iter)->get_position(x_pos,y_pos);
		(*iter)->get_size(x_size,y_size);

		canvas_window_size += strprintf("%s %d %d %d %d\t",
										canvas->get_relative_id(canvas->get_root()).c_str(),
										x_pos,  y_pos,
										x_size, y_size);
	}

	add_recent_file(absolute_path(instance->get_file_name()), canvas_window_size);
}

void
App::add_recent_file(const std::string &file_name, const std::string &window_size)
{
	std::string filename(file_name);

	assert(!filename.empty());

	if(filename.empty())
		return;

	// Toss out any "hidden" files
	if(basename(filename)[0]=='.')
		return;

	// If we aren't an absolute path, turn ourselves into one
	if(!is_absolute_path(filename))
		filename=absolute_path(filename);

	std::string old_window_size;

	list<string>::iterator iter;
	list<string>::iterator iter_wsize;
	// Check to see if the file is already on the list.
	// If it is, then remove it from the list
	for(iter=recent_files.begin(), iter_wsize=recent_files_window_size.begin();iter!=recent_files.end();iter++, iter_wsize++)
		if(*iter==filename)
		{
			recent_files.erase(iter);
			old_window_size = *iter_wsize;
			recent_files_window_size.erase(iter_wsize);
			break;
		}


	// Push the filename to the front of the list
	recent_files.push_front(filename);
	if(window_size.empty())
		recent_files_window_size.push_front(old_window_size);
	else
		recent_files_window_size.push_front(window_size);

	// Clean out the files at the end of the list.
	while(recent_files.size()>(unsigned)get_max_recent_files())
	{
		recent_files.pop_back();
		recent_files_window_size.pop_back();
	}

	signal_recent_files_changed_();

	return;
}

static Time::Format _App_time_format(Time::FORMAT_NORMAL);

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
		do{
			std::string filename=get_config_file("recentfiles")+std::string("_window_size");

			std::ofstream file(filename.c_str());

			if(!file)
			{
				synfig::warning("Unable to save %s",filename.c_str());
				break;
			}

			list<string>::reverse_iterator iter;

			for(iter=recent_files_window_size.rbegin();iter!=recent_files_window_size.rend();iter++)
				file<<(*iter).c_str()<<endl;

		}while(0);
		std::string filename=get_config_file("settings");
		synfigapp::Main::settings().save_to_file(filename);

	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to save settings.");
	}
}

void
App::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		{
			std::string filename=get_config_file("accelrc");
			Gtk::AccelMap::load(filename);
		}
		{
			bool window_size_broken = false;

			std::string filename=get_config_file("recentfiles");
			std::string filename_window_size=filename+std::string("_window_size");

			std::ifstream file(filename.c_str());
			std::ifstream file_window_size(filename_window_size.c_str());

			if(!file_window_size)
				window_size_broken = true;

			while(file)
			{
				std::string recent_file;
				std::string recent_file_window_size;
				getline(file,recent_file);
				if(!window_size_broken)
					getline(file_window_size,recent_file_window_size);
				if(!recent_file.empty())
				{
					if(!window_size_broken && !file_window_size)
						window_size_broken = true;
					if (std::ifstream(recent_file.c_str()))
					{
						if(!window_size_broken)
							add_recent_file(recent_file,recent_file_window_size);
						else
							add_recent_file(recent_file);
					}
				}
			}
			if(!window_size_broken && file_window_size)
				window_size_broken = true;

			if(window_size_broken)
			{
				recent_files_window_size.clear();
				recent_files_window_size.resize(recent_files.size());
			}
		}
		std::string filename=get_config_file("settings");
		if(!synfigapp::Main::settings().load_from_file(filename))
		{
			//std::string filename=Glib::locale_from_utf8(Glib::build_filename(Glib::get_home_dir(),".synfigrc"));
			//if(!synfigapp::Main::settings().load_from_file(filename))
			{
				gamma.set_gamma(1.0/2.2);
				reset_initial_window_configuration();
			}
		}

	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load settings.");
	}
}

void
App::reset_initial_window_configuration()
{
	Glib::RefPtr<Gdk::Display> display(Gdk::Display::get_default());
	Glib::RefPtr<const Gdk::Screen> screen(display->get_default_screen());
	Gdk::Rectangle rect;
	// A proper way to obtain the primary monitor is to use the
	// Gdk::Screen::get_primary_monitor () const member. But as it
	// was introduced in gtkmm 2.20 I assume that the monitor 0 is the
	// primary one.
	screen->get_monitor_geometry(0,rect);
#define hpanel_width 79.0f
#define hpanel_height 25.0f
#define vpanel_width 20.0f
#define vpanel_height 100.0f
#define vdock 20.0f
#define hdock 20.0f

/* percentages referred to width or height of the screen
 *---------------------------------------------------------------------*
 *    t   |                                                |
 *    o   |                                                |
 *    o   |                                                |vdock%
 *    l   |                                                |
 *    b   |                                                |------------
 *    o   |                                                |
 *    x   |                                                |vdock%
 * --------                                                |
 *                                                         |
 *                                                         |------------
 *                                                         |
 *                                                         |vdock%
 *                                                         |
 *                                                         |
 *-----hdock%----------------------------------------------|------------
 *             |                                           |
 *             |                                           |vdock%
 *             |                                           |
 *             |                                           |
 * --------------------------------------------------------------------*
*/
// Vertical Panel
	int v_xpos=rect.get_x() + rect.get_width()*(1.0-vpanel_width/100.0);
	int v_xsize=rect.get_width()*vpanel_width/100.0;
	int v_ypos=rect.get_y();
	int v_ysize=rect.get_height()*vpanel_height/100.0;
	std::string v_pos(strprintf("%d %d", v_xpos, v_ypos));
	std::string v_size(strprintf("%d %d", v_xsize, v_ysize));
// Horizontal Panel
	int h_xpos=rect.get_x();
	int h_xsize=rect.get_width()*hpanel_width/100.0;
	int h_ypos=rect.get_y()+ rect.get_height()*(1.0-hpanel_height/100.0);;
	int h_ysize=rect.get_height()*hpanel_height/100.0;
	std::string h_pos(strprintf("%d %d", h_xpos, h_ypos));
	std::string h_size(strprintf("%d %d", h_xsize, h_ysize));
	int v_dock1 = rect.get_height()*vdock*0.8/100.0;
	int v_dock2 = rect.get_height()*vdock*0.6/100.0;
	int v_dock3 = rect.get_height()*vdock*1.1/100.0;
	int h_dock = rect.get_width()*hdock/100.0;
//Contents size
	std::string v_contents(strprintf("%d %d %d", v_dock1, v_dock2, v_dock3));
	std::string h_contents(strprintf("%d", h_dock));
// Tool Box position
	std::string tbox_pos(strprintf("%d %d", rect.get_x(), rect.get_y()));
/*
	synfig::info("tool box pos: %s", tbox_pos.c_str());
	synfig::info("v_contents sizes: %s", v_contents.c_str());
	synfig::info("v_pos: %s", v_pos.c_str());
	synfig::info("v_sizes: %s", v_size.c_str());
	synfig::info("h_contents sizes: %s", h_contents.c_str());
	synfig::info("h_pos: %s", h_pos.c_str());
	synfig::info("h_sizes: %s", h_size.c_str());
*/
	synfigapp::Main::settings().set_value("dock.dialog.1.comp_selector","1");
	synfigapp::Main::settings().set_value("dock.dialog.1.contents","navigator - info pal_edit pal_browse - tool_options history canvases - layers groups");
	synfigapp::Main::settings().set_value("dock.dialog.1.contents_size",v_contents);
	synfigapp::Main::settings().set_value("dock.dialog.1.size",v_size);
	synfigapp::Main::settings().set_value("dock.dialog.1.pos",v_pos);
	synfigapp::Main::settings().set_value("dock.dialog.2.comp_selector","0");
	synfigapp::Main::settings().set_value("dock.dialog.2.contents","params children keyframes | timetrack curves meta_data");
	synfigapp::Main::settings().set_value("dock.dialog.2.contents_size",h_contents);
	synfigapp::Main::settings().set_value("dock.dialog.2.size",h_size);
	synfigapp::Main::settings().set_value("dock.dialog.2.pos",h_pos);
	synfigapp::Main::settings().set_value("window.toolbox.pos",tbox_pos);

	dock_manager->show_all_dock_dialogs();
}

void
App::reset_initial_preferences()
{
	synfigapp::Main::settings().set_value("pref.distance_system","pt");
	synfigapp::Main::settings().set_value("pref.use_colorspace_gamma","1");
#ifdef SINGLE_THREADED
	synfigapp::Main::settings().set_value("pref.single_threaded","1");
#endif
	synfigapp::Main::settings().set_value("pref.restrict_radius_ducks","1");
	synfigapp::Main::settings().set_value("pref.resize_imported_images","0");
	synfigapp::Main::settings().set_value("pref.custom_filename_prefix",DEFAULT_FILENAME_PREFIX);
	synfigapp::Main::settings().set_value("pref.preferred_x_size","480");
	synfigapp::Main::settings().set_value("pref.preferred_y_size","270");
	synfigapp::Main::settings().set_value("pref.predefined_size",DEFAULT_PREDEFINED_SIZE);
	synfigapp::Main::settings().set_value("pref.preferred_fps","24.0");
	synfigapp::Main::settings().set_value("pref.predefined_fps",DEFAULT_PREDEFINED_FPS);
	synfigapp::Main::settings().set_value("sequence_separator", ".");
	synfigapp::Main::settings().set_value("navigator_uses_cairo", "0");
	synfigapp::Main::settings().set_value("workarea_uses_cairo", "0");
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
	if(shutdown_in_progress)return;

	get_ui_interface()->task(_("Quit Request"));
	if(Busy::count)
	{
		dialog_error_blocking(_("Cannot quit!"),_("Tasks are currently running.\nPlease cancel the current tasks and try again"));
		return;
	}

	std::list<etl::handle<Instance> >::iterator iter;
	for(iter=instance_list.begin();!instance_list.empty();iter=instance_list.begin())
	{
		if(!(*iter)->safe_close())
			return;

/*
		if((*iter)->synfigapp::Instance::get_action_count())
		{
			handle<synfigapp::UIInterface> uim;
			uim=(*iter)->find_canvas_view((*iter)->get_canvas())->get_ui_interface();
			assert(uim);
			string str=strprintf(_("Would you like to save your changes to %s?"),(*iter)->get_file_name().c_str() );
			switch(uim->yes_no_cancel((*iter)->get_canvas()->get_name(),str,synfigapp::UIInterface::RESPONSE_YES))
			{
				case synfigapp::UIInterface::RESPONSE_NO:
					break;
				case synfigapp::UIInterface::RESPONSE_YES:
					(*iter)->save();
					break;
				case synfigapp::UIInterface::RESPONSE_CANCEL:
					return;
				default:
					assert(0);
					return;
			}
		}


		if((*iter)->synfigapp::Instance::is_modified())
		{
			handle<synfigapp::UIInterface> uim;
			uim=(*iter)->find_canvas_view((*iter)->get_canvas())->get_ui_interface();
			assert(uim);
			string str=strprintf(_("%s has changes not yet on the CVS repository.\nWould you like to commit these changes?"),(*iter)->get_file_name().c_str() );
			switch(uim->yes_no_cancel((*iter)->get_canvas()->get_name(),str,synfigapp::UIInterface::RESPONSE_YES))
			{
				case synfigapp::UIInterface::RESPONSE_NO:
					break;
				case synfigapp::UIInterface::RESPONSE_YES:
					(*iter)->dialog_cvs_commit();
					break;
				case synfigapp::UIInterface::RESPONSE_CANCEL:
					return;
				default:
					assert(0);
					return;
			}
		}
*/

		// This next line causes things to crash for some reason
		//(*iter)->close();
	}

	instance_list.clear();

	while(studio::App::events_pending())studio::App::iteration(false);

	Gtk::Main::quit();
	auto_recover->normal_shutdown();

	get_ui_interface()->task(_("Quit Request sent"));
}

void
App::show_setup()
{
	dialog_setup->refresh();
	dialog_setup->show();
}

gint Signal_Open_Ok(GtkWidget */*widget*/, int *val){*val=1;return 0;}
gint Signal_Open_Cancel(GtkWidget */*widget*/, int *val){*val=2;return 0;}

//#ifdef WIN32
//#define USE_WIN32_FILE_DIALOGS 1
//#endif

#ifdef USE_WIN32_FILE_DIALOGS
static OPENFILENAME ofn={};
#endif

#ifdef WIN32
#include <gdk/gdkwin32.h>
#endif

bool
App::dialog_open_file(const std::string &title, std::string &filename, std::string preference)
{
	// info("App::dialog_open_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());
	// TODO: Win32 native dialod not ready yet
#ifdef USE_WIN32_FILE_DIALOGS
	static TCHAR szFilter[] = TEXT ("All Files (*.*)\0*.*\0\0") ;

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
		prev_path = ".";

	prev_path = absolute_path(prev_path);

    Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(title, Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog->set_current_folder(prev_path);
    dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_ACCEPT);

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
App::dialog_open_file_with_history_button(const std::string &title, std::string &filename, bool &show_history, std::string preference)
{
	// info("App::dialog_open_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());

// TODO: Win32 native dialog not ready yet
//#ifdef USE_WIN32_FILE_DIALOGS
#if 0
	static TCHAR szFilter[] = TEXT ("All Files (*.*)\0*.*\0\0") ;

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
		prev_path = ".";

	prev_path = absolute_path(prev_path);

    Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(title, Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog->set_current_folder(prev_path);
    dialog->add_button("Open history", RESPONSE_ACCEPT_WITH_HISTORY);
    dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_ACCEPT);

    if (filename.empty())
		dialog->set_filename(prev_path);
	else if (is_absolute_path(filename))
		dialog->set_filename(filename);
	else
		dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);

    int response = dialog->run();
    if (response == Gtk::RESPONSE_ACCEPT || response == RESPONSE_ACCEPT_WITH_HISTORY) {
        filename = dialog->get_filename();
        show_history = response == RESPONSE_ACCEPT_WITH_HISTORY;
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
App::dialog_save_file(const std::string &title, std::string &filename, std::string preference)
{
	// info("App::dialog_save_file('%s', '%s', '%s')", title.c_str(), filename.c_str(), preference.c_str());

#if USE_WIN32_FILE_DIALOGS
	static TCHAR szFilter[] = TEXT ("All Files (*.*)\0*.*\0\0") ;

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
		prev_path=".";

	prev_path = absolute_path(prev_path);

    Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(title, Gtk::FILE_CHOOSER_ACTION_SAVE);

    dialog->set_current_folder(prev_path);
    dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_ACCEPT);

	Widget_Enum *file_type_enum = 0;
	if (preference == ANIMATION_DIR_PREFERENCE)
	{
		file_type_enum = manage(new Widget_Enum());
		file_type_enum->set_param_desc(ParamDesc().set_hint("enum")
				.add_enum_value(synfig::RELEASE_VERSION_0_63_05, "0.64.0", strprintf("0.64.0 (%s)", _("current")))
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

		Gtk::HBox *hbox = manage(new Gtk::HBox);
		hbox->pack_start(*manage(new Gtk::Label(_("File Format Version: "))),Gtk::PACK_SHRINK,0);
		hbox->pack_start(*file_type_enum,Gtk::PACK_EXPAND_WIDGET,0);
		hbox->show_all();

		dialog->set_extra_widget(*hbox);
	}

    if (filename.empty())
		dialog->set_filename(prev_path);
    else
	{
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

    if(dialog->run() == GTK_RESPONSE_ACCEPT) {
		if (preference == ANIMATION_DIR_PREFERENCE)
			set_file_version(synfig::ReleaseVersion(file_type_enum->get_value()));
        filename = dialog->get_filename();
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
App::dialog_select_list_item(const std::string &title, const std::string &message, const std::list<std::string> &list, int &item_index)
{
	Gtk::Dialog dialog(title, true, true);

	Gtk::Label label(message, 0, 0);
	label.set_line_wrap();

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

	Gtk::TreeView tree(list_store);
	Gtk::TreeViewColumn column_index("", model_columns.column_index);
	Gtk::TreeViewColumn column_main("", model_columns.column_main);
	column_index.set_visible(false);
	tree.append_column(column_index);
	tree.append_column(column_main);

	Gtk::TreeModel::Row selected_row = list_store->children()[item_index];
	if (selected_row)
		tree.get_selection()->select(selected_row);

	Gtk::Table table(1, 2);
	table.attach(label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
	table.attach(tree, 0, 1, 1, 2);

	dialog.get_vbox()->pack_start(table);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_ACCEPT);
	dialog.set_default_size(300, 450);
	dialog.show_all();

	if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
		item_index = tree.get_selection()->get_selected()->get_value(model_columns.column_index);
		return true;
	}

	return false;
}


void
App::dialog_error_blocking(const std::string &title, const std::string &message)
{
	dialog_warning_blocking(title, message, Gtk::Stock::DIALOG_ERROR);
}

void
App::dialog_warning_blocking(const std::string &title, const std::string &message, const Gtk::StockID &stock_id)
{
	Gtk::Dialog dialog(title, true, true);
	Gtk::ScrolledWindow scrolled;
	Gtk::Label label(message, 0, 0);
	label.set_line_wrap();
	scrolled.add(label);
	scrolled.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolled.set_shadow_type(Gtk::SHADOW_NONE);
	Gtk::Table table(2, 2);
	table.set_col_spacings(10);
	Gtk::Image image(stock_id, Gtk::IconSize(Gtk::ICON_SIZE_DIALOG));
	table.attach(image, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 1, 1);
	table.attach(scrolled, 1, 2, 0, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
	dialog.get_vbox()->pack_start(table);
	dialog.add_button(Gtk::StockID("gtk-close"),1);
	dialog.set_default_size(450, 200);
	dialog.show_all();
	dialog.run();
}

bool
App::dialog_yes_no(const std::string &title, const std::string &message)
{
	Gtk::Dialog dialog(
		title,		// Title
		true,		// Modal
		true		// use_separator
	);
	Gtk::Label label(message);
	label.show();

	dialog.get_vbox()->pack_start(label);
	dialog.add_button(Gtk::StockID("gtk-yes"),1);
	dialog.add_button(Gtk::StockID("gtk-no"),0);
	dialog.show();
	return dialog.run();
}

int
App::dialog_yes_no_cancel(const std::string &title, const std::string &message)
{
	Gtk::Dialog dialog(
		title,		// Title
		true,		// Modal
		true		// use_separator
	);
	Gtk::Label label(message);
	label.show();

	dialog.get_vbox()->pack_start(label);
	dialog.add_button(Gtk::StockID("gtk-yes"),1);
	dialog.add_button(Gtk::StockID("gtk-no"),0);
	dialog.add_button(Gtk::StockID("gtk-cancel"),2);
	dialog.show();
	return dialog.run();
}

void
App::dialog_not_implemented()
{
	Gtk::MessageDialog dialog(_("Feature not available"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
	dialog.set_secondary_text(_("Sorry, this feature has not yet been implemented."));
	dialog.run();
}

static bool
try_open_url(const std::string &url)
{
#ifdef WIN32
	return ShellExecute(GetDesktopWindow(), "open", url.c_str(), NULL, NULL, SW_SHOW);
#else // !WIN32
	std::vector<std::string> command_line;
	std::vector<std::string> browsers;
	browsers.reserve(23);

	// Browser wrapper scripts
#ifdef USE_OPEN_FOR_URLS
	browsers.push_back("open");              // Apple MacOS X wrapper, on Linux it opens a virtual console
#endif
	browsers.push_back("xdg-open");          // XDG wrapper
	browsers.push_back("sensible-browser");  // Debian wrapper
	browsers.push_back("gnome-open");        // GNOME wrapper
	browsers.push_back("kfmclient");         // KDE wrapper
	browsers.push_back("exo-open");          // XFCE wrapper

	// Alternatives system
	browsers.push_back("gnome-www-browser"); // Debian GNOME alternative
	browsers.push_back("x-www-browser");     // Debian GUI alternative

	// Individual browsers
	browsers.push_back("firefox");
	browsers.push_back("epiphany-browser");
	browsers.push_back("epiphany");
	browsers.push_back("konqueror");
	browsers.push_back("iceweasel");
	browsers.push_back("mozilla");
	browsers.push_back("netscape");
	browsers.push_back("icecat");
	browsers.push_back("galeon");
	browsers.push_back("midori");
	browsers.push_back("safari");
	browsers.push_back("opera");
	browsers.push_back("amaya");
	browsers.push_back("netsurf");
	browsers.push_back("dillo");

	// Try the user-specified browser first
	command_line.push_back(App::browser_command);
	if( command_line[0] == "kfmclient" ) command_line.push_back("openURL");
	command_line.push_back(url);

	try { Glib::spawn_async(".", command_line, Glib::SPAWN_SEARCH_PATH); return true; }
	catch( Glib::SpawnError& exception ){

		while ( !browsers.empty() )
		{
			// Skip the browser if we already tried it
			if( browsers[0] == App::browser_command )
				continue;

			// Construct the command line
			command_line.clear();
			command_line.push_back(browsers[0]);
			if( command_line[0] == "kfmclient" ) command_line.push_back("openURL");
			command_line.push_back(url);

			// Remove the browser from the list
			browsers.erase(browsers.begin());

			// Try to spawn the browser
			try { Glib::spawn_async(".", command_line, Glib::SPAWN_SEARCH_PATH); }
			// Failed, move on to the next one
			catch(Glib::SpawnError& exception){ continue; }
			return true; // No exception means we succeeded!
		}
	}

	return false;
#endif // !WIN32
}

void
App::dialog_help()
{
	if (!try_open_url("http://synfig.org/wiki/Category:Manual"))
	{
		Gtk::MessageDialog dialog(_("Documentation"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true);
		dialog.set_secondary_text(_("Documentation for Synfig Studio is available on the website:\n\nhttp://synfig.org/wiki/Category:Manual"));
		dialog.set_title(_("Help"));
		dialog.run();
	}
}

void
App::open_url(const std::string &url)
{
	if(!try_open_url(url))
	{
		Gtk::MessageDialog dialog(_("No browser was found. Please load this website manually:"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		dialog.set_secondary_text(url);
		dialog.set_title(_("No browser found"));
		dialog.run();
	}
}

bool
App::dialog_entry(const std::string &title, const std::string &message,std::string &text)
{
	Gtk::Dialog dialog(
		title,		// Title
		true,		// Modal
		true);		// use_separator

	Gtk::Label label(message);
	label.show();
	dialog.get_vbox()->pack_start(label);

	Gtk::Entry entry;
	entry.set_text(text);
	entry.show();
	entry.set_activates_default(true);

	dialog.get_vbox()->pack_start(entry);

	dialog.add_button(Gtk::StockID("gtk-ok"),Gtk::RESPONSE_OK);
	dialog.add_button(Gtk::StockID("gtk-cancel"),Gtk::RESPONSE_CANCEL);
	dialog.set_default_response(Gtk::RESPONSE_OK);

	entry.signal_activate().connect(sigc::bind(sigc::mem_fun(dialog,&Gtk::Dialog::response),Gtk::RESPONSE_OK));
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_OK)
		return false;

	text=entry.get_text();

	return true;
}

bool
App::dialog_paragraph(const std::string &title, const std::string &message,std::string &text)
{
	Gtk::Dialog dialog(
		title,		// Title
		true,		// Modal
		true);		// use_separator

	Gtk::Label label(message);
	label.show();
	dialog.get_vbox()->pack_start(label);

	Glib::RefPtr<Gtk::TextBuffer> text_buffer(Gtk::TextBuffer::create());
	text_buffer->set_text(text);
	Gtk::TextView text_view(text_buffer);
	text_view.show();

	dialog.get_vbox()->pack_start(text_view);

	dialog.add_button(Gtk::StockID("gtk-ok"),Gtk::RESPONSE_OK);
	dialog.add_button(Gtk::StockID("gtk-cancel"),Gtk::RESPONSE_CANCEL);
	dialog.set_default_response(Gtk::RESPONSE_OK);

	//text_entry.signal_activate().connect(sigc::bind(sigc::mem_fun(dialog,&Gtk::Dialog::response),Gtk::RESPONSE_OK));
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_OK)
		return false;

	text=text_buffer->get_text();

	return true;
}

bool
App::open(std::string filename)
{
	return open_as(filename,filename);
}

bool
App::open_as(std::string filename,std::string as,synfig::FileContainerZip::file_size_t truncate_storage_size)
{
#ifdef WIN32
    size_t buf_size = PATH_MAX - 1;
    char* long_name = (char*)malloc(buf_size);
    long_name[0] = '\0';
    if(GetLongPathName(as.c_str(),long_name,sizeof(long_name)));
    // when called from autorecover.cpp, filename doesn't exist, and so long_name is empty
    // don't use it if that's the case
    if (long_name[0] != '\0')
        as=String(long_name);
    free(long_name);
#endif

	try
	{
		OneMoment one_moment;
		String errors, warnings;

		// TODO: move literal "container:" into common place
		std::string canvas_filename = filename;
		etl::handle< FileSystemGroup > file_system(new FileSystemGroup(FileSystemNative::instance()));
		etl::handle< FileContainerTemporary > container(new FileContainerTemporary());
		file_system->register_system("#", container);

		// TODO: move literal ".sfg" into common place
		if (etl::filename_extension(filename) == ".sfg")
		{
			if (!container->open_from_history(filename, truncate_storage_size))
				throw (String)strprintf(_("Unable to open container \"%s\"\n\n"),filename.c_str());
			// TODO: move literal "project.sifz" into common place
			canvas_filename = "#project.sifz";
		}
		else
		{
			if (!container->create(std::string()))
				throw (String)strprintf(_("Unable to create container\n\n"),filename.c_str());
		}

		etl::handle<synfig::Canvas> canvas(open_canvas_as(file_system->get_identifier(canvas_filename),as,errors,warnings));
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
				dialog_warning_blocking(_("Warnings"), strprintf("%s:\n\n%s", _("Warnings"), warnings.c_str()));

			if (as.find(custom_filename_prefix.c_str()) != 0)
				add_recent_file(as);

			handle<Instance> instance(Instance::create(canvas, container));

			if(!instance)
				throw (String)strprintf(_("Unable to create instance for \"%s\""),filename.c_str());

			set_recent_file_window_size(instance);

			one_moment.hide();

			if(instance->is_updated() && App::dialog_yes_no(_("CVS Update"), _("There appears to be a newer version of this file available on the CVS repository.\nWould you like to update now? (It would probably be a good idea)")))
				instance->dialog_cvs_update();
		}
	}
	catch(String x)
	{
		dialog_error_blocking(_("Error"), x);
		return false;
	}
	catch(runtime_error x)
	{
		dialog_error_blocking(_("Error"), x.what());
		return false;
	}
	catch(...)
	{
		dialog_error_blocking(_("Error"), _("Uncaught error on file open (BUG)"));
		return false;
	}

	return true;
}

// this is called from autorecover.cpp:
//   App::open_as(get_shadow_file_name(filename),filename)
// other than that, 'filename' and 'as' are the same
bool
App::open_from_temporary_container_as(std::string container_filename_base,std::string as)
{
	try
	{
		OneMoment one_moment;
		String errors, warnings;

		// TODO: move literals "container:" and "project.sifz" into common place
		std::string canvas_filename = "#project.sifz";
		etl::handle< FileSystemGroup > file_system(new FileSystemGroup(FileSystemNative::instance()));
		etl::handle< FileContainerTemporary > container(new FileContainerTemporary());
		file_system->register_system("#", container);

		if (!container->open_temporary(container_filename_base))
			throw (String)strprintf(_("Unable to open temporary container \"%s\"\n\n"),container_filename_base.c_str());

		etl::handle<synfig::Canvas> canvas(open_canvas_as(file_system->get_identifier(canvas_filename),as,errors,warnings));
		if(canvas && get_instance(canvas))
		{
			get_instance(canvas)->find_canvas_view(canvas)->present();
			info("%s is already open", canvas_filename.c_str());
			// throw (String)strprintf(_("\"%s\" appears to already be open!"),filename.c_str());
		}
		else
		{
			if(!canvas)
				throw (String)strprintf(_("Unable to load \"%s\":\n\n"),container_filename_base.c_str()) + errors;

			if (warnings != "")
				dialog_warning_blocking(_("Warnings"), strprintf("%s:\n\n%s", _("Warnings"), warnings.c_str()));

			if (as.find(custom_filename_prefix.c_str()) != 0)
				add_recent_file(as);

			handle<Instance> instance(Instance::create(canvas, container));

			if(!instance)
				throw (String)strprintf(_("Unable to create instance for \"%s\""),container_filename_base.c_str());

			set_recent_file_window_size(instance);

			one_moment.hide();

			if(instance->is_updated() && App::dialog_yes_no(_("CVS Update"), _("There appears to be a newer version of this file available on the CVS repository.\nWould you like to update now? (It would probably be a good idea)")))
				instance->dialog_cvs_update();
		}
	}
	catch(String x)
	{
		dialog_error_blocking(_("Error"), x);
		return false;
	}
	catch(runtime_error x)
	{
		dialog_error_blocking(_("Error"), x.what());
		return false;
	}
	catch(...)
	{
		dialog_error_blocking(_("Error"), _("Uncaught error on file open (BUG)"));
		return false;
	}

	return true;
}


void
App::new_instance()
{
	handle<synfig::Canvas> canvas=synfig::Canvas::create();

	String file_name(strprintf("%s%d", App::custom_filename_prefix.c_str(), Instance::get_count()+1));
	canvas->set_name(file_name);
	file_name += ".sifz";

	canvas->rend_desc().set_frame_rate(preferred_fps);
	canvas->rend_desc().set_time_start(0.0);
	canvas->rend_desc().set_time_end(5.0);
	canvas->rend_desc().set_x_res(DPI2DPM(72.0f));
	canvas->rend_desc().set_y_res(DPI2DPM(72.0f));
	// The top left and botton right positions are expressed in units
	// Original convention is that 1 unit = 60 pixels
	canvas->rend_desc().set_tl(Vector(-(preferred_x_size/60.0)/2.0,(preferred_y_size/60.0)/2.0));
	canvas->rend_desc().set_br(Vector((preferred_x_size/60.0)/2.0,-(preferred_y_size/60.0)/2.0));
	canvas->rend_desc().set_w(preferred_x_size);
	canvas->rend_desc().set_h(preferred_y_size);
	canvas->rend_desc().set_antialias(1);
	canvas->rend_desc().set_flags(RendDesc::PX_ASPECT|RendDesc::IM_SPAN);
	canvas->set_file_name(file_name);
	canvas->keyframe_list().add(synfig::Keyframe());

	etl::handle< FileSystemGroup > file_system(new FileSystemGroup(FileSystemNative::instance()));
	etl::handle< FileContainerTemporary > container(new FileContainerTemporary());
	file_system->register_system("#", container);
	container->create(std::string());
	canvas->set_identifier(file_system->get_identifier(file_name));

	handle<Instance> instance = Instance::create(canvas, container);

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
	if (filename.empty())
		filename="*.sif";

	bool show_history = false;
	while(dialog_open_file_with_history_button("Open", filename, show_history, ANIMATION_DIR_PREFERENCE))
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
			if (!dialog_select_list_item(_("Open"), _("Select one of previous versions of file"), list, index))
				continue;

			// find selected entry in list (descending)
			for(std::list<FileContainerZip::HistoryRecord>::const_reverse_iterator i = history.rbegin(); i != history.rend(); i++)
				if (0 == index--)
					truncate_storage_size = i->storage_size;
		}

		if(open_as(filename,filename,truncate_storage_size))
			break;

		get_ui_interface()->error(_("Unable to open file"));
	}
}

void
App::set_selected_instance(etl::loose_handle<Instance> instance)
{
/*	if(get_selected_instance()==instance)
	{
		selected_instance=instance;
		signal_instance_selected()(instance);
		return;
	}
	else
	{
*/
		selected_instance=instance;
		if(get_selected_canvas_view() && get_selected_canvas_view()->get_instance()!=instance)
		{
			if(instance)
			{
				instance->focus(instance->get_canvas());
			}
			else
				set_selected_canvas_view(0);
		}
		signal_instance_selected()(instance);
}

void
App::set_selected_canvas_view(etl::loose_handle<CanvasView> canvas_view)
{
	selected_canvas_view=canvas_view;
	signal_canvas_view_focus()(selected_canvas_view);
	if(canvas_view)
	{
		selected_instance=canvas_view->get_instance();
		signal_instance_selected()(selected_instance);
	}
/*
	if(get_selected_canvas_view()==canvas_view)
	{
		signal_canvas_view_focus()(selected_canvas_view);
		signal_instance_selected()(canvas_view->get_instance());
		return;
	}
	selected_canvas_view=canvas_view;
	if(canvas_view && canvas_view->get_instance() != get_selected_instance())
		set_selected_instance(canvas_view->get_instance());
	signal_canvas_view_focus()(selected_canvas_view);
*/
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
	return app_base_path_;
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
