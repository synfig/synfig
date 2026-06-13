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

#include <giomm/themedicon.h>
#include <glibmm/main.h>

#include <gui/actiondatabase.h>
#include <gui/actionwidgethelper.h>
#include <gui/app.h>
#include <gui/dialogs/dialog_pasteoptions.h>
#include <gui/instance.h>
#include <gui/localization.h>
#include <gui/trees/layertree.h>
#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/layers/layer_skeleton.h>
#include <synfig/synfig_iterations.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfigapp/selectionmanager.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static const std::string group_name {"layer"};

/* === P R O C E D U R E S ================================================= */

// Maybe this function should be generalized to a foreach_valuenode() and placed in synfig or synfigapp folder
// synfigapp action LayerDuplicate use something similar. Same for ValueNode_Bone::fix_bones_referenced_by()
/// \param canvas Local canvas. If a value node doesn't belong to this canvas, it is from an 'external' one
/// \param value_node The 'root' value node from where we search for other linked/pointed value nodes
/// \param[out] foreign_exported_valuenodes List of the external exported value nodes found
static void
search_for_foreign_exported_value_nodes(Canvas::LooseHandle canvas, ValueNode::LooseHandle value_node, std::vector<ValueNode::LooseHandle>& foreign_exported_valuenodes, std::set<ValueNode::LooseHandle>& visited_valuenodes)
{
	if (!value_node) {
		synfig::warning("%s:%d null valuenode?!\n", __FILE__, __LINE__);
		assert(false);
		return;
	}

	if (visited_valuenodes.count(value_node))
		return;

	visited_valuenodes.insert(value_node);

	if (value_node->is_exported()) {
		if (value_node->get_root_canvas() != canvas->get_root())
			foreign_exported_valuenodes.push_back(value_node);
	}

	if (auto linkable_vn = LinkableValueNode::Handle::cast_dynamic(value_node)) {
		for (int i=0; i < linkable_vn->link_count(); i++) {
			search_for_foreign_exported_value_nodes(canvas, linkable_vn->get_link(i), foreign_exported_valuenodes, visited_valuenodes);
		}
	} else if (auto const_vn = ValueNode_Const::Handle::cast_dynamic(value_node)) {
		if (const_vn->get_type() == type_bone_valuenode) {
			ValueNode_Bone::Handle bone_vn = const_vn->get_value().get(ValueNode_Bone::Handle());
			search_for_foreign_exported_value_nodes(canvas, bone_vn.get(), foreign_exported_valuenodes, visited_valuenodes);
		}
	} else if (auto animated_vn = ValueNode_Animated::Handle::cast_dynamic(value_node)) {
		const ValueNode_Animated::WaypointList& list(animated_vn->waypoint_list());
		for (ValueNode_Animated::WaypointList::const_iterator iter = list.cbegin(); iter != list.cend(); ++iter) {
			search_for_foreign_exported_value_nodes(canvas, iter->get_value_node(), foreign_exported_valuenodes, visited_valuenodes);
		}
	} else {
		// actually there is a known case: PlaceholderValueNode
		// but maybe user has custom valuenode modules...
		warning(_("Unknown value node type (%s) to search into it. Ignoring it."), value_node->get_local_name().c_str());
	}
}

/// \param canvas Local canvas. If a value node doesn't belong to this canvas, it is from an 'external' one
/// \param layer_list Layers from where it searches for exported valuenodes that are not from canvas
/// \param[out] foreign_exported_valuenodes List of the external exported value nodes found
static void
search_for_foreign_exported_value_nodes(Canvas::LooseHandle canvas, std::list<Layer::Handle> layer_list, std::vector<ValueNode::LooseHandle>& foreign_exported_valuenodes)
{
	auto fetch_exported_valuenodes_from_layer = [&canvas, &foreign_exported_valuenodes](Layer::LooseHandle layer, const TraverseLayerStatus& /*status*/) {
		std::set<ValueNode::LooseHandle> visited_valuenodes;
		for (auto dyn_param : layer->dynamic_param_list()) {
			auto value_node = dyn_param.second;
			if (!value_node) {
				error(_("Internal error: layer dynamic parameter list element could not be null"));
				continue;
			}
			search_for_foreign_exported_value_nodes(canvas, value_node, foreign_exported_valuenodes, visited_valuenodes);
		}
	};

	for (Layer::LooseHandle layer : layer_list) {
		traverse_layers(layer, fetch_exported_valuenodes_from_layer);
	}
}

