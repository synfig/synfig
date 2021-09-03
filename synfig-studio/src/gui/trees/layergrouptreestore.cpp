/* === S Y N F I G ========================================================= */
/*!	\file layergrouptreestore.cpp
**	\brief Layer set tree model
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2017 caryoscelus
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

#include <gui/trees/layergrouptreestore.h>

#include <gtkmm/button.h>

#include <gui/app.h>
#include <gui/docks/dockmanager.h>
#include <gui/docks/dockable.h>
#include <gui/localization.h>

#include <synfig/general.h>

#include <synfigapp/action.h>
#include <synfigapp/instance.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define GROUP_NEST_CHAR	'.'

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static LayerGroupTreeStore::Model& ModelHack()
{
	static LayerGroupTreeStore::Model* model(0);
	if(!model)model=new LayerGroupTreeStore::Model;
	return *model;
}

LayerGroupTreeStore::LayerGroupTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_):
	Gtk::TreeStore			(ModelHack()),
	canvas_interface_		(canvas_interface_)
{
	layer_icon=Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-layer"),Gtk::ICON_SIZE_SMALL_TOOLBAR);
	group_icon=Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-group"),Gtk::ICON_SIZE_SMALL_TOOLBAR);

	// Connect Signals to Terminals
	canvas_interface()->signal_layer_status_changed().connect(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_layer_status_changed));
	canvas_interface()->signal_layer_new_description().connect(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_layer_new_description));

	canvas_interface()->get_canvas()->signal_group_added().connect(sigc::hide_return(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_group_added)));
	canvas_interface()->get_canvas()->signal_group_removed().connect(sigc::hide_return(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_group_removed)));
	canvas_interface()->get_canvas()->signal_group_changed().connect(sigc::hide_return(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_group_changed)));

	canvas_interface()->get_canvas()->signal_group_pair_added().connect(sigc::hide_return(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_group_pair_added)));
	canvas_interface()->get_canvas()->signal_group_pair_removed().connect(sigc::hide_return(sigc::mem_fun(*this,&studio::LayerGroupTreeStore::on_group_pair_removed)));

	rebuild();
}

LayerGroupTreeStore::~LayerGroupTreeStore()
{
	//clear();

	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("LayerGroupTreeStore::~LayerGroupTreeStore(): Deleted");
}

bool
LayerGroupTreeStore::search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring& x,const TreeModel::iterator& iter)
{
	const Model model;

	Glib::ustring substr(x.uppercase());
	Glib::ustring label((*iter)[model.label]);
	label=label.uppercase();

	return label.find(substr)==Glib::ustring::npos;
}


Glib::RefPtr<LayerGroupTreeStore>
LayerGroupTreeStore::create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_)
{
	return Glib::RefPtr<LayerGroupTreeStore>(new LayerGroupTreeStore(canvas_interface_));
}

void
LayerGroupTreeStore::get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const
{
	if(column==model.child_layers.index())
	{
		Glib::Value<LayerList> x;
		x.init(x.value_type());

		if((bool)(*iter)[model.is_group])
		{
			std::set<Layer::Handle> layer_set(canvas_interface()->get_canvas()->get_layers_in_group((Glib::ustring)(*iter)[model.group_name]));

			x.set(LayerList(layer_set.begin(),layer_set.end()));
		}
		else if((bool)(*iter)[model.is_layer])
		{
			LayerList layer_list;
			layer_list.push_back((Layer::Handle)(*iter)[model.layer]);
			x.set(layer_list);
		}

		value.init(x.value_type());
		value=x;
	}
	else if(column==model.all_layers.index())
	{
		Glib::Value<LayerList> x;
		x.init(x.value_type());

		if((bool)(*iter)[model.is_group])
		{
			LayerList layer_list;
			Gtk::TreeModel::iterator child_iter(iter->children().begin());
			for(;child_iter;++child_iter)
			{
				LayerList layer_list2((LayerList)(*child_iter)[model.all_layers]);
				//for(;layer_list2.size();layer_list2.pop_front())
				for(;!layer_list2.empty();layer_list2.pop_front())
					layer_list.push_back(layer_list2.front());
			}
			x.set(layer_list);
		}
		else if((bool)(*iter)[model.is_layer])
		{
			LayerList layer_list;
			layer_list.push_back((Layer::Handle)(*iter)[model.layer]);
			x.set(layer_list);
		}

		value.init(x.value_type());
		value=x;
	}
	else if (column == model.z_depth.index())
	{
		Glib::Value<float> x;
		x.init(x.value_type());
		if ((bool)(*iter)[model.is_layer])
		{
			Layer::Handle layer = (Layer::Handle)(*iter)[model.layer];
			x.set(layer->get_true_z_depth(canvas_interface()->get_time()));
		}
		else
		{
			x.set(0.0);
		}
		value.init(x.value_type());
		value = x;
	}
	else if(column==model.group_name.index())
	{
		if((bool)(*iter)[model.is_group])
			return Gtk::TreeStore::get_value_vfunc(iter,column,value);
		return get_value_vfunc(iter->parent(),column,value);
	}
	else if(column==model.parent_group_name.index())
	{
		if(iter->parent())
			return get_value_vfunc(iter->parent(),model.group_name.index(),value);
		Glib::Value<Glib::ustring> x;
		x.init(x.value_type());
		x.set(Glib::ustring());
		value.init(x.value_type());
		value=x;
	}
	else if(column==model.label.index())
	{
		if((bool)(*iter)[model.is_group])
		{
			Glib::Value<Glib::ustring> x;
			x.init(x.value_type());

			Glib::ustring group_name((*iter)[model.group_name]);

			// Get rid of any parent group crap
			while(group_name.find(GROUP_NEST_CHAR)!=Glib::ustring::npos)
				group_name=Glib::ustring(group_name,group_name.find(GROUP_NEST_CHAR)+1,Glib::ustring::npos);

			x.set(group_name);

			value.init(x.value_type());
			value=x;
		}
		else if((bool)(*iter)[model.is_layer])
		{
			synfig::Layer::Handle layer((*iter)[model.layer]);

			if(!layer)return;

			Glib::Value<Glib::ustring> x;
			x.init(x.value_type());

			x.set(layer->get_non_empty_description());

			value.init(x.value_type());
			value=x;
		}
	}
	else
	if(column==model.tooltip.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Glib::ustring> x;
		x.init(x.value_type());

		x.set(layer->get_local_name());

		value.init(x.value_type());
		value=x;
	}
	else
	if(column==model.canvas.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Canvas::Handle> x;
		x.init(x.value_type());

		x.set(layer->get_canvas());

		value.init(x.value_type());
		value=x;
	}
	else
	if(column==model.active.index())
	{
		Glib::Value<bool> x;
		x.init(x.value_type());

		if((bool)(*iter)[model.is_layer])
		{
			synfig::Layer::Handle layer((*iter)[model.layer]);
			x.set(layer->active());
		}
		else if((bool)(*iter)[model.is_group])
		{
			int activecount(0),total(0);
			Gtk::TreeModel::iterator child_iter(iter->children().begin());
			for(;child_iter;++child_iter)
			{
				total++;
				if((*child_iter)[model.active])
					activecount++;
			}
			x.set(activecount>0);
		}
		else
			x.set(false);

		value.init(x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.icon.index())
	{
		Glib::Value<Glib::RefPtr<Gdk::Pixbuf> > x;
		x.init(x.value_type());

		if((bool)(*iter)[model.is_layer])
		{
			synfig::Layer::Handle layer((*iter)[model.layer]);
			if(!layer)return;
			//x.set(layer_icon);
			x.set(get_tree_pixbuf_layer(layer->get_name()));
		}
		if((bool)(*iter)[model.is_group])
			x.set(group_icon);

		value.init(x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
		Gtk::TreeStore::get_value_vfunc(iter,column,value);
}

void
LayerGroupTreeStore::set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value)
{
	//if(!iterator_sane(row))
	//	return;

	if(column>=get_n_columns_vfunc())
	{
		g_warning("LayerGroupTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(!g_value_type_compatible(G_VALUE_TYPE(value.gobj()),get_column_type_vfunc(column)))
	{
		g_warning("LayerGroupTreeStore::set_value_impl: Bad value type");
		return;
	}

	try
	{
		if(column==model.label.index())
		{
			Glib::Value<Glib::ustring> x;
			x.init(model.label.type());
			g_value_copy(value.gobj(),x.gobj());

			if((bool)(*iter)[model.is_layer])
			{
				synfig::Layer::Handle layer((*iter)[model.layer]);
				if(!layer)
					return;
				synfig::String new_desc(x.get());

				if(new_desc==layer->get_local_name())
					new_desc=synfig::String();

				if(new_desc==layer->get_description())
					return;

				synfigapp::Action::Handle action(synfigapp::Action::create("LayerSetDesc"));

				if(!action)
					return;

				action->set_param("canvas",canvas_interface()->get_canvas());
				action->set_param("canvas_interface",canvas_interface());
				action->set_param("layer",layer);
				action->set_param("new_description",synfig::String(x.get()));

				canvas_interface()->get_instance()->perform_action(action);
				return;
			}
			else if((bool)(*iter)[model.is_group])
			{
				synfig::String group((Glib::ustring)(*iter)[model.label]);
				synfig::String new_group(x.get());

				if(x.get()==group)
					return;

				Glib::ustring group_name((*iter)[model.group_name]);
				group=group_name;
				new_group.clear();

				// Get rid of any parent group crap
				while(group_name.find(GROUP_NEST_CHAR)!=Glib::ustring::npos)
				{
					new_group+=Glib::ustring(group_name,0,group_name.find(GROUP_NEST_CHAR)+1);
					group_name=Glib::ustring(group_name,group_name.find(GROUP_NEST_CHAR)+1,Glib::ustring::npos);
				}
				new_group+=x.get();

				// Check to see if this group is real or not.
				// If it isn't real, then renaming it is a cinch.
				// We know it isn't real if it doesn't have any
				// children yet.
				if(iter->children().empty())
				{
					(*iter)[model.group_name]=new_group;
				}
				else
				{
					synfigapp::Action::Handle action(synfigapp::Action::create("GroupRename"));

					if(!action)
						return;

					action->set_param("canvas",canvas_interface()->get_canvas());
					action->set_param("canvas_interface",canvas_interface());
					action->set_param("group",group);
					action->set_param("new_group",new_group);

					canvas_interface()->get_instance()->perform_action(action);
				}
				return;
			}
			return;
		}
		else
		if(column==model.active.index())
		{
			Glib::Value<bool> x;
			x.init(model.active.type());
			g_value_copy(value.gobj(),x.gobj());

			if((bool)(*iter)[model.is_layer])
			{
				synfig::Layer::Handle layer((*iter)[model.layer]);
				if(!layer)return;

				synfigapp::Action::Handle action(synfigapp::Action::create("LayerActivate"));

				if(!action)
					return;

				action->set_param("canvas",canvas_interface()->get_canvas());
				action->set_param("canvas_interface",canvas_interface());
				action->set_param("layer",layer);
				action->set_param("new_status",bool(x.get()));


				canvas_interface()->get_instance()->perform_action(action);
				return;
			}
			else if(!iter->children().empty())
			{
				synfigapp::Action::PassiveGrouper group(
					get_canvas_interface()->get_instance().get(),
					String(
						x.get()?_("Activate "):_("Deactivate ")
					)+(Glib::ustring)(*iter)[model.label]
				);

				Gtk::TreeModel::iterator child_iter(iter->children().begin());

				for(;child_iter;++child_iter)
					(*child_iter)[model.active]=x.get();

				Gtk::TreeStore::set_value_impl(iter,column, value);
			}
		}
		else
			Gtk::TreeStore::set_value_impl(iter,column, value);

	}
	catch(std::exception& x)
	{
		g_warning("%s", x.what());
	}
}




bool
LayerGroupTreeStore::row_draggable_vfunc (const TreeModel::Path& /*path*/)const
{
	//if(!get_iter(path)) return false;
//	Gtk::TreeModel::Row row(*get_iter(path));

	return true;
}

