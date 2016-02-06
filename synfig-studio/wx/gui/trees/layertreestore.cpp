/* === S Y N F I G ========================================================= */
/*!	\file layertreestore.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include <glibmm/main.h>

#include "layertreestore.h"
#include "iconcontroller.h"
#include <gtkmm/button.h>
#include <synfig/paramdesc.h>
#include <synfigapp/action.h>
#include <synfigapp/instance.h>
#include "app.h"
#include "instance.h"
#include <synfig/layers/layer_pastecanvas.h>
#include <synfigapp/action_system.h>
#include <synfig/context.h>

#include <gtk/gtk.h>
#include <ETL/clock>
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static LayerTreeStore::Model& ModelHack()
{
	static LayerTreeStore::Model* model(0);
	if(!model)model=new LayerTreeStore::Model;
	return *model;
}

LayerTreeStore::LayerTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_):
	Gtk::TreeStore			(ModelHack()),
	queued					(false),
	canvas_interface_		(canvas_interface_)
{
	layer_icon=Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-layer"),Gtk::ICON_SIZE_SMALL_TOOLBAR);

	// Connect Signals to Terminals
	canvas_interface()->signal_layer_status_changed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_status_changed));
	canvas_interface()->signal_layer_exclude_from_rendering_changed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_exclude_from_rendering_changed));
	canvas_interface()->signal_layer_z_range_changed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_z_range_changed));
	canvas_interface()->signal_layer_lowered().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_lowered));
	canvas_interface()->signal_layer_raised().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_raised));
	canvas_interface()->signal_layer_removed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_removed));
	canvas_interface()->signal_layer_inserted().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_inserted));
	canvas_interface()->signal_layer_moved().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_moved));
	//canvas_interface()->signal_layer_param_changed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_param_changed));
	canvas_interface()->signal_layer_new_description().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_layer_new_description));

	canvas_interface()->signal_time_changed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::refresh));

	//canvas_interface()->signal_value_node_changed().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_value_node_changed));
	//canvas_interface()->signal_value_node_added().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_value_node_added));
	//canvas_interface()->signal_value_node_deleted().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_value_node_deleted));
	//canvas_interface()->signal_value_node_replaced().connect(sigc::mem_fun(*this,&studio::LayerTreeStore::on_value_node_replaced));

	set_default_sort_func(sigc::ptr_fun(index_sorter));

//	rebuild();
}

LayerTreeStore::~LayerTreeStore()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("LayerTreeStore::~LayerTreeStore(): Deleted");
}

int
LayerTreeStore::z_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs)
{
	const Model model;

	float diff((float)(*rhs)[model.z_depth]-(float)(*lhs)[model.z_depth]);

	if(diff<0)
		return -1;
	if(diff>0)
		return 1;
	return 0;
}

int
LayerTreeStore::index_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs)
{
	const Model model;

	return ((int)(*rhs)[model.index]-(int)(*lhs)[model.index]);
}

bool
LayerTreeStore::search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring& x,const TreeModel::iterator& iter)
{
	const Model model;

	Glib::ustring substr(x.uppercase());
	Glib::ustring label((*iter)[model.label]);
	label=label.uppercase();

	return label.find(substr)==Glib::ustring::npos;
}


Glib::RefPtr<LayerTreeStore>
LayerTreeStore::create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_)
{
	return Glib::RefPtr<LayerTreeStore>(new LayerTreeStore(canvas_interface_));
}

void
LayerTreeStore::get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const
{
	if(column==model.index.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<int> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(layer->get_depth());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.z_depth.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<float> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(layer->get_true_z_depth(canvas_interface()->get_time()));

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.children_lock.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());
		x.set(false);

		ValueBase v(layer->get_param("children_lock"));
		if(v.same_type_as(bool()))
			x.set(v.get(bool()));

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.label.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(layer->get_non_empty_description());

		g_value_init(value.gobj(),x.value_type());
		//g_value_copy(x.gobj(),value.gobj());
		value=x;
	}
	else if(column==model.tooltip.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());


		x.set(layer->get_local_name());

		g_value_init(value.gobj(),x.value_type());
		//g_value_copy(x.gobj(),value.gobj());
		value=x;
	}
	else if(column==model.canvas.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Canvas::Handle> x;
		g_value_init(x.gobj(),x.value_type());


		x.set(layer->get_canvas());

		g_value_init(value.gobj(),x.value_type());
		//g_value_copy(x.gobj(),value.gobj());
		value=x;
	}
	else if(column==model.active.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(layer->active());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.exclude_from_rendering.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(layer->get_exclude_from_rendering());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.style.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Pango::Style> x;
		g_value_init(x.gobj(),x.value_type());
		//Change style to italic for current layer in treeview in case of excluded from rendering
		x.set(layer->get_exclude_from_rendering() ? Pango::STYLE_ITALIC : Pango::STYLE_NORMAL);

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.weight.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Pango::Weight> x;
		g_value_init(x.gobj(),x.value_type());

		etl::handle<Layer_PasteCanvas> paste=
			etl::handle<Layer_PasteCanvas>::cast_dynamic(
				layer->get_parent_paste_canvas_layer() );
		if(paste)
		{
			etl::handle<synfig::Canvas> sub_canvas=paste->get_param("canvas").get(sub_canvas);
			if(sub_canvas && !sub_canvas->is_inline())
			{
				Gtk::TreeRow row=*iter;
				if(*row.parent())
				{
					paste = etl::handle<Layer_PasteCanvas>::cast_dynamic(
							Layer::Handle((*row.parent())[model.layer]) );
				}
			}
		}
		if(paste)
		{
			//Change style to bold for current layer in treeview in case of visible in z_depth_visibility
			synfig::ContextParams cp;
			paste->apply_z_range_to_params(cp);
			float visibility=synfig::Context::z_depth_visibility(cp, *layer);
			x.set(visibility==1.0 && cp.z_range ? Pango::WEIGHT_BOLD : Pango::WEIGHT_NORMAL);
		}
		else
			x.set(Pango::WEIGHT_NORMAL);

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else if(column==model.icon.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);
		if(!layer)return;

		Glib::Value<Glib::RefPtr<Gdk::Pixbuf> > x;
		g_value_init(x.gobj(),x.value_type());

		//x.set(layer_icon);
		x.set(get_tree_pixbuf_layer(layer->get_name()));

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
		Gtk::TreeStore::get_value_vfunc(iter,column,value);
}

void
LayerTreeStore::set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value)
{
	//if(!iterator_sane(row))
	//	return;

	if(column>=get_n_columns_vfunc())
	{
		g_warning("LayerTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(!g_value_type_compatible(G_VALUE_TYPE(value.gobj()),get_column_type_vfunc(column)))
	{
		g_warning("LayerTreeStore::set_value_impl: Bad value type");
		return;
	}

	try
	{
		if(column==model.label.index())
		{
			Glib::Value<Glib::ustring> x;
			g_value_init(x.gobj(),model.label.type());
			g_value_copy(value.gobj(),x.gobj());

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
		else if(column==model.active.index())
		{
			synfig::Layer::Handle layer((*iter)[model.layer]);

			if(!layer)return;

			Glib::Value<bool> x;
			g_value_init(x.gobj(),model.active.type());
			g_value_copy(value.gobj(),x.gobj());

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
		else if(column==model.exclude_from_rendering.index())
		{
			synfig::Layer::Handle layer((*iter)[model.layer]);

			if(!layer)return;

			Glib::Value<bool> x;
			g_value_init(x.gobj(),model.exclude_from_rendering.type());
			g_value_copy(value.gobj(),x.gobj());

			synfigapp::Action::Handle action(synfigapp::Action::create("LayerSetExcludeFromRendering"));

			if(!action)
				return;

			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("layer",layer);
			action->set_param("new_state",bool(x.get()));

			canvas_interface()->get_instance()->perform_action(action);
			return;
		}
		else
			Gtk::TreeStore::set_value_impl(iter,column, value);

	}
	catch(std::exception x)
	{
		g_warning("%s", x.what());
	}
}




bool
LayerTreeStore::row_draggable_vfunc (const TreeModel::Path& /*path*/)const
{
	//if(!get_iter(path)) return false;
//	Gtk::TreeModel::Row row(*get_iter(path));

	return true;
//	return (bool)true;
}

