/* === S Y N F I G ========================================================= */
/*!	\file layeractionmanager.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <glibmm.h>
#include "layeractionmanager.h"
#include "trees/layertree.h"
#include <synfig/context.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfigapp/action_param.h>
#include "instance.h"
#include <synfigapp/selectionmanager.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

static const guint no_prev_popup((guint)-1);

/* === M A C R O S ========================================================= */

//#define ONE_ACTION_GROUP 1

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LayerActionManager::LayerActionManager():
	action_group_(Gtk::ActionGroup::create("action_group_layer_action_manager")),
	popup_id_(no_prev_popup),
	action_group_copy_paste(Gtk::ActionGroup::create("action_group_copy_paste")),
	queued(false)
{
	action_cut_=Gtk::Action::create(
		"cut",
		Gtk::StockID("gtk-cut")
	);
	action_cut_->signal_activate().connect(
		sigc::mem_fun(
			*this,
			&LayerActionManager::cut
		)
	);
	action_copy_=Gtk::Action::create(
		"copy",
		Gtk::StockID("gtk-copy")
	);
	action_copy_->signal_activate().connect(
		sigc::mem_fun(
			*this,
			&LayerActionManager::copy
		)
	);
	action_paste_=Gtk::Action::create(
		"paste",
		Gtk::StockID("gtk-paste")
	);
	action_paste_->signal_activate().connect(
		sigc::mem_fun(
			*this,
			&LayerActionManager::paste
		)
	);


	action_amount_inc_=Gtk::Action::create(
		"amount-inc",
		Gtk::StockID("gtk-add"),
		_("Increase Amount"),_("Increase Amount")
	);
	action_amount_inc_->signal_activate().connect(
		sigc::mem_fun(
			*this,
			&LayerActionManager::amount_inc
		)
	);

	action_amount_dec_=Gtk::Action::create(
		"amount-dec",
		Gtk::StockID("gtk-remove"),
		_("Decrease Amount"),_("Decrease Amount")
	);
	action_amount_dec_->signal_activate().connect(
		sigc::mem_fun(
			*this,
			&LayerActionManager::amount_dec
		)
	);

	action_amount_=Gtk::Action::create(
		"amount",
		Gtk::StockID("gtk-index"),
		_("Amount"),_("Amount")
	);

	action_select_all_child_layers_=Gtk::Action::create(
		"select-all-child-layers",
		Gtk::StockID("synfig-select_all_child_layers"),
		_("Select All Child Layers"),_("Select All Child Layers")
	);
	action_select_all_child_layers_->set_sensitive(false);
}

LayerActionManager::~LayerActionManager()
{
}

void
LayerActionManager::set_ui_manager(const Glib::RefPtr<Gtk::UIManager> &x)
{
	clear();

#ifdef ONE_ACTION_GROUP
	if(ui_manager_)	get_ui_manager()->remove_action_group(action_group_);
	ui_manager_=x;
	if(ui_manager_)	get_ui_manager()->insert_action_group(action_group_);
#else
	ui_manager_=x;
#endif
}

void
LayerActionManager::set_layer_tree(LayerTree* x)
{
	selection_changed_connection.disconnect();
	layer_tree_=x;
	if(layer_tree_)
	{
		selection_changed_connection=layer_tree_->get_selection()->signal_changed().connect(
			sigc::mem_fun(*this,&LayerActionManager::queue_refresh)
		);
	}
}

void
LayerActionManager::set_canvas_interface(const etl::handle<synfigapp::CanvasInterface> &x)
{
	canvas_interface_=x;
}

void
LayerActionManager::clear()
{
	if(ui_manager_)
	{
		// Clear out old stuff
		if(popup_id_!=no_prev_popup)
		{
			get_ui_manager()->remove_ui(popup_id_);
			if(action_group_)get_ui_manager()->ensure_update();
			popup_id_=no_prev_popup;
			if(action_group_)while(!action_group_->get_actions().empty())action_group_->remove(*action_group_->get_actions().begin());
#ifdef ONE_ACTION_GROUP
#else
			if(action_group_)get_ui_manager()->remove_action_group(action_group_);
			action_group_=Gtk::ActionGroup::create("action_group_layer_action_manager");
#endif
		}
	}

	while(!update_connection_list.empty())
	{
		update_connection_list.front().disconnect();
		update_connection_list.pop_front();
	}
}

void
LayerActionManager::queue_refresh()
{
	if(queued)
		return;

	//queue_refresh_connection.disconnect();
	queue_refresh_connection=Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&LayerActionManager::refresh),
			false
		)
	);

	queued=true;
}