/// \param layer Where to look for replaceable value nodes
/// \param valuenode_replacements Maps the valuenode ID to be replaced -> (new value node, new ID for this new value node - if different)
static void
replace_exported_value_nodes(Layer::LooseHandle layer, const std::map<ValueNode::Handle,std::pair<ValueNode::Handle, std::string>>& valuenode_replacements)
{
	auto get_correspondent_clone = [valuenode_replacements](const ValueNode::LooseHandle& vn) -> ValueNode::LooseHandle {
		auto iter = valuenode_replacements.find(vn);
		if (iter != valuenode_replacements.end()) {
			auto replacement = iter->second.first;
			return replacement;
		}
		return nullptr;
	};
	replace_value_nodes(layer, get_correspondent_clone);
}

// COPIED FROM synfigapp/actions/layerduplicate.cpp
/// Remove the layers that are inside an already listed group-kind layer, as they would be duplicated twice
static std::list<Layer::Handle>
remove_layers_inside_included_pastelayers(const std::list<Layer::Handle>& layer_list)
{
	std::vector<Layer::Handle> layerpastecanvas_list;
	for (const auto& layer : layer_list) {
		if (dynamic_cast<Layer_PasteCanvas*>(layer.get())) {
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

LayerActionManager::LayerActionManager()
	: action_widget_(nullptr),
	  layer_tree_(nullptr),
	  action_group_(Gio::SimpleActionGroup::create()),
	  queued(false),
	  menuitem_layer_(nullptr),
	  menuitem_layer2_(nullptr)
{
	action_cut_ = Gio::SimpleAction::create("cut");
	action_cut_->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &LayerActionManager::cut)));
	action_copy_ = Gio::SimpleAction::create("copy");
	action_copy_->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &LayerActionManager::copy)));
	action_paste_ = Gio::SimpleAction::create("paste");
	action_paste_->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &LayerActionManager::paste)));

	action_amount_inc_ = Gio::SimpleAction::create("increase-amount");
	action_amount_inc_->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &LayerActionManager::amount_inc)));

	action_amount_dec_ = Gio::SimpleAction::create("decrease-amount");
	action_amount_dec_->signal_activate().connect(sigc::hide(sigc::mem_fun(*this, &LayerActionManager::amount_dec)));

	action_select_all_child_layers_ = Gio::SimpleAction::create("select-all-child-layers");
	action_select_all_child_layers_->set_enabled(false);

	action_new_ = Gio::SimpleAction::create("new", Glib::VARIANT_TYPE_STRING);
	action_new_->signal_activate().connect(sigc::mem_fun(*this, &LayerActionManager::on_add_layer_selected));

	action_group_->add_action(action_cut_);
	action_group_->add_action(action_copy_);
	action_group_->add_action(action_paste_);

	action_group_->add_action(action_amount_inc_);
	action_group_->add_action(action_amount_dec_);

	action_group_->add_action(action_select_all_child_layers_);

	action_group_->add_action(action_new_);
}

LayerActionManager::~LayerActionManager()
{
}

