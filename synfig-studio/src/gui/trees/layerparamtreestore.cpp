/* === S Y N F I G ========================================================= */
/*!	\file layerparamtreestore.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include <gui/trees/layerparamtreestore.h>

#include <glibmm/main.h>

#include <gui/app.h>
#include <gui/trees/layertree.h>
#include <gui/localization.h>

#include <synfig/general.h>
#include <synfig/valuenodes/valuenode_bone.h>

#include <synfigapp/action_system.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static LayerParamTreeStore::Model& ModelHack()
{
	static LayerParamTreeStore::Model* model(0);
	if(!model)model=new LayerParamTreeStore::Model;
	return *model;
}

LayerParamTreeStore::LayerParamTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_,LayerTree* layer_tree):
	Gtk::TreeStore			(ModelHack()),
	CanvasTreeStore			(canvas_interface_),
	layer_tree				(layer_tree)
{
	queued=0;
	// Connect all the signals
	canvas_interface()->signal_value_node_changed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_changed));
	canvas_interface()->signal_value_node_renamed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_renamed));
	canvas_interface()->signal_value_node_added().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_added));
	canvas_interface()->signal_value_node_deleted().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_deleted));
	canvas_interface()->signal_value_node_replaced().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_replaced));
	canvas_interface()->signal_layer_param_changed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_layer_param_changed));

	canvas_interface()->signal_value_node_child_added().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_child_added));
	canvas_interface()->signal_value_node_child_removed().connect(sigc::mem_fun(*this,&studio::LayerParamTreeStore::on_value_node_child_removed));

	// This is for updating timetrack when empty/disabled keyframe is moved/deleted
	// Looks a bit hackish, but I don't know the other way to do that. --KD
	canvas_interface()->signal_keyframe_changed().connect(sigc::hide_return(sigc::hide(sigc::mem_fun(*this,&studio::LayerParamTreeStore::queue_refresh))));
	canvas_interface()->signal_keyframe_removed().connect(sigc::hide_return(sigc::hide(sigc::mem_fun(*this,&studio::LayerParamTreeStore::queue_refresh))));


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

	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("LayerParamTreeStore::~LayerParamTreeStore(): Deleted");
}

Glib::RefPtr<LayerParamTreeStore>
LayerParamTreeStore::create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_, LayerTree*layer_tree)
{
	return Glib::RefPtr<LayerParamTreeStore>(new LayerParamTreeStore(canvas_interface_,layer_tree));
}



void
LayerParamTreeStore::get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const
{
	if(column<0)
	{
		synfig::error("LayerParamTreeStore::get_value_vfunc(): Bad column!");
		return;
	}

/*	if(column==model.label.index())
	{
		synfig::Layer::Handle layer((*iter)[model.layer]);

		if(!layer)return;

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(layer->get_non_empty_description());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
*/
	if(column==model.label.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
		Glib::ustring label;

		if(!(*iter)[model.is_toplevel])
			return CanvasTreeStore::get_value_vfunc(iter,column,value);
		synfig::ParamDesc param_desc((*iter)[model.param_desc]);
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

		x.set(path.size()<=1);

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
			Glib::Value<synfig::ValueBase> x;
			// We did not find a compatible converter, so for now, let's leave it as is
			g_value_init(x.gobj(),model.value.type());
			g_value_copy(value.gobj(),x.gobj());

			if((bool)(*iter)[model.is_toplevel])
			{
				synfigapp::Action::PassiveGrouper group(canvas_interface()->get_instance().get(),_("Set Layer Parameters"));

				synfig::ParamDesc param_desc((*iter)[model.param_desc]);

				LayerList mylist(layer_list);
				LayerList::iterator iter2(mylist.begin());

				//for(;iter2!=layer_list.end();++iter2)
				for(;iter2!=mylist.end();iter2++)
				{
					if(!canvas_interface()->change_value(synfigapp::ValueDesc(*iter2,param_desc.get_name()),x.get()))
					{
						// ERROR!
						group.cancel();
						App::dialog_message_1b(
								"ERROR",
								_("Unable to set all layer parameters."),
								"details",
								_("Close"));

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
		else
*/
		CanvasTreeStore::set_value_impl(iter,column, value);
	}
	catch(std::exception& x)
	{
		g_warning("%s", x.what());
	}
}










void
LayerParamTreeStore::rebuild()
{
	// Profiler profiler("LayerParamTreeStore::rebuild()");
	if(queued)queued=0;
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
		Layer::Handle layer_0;

		static ParamVocab::iterator find_param_desc(ParamVocab& vocab, const synfig::String& x)
		{
			ParamVocab::iterator iter;

			for(iter=vocab.begin();iter!=vocab.end();++iter)
				if(iter->get_name()==x)
					break;
			return iter;
		}

		void process_vocab(synfig::Layer::Handle layer_n)
		{
			ParamVocab x = layer_n->get_param_vocab();
			ParamVocab::iterator iter;

			for(iter=vocab.begin();iter!=vocab.end();++iter)
			{
				String name(iter->get_name());
				ParamVocab::iterator iter2(find_param_desc(x,name));
				if(iter2==x.end() ||
				   layer_0->get_param(name).get_type() != layer_n->get_param(name).get_type())
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
		rebuild_helper.layer_0=*iter;

		for(++iter;iter!=layer_list.end();++iter)
		{
			rebuild_helper.process_vocab(*iter);
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
		synfigapp::ValueDesc value_desc(layer_list.front(),iter->get_name());
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
		if(value_desc.get_value_type()==type_canvas)
		{
			Canvas::Handle canvas_handle = value_desc.get_value().get(Canvas::Handle());
			if(canvas_handle) changed_connection_list.push_back(
				canvas_handle->signal_changed().connect(
					sigc::mem_fun(
						this,
						&LayerParamTreeStore::changed
					)
				)
			);
		}
		//row[model.label] = iter->get_local_name();
		row[model.tooltip] = iter->get_local_name()+": "+iter->get_description();
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
				while(!row.children().empty() && erase(row.children().begin()))
					;
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
	if(queued)queued=0;

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
				while(!row.children().empty() && erase(row.children().begin()))
					;
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
LayerParamTreeStore::set_row(Gtk::TreeRow row,synfigapp::ValueDesc value_desc)
{
	Gtk::TreeModel::Children children = row.children();
	while(!children.empty() && erase(children.begin()))
		;

	CanvasTreeStore::set_row(row,value_desc);
}

void
LayerParamTreeStore::on_value_node_added(synfig::ValueNode::Handle /*value_node*/)
{
//	queue_refresh();
}

void
LayerParamTreeStore::on_value_node_deleted(synfig::ValueNode::Handle /*value_node*/)
{
//	queue_refresh();
}

void
LayerParamTreeStore::on_value_node_child_added(synfig::ValueNode::Handle /*value_node*/,synfig::ValueNode::Handle /*child*/)
{
	queue_rebuild();
}

void
LayerParamTreeStore::on_value_node_child_removed(synfig::ValueNode::Handle value_node, synfig::ValueNode::Handle /*child*/)
{
	TreeModel::iterator iter_to_remove;
	foreach_iter([&](const TreeModel::iterator &iter) -> bool {
		Gtk::TreeRow row = *iter;
		synfig::ValueNode::Handle row_value_node = row[model.value_node];
		if (row_value_node == value_node) {
			iter_to_remove = iter;
			return true;
		}
		return false;
	});

	if (iter_to_remove) {
		erase(iter_to_remove);
		rebuild();
	}
}

void
LayerParamTreeStore::on_value_node_changed(synfig::ValueNode::Handle /*value_node*/)
{
	queue_refresh();
}

void
LayerParamTreeStore::on_value_node_renamed(synfig::ValueNode::Handle /*value_node*/)
{
	rebuild();
}

void
LayerParamTreeStore::on_value_node_replaced(synfig::ValueNode::Handle /*replaced_value_node*/,synfig::ValueNode::Handle /*new_value_node*/)
{
	// this used to be "queue_refresh();" but it was crashing when we
	// first animated a bone's parent parameter (if the params panel
	// was quite large).  the replaced_value_node handle was out of
	// scope by the time the tree was rebuilt, and the tree code was
	// failing as a result.  not sure how the tree code has a pointer
	// rather than a handle - maybe it has a loosehandle, that would
	// cause the problem I was seeing
	rebuild();
}

void
LayerParamTreeStore::on_layer_param_changed(synfig::Layer::Handle handle, synfig::String param_name)
{
	//when a freetype layer param's text has changed, signal the layer the new text content

	if (param_name == "text")
	{
		const String &temp = String();
		const String textContent = handle->get_param("text").get(temp);
		handle->set_description(textContent);
		canvas_interface()->signal_layer_new_description()(handle, textContent);
	}

	queue_refresh();
}

bool
LayerParamTreeStore::find_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter)
{
    //! check level0
    for(iter = children().begin(); iter && iter != children().end(); ++iter)
    {
        if(value_desc.is_value_node())
        {
            //! transformation handle is at level 0, force to inspect deeper if case.
            if( (value_desc.get_value_node()==((synfigapp::ValueDesc)(*iter)[model.value_desc]).get_value_node()) &&
                    (value_desc.get_value_type() == type_transformation)
                    )
            {
                Gtk::TreeIter iter2 = iter->children().begin();
                for( ; iter2 && iter2 != iter->children().end(); ++iter2)
                {
                    if ( ((synfigapp::ValueDesc)(*iter2)[model.value_desc]).get_name() ==
                            value_desc.get_sub_name() )
                    {
                        iter = iter2;
                        return true;
                    }
                }
            }
        }
        //! something found
        if (value_desc==(*iter)[model.value_desc])
            return true;
    }

    //! let's go for deepness
    Gtk::TreeIter iter2;
    for(iter2 = children().begin(); iter2 && iter2 != children().end(); ++iter2)
    {
        if((*iter2).children().empty())
            continue;

        if(find_value_desc(value_desc,iter,iter2->children()))
            return true;
    }
    return false;
}

bool
LayerParamTreeStore::find_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter, const Gtk::TreeNodeChildren child_iter)
{
    // actual level from child_iter
    for(iter = child_iter.begin(); iter && iter != child_iter.end(); ++iter)
    {
        if(value_desc.is_value_node())
        {
            // for vertex handle force to inspect deeper
            if( value_desc.get_value_node()==((synfigapp::ValueDesc)(*iter)[model.value_desc]).get_value_node() &&
                    (value_desc.get_value_type() == type_bline_point)
                    )
// seems to be not necessary to check the ValueNode
//                     if((ValueNode_Composite::Handle::cast_dynamic(
//                             ((synfigapp::ValueDesc)(*iter)[model.value_desc]).get_value_node()))
//                             )
                 {
                     //check iteratively spline point (bline) children for same name.
                     Gtk::TreeIter iter2 = iter->children().begin();
                     for( ; iter2 && iter2 != iter->children().end(); ++iter2)
                     {
                         if ( ((synfigapp::ValueDesc)(*iter2)[model.value_desc]).get_name() ==
                                 value_desc.get_sub_name() )
                         {
                             iter = iter2;
                             return true;
                         }
                     }
                 }
        }
        if (value_desc==(*iter)[model.value_desc])
            return true;
    }

    Gtk::TreeIter iter2 = child_iter.begin();

    //! for bones, do not inspect recursively to don't get trapped in bones hierarchy
    if( ((synfigapp::ValueDesc)(*iter2)[model.value_desc]).parent_is_value_node() )
    {
        if(ValueNode_Bone::Handle::cast_dynamic(
                ((synfigapp::ValueDesc)(*iter2)[model.value_desc]).get_parent_value_node()))
            return false;
    }

    for( ; iter2 && iter2 != child_iter.end(); ++iter2)
    {
        if((*iter2).children().empty())
            continue;

        if(find_value_desc(value_desc,iter,iter2->children()))
            return true;
    }

    return false;
}
