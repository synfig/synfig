/* === S Y N F I G ========================================================= */
/*!	\file groupactionmanager.cpp
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

#include "groupactionmanager.h"

#include <glibmm/main.h>

#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/trees/layergrouptree.h>
#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

//#define ONE_ACTION_GROUP 1

/* === G L O B A L S ======================================================= */

static const std::string group_name {"layer-set"};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

GroupActionManager::GroupActionManager():
	group_tree_(),
	action_group_(Gio::SimpleActionGroup::create()),
	queued(false)
{ }

GroupActionManager::~GroupActionManager()
{ }

void
GroupActionManager::set_action_widget(Gtk::Widget* x)
{
	clear();

	action_widget_ = x;
}

void
GroupActionManager::set_group_tree(LayerGroupTree* x)
{
	selection_changed_connection.disconnect();
	group_tree_=x;
	if(group_tree_)
	{
		selection_changed_connection=group_tree_->get_selection()->signal_changed().connect(
			sigc::mem_fun(*this,&GroupActionManager::queue_refresh)
		);
	}
}

void
GroupActionManager::set_canvas_interface(const etl::handle<synfigapp::CanvasInterface> &x)
{
	canvas_interface_=x;
}

void
GroupActionManager::clear()
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
}

void
GroupActionManager::queue_refresh()
{
	if(queued)
		return;

	//queue_refresh_connection.disconnect();
	queue_refresh_connection=Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&GroupActionManager::refresh),
			false
		)
	);

	queued=true;
}

void
GroupActionManager::refresh()
{
	if(queued)
	{
		queued=false;
		//queue_refresh_connection.disconnect();
	}


	clear();

	// Make sure we are ready
	if (!action_widget_ || !group_tree_ || !canvas_interface_) {
		synfig::error("GroupActionManager::refresh(): Not ready!");
		return;
	}

	if (group_tree_->get_selection()->count_selected_rows() == 0)
		return;

	{
		action_group_->add_action("group_add",
			sigc::mem_fun(*this, &GroupActionManager::on_action_add)
		);


//		bool multiple_selected(group_tree_->get_selection()->count_selected_rows()>1);
		LayerGroupTree::LayerList selected_layers(group_tree_->get_selected_layers());
		std::list<synfig::String> selected_groups(group_tree_->get_selected_groups());

		{
			bool canvas_set(false);
			synfigapp::Action::ParamList param_list;
			param_list.add("time",get_canvas_interface()->get_time());
			param_list.add("canvas_interface",get_canvas_interface());

			{
				LayerGroupTree::LayerList::iterator iter;

				for(iter=selected_layers.begin();iter!=selected_layers.end();++iter)
				{
					if(!canvas_set)
					{
						param_list.add("canvas",Canvas::Handle(Layer::Handle(*iter)->get_canvas()));
						canvas_set=true;
					}
					param_list.add("layer",Layer::Handle(*iter));
				}
			}

			{
				std::list<synfig::String>::iterator iter;

				for(iter=selected_groups.begin();iter!=selected_groups.end();++iter)
				{
					param_list.add("group",*iter);
				}
			}

			if(!canvas_set)
			{
				param_list.add("canvas",Canvas::Handle(get_canvas_interface()->get_canvas()));
				canvas_set=true;
			}

			etl::handle<studio::Instance>::cast_static(get_canvas_interface()->get_instance())->
				add_actions_to_group(action_group_, param_list, synfigapp::Action::CATEGORY_GROUP);
			}
	}

	// String full_ui_info;
	// full_ui_info="<ui><popup action='menu-main'><menu action='menu-group'>"+ui_info+"</menu></popup></ui>";
	// popup_id_=get_ui_manager()->add_ui_from_string(full_ui_info);
	// full_ui_info="<ui><menubar action='menubar-main'><menu action='menu-group'>"+ui_info+"</menu></menubar></ui>";
	// popup_id_=get_ui_manager()->add_ui_from_string(full_ui_info);

	action_widget_->insert_action_group(group_name, action_group_);
}

void
GroupActionManager::on_action_add()
{
	LayerGroupTreeStore::Model model;

	String group_name;

	Gtk::TreeIter selected_iter;

	if(group_tree_->get_selection()->count_selected_rows())
	{
		selected_iter=(
			group_tree_->get_model()->get_iter(
				(*group_tree_->get_selection()->get_selected_rows().begin())
			)
		);
		if(selected_iter && selected_iter->parent())
			group_name=(Glib::ustring)(*selected_iter->parent())[model.group_name]+'.';
	}

	group_name+=_("Unnamed Set");

	Gtk::TreePath path(group_tree_->get_model()->on_group_added(group_name));

	group_tree_->expand_to_path(path);
}