void
LayerActionManager::set_action_widget_and_menus(Gtk::Widget* x, Gtk::MenuItem* menuitem_layer, Gtk::MenuItem* menuitem_layer2)
{
	clear();

	action_widget_ = x;
	menuitem_layer_ = menuitem_layer;
	menuitem_layer2_ = menuitem_layer2;
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
	// Remove all synfigapp actions and special actions for specific layer types
	if (action_widget_) {
		const auto preservable_actions = {"cut", "copy", "paste", "increase-amount", "decrease-amount", "select-all-child-layers", "new"};

		if (action_group_) {
			auto actions = action_group_->list_actions();
			for (const auto& action_name : actions) {
				if (std::find(preservable_actions.begin(), preservable_actions.end(), action_name) != preservable_actions.end())
					continue;
				action_group_->remove_action(action_name);
			}
		}

		action_widget_->remove_action_group(group_name);
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
	if (!layer_tree_ || !canvas_interface_ || !action_widget_) {
		synfig::error("LayerActionManager::refresh(): Not ready!");
		return;
	}

	const synfigapp::SelectionManager::LayerList layer_list = layer_tree_->get_selected_layers();
	const Layer::Handle layer(layer_tree_->get_selected_layer());
	const int layer_selection_count = layer_list.size();
	const bool single_selection(layer_selection_count == 1);
	const bool is_layer_pastecanvas_selected = single_selection && Layer_PasteCanvas::Handle::cast_dynamic(layer);

	action_new_->set_enabled(!get_canvas_interface().empty());

	action_amount_inc_->set_enabled(layer_selection_count > 0);
	action_amount_dec_->set_enabled(layer_selection_count > 0);

	action_select_all_child_layers_->set_enabled(is_layer_pastecanvas_selected);

	action_copy_->set_enabled(layer_selection_count > 0);
	action_cut_->set_enabled(layer_selection_count > 0);
	action_paste_->set_enabled(!clipboard_.empty());

	if (layer_selection_count > 0) {
		// 0: New layer

		// 1st: Increase/Decrease Amount

		// 2nd: Select All Child Layers
		if (is_layer_pastecanvas_selected) {
			if (select_all_child_layers_connection)
				select_all_child_layers_connection.disconnect();

			select_all_child_layers_connection = action_select_all_child_layers_->signal_activate().connect(
				sigc::hide(sigc::bind(sigc::mem_fun(*layer_tree_,
											&studio::LayerTree::select_all_children_layers),
						   Layer::LooseHandle(layer))));
		}

		// 3rd: synfigapp Actions viable for all selected layers
		synfigapp::Action::ParamList param_list;
		{
			bool canvas_set(false);
			param_list.add("time",get_canvas_interface()->get_time());
			param_list.add("canvas_interface",get_canvas_interface());
			for (const auto& layer : layer_list) {
				update_connection_list.push_back(
					layer->signal_changed().connect(
						sigc::mem_fun(*this, &LayerActionManager::queue_refresh)
						)
					);

				if (!canvas_set) {
					param_list.add("canvas",Canvas::Handle(layer->get_canvas()));
					canvas_set=true;
					update_connection_list.push_back(
						layer->get_canvas()->signal_changed().connect(
							sigc::mem_fun(*this, &LayerActionManager::queue_refresh)
							)
						);
				}
				param_list.add("layer", layer);
			}
		}

		auto instance = etl::handle<studio::Instance>::cast_static(get_canvas_interface()->get_instance());

		instance->add_actions_to_group(action_group_, param_list, synfigapp::Action::CATEGORY_LAYER);

		// 4th: special actions for specific layer types
		instance->add_special_layer_actions_to_group(action_group_, layer);

		// 5th: common edit operations
		// Nothing to do: never erased between refreshes/canvas switches
	}

	// Update Layer menu in main menu bar
	Glib::RefPtr<Gio::Menu> menu_layer = create_context_menu(layer_list);
	const bool has_context_menu = menu_layer.get();
	if (!has_context_menu) {
		menu_layer = Gio::Menu::create();

		auto edit_menu_section = Gio::Menu::create();
		edit_menu_section->append_item(ActionWidgetHelper::create_menu_item_for_action(group_name + ".cut"));
		edit_menu_section->append_item(ActionWidgetHelper::create_menu_item_for_action(group_name + ".copy"));
		edit_menu_section->append_item(ActionWidgetHelper::create_menu_item_for_action(group_name + ".paste"));
		menu_layer->append_section(edit_menu_section);
	}

	menu_layer->prepend_submenu(_("New Layer"), get_add_layer_menu());

	auto menu = Gtk::manage(new Gtk::Menu(menu_layer));
	menu->show_all();

	menuitem_layer_->set_submenu(*menu);

	auto menu2 = Gtk::manage(new Gtk::Menu(menu_layer));
	menu2->show_all();
	menuitem_layer2_->set_submenu(*menu2);

	action_widget_->insert_action_group(group_name, action_group_);
}

void
LayerActionManager::cut()
{
	copy();

	auto layer_remove_action = action_group_->lookup_action("action-LayerRemove");
	if (layer_remove_action) {
		layer_remove_action->activate();
	}
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

	action_paste_->set_enabled(!clipboard_.empty());

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

	// we paste layers right above the current layer selection
	Layer::Handle layer(layer_tree_->get_selected_layer());
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}

	ValueNodeReplacementMap valuenode_replacements;

	bool user_accepted = query_user_about_foreign_exported_value_nodes(canvas, valuenode_replacements);
	if (!user_accepted)
		return;
	if (!valuenode_replacements.empty())
		export_value_nodes(canvas, valuenode_replacements);

	synfigapp::SelectionManager::LayerList layer_selection;

	for(std::list<synfig::Layer::Handle>::iterator iter=clipboard_.begin();iter!=clipboard_.end();++iter)
	{
		layer=(*iter)->clone(canvas, guid);
		layer_selection.push_back(layer);

		replace_exported_value_nodes(layer, valuenode_replacements);

		synfigapp::Action::Handle action(synfigapp::Action::create("LayerAdd"));

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

		Layer_PasteCanvas::Handle paste = Layer_PasteCanvas::Handle::cast_dynamic(layer);
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
				 ; ++iter)
			if (layer->dynamic_param_list().count(iter->first)==0 && iter->second.get_type()==type_canvas)
			{
				Canvas::Handle subcanvas(iter->second.get(Canvas::Handle()));
				if (subcanvas && subcanvas->is_inline())
					for (IndependentContext iter = subcanvas->get_independent_context(); iter != subcanvas->end(); ++iter)
						export_dup_nodes(*iter, canvas, index);
			}

		for (Layer::DynamicParamList::const_iterator iter(layer->dynamic_param_list().begin())
				 ; iter != layer->dynamic_param_list().end()
				 ; ++iter)
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

bool LayerActionManager::query_user_about_foreign_exported_value_nodes(Canvas::Handle canvas, ValueNodeReplacementMap& valuenode_replacements) const
{
	std::vector<ValueNode::LooseHandle> foreign_exported_valuenode_list;

	search_for_foreign_exported_value_nodes(canvas, clipboard_, foreign_exported_valuenode_list);

	if (!foreign_exported_valuenode_list.empty()) {
		auto dlg = Dialog_PasteOptions::create(*App::main_window);
		dlg->set_value_nodes(foreign_exported_valuenode_list);
		dlg->set_destination_canvas(canvas);
		int ret = dlg->run();
		if (ret != Gtk::RESPONSE_OK)
			return false;

		std::map<ValueNode::LooseHandle, std::string> user_choices;
		dlg->get_user_choices(user_choices);

		for (auto item : user_choices) {
			// Will it be linked to the external canvas/file? (i.e. not to be copied?)
			if (item.second.empty())
				continue;

			const ValueNode::LooseHandle foreign_value_node = item.first;
			const std::string& modified_id = item.second;

			// shall the exported valuenode be cloned or linked to a locally existent one?
			if (modified_id.empty()) {
				// foreign link
			} else {
				ValueNode::Handle local_canvas_value_node;
				try {
					local_canvas_value_node = canvas->find_value_node(modified_id, true);
				} catch (...) {
				}
				const bool link_to_local_canvas = static_cast<bool>(local_canvas_value_node);
				if (link_to_local_canvas) {
					valuenode_replacements[foreign_value_node] = std::pair<ValueNode::Handle, std::string>(local_canvas_value_node, "");
				} else {
					ValueNode::Handle cloned_value_node = foreign_value_node->clone(canvas);// TODO Use paste guid?!
					valuenode_replacements[foreign_value_node] = std::pair<ValueNode::Handle, std::string>(cloned_value_node, modified_id);
				}
			}
		}

		// Now we fix the valuenode copies that should refer to other valuenode that were copied as well
		// as they should not be linked to external file
		auto get_correspondent_clone = [valuenode_replacements](ValueNode::LooseHandle vn) -> ValueNode::LooseHandle {
			auto iter = valuenode_replacements.find(vn);
			if (iter != valuenode_replacements.end()) {
				auto replacement = iter->second.first;
				return replacement;
			}
			return nullptr;
		};
		for (auto item : valuenode_replacements)
			replace_value_nodes(item.second.first, get_correspondent_clone);
	}
	return true;
}

void
LayerActionManager::export_value_nodes(Canvas::Handle canvas, const ValueNodeReplacementMap& valuenodes) const
{
	std::set<std::string> exported_ids;

	for (auto item : valuenodes) {

		synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeAdd"));

		assert(action);
		if(!action)
			return;

		action->set_param("canvas",canvas);
		action->set_param("canvas_interface",etl::loose_handle<synfigapp::CanvasInterface>(get_canvas_interface()));

//		const std::string& original_id = item.first;
		const std::string& modified_id = item.second.second;

		// if ID isn't modified, it doesn't need to (re)export it
		if (modified_id.empty())
			continue;

		// if ID is already prepared to export, don't do it again
		if (exported_ids.count(modified_id))
			continue;

		try {
			canvas->find_value_node(modified_id, true);
		} catch (...) {
			action->set_param("new",item.second.first);
			action->set_param("name",modified_id);

			if(!action->is_ready())
			{
				continue;
			}

			if(!get_instance()->perform_action(action))
			{
				error(_("Couldn't export value node %s"), modified_id.c_str());
				continue;
			}

			exported_ids.insert(modified_id);
		}
	}
}

void
LayerActionManager::on_add_layer_selected(const Glib::VariantBase& v)
{
	const std::string layer_name = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v).get();
	bool should_stop = signal_add_layer_selected_(layer_name);
	if (should_stop)
		return;
	if (!get_canvas_interface()) {
		synfig::error(_("Internal Error: LayerActionManager without CanvasInterface called to add layer: %s"), layer_name.c_str());
		return;
	}
	get_canvas_interface()->add_layer_to(layer_name, get_canvas_interface()->get_canvas());
}

