/* === S Y N F I G ========================================================= */
/*!	\file keyframeactionmanager.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "keyframeactionmanager.h"

#include <glibmm/main.h>
#include <giomm/themedicon.h>


#include <synfig/general.h>

#include <gui/iconcontroller.h>
#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/trees/keyframetree.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static const std::string group_name {"keyframe"};

/* === P R O C E D U R E S ================================================= */

static Glib::RefPtr<Gio::MenuItem>
create_menu_item_for_synfigapp_action(const std::string& group_prefix, const std::string& action_name)
{
	auto action_it = synfigapp::Action::book().find(action_name);
	if (action_it == synfigapp::Action::book().end()) {
		synfig::error(_("Internal error: can't find synfigapp action to create its menu item: '%s'"), action_name.c_str());
		return {}; // FIXME: SHOULD RETURN NULL OR an empty MenuItem?
	}

	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	auto item = Gio::MenuItem::create(action_it->second.local_name, strprintf("%s.action-%s", group_prefix.c_str(), action_name.c_str()));
	item->set_icon(Gio::ThemedIcon::create(get_action_icon_name(action_it->second) + symbolic_suffix));
	return item;
}

static Glib::RefPtr<Gio::MenuItem>
create_menu_item_for_action(const std::string& action_name, const std::string& icon_name, const std::string& label)
{
	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	auto item = Gio::MenuItem::create(label, action_name);
	if (!icon_name.empty())
		item->set_icon(Gio::ThemedIcon::create(icon_name + symbolic_suffix));
	return item;
}

/* === M E T H O D S ======================================================= */

KeyframeActionManager::KeyframeActionManager():
	keyframe_tree_(),
	action_group_(Gio::SimpleActionGroup::create()),
	queued(false)
{ }

KeyframeActionManager::~KeyframeActionManager()
{ }

void
KeyframeActionManager::set_action_widget_and_menu(Gtk::Widget* x, Glib::RefPtr<Gio::Menu>& menu_keyframe)
{
	// clear();

	action_widget_ = x;
	menu_keyframe_ = menu_keyframe;
}

void
KeyframeActionManager::set_keyframe_tree(KeyframeTree* x)
{
	selection_changed_connection.disconnect();
	keyframe_tree_ = x;

	if (keyframe_tree_) {
		selection_changed_connection = keyframe_tree_->get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &KeyframeActionManager::queue_refresh) );
	}
}

void
KeyframeActionManager::set_canvas_interface(const etl::handle<synfigapp::CanvasInterface> &x)
{
	time_changed_connection.disconnect();
	canvas_interface_ = x;

	// refresh keyframes list connected animation time position change
	if (canvas_interface_) {
		time_changed_connection = canvas_interface_->signal_time_changed().connect(
			sigc::mem_fun(*this, &KeyframeActionManager::queue_refresh) );
	}
}

Glib::RefPtr<Gio::SimpleActionGroup>
KeyframeActionManager::get_action_group() const
{
	return action_group_;
}

void
KeyframeActionManager::clear()
{
	if (action_group_) {
		auto actions = action_group_->list_actions();
		for (const auto& action_name : actions) {
			action_group_->remove_action(action_name);
		}
	}

	if (action_widget_) {
		action_widget_->remove_action_group(group_name);
	}

	if (menu_keyframe_) {
		menu_keyframe_->remove_all();
	}
}

void
KeyframeActionManager::queue_refresh()
{
	if (queued)
		return;

	queue_refresh_connection.disconnect();
	queue_refresh_connection = Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::mem_fun(*this, &KeyframeActionManager::refresh),
			false ));

	queued = true;
}

/*! \fn KeyframeActionManager::on_keyframe_properties()
**	\brief Signal handler for selected keyframe properties
*/
void
KeyframeActionManager::on_keyframe_properties()
{
	signal_show_keyframe_properties_();
}

/*! \fn KeyframeActionManager::on_keyframe_toggle()
**	\brief Signal handler for selected keyframe toggle
*/
void
KeyframeActionManager::on_keyframe_toggle()
{
	signal_keyframe_toggle_();
}

/*! \fn KeyframeActionManager::on_keyframe_description_set()
**	\brief Signal handler for selected keyframe description change
*/
void
KeyframeActionManager::on_keyframe_description_set()
{
	signal_keyframe_description_set_();
}

