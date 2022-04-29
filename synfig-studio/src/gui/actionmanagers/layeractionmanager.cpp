/* === S Y N F I G ========================================================= */
/*!	\file layeractionmanager.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "layeractionmanager.h"

#include <glibmm/main.h>

#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/trees/layertree.h>
#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/layers/layer_skeleton.h>
#include <synfigapp/selectionmanager.h>
#include <gui/app.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

static const guint no_prev_popup((guint)-1);

/* === M A C R O S ========================================================= */

//#define ONE_ACTION_GROUP 1

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

// COPIED FROM synfigapp/actions/layerduplicate.cpp
/// Remove the layers that are inside an already listed group-kind layer, as they would be duplicated twice
static std::list<Layer::Handle>
remove_layers_inside_included_pastelayers(const std::list<Layer::Handle>& layer_list)
{
	std::vector<Layer::Handle> layerpastecanvas_list;
	for (const auto& layer : layer_list) {
		if (Layer_PasteCanvas* pastecanvas = dynamic_cast<Layer_PasteCanvas*>(layer.get())) {
			layerpastecanvas_list.push_back(layer);
		}
	}

	std::list<Layer::Handle> clean_layer_list;
	for (const Layer::Handle& layer : layer_list) {
		bool is_inside_a_selected_pastecanvas = false;
		auto parent_paste_canvas = layer->get_parent_paste_canvas_layer();
		while (parent_paste_canvas) {
			if (std::find(layerpastecanvas_list.begin(), layerpastecanvas_list.end(), parent_paste_canvas) != layerpastecanvas_list.end()) {
				is_inside_a_selected_pastecanvas = true;
				break;
			}
			parent_paste_canvas = parent_paste_canvas->get_parent_paste_canvas_layer();
		}
		if (!is_inside_a_selected_pastecanvas)
			clean_layer_list.push_back(layer);
	}
	return clean_layer_list;
}

/* === M E T H O D S ======================================================= */