bool
LayerGroupTreeStore::drag_data_get_vfunc (const TreeModel::Path& path, Gtk::SelectionData& selection_data)const
{
	if(!const_cast<LayerGroupTreeStore*>(this)->get_iter(path)) return false;
	//synfig::info("Dragged data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("Dragged data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("Dragged selection=\"%s\"",gdk_atom_name(selection_data->selection));

	Gtk::TreeModel::Row row(*const_cast<LayerGroupTreeStore*>(this)->get_iter(path));

	if((bool)row[model.is_layer])
	{
		Layer* layer(((Layer::Handle)row[model.layer]).get());
		assert(layer);

		std::vector<Layer*> layers;

		layers.push_back(layer);

		selection_data.set("LAYER", 8, reinterpret_cast<const guchar*>(&layers.front()), sizeof(void*)*layers.size());

		return true;
	}
	else if((bool)row[model.is_group])
	{
		synfig::String group((Glib::ustring)row[model.group_name]);
		if(group.empty())
			return false;

		selection_data.set("GROUP", 8, reinterpret_cast<const guchar*>(&*group.begin()), sizeof(void*)*group.size());

		return true;
	}

	return false;
}

bool
LayerGroupTreeStore::drag_data_delete_vfunc (const TreeModel::Path& /*path*/)
{
	return true;
}