/*! \fn KeyframeActionManager::on_add_keyframe()
**	\brief Signal handler for add keyframe
*/
void
KeyframeActionManager::on_add_keyframe()
{
	synfigapp::Action::Handle action(synfigapp::Action::create("KeyframeAdd"));

	if(!action)
		return;

	action->set_param("canvas",canvas_interface_->get_canvas());
	action->set_param("canvas_interface",canvas_interface_);
	action->set_param("keyframe",Keyframe(canvas_interface_->get_time()));

	canvas_interface_->get_instance()->perform_action(action);
}

/*! \fn KeyframeActionManager::refresh()
**	\brief Refresh the action and signals connection for the selected keyframe
*/
void
KeyframeActionManager::refresh()
{
	KeyframeTreeStore::Model model;

	queued = false;
	queue_refresh_connection.disconnect();

	clear();

	// Make sure we are ready
	if (!action_widget_ || !keyframe_tree_ || !canvas_interface_) {
		synfig::error("KeyframeActionManager::refresh(): Not ready!");
		return;
	}

	{
		synfigapp::Action::ParamList param_list;
		param_list.add("time",get_canvas_interface()->get_time());
		param_list.add("canvas",get_canvas_interface()->get_canvas());
		param_list.add("canvas_interface",get_canvas_interface());
		if(keyframe_tree_->get_selection()->count_selected_rows()==1)
		{
			Keyframe keyframe((*keyframe_tree_->get_selection()->get_selected())[model.keyframe]);
			param_list.add("keyframe",keyframe);
		}

		etl::handle<studio::Instance>::cast_static(
			get_canvas_interface()->get_instance()
		)->add_actions_to_group(
			action_group_,
			param_list,
			synfigapp::Action::CATEGORY_KEYFRAME
		);
	}

	if (action_group_->lookup_action("action-KeyframeAdd")) {
		action_group_->remove("action-KeyframeAdd");
	}

	auto action_kf_add = action_group_->add_action("action-KeyframeAdd", sigc::mem_fun(*this, &KeyframeActionManager::on_add_keyframe));

	//Keyframe properties definition
	auto action_kf_properties = action_group_->add_action("properties", sigc::mem_fun(*this, &KeyframeActionManager::on_keyframe_properties));

	// Keyframe activate status definition
	auto action_kf_toggle = action_group_->add_action("toggle",sigc::mem_fun(*this, &KeyframeActionManager::on_keyframe_toggle));

	// Keyframe description definition
	auto action_kf_description = action_group_->add_action("description-set",sigc::mem_fun(*this, &KeyframeActionManager::on_keyframe_description_set));

	//activate actions depending on context
	{
		//get the keyframe at current time
		bool kf_at_current_time = false;
		KeyframeList::iterator iter;
		if (canvas_interface_->get_canvas()->keyframe_list().find(canvas_interface_->get_time(), iter)) {
			kf_at_current_time = true;
			if (auto action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group_->lookup_action("action-KeyframeDuplicate")))
				action->set_enabled(false);
		}

		//get the beginning and ending time of the time slider
		Time begin_time=canvas_interface_->get_canvas()->rend_desc().get_time_start();
		Time end_time=canvas_interface_->get_canvas()->rend_desc().get_time_end();
		//enable add key frame action if animation duration != 0
		action_kf_add->set_enabled(!kf_at_current_time && (begin_time != end_time));

		if (keyframe_tree_->get_selection()->count_selected_rows() == 0) {
			action_kf_properties->set_enabled(false);
			action_kf_toggle->set_enabled(false);
			action_kf_description->set_enabled(false);
		}
	}

	// this popup menu is used from widget_keyframe_list

	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	auto item = create_menu_item_for_synfigapp_action("keyframe", "KeyframeAdd");
	menu_keyframe_->append_item(item);
	item = create_menu_item_for_synfigapp_action("keyframe", "KeyframeDuplicate");
	menu_keyframe_->append_item(item);
	item = create_menu_item_for_synfigapp_action("keyframe", "KeyframeRemove");
	menu_keyframe_->append_item(item);
	item = create_menu_item_for_action("keyframe.properties", "document-properties", _("Keyframe Properties"));
	menu_keyframe_->append_item(item);
	item = create_menu_item_for_action("keyframe.toggle", "", _("Toggle Keyframe"));
	menu_keyframe_->append_item(item);
	item = create_menu_item_for_action("keyframe.description-set", "", _("Set Keyframe Description"));
	menu_keyframe_->append_item(item);

	action_widget_->insert_action_group(group_name, action_group_);
}

Glib::RefPtr<Gio::Menu>
KeyframeActionManager::get_menu_for_selected_keyframe() const
{
	return menu_keyframe_;
}