bool
LayerTreeStore::drag_data_get_vfunc (const TreeModel::Path& path, Gtk::SelectionData& selection_data)const
{
	if(!const_cast<LayerTreeStore*>(this)->get_iter(path)) return false;
	//synfig::info("Dragged data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("Dragged data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("Dragged selection=\"%s\"",gdk_atom_name(selection_data->selection));

	Gtk::TreeModel::Row row(*const_cast<LayerTreeStore*>(this)->get_iter(path));

	if((bool)true)
	{
		Layer* layer(((Layer::Handle)row[model.layer]).get());
		assert(layer);
		bool included(false);

		//gtk_selection_data_set (selection_data, gdk_atom_intern("LAYER",false), 8, reinterpret_cast<const guchar*>(&layer), sizeof(layer));

		std::vector<Layer*> layers;
		// The following is a hack for multiple row DND
		{
			synfigapp::SelectionManager::LayerList bleh(get_canvas_interface()->get_selection_manager()->get_selected_layers());
			if(bleh.empty())
			{
				selection_data.set("LAYER", 8, reinterpret_cast<const guchar*>(&layer), sizeof(layer));
				return true;
			}
			while(!bleh.empty())
			{
				if(bleh.back().get()==layer)
					included=true;
				layers.push_back(bleh.back().get());
				bleh.pop_back();
			}
		}
		if(!included)
			layers.push_back(layer);
		selection_data.set("LAYER", 8, reinterpret_cast<const guchar*>(&layers.front()), sizeof(void*)*layers.size());

		return true;
	}
	return false;
}

