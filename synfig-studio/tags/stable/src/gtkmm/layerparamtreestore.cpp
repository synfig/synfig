/* === S I N F G =========================================================== */
/*!	\file childrentreestore.cpp
**	\brief Template File
**
**	$Id: layerparamtreestore.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "layerparamtreestore.h"
#include "iconcontroler.h"
#include <gtkmm/button.h>
#include <sinfg/paramdesc.h>
#include "layertree.h"
#include <sinfgapp/action_system.h>
#include <sinfgapp/instance.h>
#include "app.h"
#include <ETL/clock>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

class Profiler : private etl::clock
{
	const std::string name;
public:
	Profiler(const std::string& name):name(name) { reset(); }
	~Profiler() { float time(operator()()); sinfg::info("%s: took %f msec",name.c_str(),time*1000); }
};

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static LayerParamTreeStore::Model& ModelHack()
{
	static LayerParamTreeStore::Model* model(0);
	if(!model)model=new LayerParamTreeStore::Model;
	return *model;
}

LayerParamTreeStore::LayerParamTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_,LayerTree* layer_tree):
	Gtk::TreeStore			(ModelHack()),
	CanvasTreeStore			(canvas_interface_),
	layer_tree				(layer_tree)
{
	queued=false;
	// Connect all the signals
	canvas_interface()->signal_value_node_changed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_changed));
	canvas_interface()->signal_value_node_added().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_added));
	canvas_interface()->signal_value_node_deleted().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_deleted));
	canvas_interface()->signal_value_node_replaced().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_replaced));
	canvas_interface()->signal_layer_param_changed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_layer_param_changed));

	canvas_interface()->signal_value_node_child_added().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_child_added));
	canvas_interface()->signal_value_node_child_removed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_child_removed));
	
	
	layer_tree->get_selection()->signal_changed().connect(sigc::mem_fun(*this,&LayerParamTreeStore::queue_rebuild));
	
	signal_changed().connect(sigc::mem_fun(*this,&LayerParamTreeStore::queue_refresh));
	rebuild();
}

LayerParamTreeStore::~LayerParamTreeStore()
{
	while(!changed_connection_list.empty())
	{
		changed_connection_list.back().disconnect();
		changed_connection_list.pop_back();
	}
	sinfg::info("LayerParamTreeStore::~LayerParamTreeStore(): Deleted");
}

Glib::RefPtr<LayerParamTreeStore>
LayerParamTreeStore::create(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_, LayerTree*layer_tree)
{
	return Glib::RefPtr<LayerParamTreeStore>(new LayerParamTreeStore(canvas_interface_,layer_tree));
}



void
LayerParamTreeStore::get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const
{
	if(column<0)
	{
		sinfg::error("LayerParamTreeStore::get_value_vfunc(): Bad column!");
		return;
	}
	
/*	if(column==model.label.index())
	{
		sinfg::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());


		if(!layer->get_description().empty())
			x.set(layer->get_description());
		else
			x.set(layer->get_local_name());
		
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
*/
	if(column==model.label.index())
	{
		sinfgapp::ValueDesc value_desc((*iter)[model.value_desc]);
		Glib::ustring label;
		
		if(!(*iter)[model.is_toplevel])
			return CanvasTreeStore::get_value_vfunc(iter,column,value);
		sinfg::ParamDesc param_desc((*iter)[model.param_desc]);
		label=param_desc.get_local_name();
		
		if(!(*iter)[model.is_inconsistent])
		if(value_desc.is_value_node() && value_desc.get_value_node()->is_exported())
		{
			label+=strprintf(" (%s)",value_desc.get_value_node()->get_id().c_str());
		}
		
		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(label);
		
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_toplevel.index())
	{
		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		TreeModel::Path path(get_path(iter));
		
		x.set(path.get_depth()<=1);
		
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_inconsistent.index())
	{
		if((*iter)[model.is_toplevel])
		{
			CanvasTreeStore::get_value_vfunc(iter,column,value);
			return;
		}
		
		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(false);
		
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	CanvasTreeStore::get_value_vfunc(iter,column,value);
}