void
LayerActionManager::refresh()
{
	if(queued)
	{
		queued=false;
		//queue_refresh_connection.disconnect();
	}


	clear();

	// Make sure we are ready
	if(!ui_manager_ || !layer_tree_ || !canvas_interface_)
	{
		synfig::error("LayerActionManager::refresh(): Not ready!");
		return;
	}

	String ui_info;

	action_paste_->set_sensitive(!clipboard_.empty());
	action_group_->add(action_paste_);

	if(layer_tree_->get_selection()->count_selected_rows()!=0)
	{
		bool multiple_selected(layer_tree_->get_selection()->count_selected_rows()>1);
		Layer::Handle layer(layer_tree_->get_selected_layer());

		{
			bool canvas_set(false);
			synfigapp::Action::ParamList param_list;
			param_list.add("time",get_canvas_interface()->get_time());
			param_list.add("canvas_interface",get_canvas_interface());
			{
				synfigapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());
				synfigapp::SelectionManager::LayerList::iterator iter;
				action_copy_->set_sensitive(!layer_list.empty());
				action_cut_->set_sensitive(!layer_list.empty());
				action_group_->add(action_copy_);
				action_group_->add(action_cut_);

				action_amount_inc_->set_sensitive(!layer_list.empty());
				action_amount_dec_->set_sensitive(!layer_list.empty());
				action_amount_->set_sensitive(!layer_list.empty());
				action_group_->add(action_amount_inc_);
				action_group_->add(action_amount_dec_);
				action_group_->add(action_amount_);

				for(iter=layer_list.begin();iter!=layer_list.end();++iter)
				{
					update_connection_list.push_back(
						(*iter)->signal_changed().connect(
							sigc::mem_fun(*this, &LayerActionManager::queue_refresh)
						)
					);

					if(!canvas_set)
					{
						param_list.add("canvas",Canvas::Handle((*iter)->get_canvas()));
						canvas_set=true;
						update_connection_list.push_back(
							(*iter)->get_canvas()->signal_changed().connect(
								sigc::mem_fun(*this, &LayerActionManager::queue_refresh)
							)
						);
					}
					param_list.add("layer",Layer::Handle(*iter));
				}
			}

			if(!multiple_selected && etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
			{
				if (select_all_child_layers_connection)
					select_all_child_layers_connection.disconnect();

				select_all_child_layers_connection = action_select_all_child_layers_->signal_activate().connect(
					sigc::bind(sigc::mem_fun(*layer_tree_,
											 &studio::LayerTree::select_all_children_layers),
							   Layer::LooseHandle(layer)));

				action_select_all_child_layers_->set_sensitive(true);

				ui_info+="<menuitem action='select-all-child-layers'/>";
			}
			else
				action_select_all_child_layers_->set_sensitive(false);

			handle<studio::Instance>::cast_static(get_canvas_interface()->get_instance())->
				add_actions_to_group(action_group_, ui_info,   param_list, synfigapp::Action::CATEGORY_LAYER);
		}
	}

	String full_ui_info;
	full_ui_info=
			 "<ui>"
			   "<popup action='menu-main'>"
			     "<menu action='menu-layer'>" +
			 	   ui_info +
				   "<separator/>"
			       "<menuitem action='cut' />"
			 	   "<menuitem action='copy' />"
			 	   "<menuitem action='paste' />"
			 	   "<separator/>"
			     "</menu>"
			   "</popup>" +
			 "</ui>";
	popup_id_=get_ui_manager()->add_ui_from_string(full_ui_info);
	full_ui_info=
			 "<ui>"
			   "<menubar action='menubar-main'>"
			     "<menu action='menu-layer'>" +
			 	   ui_info +
				   "<separator/>"
			       "<menuitem action='cut' />"
			 	   "<menuitem action='copy' />"
			 	   "<menuitem action='paste' />"
			 	   "<separator/>"
			     "</menu>"
			   "</menubar>" +
			 "</ui>";
	popup_id_=get_ui_manager()->add_ui_from_string(full_ui_info);
#ifdef ONE_ACTION_GROUP
#else
	get_ui_manager()->insert_action_group(action_group_);
#endif
}

void
LayerActionManager::cut()
{
	copy();
	if(action_group_->get_action("action-LayerRemove"))
		action_group_->get_action("action-LayerRemove")->activate();
}

void
LayerActionManager::copy()
{
	synfigapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());
	clipboard_.clear();
	synfig::GUID guid;

	while(!layer_list.empty())
	{
		clipboard_.push_back(layer_list.front()->clone(0, guid));
		layer_list.pop_front();
	}

	action_paste_->set_sensitive(!clipboard_.empty());

	//queue_refresh();
}

