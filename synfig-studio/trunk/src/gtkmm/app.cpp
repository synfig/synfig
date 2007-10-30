/* === S Y N F I G ========================================================= */
/*!	\file app.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include <gtk/gtk.h>

#include <synfig/loadcanvas.h>

#include "app.h"
#include "about.h"
#include "instance.h"
#include "canvasview.h"
#include "dialog_setup.h"
#include "dialog_gradient.h"
#include "dialog_color.h"
#include "toolbox.h"
#include "compview.h"
#include "onemoment.h"

#include "dockmanager.h"

#include "state_eyedrop.h"
#include "state_normal.h"
#include "state_draw.h"
#include "state_fill.h"
#include "state_bline.h"
#include "state_polygon.h"
#include "state_sketch.h"
#include "state_gradient.h"
#include "state_circle.h"
#include "state_rectangle.h"
#include "state_smoothmove.h"
#include "state_scale.h"
#include "state_width.h"
#include "state_rotate.h"
#include "state_zoom.h"

#include "devicetracker.h"
#include "dialog_tooloptions.h"

#include "autorecover.h"

#include <synfigapp/settings.h>
#include "dock_history.h"
#include "dock_canvases.h"
#include "dock_keyframes.h"
#include "dock_layers.h"
#include "dock_params.h"
#include "dock_metadata.h"
#include "dock_children.h"
#include "dock_info.h"
#include "dock_navigator.h"
#include "dock_layergroups.h"
#include "dock_timetrack.h"
#include "dock_curves.h"

#include "mod_palette/mod_palette.h"
#include "mod_mirror/mod_mirror.h"

#include <sys/stat.h>

#include "ipc.h"

#include "module.h"

#include "statemanager.h"

#ifdef WITH_FMOD
#include <fmod.h>
#endif

#ifdef WIN32
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif
#include <gtkmm/accelmap.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/filechooserdialog.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef SYNFIG_USER_APP_DIR
#ifdef __APPLE__
#define SYNFIG_USER_APP_DIR	"Library/Synfig"
#elif defined(_WIN32)
#define SYNFIG_USER_APP_DIR	"Synfig"
#else
#define SYNFIG_USER_APP_DIR	".synfig"
#endif
#endif

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
bool studio::App::single_threaded=false;

static int max_recent_files_=25;
int studio::App::get_max_recent_files() { return max_recent_files_; }
void studio::App::set_max_recent_files(int x) { max_recent_files_=x; }

static synfig::String app_base_path_;

namespace studio {

bool
really_delete_widget(Gtk::Widget *widget)
{
	synfig::info("really delete %x", (unsigned int)widget);
	delete widget;
	return false;
}

// nasty workaround - when we've finished with a popup menu, we want to delete it
// attaching to the signal_hide() signal gets us here before the action on the menu has run,
// so schedule the real delete to happen in 50ms, giving the action a chance to run
void
delete_widget(Gtk::Widget *widget)
{
	synfig::info("delete %x", (unsigned int)widget);
	Glib::signal_timeout().connect(sigc::bind(sigc::ptr_fun(&really_delete_widget), widget), 50);
}

}; // END of namespace studio
studio::StateManager* state_manager;




class GlobalUIInterface : public synfigapp::UIInterface
{
public:

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
		std::cerr<<task<<std::endl;
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
		std::cerr<<"warning: "<<err<<std::endl;
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

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef union {
	struct {
		U32 serial;
		U32 checksum;
	} element;
	U8	raw[8];
} V_KeyUnwound;

static inline U32 hash_U32(U32 i)
{
	i=i*1664525+1013904223;
	i=i*1664525+1013904223;
	i=i*1664525+1013904223;
	return i;
}

#ifdef BIG_ENDIAN
static const int endian_fix_table[8] = { 3, 2, 1, 0, 7, 6, 5, 4 } ;
#define endian_fix(x) (endian_fix_table[x])
#else
#define endian_fix(x) (x)
#endif

int v_unwind_key(V_KeyUnwound* unwound, const char* key)
{
	int i;
	unwound->element.serial=0;
	unwound->element.checksum=0;

	for(i=0;i<16;i++)
	{
		U8 data;

		switch(key[i])
		{
			case '0': data=0; break;
			case '1': data=1; break;
			case '2': data=2; break;
			case '3': data=3; break;
			case '4': data=4; break;
			case '5': data=5; break;
			case '6': data=6; break;
			case '7': data=7; break;
			case '8': data=8; break;
			case '9': data=9; break;
			case 'a': case 'A':  data=10; break;
			case 'b': case 'B':  data=11; break;
			case 'c': case 'C':  data=12; break;
			case 'd': case 'D':  data=13; break;
			case 'e': case 'E':  data=14; break;
			case 'f': case 'F':  data=15; break;
			default: return 0; break;
		}
		int bit=i*2;
		unwound->element.checksum|=(((U32)data&3)<<bit);
		unwound->element.serial|=(((U32)(data>>2)&3)<<bit);
	}
	return 1;
}

int v_key_check(const char* key, U32* serial, U32 appid)
{
	V_KeyUnwound unwound_key;
	U32 appid_mask_a=hash_U32(appid);
	U32 appid_mask_b=hash_U32(appid_mask_a);

	if(!v_unwind_key(&unwound_key, key))
	{
		// Invalid characters in key
		return 0;
	}


	// Undo obfuscation pass
	{
		U32 next=hash_U32(unwound_key.raw[endian_fix(7)]);
		int i;
		for(i=0;i<7;i++)
		{
			next=hash_U32(next);
			unwound_key.raw[endian_fix(i)]^=(next>>24);
		}
	}

	unwound_key.element.serial^=appid_mask_a;
	unwound_key.element.checksum^=appid_mask_b;

	*serial=unwound_key.element.serial;

	return unwound_key.element.checksum==hash_U32(unwound_key.element.serial);
}


#ifdef _WIN32
# ifdef LICENSE_KEY_REQUIRED
int check_license(String basedir)
# else
int check_license(String /*basedir*/)
# endif
#else
int check_license(String /*basedir*/)
#endif
{
#ifdef LICENSE_KEY_REQUIRED
	String key;
	String license_file;

#ifndef _WIN32
	license_file="/usr/local/etc/.synfiglicense";
#else
	license_file=basedir+"\\etc\\.synfiglicense";
#endif

	try {
		key=Glib::file_get_contents(license_file);
	} catch (Glib::FileError) { }
	U32 serial(0);
	if(!v_key_check(key.c_str(),&serial,0xdeadbeef))
	{
		while(!v_key_check(key.c_str(),&serial,0xdeadbeef))
		{
			key.clear();

			if(!App::dialog_entry(
				_("Synfig Studio Authentication"),
				_("Please enter your license key below. You will not\nbe able to use this software without a valid license key."),
				key
			))
				throw String("No License");
		}

		FILE* file=fopen(license_file.c_str(),"w");
		if(file)
		{
			fprintf(file,"%s",key.c_str());
			fclose(file);
		}
		else
			synfig::error("Unable to save license key!");
	}
	synfig::info("License Authenticated -- Serial #%05d",serial);
	return serial;
#else
	return 1;
#endif
}

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
	DEBUGPOINT();
	std::list<Glib::RefPtr<Gtk::ActionGroup> > prev_groups(ui_manager->get_action_groups());
	std::list<Glib::RefPtr<Gtk::ActionGroup> >::reverse_iterator iter;

	DEBUGPOINT();
	for(iter=prev_groups.rbegin();iter!=prev_groups.rend();++iter)
	{
		DEBUGPOINT();
		if(*iter && (*iter)->get_name()!="menus")
		{
			synfig::info("Removing action group "+(*iter)->get_name());
			ui_manager->remove_action_group(*iter);
		}
	}
	DEBUGPOINT();
	ui_manager->insert_action_group(group,0);

	DEBUGPOINT();
	for(;!prev_groups.empty();prev_groups.pop_front())
	{
		if(prev_groups.front() && prev_groups.front()!=group && prev_groups.front()->get_name()!="menus")
			ui_manager->insert_action_group(prev_groups.front(),1);
	}
	DEBUGPOINT();
}
*/
class Preferences : public synfigapp::Settings
{
public:
	virtual bool get_value(const synfig::String& key, synfig::String& value)const
	{
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
		if(key=="single_threaded")
		{
			value=strprintf("%i",(int)App::single_threaded);
			return true;
		}
		if(key=="auto_recover_backup_interval")
		{
			value=strprintf("%i",App::auto_recover->get_timeout());
			return true;
		}

		return synfigapp::Settings::get_value(key,value);
	}