void
LayerParamTreeStore::set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value)
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
		if(column==model.value.index())
		{
			Glib::Value<sinfg::ValueBase> x;
			g_value_init(x.gobj(),model.value.type());
			g_value_copy(value.gobj(),x.gobj());
			
			if((bool)(*iter)[model.is_toplevel])
			{
				sinfgapp::Action::PassiveGrouper group(canvas_interface()->get_instance().get(),_("Set Layer Params"));

				sinfg::ParamDesc param_desc((*iter)[model.param_desc]);

				LayerList::iterator iter2(layer_list.begin());
				
				for(;iter2!=layer_list.end();++iter2)
				{
					if(!canvas_interface()->change_value(sinfgapp::ValueDesc(*iter2,param_desc.get_name()),x.get()))
					{
						// ERROR!
						group.cancel();
						App::dialog_error_blocking(_("Error"),_("Unable to set all layer parameters."));
						
						return;
					}
				}
			}
			else
			{
				canvas_interface()->change_value((*iter)[model.value_desc],x.get());
			}
			return;
		}
		else
/*
		if(column==model.active.index())
		{
			sinfg::Layer::Handle layer((*iter)[model.layer]);
			
			if(!layer)return;

			Glib::Value<bool> x;
			g_value_init(x.gobj(),model.active.type());
			g_value_copy(value.gobj(),x.gobj());
			
			sinfgapp::Action::Handle action(sinfgapp::Action::create("layer_activate"));
			
			if(!action)
				return;
			
			action->set_param("canvas",canvas_interface()->get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("layer",layer);
			action->set_param("new_status",bool(x.get()));
			
			canvas_interface()->get_instance()->perform_action(action);
			return;
		}
		else
*/
		CanvasTreeStore::set_value_impl(iter,column, value);
	}
	catch(std::exception x)
	{
		g_warning(x.what());
	}	
}










void
LayerParamTreeStore::rebuild()
{
	Profiler profiler("LayerParamTreeStore::rebuild()");
	if(queued)queued=false;
	clear();
	layer_list=layer_tree->get_selected_layers();
	
	if(layer_list.size()<=0)
		return;
	
	// Get rid of all the connections,
	// and clear the connection map.
	//while(!connection_map.empty())connection_map.begin()->second.disconnect(),connection_map.erase(connection_map.begin());
	while(!changed_connection_list.empty())
	{
		changed_connection_list.back().disconnect();
		changed_connection_list.pop_back();
	}

	struct REBUILD_HELPER
	{
		ParamVocab vocab;

		static ParamVocab::iterator find_param_desc(ParamVocab& vocab, const sinfg::String& x)
		{
			ParamVocab::iterator iter;
	
			for(iter=vocab.begin();iter!=vocab.end();++iter)
				if(iter->get_name()==x)
					break;
			return iter;
		}
		
		void process_vocab(ParamVocab x)
		{
			ParamVocab::iterator iter;
	
			for(iter=vocab.begin();iter!=vocab.end();++iter)
			{
				ParamVocab::iterator iter2(find_param_desc(x,iter->get_name()));
				if(iter2==x.end())
				{
					// remove it and start over
					vocab.erase(iter);
					iter=vocab.begin();
					iter--;
					continue;
				}
			}
		}
		
	} rebuild_helper;

	
	{
		LayerList::iterator iter(layer_list.begin());
		rebuild_helper.vocab=(*iter)->get_param_vocab();
		
		for(++iter;iter!=layer_list.end();++iter)
		{
			rebuild_helper.process_vocab((*iter)->get_param_vocab());
			changed_connection_list.push_back(
				(*iter)->signal_changed().connect(
					sigc::mem_fun(
						*this,
						&LayerParamTreeStore::changed
					)
				)
			);
		}
	}
	
	ParamVocab::iterator iter;
	for(iter=rebuild_helper.vocab.begin();iter!=rebuild_helper.vocab.end();++iter)
	{
		if(iter->get_hidden())
			continue;
		
		/*
		if(iter->get_animation_only())
		{
			int length(layer_list.front()->get_canvas()->rend_desc().get_frame_end()-layer_list.front()->get_canvas()->rend_desc().get_frame_start());
			if(!length)
				continue;
		}
		*/
		Gtk::TreeRow row(*(append()));
		sinfgapp::ValueDesc value_desc(layer_list.front(),iter->get_name());
		CanvasTreeStore::set_row(row,value_desc);
		if(value_desc.is_value_node())
		{
			changed_connection_list.push_back(
				value_desc.get_value_node()->signal_changed().connect(
					sigc::mem_fun(
						this,
						&LayerParamTreeStore::changed
					)
				)
			);
		}
		if(value_desc.get_value_type()==ValueBase::TYPE_CANVAS)
		{
			changed_connection_list.push_back(
				value_desc.get_value().get(Canvas::Handle())->signal_changed().connect(
					sigc::mem_fun(
						this,
						&LayerParamTreeStore::changed
					)
				)
			);
		}
		//row[model.label] = iter->get_local_name();
		row[model.param_desc] = *iter;
		row[model.canvas] = layer_list.front()->get_canvas();
		row[model.is_inconsistent] = false;
		//row[model.is_toplevel] = true;


		LayerList::iterator iter2(layer_list.begin());
		ValueBase value((*iter2)->get_param(iter->get_name()));
		for(++iter2;iter2!=layer_list.end();++iter2)
		{
			if(value!=((*iter2)->get_param(iter->get_name())))
			{
				row[model.is_inconsistent] = true;
				while(!row.children().empty() && erase(row.children().begin()));
				break;
			}	
		}
	}	
}