void
LayerActionManager::paste()
{
	synfig::GUID guid;

	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Paste"));

	Canvas::Handle canvas(get_canvas_interface()->get_canvas());
	int depth(0);

	// we are temporarily using the layer to hold something
	Layer::Handle layer(layer_tree_->get_selected_layer());
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}

	synfigapp::SelectionManager::LayerList layer_selection;

	for(std::list<synfig::Layer::Handle>::iterator iter=clipboard_.begin();iter!=clipboard_.end();++iter)
	{
		layer=(*iter)->clone(canvas, guid);
		layer_selection.push_back(layer);
		synfigapp::Action::Handle 	action(synfigapp::Action::create("LayerAdd"));

		assert(action);
		if(!action)
			return;

		action->set_param("canvas",canvas);
		action->set_param("canvas_interface",etl::loose_handle<synfigapp::CanvasInterface>(get_canvas_interface()));
		action->set_param("new",layer);

		if(!action->is_ready())
		{
			return;
		}

		if(!get_instance()->perform_action(action))
		{
			return;
		}

		etl::handle<Layer_PasteCanvas> paste = etl::handle<Layer_PasteCanvas>::cast_dynamic(layer);
		if (paste) paste->update_renddesc();

		// synfig::info("DEPTH=%d",depth);

		// Action to move the layer (if necessary)
		if(depth>0)
		{
			synfigapp::Action::Handle 	action(synfigapp::Action::create("LayerMove"));

			assert(action);
			if(!action)
				return;

			action->set_param("canvas",canvas);
			action->set_param("canvas_interface",etl::loose_handle<synfigapp::CanvasInterface>(get_canvas_interface()));
			action->set_param("layer",layer);
			action->set_param("new_index",depth);

			if(!action->is_ready())
			{
				//get_ui_interface()->error(_("Move Action Not Ready"));
				//return 0;
				return;
			}

			if(!get_instance()->perform_action(action))
			{
				//get_ui_interface()->error(_("Move Action Not Ready"));
				//return 0;
				return;
			}
		}
		depth++;

		// automatically export the Index parameter of Duplicate layers when pasting
		int index = 1;
		export_dup_nodes(layer, canvas, index);
	}
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
}

void
LayerActionManager::export_dup_nodes(synfig::Layer::Handle layer, Canvas::Handle canvas, int &index)
{
	// automatically export the Index parameter of Duplicate layers when pasting
	if (layer->get_name() == "duplicate")
		while (true)
		{
			String name = strprintf(_("Index %d"), index++);
			try
			{
				canvas->find_value_node(name, true);
			}
			catch (Exception::IDNotFound x)
			{
				get_canvas_interface()->add_value_node(layer->dynamic_param_list().find("index")->second, name);
				break;
			}
		}
	else
	{
		Layer::ParamList param_list(layer->get_param_list());
		for (Layer::ParamList::const_iterator iter(param_list.begin())
				 ; iter != param_list.end()
				 ; iter++)
			if (layer->dynamic_param_list().count(iter->first)==0 && iter->second.get_type()==type_canvas)
			{
				Canvas::Handle subcanvas(iter->second.get(Canvas::Handle()));
				if (subcanvas && subcanvas->is_inline())
					for (IndependentContext iter = subcanvas->get_independent_context(); iter != subcanvas->end(); iter++)
						export_dup_nodes(*iter, canvas, index);
			}

		for (Layer::DynamicParamList::const_iterator iter(layer->dynamic_param_list().begin())
				 ; iter != layer->dynamic_param_list().end()
				 ; iter++)
			if (iter->second->get_type()==type_canvas)
			{
				Canvas::Handle canvas((*iter->second)(0).get(Canvas::Handle()));
				if (canvas->is_inline())
					//! \todo do we need to implement this?  and if so, shouldn't we check all canvases, not just the one at t=0s?
					warning("%s:%d not yet implemented - do we need to export duplicate valuenodes in dynamic canvas parameters?", __FILE__, __LINE__);
			}
	}
}

void
LayerActionManager::amount_inc()
{
	float adjust(0.1);

	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Increase Amount"));
	synfigapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());

	for (; !layer_list.empty(); layer_list.pop_front())
	{
		ValueBase value(layer_list.front()->get_param("amount"));
		if(value.same_type_as(Real()))
			get_canvas_interface()->change_value(synfigapp::ValueDesc(layer_list.front(),"amount"),value.get(Real())+adjust);
	}
}

void
LayerActionManager::amount_dec()
{
	float adjust(-0.1);

	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Decrease Amount"));
	synfigapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());

	for (; !layer_list.empty(); layer_list.pop_front())
	{
		ValueBase value(layer_list.front()->get_param("amount"));
		if(value.same_type_as(Real()))
			get_canvas_interface()->change_value(synfigapp::ValueDesc(layer_list.front(),"amount"),value.get(Real())+adjust);
	}
}