bool
LayerGroupTreeStore::row_drop_possible_vfunc (const TreeModel::Path& dest, const Gtk::SelectionData& selection_data)const
{
	Gtk::TreeIter iter(const_cast<LayerGroupTreeStore*>(this)->get_iter(dest));
	if(!iter) return false;

	if(synfig::String(selection_data.get_data_type())=="LAYER")
		return true;

	if(synfig::String(selection_data.get_data_type())=="GROUP")
	{
		synfig::String dest_group((Glib::ustring)(*iter)[model.group_name]);
		synfig::String src_group(reinterpret_cast<const gchar*>(selection_data.get_data()));
		//synfig::String src_group(const_cast<gchar*>(selection_data.get_data()));

		// Avoid putting a group inside of itself
		if(dest_group.size()>src_group.size() && src_group==String(dest_group,0,src_group.size()))
			return false;
		return true;
	}

	return false;
	//synfig::info("possible_drop -- data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("possible_drop -- data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("possible_drop -- selection=\"%s\"",gdk_atom_name(selection_data->selection));

	//Gtk::TreeModel::Row row(*get_iter(dest));

/*	if(synfig::String(selection_data.get_data_type())=="LAYER" && (bool)true)
		return true;
*/
	return false;
}

bool
LayerGroupTreeStore::drag_data_received_vfunc (const TreeModel::Path& dest, const Gtk::SelectionData& selection_data)
{
	if(!get_iter(dest)) return false;
//	bool ret=false;
	//int i(0);

	Gtk::TreeModel::Row row(*get_iter(dest));

	//synfig::info("Dropped data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("Dropped data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("Dropped selection=\"%s\"",gdk_atom_name(selection_data->selection));
	synfigapp::Action::PassiveGrouper passive_grouper(canvas_interface()->get_instance().get(),_("Reset"));

	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		synfig::String dest_group;

		dest_group=(Glib::ustring)row[model.group_name];

		if(dest_group.empty())
			return false;

		if(synfig::String(selection_data.get_data_type())=="LAYER")
		{
			synfigapp::Action::Handle action(synfigapp::Action::create("GroupAddLayers"));

			if(!action)
				return false;

			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("group",dest_group);

			for(unsigned int i=0;i<selection_data.get_length()/sizeof(void*);i++)
			{
				Layer::Handle layer(reinterpret_cast<Layer**>(const_cast<guint8*>(selection_data.get_data()))[i]);
				assert(layer);

				action->set_param("layer",layer);
			}
			if(!canvas_interface()->get_instance()->perform_action(action))
			{
				passive_grouper.cancel();
				return false;
			}
			return true;
		}
		if(synfig::String(selection_data.get_data_type())=="GROUP")
		{
			synfig::String src_group(reinterpret_cast<const gchar*>(selection_data.get_data()));
			synfig::String group(src_group);

			// Get rid of any parent group crap
			while(group.find(GROUP_NEST_CHAR)!=Glib::ustring::npos)
				group=Glib::ustring(group,group.find(GROUP_NEST_CHAR)+1,Glib::ustring::npos);

			group=dest_group+GROUP_NEST_CHAR+group;

			synfigapp::Action::Handle action(synfigapp::Action::create("GroupRename"));

			if(!action)
				return false;

			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("group",src_group);
			action->set_param("new_group",group);

			if(!canvas_interface()->get_instance()->perform_action(action))
			{
				passive_grouper.cancel();
				return false;
			}
			return true;
		}
	}