bool
LayerTreeStore::drag_data_delete_vfunc (const TreeModel::Path& /*path*/)
{
	return true;
}

bool
LayerTreeStore::row_drop_possible_vfunc (const TreeModel::Path& dest, const Gtk::SelectionData& selection_data)const
{
	//if(!const_cast<LayerTreeStore*>(this)->get_iter(dest)) return false;

	//synfig::info("possible_drop -- data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("possible_drop -- data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("possible_drop -- selection=\"%s\"",gdk_atom_name(selection_data->selection));

	//Gtk::TreeModel::Row row(*get_iter(dest));

	if(synfig::String(selection_data.get_data_type())=="LAYER" && (bool)true)
	{
		//Layer::Handle src(reinterpret_cast<Layer**>(const_cast<guint8*>(selection_data.get_data()))[i]);
		//assert(src);

		//return true;
		TreeModel::Path dest_parent(dest);
		if(!dest_parent.up() || dest.size()==1)
		{
			//row=(*get_iter(dest));
			//dest_canvas=(Canvas::Handle)(row[model.canvas]);
			return true;
		}
		else if((bool)const_cast<LayerTreeStore*>(this)->get_iter(dest_parent))
			return (bool)(Canvas::Handle)(*const_cast<LayerTreeStore*>(this)->get_iter(dest_parent))[model.contained_canvas];
	}
	return false;
}