Glib::RefPtr<Gio::Menu>
LayerActionManager::get_add_layer_menu()
{
	if (!menu_add_layer_)
		menu_add_layer_ = create_add_layer_menu();
	return menu_add_layer_;
}

Glib::RefPtr<Gio::Menu>
LayerActionManager::create_add_layer_menu()
{
	// CREATE ONLY ONCE and update when new are registered or unregistered
	Glib::RefPtr<Gio::Menu> menu_add_layer = Gio::Menu::create();

	// map: category local name -> (layer name, layer local name)
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> layer_category_map;

	// Build layer creation actions
	for (const auto& lyr : synfig::Layer::book()) {
		if(lyr.second.category==CATEGORY_DO_NOT_USE)
			continue;

		layer_category_map[dgettext("synfig", lyr.second.category.c_str())].push_back({lyr.first, lyr.second.local_name});
	}

	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";

	for (const auto& item : layer_category_map) {
		const std::string& category = item.first;
		auto submenu = Gio::Menu::create();
		for (const auto& lyr : item.second) {
			const std::string& layer_name = lyr.first;
			const std::string& layer_local_name = lyr.second;

			auto item = Gio::MenuItem::create(layer_local_name, strprintf("layer.new(\"%s\")", layer_name.c_str()));
			item->set_icon(Gio::ThemedIcon::create(layer_icon_name(layer_name) + symbolic_suffix));
			submenu->append_item(item);
		}
		menu_add_layer->append_submenu(category, submenu);
	}

	return menu_add_layer;
}