/*	// Save the selection data
	synfigapp::SelectionManager::LayerList selected_layer_list=canvas_interface()->get_selection_manager()->get_selected_layers();

	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Canvas::Handle dest_canvas;
		Layer::Handle dest_layer;

		dest_canvas=(Canvas::Handle)(row[model.canvas]);
		dest_layer=(Layer::Handle)(row[model.layer]);
		assert(dest_canvas);

		if(!dest_layer)
			return false;

		int dest_layer_depth=dest_layer->get_depth();

		if(synfig::String(selection_data.get_data_type())=="LAYER")for(i=0;i<selection_data.get_length()/sizeof(void*);i++)
		{
			//synfig::info("dest_layer_depth=%d",dest_layer_depth);

			Layer::Handle src(reinterpret_cast<Layer**>(const_cast<guint8*>(selection_data.get_data()))[i]);
			assert(src);
			if(dest_layer==src)
				continue;

			// In this case, we are just moving.
//			if(dest_canvas==src->get_canvas())
			{
				if(dest_canvas==src->get_canvas() && dest_layer_depth && dest_layer_depth>src->get_depth())
					dest_layer_depth--;
				if(dest_canvas==src->get_canvas() && dest_layer_depth==src->get_depth())
					continue;

				synfigapp::Action::Handle action(synfigapp::Action::create("LayerMove"));
				action->set_param("canvas",dest_canvas);
				action->set_param("canvas_interface",canvas_interface());
				action->set_param("layer",src);
				action->set_param("new_index",dest_layer_depth);
				action->set_param("dest_canvas",dest_canvas);
				if(canvas_interface()->get_instance()->perform_action(action))
				{
					ret=true;
				}
				else
				{
					passive_grouper.cancel();
					return false;
				}
				continue;
			}
		}
	}
	synfig::info("I supposedly moved %d layers",i);

	// Reselect the previously selected layers
	canvas_interface()->get_selection_manager()->set_selected_layers(selected_layer_list);

	return ret;
	*/
	return false;
}