bool
LayerTreeStore::drag_data_received_vfunc (const TreeModel::Path& dest, const Gtk::SelectionData& selection_data)
{

	//if(!dest_parent.up() || !get_iter(dest)) return false;

	bool ret=false;
	int i(0);


	//synfig::info("Dropped data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("Dropped data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("Dropped selection=\"%s\"",gdk_atom_name(selection_data->selection));
	synfigapp::Action::PassiveGrouper passive_grouper(canvas_interface()->get_instance().get(),_("Move Layers"));

	// Save the selection data
	synfigapp::SelectionManager::LayerList selected_layer_list=canvas_interface()->get_selection_manager()->get_selected_layers();

	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Gtk::TreeModel::Row row;
		Canvas::Handle dest_canvas;

		int dest_layer_depth=dest.back();

		TreeModel::Path dest_parent(dest);
		if(!dest_parent.up() || !get_iter(dest_parent))
		{
			TreeModel::Path dest_(dest);
			if(!get_iter(dest_))
				dest_.prev();

			if(!get_iter(dest_))
				return false;

			{
				row=(*get_iter(dest_));
				dest_canvas=(Canvas::Handle)(row[model.canvas]);
			}
		}
		else
		{
			row=(*get_iter(dest_parent));
			dest_canvas=row[model.contained_canvas];
		}

		assert(dest_canvas);

		Layer::Handle dest_layer(row[model.layer]);

		if(synfig::String(selection_data.get_data_type())=="LAYER")for(unsigned int i=0;i<selection_data.get_length()/sizeof(void*);i++)
		{
			//synfig::info("dest_layer_depth=%d",dest_layer_depth);

			Layer::Handle src(reinterpret_cast<Layer**>(const_cast<guint8*>(selection_data.get_data()))[i]);
			assert(src);
			if(dest_layer==src)
				continue;

			if(dest_canvas==src->get_canvas() && src->get_depth()<dest_layer_depth)
				dest_layer_depth--;

			// In this case, we are just moving.
//			if(dest_canvas==src->get_canvas())
			{
				//if(dest_canvas==src->get_canvas() && dest_layer_depth && dest_layer_depth>src->get_depth())
				//	dest_layer_depth--;
				if(dest_canvas==src->get_canvas() && dest_layer_depth==src->get_depth())
					continue;

				synfigapp::Action::Handle action(synfigapp::Action::create("LayerMove"));
				action->set_param("canvas",dest_canvas);
				action->set_param("canvas_interface",canvas_interface());
				action->set_param("layer",src);
				action->set_param("new_index",dest_layer_depth);
				action->set_param("dest_canvas",dest_canvas);
				if(canvas_interface()->get_instance()->perform_action(action))
					ret=true;
				else
				{
					passive_grouper.cancel();
					return false;
				}
				continue;
			}
			/*else // In this case we need to remove and then add
			{

				synfigapp::Action::Handle action;
				action=synfigapp::Action::create("LayerRemove");
				action->set_param("canvas",Canvas::Handle(src->get_canvas()));
				if(!action->set_param("canvas_interface",App::get_instance(src->get_canvas())->find_canvas_interface(src->get_canvas())))
					action->set_param("canvas_interface",canvas_interface());
				action->set_param("layer",src);
				if(!canvas_interface()->get_instance()->perform_action(action))
				{
					passive_grouper.cancel();
					ret=false;
					return false;
				}

				action=synfigapp::Action::create("LayerAdd");
				action->set_param("canvas",dest_canvas);
				action->set_param("canvas_interface",canvas_interface());
				action->set_param("new",src);
				if(!canvas_interface()->get_instance()->perform_action(action))
				{
					passive_grouper.cancel();
					ret=false;
					return false;
				}

				if(dest_layer_depth!=0)
				{
					action=synfigapp::Action::create("LayerMove");
					action->set_param("canvas",dest_canvas);
					action->set_param("canvas_interface",canvas_interface());
					action->set_param("layer",src);
					action->set_param("new_index",dest_layer_depth);
					if(!canvas_interface()->get_instance()->perform_action(action))
					{
						passive_grouper.cancel();
						ret=false;
						return false;
					}
				}
				ret=true;
			}
			*/
		}
	}
	synfig::info("I supposedly moved %d layers",i);

	// Reselect the previously selected layers
	canvas_interface()->get_selection_manager()->set_selected_layers(selected_layer_list);

	return ret;
}

void
LayerTreeStore::queue_rebuild()
{
	if (queued) return;
	queued = false;
	queue_connection.disconnect();
	queue_connection=Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&LayerTreeStore::rebuild),
			false
		)
	,150);
}

void
LayerTreeStore::rebuild()
{
	if (queued) queued = false;

	// disconnect any subcanvas_changed connections
	std::map<synfig::Layer::Handle, sigc::connection>::iterator iter;
	for (iter = subcanvas_changed_connections.begin(); iter != subcanvas_changed_connections.end(); iter++)
		iter->second.disconnect();
	subcanvas_changed_connections.clear();

	//etl::clock timer;timer.reset();

	//synfig::warning("---------rebuilding layer table---------");
	// Save the selection data
	synfigapp::SelectionManager::LayerList layer_list=canvas_interface()->get_selection_manager()->get_selected_layers();

	// Clear out the current list
	clear();

	// Go ahead and add all the layers
	std::for_each(
		canvas_interface()->get_canvas()->rbegin(), canvas_interface()->get_canvas()->rend(),
		sigc::mem_fun(*this, &studio::LayerTreeStore::on_layer_added)
	);

	// Reselect the previously selected layers
	if(!layer_list.empty())
		canvas_interface()->get_selection_manager()->set_selected_layers(layer_list);

	//synfig::info("LayerTreeStore::rebuild() took %f seconds",float(timer()));
}