	virtual bool set_value(const synfig::String& key,const synfig::String& value)
	{
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
		if(key=="single_threaded")
		{
			int i(atoi(value.c_str()));
			App::single_threaded=i;
			return true;
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
		ret.push_back("single_threaded");
		ret.push_back("auto_recover_backup_interval");
		return ret;
	}
};

static ::Preferences _preferences;

void
init_ui_manager()
{
	Glib::RefPtr<Gtk::ActionGroup> menus_action_group = Gtk::ActionGroup::create("menus");

	Glib::RefPtr<Gtk::ActionGroup> toolbox_action_group = Gtk::ActionGroup::create("toolbox");

	Glib::RefPtr<Gtk::ActionGroup> actions_action_group = Gtk::ActionGroup::create();

	menus_action_group->add( Gtk::Action::create("menu-file", "_File") );
	menus_action_group->add( Gtk::Action::create("menu-edit", "_Edit") );
	menus_action_group->add( Gtk::Action::create("menu-view", "_View") );
	menus_action_group->add( Gtk::Action::create("menu-canvas", "_Canvas") );
	menus_action_group->add( Gtk::Action::create("menu-layer", "_Layer") );
	menus_action_group->add( Gtk::Action::create("menu-duck-mask", "Show/Hide Ducks") );
	menus_action_group->add( Gtk::Action::create("menu-preview-quality", "Preview Quality") );
	menus_action_group->add( Gtk::Action::create("menu-layer-new", "New Layer") );
	menus_action_group->add( Gtk::Action::create("menu-keyframe", "Keyframe") );
	menus_action_group->add( Gtk::Action::create("menu-group", "Group") );
	menus_action_group->add( Gtk::Action::create("menu-state", "State") );
	menus_action_group->add( Gtk::Action::create("menu-toolbox", "Toolbox") );

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

#define DEFINE_ACTION(x,stock) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock) ); /*action->set_sensitive(false);*/ actions_action_group->add(action); }
#define DEFINE_ACTION2(x,stock,label) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock,label,label) ); /*action->set_sensitive(false);*/ actions_action_group->add(action); }
#define DEFINE_ACTION_SIG(group,x,stock,sig) { Glib::RefPtr<Gtk::Action> action( Gtk::Action::create(x, stock) ); /*action->set_sensitive(false);*/ group->add(action,sig); }

	DEFINE_ACTION2("keyframe-properties", Gtk::StockID("gtk-properties"), _("Keyframe Properties"));
	DEFINE_ACTION("about", Gtk::StockID("synfig-about"));
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


	DEFINE_ACTION("undo", Gtk::StockID("gtk-undo"));
	DEFINE_ACTION("redo", Gtk::StockID("gtk-redo"));
	DEFINE_ACTION("cut", Gtk::StockID("gtk-cut"));
	DEFINE_ACTION("copy", Gtk::StockID("gtk-copy"));
	DEFINE_ACTION("paste", Gtk::StockID("gtk-paste"));
	DEFINE_ACTION("select-all-ducks", _("Select All Ducks"));
	DEFINE_ACTION("unselect-all-layers", _("Unselect All Layers"));
	DEFINE_ACTION("properties", _("Properties"));

	DEFINE_ACTION("mask-position-ducks", _("Show Position Ducks"));
	DEFINE_ACTION("mask-vertex-ducks", _("Show Vertex Ducks"));
	DEFINE_ACTION("mask-tangent-ducks", _("Show Tangent Ducks"));
	DEFINE_ACTION("mask-radius-ducks", _("Show Radius Ducks"));
	DEFINE_ACTION("mask-width-ducks", _("Show Width Ducks"));
	DEFINE_ACTION("mask-angle-ducks", _("Show Angle Ducks"));
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
	DEFINE_ACTION("play", _("Play"));
	// DEFINE_ACTION("pause", _("Pause"));
	DEFINE_ACTION("stop", _("Stop"));
	DEFINE_ACTION("toggle-grid-show", _("Toggle Grid Show"));
	DEFINE_ACTION("toggle-grid-snap", _("Toggle Grid Snap"));
	DEFINE_ACTION("toggle-guide-show", _("Toggle Guide Show"));
	DEFINE_ACTION("toggle-low-res", _("Toggle Low-Res"));
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

	DEFINE_ACTION("action-group_add", _("Add group"));

	DEFINE_ACTION("canvas-new", _("New Canvas"));

	DEFINE_ACTION("amount-inc", _("Increase Amount"));
	DEFINE_ACTION("amount-dec", _("Decrease Amount"));

#undef DEFINE_ACTION


// Set up synfigapp actions
	/*{
		synfigapp::Action::Book::iterator iter;

		for(iter=synfigapp::Action::book().begin();iter!=synfigapp::Action::book().end();++iter)
		{
			Gtk::StockID stock_id;

			if(!(iter->second.category&synfigapp::Action::CATEGORY_HIDDEN))
			{
				//Gtk::Image* image(manage(new Gtk::Image()));
				if(iter->second.task=="raise")			stock_id=Gtk::Stock::GO_UP;
				else if(iter->second.task=="lower")		stock_id=Gtk::Stock::GO_DOWN;
				else if(iter->second.task=="move_top")	stock_id=Gtk::Stock::GOTO_TOP;
				else if(iter->second.task=="move_bottom")	stock_id=Gtk::Stock::GOTO_BOTTOM;
				else if(iter->second.task=="remove")	stock_id=Gtk::Stock::DELETE;
				else if(iter->second.task=="set_on")	stock_id=Gtk::Stock::YES;
				else if(iter->second.task=="set_off")	stock_id=Gtk::Stock::NO;
				//else if(iter->second.task=="duplicate")	stock_id=Gtk::Stock::COPY;
				else if(iter->second.task=="remove")	stock_id=Gtk::Stock::DELETE;
				else									stock_id=Gtk::StockID("synfig-"+iter->second.task);

				actions_action_group->add(Gtk::Action::create(
					"action-"+iter->second.name,
					stock_id,
					iter->second.local_name,iter->second.local_name
				));
			}
		}
	}
*/


    Glib::ustring ui_info =
"<ui>"
"	<popup name='menu-toolbox' action='menu-toolbox'>"
"	<menu action='menu-file'>"
"	</menu>"
"	</popup>"
"	<popup name='menu-main' action='menu-main'>"
"	<menu action='menu-file'>"
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
"	</menu>"
"	<menu action='menu-edit'>"
"		<menuitem action='undo'/>"
"		<menuitem action='redo'/>"
"		<separator name='bleh05'/>"
"		<menuitem action='cut'/>"
"		<menuitem action='copy'/>"
"		<menuitem action='paste'/>"
"		<separator name='bleh06'/>"
"		<menuitem action='select-all-ducks'/>"
"		<menuitem action='unselect-all-layers'/>"
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
"		<separator name='bleh08'/>"
"		<menuitem action='play'/>"
//"		<menuitem action='pause'/>"
"		<menuitem action='stop'/>"
"		<menuitem action='dialog-flipbook'/>"
"		<separator name='bleh09'/>"
"		<menuitem action='toggle-grid-show'/>"
"		<menuitem action='toggle-grid-snap'/>"
"		<menuitem action='toggle-guide-show'/>"
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
"		<menuitem action='keyframe-properties'/>"
"	</menu>"
"	</popup>"

"</ui>"
;
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
#define ACCEL(path,accel)						\
	{											\
		Gtk::AccelKey accel_key(accel,path);	\
		Gtk::AccelMap::add_entry(accel_key.get_path(), accel_key.get_key(), accel_key.get_mod());	\
	}

#define ACCEL2(accel)							\
	{											\
		Gtk::AccelKey accel_key(accel);			\
		Gtk::AccelMap::add_entry(accel_key.get_path(), accel_key.get_key(), accel_key.get_mod());	\
	}

	ACCEL("<Actions>//select-all-ducks","<Control>a");
	ACCEL("<Actions>//unselect-all-layers","<Control>d");
	ACCEL("<Actions>//render","F9");
	ACCEL("<Actions>//preview","F11");
	ACCEL("<Actions>//properties","F8");
	ACCEL("<Actions>//options","F12");
	ACCEL("<Actions>//import","<control>i");
	ACCEL2(Gtk::AccelKey(GDK_Escape,static_cast<Gdk::ModifierType>(0),"<Actions>//stop"));
	ACCEL("<Actions>//toggle-grid-show","<Control>g");
	ACCEL("<Actions>//toggle-grid-snap","<Control>l");
	ACCEL2(Gtk::AccelKey('`',Gdk::CONTROL_MASK,"<Actions>//toggle-low-res"));
	ACCEL("<Actions>//mask-position-ducks", "<Mod1>1");
	ACCEL("<Actions>//mask-vertex-ducks", "<Mod1>2");
	ACCEL("<Actions>//mask-tangent-ducks", "<Mod1>3");
	ACCEL("<Actions>//mask-radius-ducks", "<Mod1>4");
	ACCEL("<Actions>//mask-width-ducks", "<Mod1>5");
	ACCEL("<Actions>//mask-angle-ducks", "<Mod1>6");

	ACCEL2(Gtk::AccelKey(GDK_Page_Up,Gdk::SHIFT_MASK,"<Actions>//action-layer_raise"));
	ACCEL2(Gtk::AccelKey(GDK_Page_Down,Gdk::SHIFT_MASK,"<Actions>//action-layer_lower"));

	ACCEL("<Actions>//quality-01","<Control>1");
	ACCEL("<Actions>//quality-02","<Control>2");
	ACCEL("<Actions>//quality-03","<Control>3");
	ACCEL("<Actions>//quality-04","<Control>4");
	ACCEL("<Actions>//quality-05","<Control>5");
	ACCEL("<Actions>//quality-06","<Control>6");
	ACCEL("<Actions>//quality-07","<Control>7");
	ACCEL("<Actions>//quality-08","<Control>8");
	ACCEL("<Actions>//quality-09","<Control>9");
	ACCEL("<Actions>//quality-10","<Control>0");
	ACCEL("<Actions>//undo","<Control>z");
	ACCEL("<Actions>//redo","<Control>r");
	ACCEL("<Actions>//action-layer_remove","Delete");

/*	ACCEL2(Gtk::AccelKey(']',static_cast<Gdk::ModifierType>(0),"<Actions>//jump-next-keyframe"));
	ACCEL2(Gtk::AccelKey('[',static_cast<Gdk::ModifierType>(0),"<Actions>//jump-prev-keyframe"));
	ACCEL2(Gtk::AccelKey('=',static_cast<Gdk::ModifierType>(0),"<Actions>//canvas-zoom-in"));
	ACCEL2(Gtk::AccelKey('-',static_cast<Gdk::ModifierType>(0),"<Actions>//canvas-zoom-out"));
	ACCEL("<Actions>//time-zoom-in","+");
	ACCEL("<Actions>//time-zoom-out","_");
*/
	ACCEL2(Gtk::AccelKey('(',Gdk::MOD1_MASK|Gdk::CONTROL_MASK,"<Actions>//amount-dec"));
	ACCEL2(Gtk::AccelKey(')',Gdk::MOD1_MASK|Gdk::CONTROL_MASK,"<Actions>//amount-inc"));

	ACCEL2(Gtk::AccelKey(']',Gdk::CONTROL_MASK,"<Actions>//jump-next-keyframe"));
	ACCEL2(Gtk::AccelKey('[',Gdk::CONTROL_MASK,"<Actions>//jump-prev-keyframe"));
	ACCEL2(Gtk::AccelKey('=',Gdk::CONTROL_MASK,"<Actions>//canvas-zoom-in"));
	ACCEL2(Gtk::AccelKey('-',Gdk::CONTROL_MASK,"<Actions>//canvas-zoom-out"));
	ACCEL2(Gtk::AccelKey('+',Gdk::CONTROL_MASK,"<Actions>//time-zoom-in"));
	ACCEL2(Gtk::AccelKey('_',Gdk::CONTROL_MASK,"<Actions>//time-zoom-out"));
	ACCEL2(Gtk::AccelKey('.',Gdk::CONTROL_MASK,"<Actions>//seek-next-frame"));
	ACCEL2(Gtk::AccelKey(',',Gdk::CONTROL_MASK,"<Actions>//seek-prev-frame"));
	ACCEL2(Gtk::AccelKey('>',Gdk::CONTROL_MASK,"<Actions>//seek-next-second"));
	ACCEL2(Gtk::AccelKey('<',Gdk::CONTROL_MASK,"<Actions>//seek-prev-second"));
	ACCEL2(Gtk::AccelKey('o',Gdk::CONTROL_MASK,"<Actions>//toggle-onion-skin"));
	ACCEL("<Actions>//play",              "<Control>p");
	ACCEL("<Actions>//seek-begin","Home");
	ACCEL("<Actions>//seek-end","End");

	ACCEL("<Actions>//state-normal",      "<Mod1>a");
	ACCEL("<Actions>//state-smooth_move", "<Mod1>v");
	ACCEL("<Actions>//state-scale",       "<Mod1>d");
	ACCEL("<Actions>//state-rotate",      "<Mod1>s");

	ACCEL("<Actions>//state-bline",       "<Mod1>b");
	ACCEL("<Actions>//state-circle",      "<Mod1>c");
	ACCEL("<Actions>//state-rectangle",   "<Mod1>r");
	ACCEL("<Actions>//state-gradient",    "<Mod1>g");

	ACCEL("<Actions>//state-eyedrop",     "<Mod1>e");
	ACCEL("<Actions>//state-fill",        "<Mod1>f");
	ACCEL("<Actions>//state-zoom",        "<Mod1>z");
	ACCEL("<Actions>//state-polygon",     "<Mod1>p");

	ACCEL("<Actions>//state-draw",        "<Mod1>w");
	ACCEL("<Actions>//state-sketch",      "<Mod1>k");
	ACCEL("<Actions>//state-width",       "<Mod1>t");
	ACCEL("<Actions>//state-mirror",      "<Mod1>m");

	ACCEL("<Actions>//canvas-zoom-fit","<Control><Shift>z");

#undef ACCEL
}

#ifdef _WIN32
#define mkdir(x,y) mkdir(x)
#endif

/* === M E T H O D S ======================================================= */

App::App(int *argc, char ***argv):
	Gtk::Main(argc,argv),
	IconController(etl::dirname((*argv)[0]))
{
	app_base_path_=etl::dirname(etl::dirname((*argv)[0]));

	int serial_;
	serial_=check_license(app_base_path_);


	ui_interface_=new GlobalUIInterface();

	gdk_rgb_init();

	// don't call thread_init() if threads are already initialized
	// on some machines bonobo_init() initialized threads before we get here
	if (!g_thread_supported())
		Glib::thread_init();

	distance_system=Distance::SYSTEM_UNITS;

	if(mkdir(get_user_app_directory().c_str(),ACCESSPERMS)<0)
	{
		if(errno!=EEXIST)
			synfig::error("UNABLE TO CREATE \"%s\"",get_user_app_directory().c_str());
	}
	else
	{
		synfig::info("Created directory \"%s\"",get_user_app_directory().c_str());
	}


	ipc=new IPC();

	try
	{
		if(!SYNFIG_CHECK_VERSION())
		{
		cerr<<"FATAL: Synfig Version Mismatch"<<endl;
		dialog_error_blocking("Synfig Studio",
			"This copy of Synfig Studio was compiled against a\n"
			"different version of libsynfig than what is currently\n"
			"installed. Synfig Studio will now abort. Try downloading\n"
			"the latest version from the Synfig website at\n"
			"http://www.synfig.com/ "
		);
		throw 40;
		}
	}
	catch(synfig::SoftwareExpired)
	{
		cerr<<"FATAL: Software Expired"<<endl;
		dialog_error_blocking("Synfig Studio",
			"This copy of Synfig Studio has expired.\n"
			"Please erase this copy, or download and\n"
			"install the latest copy from the Synfig\n"
			"website at http://www.synfig.com/ ."
		);
		throw 39;
	}
	Glib::set_application_name(_("Synfig Studio"));

	About about_window;
	about_window.set_can_self_destruct(false);
	about_window.show();

	shutdown_in_progress=false;
	SuperCallback synfig_init_cb(about_window.get_callback(),0,9000,10000);
	SuperCallback studio_init_cb(about_window.get_callback(),9000,10000,10000);

	// Initialize the Synfig library
	try { synfigapp_main=etl::smart_ptr<synfigapp::Main>(new synfigapp::Main(etl::dirname((*argv)[0]),&synfig_init_cb)); }
	catch(...)
	{
		get_ui_interface()->error("Failed to initialize synfig!");
		throw;
	}

	// add the preferences to the settings
	synfigapp::Main::settings().add_domain(&_preferences,"pref");

	try
	{
		studio_init_cb.task("Init UI Manager...");
		App::ui_manager_=studio::UIManager::create();
		init_ui_manager();

		studio_init_cb.task("Init Dock Manager...");
		dock_manager=new studio::DockManager();

		studio_init_cb.task("Init State Manager...");
		state_manager=new StateManager();

		studio_init_cb.task("Init Toolbox...");
		toolbox=new studio::Toolbox();

		studio_init_cb.task("Init Tool Options...");
		dialog_tool_options=new studio::Dialog_ToolOptions();
		dock_manager->register_dockable(*dialog_tool_options);

		studio_init_cb.task("Init History...");
		dock_history=new studio::Dock_History();
		dock_manager->register_dockable(*dock_history);

		studio_init_cb.task("Init Canvases...");
		dock_canvases=new studio::Dock_Canvases();
		dock_manager->register_dockable(*dock_canvases);

		studio_init_cb.task("Init Keyframes...");
		dock_keyframes=new studio::Dock_Keyframes();
		dock_manager->register_dockable(*dock_keyframes);

		studio_init_cb.task("Init Layers...");
		dock_layers=new studio::Dock_Layers();
		dock_manager->register_dockable(*dock_layers);

		studio_init_cb.task("Init Params...");
		dock_params=new studio::Dock_Params();
		dock_manager->register_dockable(*dock_params);

		studio_init_cb.task("Init MetaData...");
		dock_meta_data=new studio::Dock_MetaData();
		dock_manager->register_dockable(*dock_meta_data);

		studio_init_cb.task("Init Children...");
		dock_children=new studio::Dock_Children();
		dock_manager->register_dockable(*dock_children);

		studio_init_cb.task("Init Info...");
		dock_info = new studio::Dock_Info();
		dock_manager->register_dockable(*dock_info);

		studio_init_cb.task("Init Navigator...");
		dock_navigator = new studio::Dock_Navigator();
		dock_manager->register_dockable(*dock_navigator);

		studio_init_cb.task("Init Timetrack...");
		dock_timetrack = new studio::Dock_Timetrack();
		dock_manager->register_dockable(*dock_timetrack);

		studio_init_cb.task("Init Curve Editor...");
		dock_curves = new studio::Dock_Curves();
		dock_manager->register_dockable(*dock_curves);

		studio_init_cb.task("Init Layer Groups...");
		dock_layer_groups = new studio::Dock_LayerGroups();
		dock_manager->register_dockable(*dock_layer_groups);


		studio_init_cb.task("Init Color Dialog...");
		dialog_color=new studio::Dialog_Color();

		studio_init_cb.task("Init Gradient Dialog...");
		dialog_gradient=new studio::Dialog_Gradient();

		studio_init_cb.task("Init DeviceTracker...");
		device_tracker=new studio::DeviceTracker();

		studio_init_cb.task("Init Tools...");
		state_manager->add_state(&state_normal);
		state_manager->add_state(&state_smooth_move);
		state_manager->add_state(&state_scale);
		state_manager->add_state(&state_rotate);

		state_manager->add_state(&state_bline);


		state_manager->add_state(&state_circle);
		state_manager->add_state(&state_rectangle);

		state_manager->add_state(&state_gradient);
		state_manager->add_state(&state_eyedrop);
		state_manager->add_state(&state_fill);

		state_manager->add_state(&state_zoom);

		// Enabled - it's useful to be able to work with polygons without tangent ducks getting in the way.
		// I know we can switch tangent ducks off, but why not allow this kind of layer as well?
		if(!getenv("SYNFIG_DISABLE_POLYGON")) state_manager->add_state(&state_polygon);

		// Enabled for now.  Let's see whether they're good enough yet.
		if(!getenv("SYNFIG_DISABLE_DRAW"   )) state_manager->add_state(&state_draw);
		if(!getenv("SYNFIG_DISABLE_SKETCH" )) state_manager->add_state(&state_sketch);

		// Disabled by default - it doesn't work properly?
		if(getenv("SYNFIG_ENABLE_WIDTH"    )) state_manager->add_state(&state_width);

		studio_init_cb.task("Init ModPalette...");
		module_list_.push_back(new ModPalette()); module_list_.back()->start();

		studio_init_cb.task("Init ModMirror...");
		module_list_.push_back(new ModMirror()); module_list_.back()->start();


		studio_init_cb.task("Init Setup Dialog...");
		dialog_setup=new studio::Dialog_Setup();

		studio_init_cb.task("Init Input Dialog...");
		dialog_input=new Gtk::InputDialog();
		dialog_input->get_close_button()->signal_clicked().connect( sigc::mem_fun( *dialog_input, &Gtk::InputDialog::hide ) );
		dialog_input->get_save_button()->signal_clicked().connect( sigc::ptr_fun(studio::App::dialog_not_implemented) );

		studio_init_cb.task("Init auto recovery...");
		auto_recover=new AutoRecover();

		studio_init_cb.amount_complete(9250,10000);
		studio_init_cb.task("Loading Settings...");
		load_settings();
		studio_init_cb.task("Checking auto-recover...");

		studio_init_cb.amount_complete(9900,10000);

		if(auto_recover->recovery_needed())
		{
			about_window.hide();
			if(
				get_ui_interface()->yes_no(
					"Auto Recovery",
					"Synfig Studio seems to have crashed\n"
					"before you could save all your files.\n"
					"Would you like to re-open those files\n"
					"and recover your unsaved changes?"
				)==synfigapp::UIInterface::RESPONSE_YES
			)
			{
				if(!auto_recover->recover())
				{
					get_ui_interface()->error(_("Unable to fully recover from previous crash"));
				}
				else
				get_ui_interface()->error(
					_("Synfig Studio has attempted to recover\n"
					"from a previous crash. The files that it has\n"
					"recovered are NOT YET SAVED. It would be a good\n"
					"idea to review them and save them now.")
				);
			}
			about_window.show();
		}

		// Look for any files given on the command line,
		// and load them if found.
		for(;*argc>=1;(*argc)--)
			if((*argv)[*argc] && (*argv)[*argc][0]!='-')
			{
				studio_init_cb.task("Loading files...");
				about_window.hide();
				open((*argv)[*argc]);
				about_window.show();
			}

		studio_init_cb.task("Done.");
		studio_init_cb.amount_complete(10000,10000);

		toolbox->present();
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
	for(;!module_list_.empty();module_list_.pop_back());

	delete state_manager;

	delete ipc;

	delete auto_recover;

	toolbox->hide();

//	studio::App::iteration(false);

	delete toolbox;

//	studio::App::iteration(false);

//	studio::App::iteration(false);

	delete dialog_setup;

	delete dialog_gradient;

	delete dialog_color;

	delete dialog_input;

	delete dock_manager;

	instance_list.clear();

//	studio::App::iteration(false);
}

String
App::get_user_app_directory()
{
	return Glib::build_filename(Glib::get_home_dir(),SYNFIG_USER_APP_DIR);
}

synfig::String
App::get_config_file(const synfig::String& file)
{
	return Glib::build_filename(get_user_app_directory(),file);
}

void
App::add_recent_file(const std::string &file_name)
{
	std::string filename(file_name);

	assert(!filename.empty());

	if(filename.empty())
		return;

	// Toss out any "hidden" files
	if(basename(filename)[0]=='.')
		return;

	// If we aren't an absolute path, turn outselves into one
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
		recent_files.pop_back();

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
	char * old_locale;
	try
	{
	old_locale=strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
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
				file<<*iter<<endl;
		}while(0);

		std::string filename=get_config_file("settings");
		synfigapp::Main::settings().save_to_file(filename);
	setlocale(LC_NUMERIC,old_locale);
	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to save settings.");
	}
}

void
App::load_settings()
{
	char  * old_locale;
	try
	{
	old_locale=strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
		{
			std::string filename=get_config_file("accelrc");
			Gtk::AccelMap::load(filename);
		}
		{
			std::string filename=get_config_file("recentfiles");

			std::ifstream file(filename.c_str());

			while(file)
			{
				std::string recent_file;
				getline(file,recent_file);
				if(!recent_file.empty())
					add_recent_file(recent_file);
			}
		}
		std::string filename=get_config_file("settings");
		if(!synfigapp::Main::settings().load_from_file(filename))
		{
			//std::string filename=Glib::build_filename(Glib::get_home_dir(),".synfigrc");
			//if(!synfigapp::Main::settings().load_from_file(filename))
			{
				gamma.set_gamma(1.0/2.2);
				reset_initial_window_configuration();
			}
		}
	setlocale(LC_NUMERIC,old_locale);
	}
	catch(...)
	{
		synfig::warning("Caught exception when attempting to load settings.");
	}
}

void
App::reset_initial_window_configuration()
{
	synfigapp::Main::settings().set_value("dock.dialog.1.comp_selector","1");
	synfigapp::Main::settings().set_value("dock.dialog.1.contents","navigator - info pal_edit pal_browse - tool_options history canvases - layers groups");
	synfigapp::Main::settings().set_value("dock.dialog.1.contents_size","225 167 207");
	synfigapp::Main::settings().set_value("dock.dialog.1.pos","1057 32");
	synfigapp::Main::settings().set_value("dock.dialog.1.size","208 1174");
	synfigapp::Main::settings().set_value("dock.dialog.2.comp_selector","0");
	synfigapp::Main::settings().set_value("dock.dialog.2.contents","params children keyframes | timetrack curves meta_data");
	synfigapp::Main::settings().set_value("dock.dialog.2.contents_size","263");
	synfigapp::Main::settings().set_value("dock.dialog.2.pos","0 973");
	synfigapp::Main::settings().set_value("dock.dialog.2.size","1045 235");
	synfigapp::Main::settings().set_value("pref.distance_system","pt");
	synfigapp::Main::settings().set_value("pref.use_colorspace_gamma","1");
	synfigapp::Main::settings().set_value("pref.single_threaded","0");
	synfigapp::Main::settings().set_value("window.toolbox.pos","4 4");
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


	get_ui_interface()->task("Quit Request");
	if(Busy::count)
	{
		dialog_error_blocking("Cannot quit!","Tasks are currently running.\nPlease cancel the current tasks and try again");
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

	shutdown_in_progress=true;

	instance_list.clear();

	while(studio::App::events_pending())studio::App::iteration(false);

	Gtk::Main::quit();
	auto_recover->normal_shutdown();

	get_ui_interface()->task("Quit Request sent");
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
App::dialog_open_file(const std::string &title, std::string &filename)
{
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

#else
	synfig::String prev_path;
	if(!_preferences.get_value("curr_path",prev_path))
		prev_path=".";
	prev_path = absolute_path(prev_path);

    Gtk::FileChooserDialog *dialog=new Gtk::FileChooserDialog(title,Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog->set_current_folder(prev_path);
    dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_ACCEPT);
    if(!filename.empty())
		if (is_absolute_path(filename))
			dialog->set_filename(filename);
		else
			dialog->set_filename(prev_path + ETL_DIRECTORY_SEPARATOR + filename);
    if(dialog->run()==GTK_RESPONSE_ACCEPT) {
        filename=dialog->get_filename();
        delete dialog;
        return true;
    }
    delete dialog;
    return false;
    /*

	GtkWidget *ok;
	GtkWidget *cancel;
	int val=0;

	GtkWidget *fileselection;
	fileselection = gtk_file_selection_new(title.c_str());


	if(basename(filename)==filename)
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(fileselection),(prev_path+ETL_DIRECTORY_SEPARATOR).c_str());
	}
	else
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(fileselection),dirname(filename).c_str());

	gtk_file_selection_complete(GTK_FILE_SELECTION(fileselection),basename(filename).c_str());

	ok=GTK_FILE_SELECTION(fileselection)->ok_button;
	cancel=GTK_FILE_SELECTION(fileselection)->cancel_button;

	gtk_signal_connect(GTK_OBJECT(ok),"clicked",GTK_SIGNAL_FUNC(Signal_Open_Ok),&val);
	gtk_signal_connect(GTK_OBJECT(cancel),"clicked",GTK_SIGNAL_FUNC(Signal_Open_Cancel),&val);

	gtk_widget_show(fileselection);

	while(!val)
		iteration();


	if(val==1)
	{
		filename=gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileselection));
		_preferences.set_value("curr_path",dirname(filename));
	}
	else
	{
		gtk_widget_destroy(fileselection);
		return false;
	}
	gtk_widget_destroy(fileselection);
	return true;
    */
#endif
}

bool
App::dialog_save_file(const std::string &title, std::string &filename)
{
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
		_preferences.set_value("curr_path",dirname(filename));
		return true;
	}
	return false;
#else
	synfig::String prev_path;
	if(!_preferences.get_value("curr_path",prev_path))
		prev_path=".";
	prev_path = absolute_path(prev_path);

    Gtk::FileChooserDialog *dialog=new Gtk::FileChooserDialog(title,Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog->set_current_folder(prev_path);
    dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_ACCEPT);
    if(!filename.empty())
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
    if(dialog->run()==GTK_RESPONSE_ACCEPT) {
        filename=dialog->get_filename();
        delete dialog;
		_preferences.set_value("curr_path",dirname(filename));
        return true;
    }
    delete dialog;
    return false;
//	return dialog_open_file(title, filename);
#endif
}

void
App::dialog_error_blocking(const std::string &title, const std::string &message)
{
	Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
	dialog.set_title(title);
	dialog.show();
	dialog.run();
}

void
App::dialog_warning_blocking(const std::string &title, const std::string &message)
{
	Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE, true);
	dialog.set_title(title);
	dialog.show();
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
	Gtk::MessageDialog dialog("Feature not available", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
	dialog.set_secondary_text("Sorry, this feature has not yet been implemented.");
	dialog.run();
}

bool
App::dialog_entry(const std::string &title, const std::string &message,std::string &text)
{
	Gtk::Dialog dialog(
		title,		// Title
		true,		// Modal
		true		// use_separator
	);
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
App::open(std::string filename)
{
	return open_as(filename,filename);
}

// this is called from autorecover.cpp:
//   App::open_as(get_shadow_file_name(filename),filename)
// other than that, 'filename' and 'as' are the same
bool
App::open_as(std::string filename,std::string as)
{
#ifdef WIN32
    char long_name[1024];
    if(GetLongPathName(as.c_str(),long_name,sizeof(long_name)));
    as=long_name;
#endif

	try
	{
		OneMoment one_moment;

		etl::handle<synfig::Canvas> canvas(open_canvas_as(filename,as));
		if(canvas && get_instance(canvas))
		{
			get_instance(canvas)->find_canvas_view(canvas)->present();
			throw (String)strprintf(_("\"%s\" appears to already be open!"),filename.c_str());
		}
		if(!canvas)
			throw (String)strprintf(_("Unable to open file \"%s\""),filename.c_str());

		add_recent_file(as);

		handle<Instance> instance(Instance::create(canvas));

		if(!instance)
			throw (String)strprintf(_("Unable to create instance for \"%s\""),filename.c_str());

		one_moment.hide();

		if(instance->is_updated() && App::dialog_yes_no(_("CVS Update"), _("There appears to be a newer version of this file available on the CVS repository.\nWould you like to update now? (It would probably be a good idea)")))
			instance->dialog_cvs_update();
	}
	catch(String x)
	{
		dialog_error_blocking(_("Error"), x);
		return false;
	}
	catch(...)
	{
		dialog_error_blocking(_("Error"), _("Uncaught error on file open (BUG)"));
		return false;
	}

	_preferences.set_value("curr_path",dirname(as));

	return true;
}


void
App::new_instance()
{
	handle<synfig::Canvas> canvas=synfig::Canvas::create();
	canvas->set_name(strprintf("%s%d", DEFAULT_FILENAME_PREFIX, Instance::get_count()+1));

	String file_name(strprintf("%s%d.sifz", DEFAULT_FILENAME_PREFIX, Instance::get_count()+1));

	canvas->rend_desc().set_frame_rate(24.0);
	canvas->rend_desc().set_time_start(0.0);
	canvas->rend_desc().set_time_end(00.0);
	canvas->rend_desc().set_x_res(DPI2DPM(72.0f));
	canvas->rend_desc().set_y_res(DPI2DPM(72.0f));
	canvas->rend_desc().set_tl(Vector(-4,2.25));
	canvas->rend_desc().set_br(Vector(4,-2.25));
	canvas->rend_desc().set_w(480);
	canvas->rend_desc().set_h(270);
	canvas->rend_desc().set_antialias(1);
	canvas->rend_desc().set_flags(RendDesc::PX_ASPECT|RendDesc::IM_SPAN);
	canvas->set_file_name(file_name);

	Instance::create(canvas)->find_canvas_view(canvas)->canvas_properties.present();
}

void
App::dialog_open()
{
	string filename="*.sif";

	while(dialog_open_file("Open", filename))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if(find(filename.begin(),filename.end(),'*')!=filename.end())
			continue;

		if(open(filename))
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
		signal_instance_selected()(canvas_view->get_instance());
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
App::get_instance(Canvas::Handle canvas)
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
	(new class About())->show();
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