void
LayerGroupTreeStore::rebuild()
{
	rebuilding=true;
	try {

		// Clear out the current list
		clear();
		Canvas::Handle canvas(canvas_interface()->get_canvas());
		std::set<String> groups(canvas->get_groups());
		for(;!groups.empty();groups.erase(groups.begin()))
		{
			String group(*groups.begin());
			Gtk::TreeRow row(on_group_added(group));
			std::set<Layer::Handle> layers(canvas->get_layers_in_group(group));

			for(;!layers.empty();layers.erase(layers.begin()))
			{
				Gtk::TreeRow layer_row(*(prepend(row.children())));
				Layer::Handle layer(*layers.begin());
				set_row_layer(layer_row,layer);
			}
		}

		// Go ahead and add all the layers
		/*std::for_each(
			canvas_interface()->get_canvas()->rbegin(), canvas_interface()->get_canvas()->rend(),
			sigc::mem_fun(*this, &studio::LayerGroupTreeStore::on_layer_added)
		);*/
	}
	catch(...)
	{
		rebuilding=false;
		throw;
	}
	rebuilding=false;
	resort();
	// synfig::info("LayerGroupTreeStore::rebuild() took %f seconds",float(timer()));
}

void
LayerGroupTreeStore::refresh()
{
	rebuild();
}