void
LayerTreeStore::refresh()
{
	etl::clock timer;timer.reset();

	Gtk::TreeModel::Children children_(children());

	Gtk::TreeModel::Children::iterator iter;

	if(!children_.empty())
		for(iter = children_.begin(); iter && iter != children_.end(); ++iter)
		{
			Gtk::TreeRow row=*iter;
			refresh_row(row);
		}
	//synfig::info("LayerTreeStore::refresh() took %f seconds",float(timer()));
}

void
LayerTreeStore::refresh_row(Gtk::TreeModel::Row &row)
{
	Layer::Handle layer=row[model.layer];
	/*
	{
		row[model.name] = layer->get_local_name();
		if(layer->get_description().empty())
		{
			row[model.label] = layer->get_local_name();
			row[model.tooltip] = Glib::ustring("Layer");
		}
		else
		{
			row[model.label] = layer->get_description();
			row[model.tooltip] = layer->get_local_name();
		}
	}
	*/

	if(layer->dynamic_param_list().count("z_depth"))
		row[model.z_depth]=Time::begin();
	//	row_changed(get_path(row),row);

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
LayerTreeStore::set_row_layer(Gtk::TreeRow &row,synfig::Layer::Handle &handle)
{
	//row[model.id] = handle->get_name();
	//row[model.name] = handle->get_local_name();
	/*if(handle->get_description().empty())
	{
		//row[model.label] = handle->get_local_name();
		row[model.tooltip] = Glib::ustring("Layer");
	}
	else
	{
		//row[model.label] = handle->get_description();
		row[model.tooltip] = handle->get_local_name();
	}*/

	//row[model.active] = handle->active();
	row[model.layer] = handle;
	//row[model.canvas] = handle->get_canvas();
	//row[model.icon] = layer_icon;

	synfig::Layer::ParamList paramlist=handle->get_param_list();

	synfig::Layer::Vocab vocab=handle->get_param_vocab();
	synfig::Layer::Vocab::iterator iter;

	for(iter=vocab.begin();iter!=vocab.end();++iter)
	{
		if(iter->get_hidden())
			continue;
		if(handle->get_param(iter->get_name()).get_type()!=type_canvas)
			continue;

		{
			Canvas::Handle canvas;
			canvas=handle->get_param(iter->get_name()).get(canvas);
			if(!canvas)
				continue;

			Canvas::reverse_iterator iter;
			row[model.contained_canvas]=canvas;

			for(iter=canvas->rbegin();iter!=canvas->rend();++iter)
			{
				Gtk::TreeRow row_(*(prepend(row.children())));
				set_row_layer(row_,*iter);
			}
			continue;
		}

		/*
		etl::handle<ValueNode> value_node;
		if(handle.constant()->dynamic_param_list().count(iter->get_name()))
			value_node=handle->dynamic_param_list()[iter->get_name()];

		Gtk::TreeRow child_row = *(append(row.children()));
		set_row_param(
			child_row,
			handle,
			iter->get_name(),
			iter->get_local_name(),
			paramlist[iter->get_name()],
			value_node,
			&*iter
		);
		*/
	}
}

void
LayerTreeStore::on_layer_added(synfig::Layer::Handle layer)
{
	if (etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
		subcanvas_changed_connections[layer] =
			(etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))->signal_subcanvas_changed().connect(
				sigc::mem_fun(*this,&studio::LayerTreeStore::queue_rebuild)
			);

	assert(layer);
	Gtk::TreeRow row;
	if(canvas_interface()->get_canvas()==layer->get_canvas())
	{
		row=*(prepend());
	}
	else
	{
		Gtk::TreeModel::Children::iterator iter;
		if(!find_canvas_row(layer->get_canvas(),iter))
		{
			rebuild();
			return;
		}
		row=*(prepend(iter->children()));
	}
	set_row_layer(row,layer);
}

void
LayerTreeStore::on_layer_removed(synfig::Layer::Handle handle)
{
	if (etl::handle<Layer_PasteCanvas>::cast_dynamic(handle))
	{
		subcanvas_changed_connections[handle].disconnect();
		subcanvas_changed_connections.erase(handle);
	}
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
		erase(iter);
	else
	{
		synfig::error("LayerTreeStore::on_layer_removed():Unable to find layer to be removed, forced to rebuild...");
		rebuild();
	}
}

void
LayerTreeStore::on_layer_inserted(synfig::Layer::Handle handle,int depth)
{
	if(depth==0)
	{
		on_layer_added(handle);
		return;
	}

	Gtk::TreeModel::Children children_(children());
	if(canvas_interface()->get_canvas()!=handle->get_canvas())
	{
		Gtk::TreeModel::Children::iterator iter;
		if(!find_canvas_row(handle->get_canvas(),iter))
		{
			synfig::error("LayerTreeStore::on_layer_inserted():Unable to find canvas row, forced to rebuild...");
			rebuild();
			return;
		}
		children_=iter->children();
	}

	Gtk::TreeModel::Children::iterator iter(children_.begin());
	while(depth-- && iter)
	{
		++iter;
		if(!iter || iter==children_.end())
		{
			synfig::error("LayerTreeStore::on_layer_inserted():Unable to achieve desired depth, forced to rebuild...");
			rebuild();
			return;
		}
	}

	Gtk::TreeModel::Row row(*insert(iter));
	set_row_layer(row,handle);
}

void
LayerTreeStore::on_layer_status_changed(synfig::Layer::Handle handle,bool /*x*/)
{
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
		(*iter)[model.layer]=handle;
	else
	{
		synfig::warning("Couldn't find layer to be activated in layer list. Rebuilding index...");
		rebuild();
	}
}

void
LayerTreeStore::on_layer_exclude_from_rendering_changed(synfig::Layer::Handle handle,bool /*x*/)
{
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
		(*iter)[model.layer]=handle;
	else
	{
		synfig::warning("Couldn't find layer to be excluded/included from/to rendering in layer list. Rebuilding index...");
		rebuild();
	}
}

void
LayerTreeStore::on_layer_z_range_changed(synfig::Layer::Handle handle,bool /*x*/)
{
	// Seems to not work. Need to do something different like call row_changed
	// for this layer row or all its children.
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
		(*iter)[model.layer]=handle;
	else
	{
		synfig::warning("Couldn't find layer to be change the z_depth range in layer list. Rebuilding index...");
		rebuild();
	}
}


void
LayerTreeStore::on_layer_lowered(synfig::Layer::Handle layer)
{
	Gtk::TreeModel::Children::iterator iter, iter2;
	if(find_layer_row(layer,iter))
	{
		// Save the selection data
		//synfigapp::SelectionManager::LayerList layer_list=canvas_interface()->get_selection_manager()->get_selected_layers();
		iter2=iter;
		iter2++;
		if(!iter2)
		{
			rebuild();
			return;
		}

		//Gtk::TreeModel::Row row(*iter);
		Gtk::TreeModel::Row row2 = *iter2;
		synfig::Layer::Handle layer2=row2[model.layer];

		erase(iter2);
		row2=*insert(iter);
		set_row_layer(row2,layer2);

	}
	else
		rebuild();
}

void
LayerTreeStore::on_layer_raised(synfig::Layer::Handle layer)
{
	Gtk::TreeModel::Children::iterator iter, iter2;

	Gtk::TreeModel::Children children_(children());

	if(find_layer_row_(layer, canvas_interface()->get_canvas(), children_, iter,iter2))
	{
		if(iter!=iter2)
		{
			//Gtk::TreeModel::Row row = *iter;
			Gtk::TreeModel::Row row2 = *iter2;
			synfig::Layer::Handle layer2=row2[model.layer];

			erase(iter2);
			iter++;
			row2=*insert(iter);
			set_row_layer(row2,layer2);

			return;
		}
	}

	rebuild();
}

void
LayerTreeStore::on_layer_moved(synfig::Layer::Handle layer,int depth, synfig::Canvas::Handle /*canvas*/)
{
	on_layer_removed(layer);
	on_layer_inserted(layer,depth);
}

void
LayerTreeStore::on_layer_param_changed(synfig::Layer::Handle handle,synfig::String param_name)
{
	if(param_name=="z_depth")
	{
		Gtk::TreeModel::Children::iterator iter;
		if(find_layer_row(handle,iter))
		{
			(*iter)[model.z_depth]=Time::begin();
		}
	}

	/*
	Gtk::TreeModel::Children::iterator iter;
	if(find_layer_row(handle,iter))
	{
		Gtk::TreeModel::Children children(iter->children());

		for(iter = children.begin(); iter && iter != children.end(); ++iter)
		{
			if((Glib::ustring)(*iter)[model.param_name]==param_name)
			{
				Gtk::TreeRow row=*iter;
				refresh_row(row);
				return;
			}
		}
	}
	rebuild();
	*/
}

void
LayerTreeStore::on_layer_new_description(synfig::Layer::Handle handle,synfig::String desc)
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
	}
	else
	{
		rebuild();
	}
}