Glib::RefPtr<Gio::Menu>
LayerActionManager::create_context_menu(const std::list<synfig::Layer::Handle> layers) const
{
	if (layers.empty())
		return Glib::RefPtr<Gio::Menu>();

	Glib::RefPtr<Gio::Menu> context_menu = Gio::Menu::create();

	auto instance = etl::handle<studio::Instance>::cast_static(get_canvas_interface()->get_instance());

	const bool single_selection(layers.size() == 1);
	// const synfigapp::SelectionManager::LayerList layer_list = layer_tree_->get_selected_layers();
	const Layer::Handle layer(layers.front()); //  = layer_tree_->get_selected_layer();
	const bool is_layer_pastecanvas_selected = (single_selection && Layer_PasteCanvas::Handle::cast_dynamic(layer));

	const std::string symbolic_suffix = ""; // "-symbolic"

	// 1st: Increase/Decrease Amount
	const bool a_layer_has_amount_param = std::find_if(layers.cbegin(), layers.cend(),
														[] (Layer::ConstHandle layer) {
															return layer->get_param("amount").is_valid();
														}) != layers.cend();

	if (a_layer_has_amount_param) {
		std::string increase_amount_label;
		std::string decrease_amount_label;
		if (Layer_Skeleton::Handle::cast_dynamic(layer) || etl::handle<Layer_Composite>::cast_dynamic(layer)) {
			increase_amount_label = _("Increase Opacity");
			decrease_amount_label = _("Decrease Opacity");
		} else {
			increase_amount_label = _("Increase Amount");
			decrease_amount_label = _("Decrease Amount");
		}

		auto menu_item = Gio::MenuItem::create(increase_amount_label, group_name + "." + "increase-amount");
		menu_item->set_icon(Gio::ThemedIcon::create("list-add" + symbolic_suffix));
		context_menu->append_item(menu_item);

		menu_item = Gio::MenuItem::create(decrease_amount_label, group_name + "." + "decrease-amount");
		menu_item->set_icon(Gio::ThemedIcon::create("list-remove" + symbolic_suffix));
		context_menu->append_item(menu_item);
	}

	// 2nd: Select All Child Layers
	if (is_layer_pastecanvas_selected) {
		auto menu_item = ActionWidgetHelper::create_menu_item_for_action(group_name + "." + "select-all-child-layers");
		menu_item->set_icon(Gio::ThemedIcon::create("select_all_child_layers_icon" + symbolic_suffix));

		context_menu->append_item(menu_item);
	}

	// 3rd: synfigapp Actions viable for all layers
	synfigapp::Action::ParamList param_list;
	{
		bool canvas_set(false);
		param_list.add("time", get_canvas_interface()->get_time());
		param_list.add("canvas_interface", get_canvas_interface());
		for (const auto& layer : layers) {
			if (!canvas_set) {
				param_list.add("canvas", Canvas::Handle(layer->get_canvas()));
				canvas_set = true;
			}
			param_list.add("layer", layer);
		}
	}
	instance->add_actions_to_menu(group_name, context_menu, param_list, synfigapp::Action::CATEGORY_LAYER);

	// 4th: special actions for specific layers
	instance->add_special_layer_actions_to_menu(context_menu, layer);

	// 5th: common edit operations
	auto edit_menu_section = Gio::Menu::create();
	edit_menu_section->append_item(ActionWidgetHelper::create_menu_item_for_action(group_name + ".cut"));
	edit_menu_section->append_item(ActionWidgetHelper::create_menu_item_for_action(group_name + ".copy"));
	edit_menu_section->append_item(ActionWidgetHelper::create_menu_item_for_action(group_name + ".paste"));

	context_menu->append_section(edit_menu_section);

	return context_menu;
}
