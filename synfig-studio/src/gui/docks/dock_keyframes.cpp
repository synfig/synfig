/* === S Y N F I G ========================================================= */
/*!	\file dock_keyframes.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "docks/dock_keyframes.h"

#include <cassert>

#include <gtkmm/stylecontext.h>


#include <gui/app.h>
#include <synfig/general.h>

#include <gui/actiondatabase.h>
#include <gui/actionmanagers/keyframeactionmanager.h>
#include <gui/actionwidgethelper.h>
#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/trees/keyframetree.h>
#include <gui/trees/keyframetreestore.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/** Check if action needs more parameters than the basic ones to modify a single selected keyframe only */
static bool
check_if_synfigapp_action_requires_more_params(const synfigapp::Action::BookEntry& entry)
{
	const synfigapp::Action::ParamVocab params = entry.get_param_vocab(); // need to avoid incomplete actions!
	const std::vector<std::string> known_params {"time", "canvas", "canvas_interface", "keyframe"};

	for (const auto& param : params) {
		if (!param.get_optional())
			if (std::find(known_params.begin(), known_params.end(), param.get_name()) == known_params.end())
				return true;
		if (param.get_requires_multiple())
			if (param.get_name() == "keyframe")
				return true;
	}
	return false;
}

/* === M E T H O D S ======================================================= */

Dock_Keyframes::Dock_Keyframes():
	Dock_CanvasSpecific("keyframes", _("Keyframes"),"keyframe_icon"),
	keyframe_action_manager(new KeyframeActionManager())
{
	set_name("keyframes_panel");
	// Make Keyframes toolbar small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	if (keyframe_action_manager)
		keyframe_action_manager->set_action_widget_and_menu(App::main_window, App::menu_keyframe);

	if (App::get_action_database()) {

		// Register all synfigapp actions of Keyframe category with 'keyframe' prefix: 'keyframe.action-SYNFIGAPP_ACTION_NAME'
		// and does not require anything but current time, canvas, canvas_interface and keyframe
		for (const auto& item : synfigapp::Action::book()) {
			const auto& entry = item.second;
			if (entry.category & synfigapp::Action::CATEGORY_KEYFRAME && !(entry.category & synfigapp::Action::CATEGORY_HIDDEN)) {
				if (check_if_synfigapp_action_requires_more_params(entry))
					continue;
				const std::string full_action_name = synfig::strprintf("%s.action-%s", "keyframe", entry.name.c_str());
				App::get_action_database()->add({full_action_name, entry.local_name, {}, studio::get_action_icon_name(entry)});
			}
		}

		// Register custom actions of this dock with 'keyframe' prefix: 'keyframe.CUSTOM_ACTION_NAME'
		App::get_action_database()->add({"keyframe.properties", _("Keyframe Properties"), {}, "document-properties"});
		App::get_action_database()->add({"keyframe.toggle", _("Toggle Keyframe"), {}, ""});
		App::get_action_database()->add({"keyframe.description-set", _("Set Keyframe Description"), {}, ""});
	}

	keyframe_action_manager->signal_show_keyframe_properties().connect(
		sigc::mem_fun(*this,&Dock_Keyframes::show_keyframe_properties) );
	keyframe_action_manager->signal_keyframe_toggle().connect(
		sigc::mem_fun(*this,&Dock_Keyframes::keyframe_toggle) );
	keyframe_action_manager->signal_keyframe_description_set().connect(
		sigc::mem_fun(*this,&Dock_Keyframes::keyframe_description_set) );

	auto toolbar = Gtk::manage(new Gtk::Toolbar());
	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("keyframe", "KeyframeAdd"));
	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("keyframe", "KeyframeDuplicate"));
	toolbar->append(*ActionWidgetHelper::create_synfigapp_action_toolbutton("keyframe", "KeyframeRemove"));
	toolbar->append(*ActionWidgetHelper::create_action_toolbutton("keyframe.properties"));
	toolbar->show_all();
	set_toolbar(*toolbar);
}

Dock_Keyframes::~Dock_Keyframes()
{
	delete keyframe_action_manager;
}

void
Dock_Keyframes::show_keyframe_properties()
{
	if (get_canvas_view())
		get_canvas_view()->show_dialog_for_selected_keyframe();
}

void
Dock_Keyframes::keyframe_toggle()
{
	if (get_canvas_view())
		get_canvas_view()->toggle_selected_keyframe();
}

void
Dock_Keyframes::keyframe_description_set()
{
	if (get_canvas_view())
		get_canvas_view()->set_description_for_selected_keyframe();
}

/*! \fn Dock_Keyframes::refresh_rend_desc()
**	\brief Signal handler for animation render description change
*/
void
Dock_Keyframes::refresh_rend_desc()
{
	keyframe_action_manager->queue_refresh();
}

void
Dock_Keyframes::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	Glib::RefPtr<KeyframeTreeStore> keyframe_tree_store;
	keyframe_tree_store = KeyframeTreeStore::create(canvas_view->canvas_interface());

	KeyframeTree* keyframe_tree(new KeyframeTree());
	keyframe_tree->set_model(keyframe_tree_store);
	keyframe_tree->set_editable(true);

	canvas_view->set_tree_model(get_name(),keyframe_tree_store);
	canvas_view->set_ext_widget(get_name(),keyframe_tree);

	if (!App::menu_keyframe)
		App::menu_keyframe = keyframe_action_manager->get_context_menu_for_keyframe();

	// keyframe actions status are connected to animation duration
	canvas_view->canvas_interface()->signal_rend_desc_changed().connect(
		sigc::mem_fun(*this,&studio::Dock_Keyframes::refresh_rend_desc) );
}

void
Dock_Keyframes::changed_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	if (canvas_view) {
		Gtk::Widget* tree_view = canvas_view->get_ext_widget(get_name());
		assert(tree_view);

		tree_view->show();
		add(*tree_view);

		keyframe_action_manager->set_keyframe_tree(dynamic_cast<KeyframeTree*>(tree_view));
		keyframe_action_manager->set_canvas_interface(canvas_view->canvas_interface());
		keyframe_action_manager->queue_refresh();
	} else {
		keyframe_action_manager->clear();
		keyframe_action_manager->set_keyframe_tree(nullptr);
		keyframe_action_manager->set_canvas_interface(nullptr);
	}
}
