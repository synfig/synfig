/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: layeractionmanager.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layeractionmanager.h"
#include "layertree.h"
#include <sinfgapp/action_param.h>
#include "instance.h"
#include <sinfgapp/selectionmanager.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

static const guint no_prev_popup((guint)-1);

/* === M A C R O S ========================================================= */

//#define ONE_ACTION_GROUP 1

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LayerActionManager::LayerActionManager():
	action_group_(Gtk::ActionGroup::create()),
	action_group_copy_paste(Gtk::ActionGroup::create()),
	popup_id_(no_prev_popup),
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
LayerActionManager::set_canvas_interface(const etl::handle<sinfgapp::CanvasInterface> &x)
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
			action_group_=Gtk::ActionGroup::create();
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
		sinfg::error("LayerActionManager::refresh(): Not ready!");
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
			sinfgapp::Action::ParamList param_list;
			param_list.add("time",get_canvas_interface()->get_time());
			param_list.add("canvas_interface",get_canvas_interface());
			{
				sinfgapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());
				sinfgapp::SelectionManager::LayerList::iterator iter;
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
			
			if(!multiple_selected && layer->get_name()=="PasteCanvas")
			{
				action_group_->add(Gtk::Action::create(
					"select-all-child-layers",
					_("Select All Child Layers")
				),
					sigc::bind(
						sigc::mem_fun(
							*layer_tree_,
							&studio::LayerTree::select_all_children_layers
						),
						Layer::LooseHandle(layer)
					)
				);
				ui_info+="<menuitem action='select-all-child-layers'/>";
			}
			handle<studio::Instance>::cast_static(get_canvas_interface()->get_instance())->
				add_actions_to_group(action_group_, ui_info,   param_list, sinfgapp::Action::CATEGORY_LAYER);
		}
	}
	
	ui_info="<ui><menubar action='menu-main'><menu action='menu-layer'>"+ui_info+"<separator/><menuitem action='cut' /><menuitem action='copy' /><menuitem action='paste' /><separator/></menu></menubar></ui>";
	popup_id_=get_ui_manager()->add_ui_from_string(ui_info);	
#ifdef ONE_ACTION_GROUP
#else
	get_ui_manager()->insert_action_group(action_group_);
#endif
	DEBUGPOINT();
}

void
LayerActionManager::cut()
{
	copy();
	if(action_group_->get_action("action-layer_remove"))
		action_group_->get_action("action-layer_remove")->activate();
}

void
LayerActionManager::copy()
{
	sinfgapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());
	clipboard_.clear();
	sinfg::GUID guid;
	
	while(!layer_list.empty())
	{
		clipboard_.push_back(layer_list.front()->clone(guid));
		layer_list.pop_front();
	}

	action_paste_->set_sensitive(!clipboard_.empty());
	
	//queue_refresh();
}

void
LayerActionManager::paste()
{
	sinfg::GUID guid;

	// Create the action group
	sinfgapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Paste"));

	Canvas::Handle canvas(get_canvas_interface()->get_canvas());
	int depth(0);
	
	// we are temporarily using the layer to hold something
	Layer::Handle layer(layer_tree_->get_selected_layer());
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}
	
	sinfgapp::SelectionManager::LayerList layer_selection;
	
	for(std::list<sinfg::Layer::Handle>::iterator iter=clipboard_.begin();iter!=clipboard_.end();++iter)
	{
		layer=(*iter)->clone(guid);
		layer_selection.push_back(layer);
		sinfgapp::Action::Handle 	action(sinfgapp::Action::create("layer_add"));
	
		assert(action);
		if(!action)
			return;
		
		action->set_param("canvas",canvas);
		action->set_param("canvas_interface",etl::loose_handle<sinfgapp::CanvasInterface>(get_canvas_interface()));
		action->set_param("new",layer);
		
		if(!action->is_ready())
		{
			return;
		}
		
		if(!get_instance()->perform_action(action))
		{
			return;
		}
	
		sinfg::info("DEPTH=%d",depth);
		// Action to move the layer (if necessary)
		if(depth>0)
		{
			sinfgapp::Action::Handle 	action(sinfgapp::Action::create("layer_move"));
		
			assert(action);
			if(!action)
				return;
			
			action->set_param("canvas",canvas);
			action->set_param("canvas_interface",etl::loose_handle<sinfgapp::CanvasInterface>(get_canvas_interface()));
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
	}
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
}

void
LayerActionManager::amount_inc()
{
	float adjust(0.1);
	
	// Create the action group
	sinfgapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Decrease Amount"));
	
	if(adjust>0)
		group.set_name(_("Increase Amount"));

	sinfgapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());

	while(!layer_list.empty())
	{
		ValueBase value(layer_list.front()->get_param("amount"));
		if(value.same_as(Real()))
		{
			get_canvas_interface()->change_value(sinfgapp::ValueDesc(layer_list.front(),"amount"),value.get(Real())+adjust);
		}
		layer_list.pop_front();
	}
}

void
LayerActionManager::amount_dec()
{
	float adjust(-0.1);
	
	// Create the action group
	sinfgapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Decrease Amount"));
	
	if(adjust>0)
		group.set_name(_("Increase Amount"));

	sinfgapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());

	while(!layer_list.empty())
	{
		ValueBase value(layer_list.front()->get_param("amount"));
		if(value.same_as(Real()))
		{
			get_canvas_interface()->change_value(sinfgapp::ValueDesc(layer_list.front(),"amount"),value.get(Real())+adjust);
		}
		layer_list.pop_front();
	}
}