bool
LayerTreeStore::find_canvas_row_(synfig::Canvas::Handle canvas, synfig::Canvas::Handle parent, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter)
{
	if(canvas==parent)
		return false;

	{
		for(iter=layers.begin(); iter && iter != layers.end(); ++iter)
		{
			Gtk::TreeModel::Row row = *iter;
			if(canvas==(synfig::Canvas::Handle)row[model.contained_canvas])
				return true;
		}

		iter=children().end();
		//return false;
	}

	Gtk::TreeModel::Children::iterator iter2;
	//Gtk::TreeModel::Children::iterator iter3;

	for(iter2 = layers.begin(); iter2 && iter2 != layers.end(); ++iter2)
	{
		Gtk::TreeModel::Row row = *iter2;
		assert((bool)true);

		if(row.children().empty())
			continue;

		Canvas::Handle sub_canvas((*row.children().begin())[model.canvas]);
		if(!sub_canvas)
			continue;

		if(find_canvas_row_(canvas,sub_canvas,iter2->children(),iter))
			return true;
	}

	iter=children().end();
	return false;
}

bool
LayerTreeStore::find_canvas_row(synfig::Canvas::Handle canvas, Gtk::TreeModel::Children::iterator &iter)
{
	return find_canvas_row_(canvas,canvas_interface()->get_canvas(),children(),iter);
}