void
LayerGroupTreeStore::resort()
{
	// For some reason Gtk doesn't seem to have a method that does just that and
	// ignores calls to set_sort_column if sorting params are unchanged, so we
	// have to call it twice
	int sort_column;
	Gtk::SortType sort_order;
	if (get_sort_column_id(sort_column, sort_order)) {
		Gtk::SortType reverse_order = sort_order == Gtk::SORT_DESCENDING ? Gtk::SORT_ASCENDING : Gtk::SORT_DESCENDING;
		set_sort_column(sort_column, reverse_order);
		set_sort_column(sort_column, sort_order);
	}
}

void
LayerGroupTreeStore::refresh_row(Gtk::TreeModel::Row &row)
{
	if((bool)row[model.is_layer])
	{
		Layer::Handle layer=row[model.layer];


		//if(layer->dynamic_param_list().count("z_depth"))
		//	row[model.z_depth]=Time::begin();
	}

	Gtk::TreeModel::Children children = row.children();
	Gtk::TreeModel::Children::iterator iter;

	if(!children.empty())
		for(iter = children.begin(); iter && iter != children.end(); ++iter)
		{
			Gtk::TreeRow row=*iter;
			refresh_row(row);
		}
}


void
LayerGroupTreeStore::set_row_layer(Gtk::TreeRow &row,synfig::Layer::Handle &handle)
{
	row[model.is_layer] = true;
	row[model.is_group] = false;
	row[model.layer] = handle;
}

Gtk::TreeRow
LayerGroupTreeStore::on_group_added(synfig::String group)
{
	// Check to see if this group perhaps already
	// exists
	{
		Gtk::TreeModel::Children::iterator iter;
		if(find_group_row(group,  iter))
			return *iter;
	}

	if(group.find(GROUP_NEST_CHAR)!=String::npos)
	{
		Gtk::TreeModel::Children::iterator iter;
		String parent_name;
		do
		{
			if(parent_name.size())
				parent_name+=GROUP_NEST_CHAR;
			parent_name+=std::string(group,0,group.find(GROUP_NEST_CHAR));

			if(!find_group_row(parent_name, iter))
				iter=on_group_added(parent_name);

			group=String(group,group.find(GROUP_NEST_CHAR)+1,String::npos);
		}while(group.find(GROUP_NEST_CHAR)!=String::npos);

		if(parent_name.size())
			parent_name+=GROUP_NEST_CHAR;
		parent_name+=group;

		if(iter)
		{
			Gtk::TreeRow row(*(prepend(iter->children())));
			row[model.group_name]=parent_name;
			row[model.is_layer]=false;
			row[model.is_group]=true;
			on_activity();
			return row;
		}
	}

	Gtk::TreeRow row(*(append()));
	row[model.group_name]=group;
	row[model.is_layer]=false;
	row[model.is_group]=true;
	on_activity();
	return row;
}

bool
LayerGroupTreeStore::on_group_removed(synfig::String group)
{
	Gtk::TreeModel::Children::iterator iter;
	if(find_group_row(group,iter) && iter->children().size()==0)
		erase(iter);
	else
		return false;

	return true;
}

bool
LayerGroupTreeStore::on_group_changed(synfig::String /*group*/)
{
	return true;
}

void
LayerGroupTreeStore::on_group_pair_added(synfig::String group, etl::handle<synfig::Layer> layer)
{
	if(!layer->get_canvas())
		return;
	Gtk::TreeModel::Children::iterator iter;
	if(!find_group_row(group, iter))
		iter=on_group_added(group);

	Gtk::TreeRow layer_row(*(append(iter->children())));
	set_row_layer(layer_row,layer);

	resort();

	on_activity();
}

void
LayerGroupTreeStore::on_group_pair_removed(synfig::String group, etl::handle<synfig::Layer> layer)
{
	if(!layer->get_canvas())
		return;
	Gtk::TreeModel::Children::iterator iter;
	if(!find_group_row(group, iter))
		return;

	Gtk::TreeModel::Children::iterator prev,layer_iter;

	if(!find_layer_row_(layer, layer->get_canvas(), iter->children(), layer_iter, prev))
		return;

	erase(layer_iter);

	on_activity();
}