LayerActionManager::LayerActionManager():
	action_group_(Gtk::ActionGroup::create("action_group_layer_action_manager")),
	menu_popup_id_(no_prev_popup),
	menu_main_id_(no_prev_popup),
	action_group_copy_paste(Gtk::ActionGroup::create("action_group_copy_paste")),
	queued(false)
{	
	//cut
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

	simp_action_cut_=App::instance()->add_action("cut", 
		sigc::mem_fun(
			*this,
			&LayerActionManager::cut
		)
	);
	simp_action_cut_->set_enabled(false);

	//copy
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

	simp_action_copy_=App::instance()->add_action("copy",
		sigc::mem_fun(
			*this,
			&LayerActionManager::copy
		)
	);
	simp_action_copy_->set_enabled(false);

	//paste
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
	
	simp_action_paste_=App::instance()->add_action("paste",
		sigc::mem_fun(
			*this,
			&LayerActionManager::paste
		)
	);

	//amount increase
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

	simp_action_amount_inc_=App::instance()->add_action("amount-inc",
		sigc::mem_fun(
			*this,
			&LayerActionManager::amount_inc
		)
	);
	simp_action_amount_inc_->set_enabled(false);

	//amount decrease
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

	simp_action_amount_dec_=App::instance()->add_action("amount-dec",
		sigc::mem_fun(
			*this,
			&LayerActionManager::amount_dec
		)
	);
	simp_action_amount_dec_->set_enabled(false);

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
	auto menu_object = App::builder()->get_object("instance-layers");
	auto layer_submenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(menu_object);
	if (!layer_submenu)
		g_warning("Could not get sub menu!");
	else
		layer_submenu->remove_all();
	menu_object=App::builder()->get_object("special-layer-actions");
	menu_object=App::builder()->get_object("layer-inc-dec");
	auto inc_dec_menu=Glib::RefPtr<Gio::Menu>::cast_dynamic(menu_object);
	if (!inc_dec_menu)
		g_warning("Could not get sub menu!");
	else
		inc_dec_menu->remove_all();
	auto special_action_menu=Glib::RefPtr<Gio::Menu>::cast_dynamic(menu_object);
	if (!special_action_menu)
		g_warning("Could not get sub menu!");
	else
		special_action_menu->remove_all();
	if(ui_manager_)
	{
		// Clear out old stuff
		if(menu_popup_id_!=no_prev_popup || menu_main_id_!=no_prev_popup)
		{
			get_ui_manager()->remove_ui(menu_popup_id_);
			get_ui_manager()->remove_ui(menu_main_id_);
			if(action_group_)get_ui_manager()->ensure_update();
			menu_popup_id_=no_prev_popup;
			menu_main_id_=no_prev_popup;
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

	simp_action_paste_->set_enabled(!clipboard_.empty());
	auto object=App::builder()->get_object("layer-inc-dec");
	auto inc_dec_section=Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
	if(layer_tree_->get_selection()->count_selected_rows()!=0)
	{
		bool multiple_selected(layer_tree_->get_selection()->count_selected_rows()>1);
		Layer::Handle layer(layer_tree_->get_selected_layer());

		{
			bool canvas_set(false);
			synfigapp::Action::ParamList param_list;
			synfigapp::SelectionManager::LayerList layer_list;
			param_list.add("time",get_canvas_interface()->get_time());
			param_list.add("canvas_interface",get_canvas_interface());
			{
				layer_list = layer_tree_->get_selected_layers();
				synfigapp::SelectionManager::LayerList::iterator iter;
				action_copy_->set_sensitive(!layer_list.empty());
				action_cut_->set_sensitive(!layer_list.empty());
				action_group_->add(action_copy_);
				action_group_->add(action_cut_);

				action_amount_inc_->set_sensitive(!layer_list.empty());
				action_amount_dec_->set_sensitive(!layer_list.empty());
				simp_action_copy_->set_enabled(!layer_list.empty());
				simp_action_cut_->set_enabled(!layer_list.empty());

				simp_action_amount_inc_->set_enabled(!layer_list.empty());
				simp_action_amount_dec_->set_enabled(!layer_list.empty());

				if (Layer_Skeleton::Handle::cast_dynamic(layer) || etl::handle<Layer_Composite>::cast_dynamic(layer)) {
					action_amount_inc_->set_label(_("Increase Opacity"));
					action_amount_dec_->set_label(_("Decrease Opacity"));
					//TODO: need to add accelerators
					inc_dec_section->remove_all();
					set_action_inc_dec_menu(inc_dec_section, "Increase Opacity", "Decrease Opacity", true);
				} else {
					action_amount_inc_->set_label(_("Increase Amount"));
					action_amount_dec_->set_label(_("Decrease Amount"));
					inc_dec_section->remove_all();
					set_action_inc_dec_menu(inc_dec_section, "Increase Layer Amount", "Decrease Layer Amount", true);
				}
				action_group_->add(action_amount_inc_);
				action_group_->add(action_amount_dec_);
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

			//TODO: need to update this to gio::simpleaction ??
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
				add_actions_to_group(action_group_, ui_info, param_list, synfigapp::Action::CATEGORY_LAYER);

			ui_info+="<separator/>";
			handle<studio::Instance>::cast_static(get_canvas_interface()->get_instance())->
				add_special_layer_actions_to_group(action_group_, ui_info, layer_list);
		}
	}else{
		set_action_inc_dec_menu(inc_dec_section, "Increase Layer Amount", "Decrease Layer Amount", false);
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
	menu_popup_id_=get_ui_manager()->add_ui_from_string(full_ui_info);

#ifdef ONE_ACTION_GROUP
#else
	get_ui_manager()->insert_action_group(action_group_);
#endif
}

void
LayerActionManager::set_action_inc_dec_menu(Glib::RefPtr<Gio::Menu>& menu, const char* name_inc, const char* name_dec, bool isActive)
{
	//TODO: cant set accelerators to menu items dynamically???
	//menu_item->set_attribute_value("accel", Glib::Variant<std::string>::create("&lt;Control&gt;minus"));
	if(auto menu_item = Gio::MenuItem::create(_(name_inc),"app.amount-inc"))
	{
		if(auto icon = App::icon_theme()->lookup_icon("list-add", 128).load_icon())
			menu_item->set_icon(icon);
		menu->append_item(menu_item);
	}
	if(auto menu_item = Gio::MenuItem::create(_(name_dec),"app.amount-dec"))
	{	
		if(auto icon = App::icon_theme()->lookup_icon("list-remove", 128).load_icon())
			menu_item->set_icon(icon);
		menu->append_item(menu_item);
	}
	auto action = App::instance()->lookup_action("amount-inc");
	if (auto s_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action))
		s_action->set_enabled(isActive);
	action = App::instance()->lookup_action("amount-dec");
	if (auto s_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action))
		s_action->set_enabled(isActive);
}
void
LayerActionManager::cut()
{
	copy();
	if ( auto s_action = App::instance()->lookup_action("action-LayerRemove") )
		s_action->activate();
}

void
LayerActionManager::copy()
{
	synfigapp::SelectionManager::LayerList layer_list(layer_tree_->get_selected_layers());

	// remove layers that would be duplicated twice
	layer_list = remove_layers_inside_included_pastelayers(layer_list);

	clipboard_.clear();
	synfig::GUID guid;

	while(!layer_list.empty())
	{
		clipboard_.push_back(layer_list.front()->clone(0, guid));
		layer_list.pop_front();
	}
	action_paste_->set_sensitive(!clipboard_.empty());
	simp_action_paste_->set_enabled(!clipboard_.empty());

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
			catch (const Exception::IDNotFound& x)
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