void
LayerParamTreeStore::queue_refresh()
{
	if(queued)
		return;
	queued=1;
	queue_connection.disconnect();
	queue_connection=Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&LayerParamTreeStore::refresh),
			false
		)
	,150);

}

void
LayerParamTreeStore::queue_rebuild()
{
	if(queued==2)
		return;
	queued=2;
	queue_connection.disconnect();
	queue_connection=Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&LayerParamTreeStore::rebuild),
			false
		)
	,150);

}

void
LayerParamTreeStore::refresh()
{
	if(queued)queued=false;
	
	Gtk::TreeModel::Children children_(children());
	
	Gtk::TreeModel::Children::iterator iter;
	
	if(!children_.empty())
		for(iter = children_.begin(); iter && iter != children_.end(); ++iter)
		{
			Gtk::TreeRow row=*iter;
			refresh_row(row);
		}
}

void
LayerParamTreeStore::refresh_row(Gtk::TreeModel::Row &row)
{
	if(row[model.is_toplevel])
	{
		row[model.is_inconsistent] = false;
		ParamDesc param_desc(row[model.param_desc]);
		
		LayerList::iterator iter2(layer_list.begin());
		ValueBase value((*iter2)->get_param(param_desc.get_name()));
		for(++iter2;iter2!=layer_list.end();++iter2)
		{
			if(value!=((*iter2)->get_param(param_desc.get_name())))
			{
				row[model.is_inconsistent] = true;
				while(!row.children().empty() && erase(row.children().begin()));
				return;
			}	
		}
	}

	//handle<ValueNode> value_node=row[model.value_node];
	//if(value_node)
	{
		CanvasTreeStore::refresh_row(row);
		return;
	}
}

void
LayerParamTreeStore::set_row(Gtk::TreeRow row,sinfgapp::ValueDesc value_desc)
{
	Gtk::TreeModel::Children children = row.children();
	while(!children.empty() && erase(children.begin()));

	CanvasTreeStore::set_row(row,value_desc);
}

void
LayerParamTreeStore::on_value_node_added(ValueNode::Handle value_node)
{
//	queue_refresh();
}

void
LayerParamTreeStore::on_value_node_deleted(etl::handle<ValueNode> value_node)
{
//	queue_refresh();
}

void
LayerParamTreeStore::on_value_node_child_added(sinfg::ValueNode::Handle value_node,sinfg::ValueNode::Handle child)
{
	queue_rebuild();
}

void
LayerParamTreeStore::on_value_node_child_removed(sinfg::ValueNode::Handle value_node,sinfg::ValueNode::Handle child)
{
	queue_rebuild();
}

void
LayerParamTreeStore::on_value_node_changed(etl::handle<ValueNode> value_node)
{
	queue_refresh();
}

void
LayerParamTreeStore::on_value_node_replaced(sinfg::ValueNode::Handle replaced_value_node,sinfg::ValueNode::Handle new_value_node)
{
	queue_rebuild();
}

void
LayerParamTreeStore::on_layer_param_changed(sinfg::Layer::Handle handle,sinfg::String param_name)
{
	queue_refresh();
}