void
LayerGroupTreeStore::on_activity()
{
	// If we aren't rebuilding and the last action
	// had something to do with groups, then go
	// ahead and present the groups dialog.
	if(!rebuilding && canvas_interface()->get_instance()->get_most_recent_action_name().find("Group")!=String::npos)
	try
	{
		App::dock_manager->find_dockable("groups").present();
	}
	catch(...) { }
}

void
LayerGroupTreeStore::on_layer_status_changed(synfig::Layer::Handle handle,bool /*x*/)
{
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
		(*iter)[model.layer]=handle;
	else
	{
		// Not need to send a warning when a layer changes its status and
		// it is not found in any group.
		//synfig::warning("Couldn't find layer to be activated in layer list. Rebuilding index...");
		rebuild();
	}
}


void
LayerGroupTreeStore::on_layer_new_description(synfig::Layer::Handle handle,synfig::String desc)
{
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
	{
		Gtk::TreeRow row(*iter);

		Layer::Handle layer(row[model.layer]);

		if(desc.empty())
		{
			//row[model.label]=layer->get_local_name();
			row[model.tooltip]=Glib::ustring(_("Layer"));
		}
		else
			//row[model.label]=layer->get_description();
			row[model.tooltip]=layer->get_local_name();
		resort();
	}
	else
	{
		rebuild();
	}
}

bool
LayerGroupTreeStore::find_layer_row_(const synfig::Layer::Handle &layer, synfig::Canvas::Handle canvas, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev)
{
	assert(layer);

	//if(layer->get_canvas()==canvas)
	{
		for(iter=prev=layers.begin(); iter && iter != layers.end(); prev=iter++)
		{
			Gtk::TreeModel::Row row = *iter;
			if((bool)row[model.is_layer] && layer==(synfig::Layer::Handle)row[model.layer])
				return true;
		}

		iter=children().end();
		//return false;
	}

	Gtk::TreeModel::Children::iterator iter2;

	for(iter2 = layers.begin(); iter2 && iter2 != layers.end(); ++iter2)
	{
		Gtk::TreeModel::Row row = *iter2;
		assert((bool)true);

		if(row.children().empty())
			continue;

		/*Canvas::Handle canvas((*row.children().begin())[model.canvas]);
		if(!canvas)
			continue;
		*/

		if(find_layer_row_(layer,canvas,iter2->children(),iter,prev))
			return true;
	}

	iter=children().end();
	return false;
}

bool
LayerGroupTreeStore::find_layer_row(const synfig::Layer::Handle &layer, Gtk::TreeModel::Children::iterator &iter)
{
	Gtk::TreeModel::Children::iterator prev;
	return find_layer_row_(layer,canvas_interface()->get_canvas(),children(),iter,prev);
}

bool
LayerGroupTreeStore::find_group_row(const synfig::String &group, Gtk::TreeModel::Children::iterator &iter)
{
	Gtk::TreeModel::Children::iterator prev;
	return find_group_row_(group,children(),iter,prev);
}

bool
LayerGroupTreeStore::find_group_row_(const synfig::String &group, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev)
{
	//if(layer->get_canvas()==canvas)
	{
		for(iter=prev=layers.begin(); iter && iter != layers.end(); prev=iter++)
		{
			Gtk::TreeModel::Row row = *iter;
			if((bool)row[model.is_group] && group==(Glib::ustring)row[model.group_name])
				return true;
		}

		iter=children().end();
		//return false;
	}

	Gtk::TreeModel::Children::iterator iter2;

	for(iter2 = layers.begin(); iter2 && iter2 != layers.end(); ++iter2)
	{
		Gtk::TreeModel::Row row = *iter2;
		assert((bool)true);

		if(row.children().empty())
			continue;

		if(find_group_row_(group,iter2->children(),iter,prev))
			return true;
	}

	iter=children().end();
	return false;
}

bool
LayerGroupTreeStore::find_prev_layer_row(const synfig::Layer::Handle &layer, Gtk::TreeModel::Children::iterator &prev)
{
	Gtk::TreeModel::Children::iterator iter;
	if(!find_layer_row_(layer,canvas_interface()->get_canvas(),children(),iter,prev))
		return false;
	if(iter==children().begin())
		return false;
	return true;
}