bool
LayerTreeStore::find_layer_row_(const synfig::Layer::Handle &layer, synfig::Canvas::Handle /*canvas*/, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev)
{
	assert(layer);

	//if(layer->get_canvas()==canvas)
	{
		for(iter=prev=layers.begin(); iter && iter != layers.end(); prev=iter++)
		{
			Gtk::TreeModel::Row row = *iter;
			if(layer==(synfig::Layer::Handle)row[model.layer])
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

		Canvas::Handle canvas((*row.children().begin())[model.canvas]);
		if(!canvas)
			continue;

		if(find_layer_row_(layer,canvas,iter2->children(),iter,prev))
			return true;
	}

	iter=children().end();
	return false;
}

bool
LayerTreeStore::find_layer_row(const synfig::Layer::Handle &layer, Gtk::TreeModel::Children::iterator &iter)
{
	Gtk::TreeModel::Children::iterator prev;
	return find_layer_row_(layer,canvas_interface()->get_canvas(),children(),iter,prev);
}

bool
LayerTreeStore::find_prev_layer_row(const synfig::Layer::Handle &layer, Gtk::TreeModel::Children::iterator &prev)
{
	Gtk::TreeModel::Children::iterator iter;
	if(!find_layer_row_(layer,canvas_interface()->get_canvas(),children(),iter,prev))
		return false;
	if(iter==children().begin())
		return false;
	return true;
}
