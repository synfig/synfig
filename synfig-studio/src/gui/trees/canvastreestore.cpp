/* === S Y N F I G ========================================================= */
/*!	\file canvastreestore.cpp
**	\brief Canvas tree store
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
**	Copyright (c) 2016 caryoscelus
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

#include <synfig/general.h>

#include "trees/canvastreestore.h"
#include <synfig/valuenode.h>
#include "iconcontroller.h"
#include <synfig/valuenodes/valuenode_timedswap.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/boneweightpair.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <gtkmm/button.h>
#include <synfigapp/instance.h>
#include "cellrenderer/cellrenderer_value.h"
#include "cellrenderer/cellrenderer_timetrack.h"
#include <ETL/clock>
#include <synfig/interpolation.h>

#include <gui/localization.h>

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

static CanvasTreeStore::Model& ModelHack()
{
	static CanvasTreeStore::Model* model(0);
	if(!model)model=new CanvasTreeStore::Model;
	return *model;
}

CanvasTreeStore::CanvasTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_):
	Gtk::TreeStore(ModelHack()),
	canvas_interface_		(canvas_interface_)
{
}

CanvasTreeStore::~CanvasTreeStore()
{
}

ValueNode::Handle
CanvasTreeStore::expandable_bone_parent(ValueNode::Handle node)
{
	if ((!getenv("SYNFIG_DISABLE_EXPANDABLE_BONE_PARENTS")) &&
		node->get_type() == type_bone_valuenode &&
		(node->get_name() == "constant" || node->get_name() == "animated"))
		if (ValueNode::Handle bone_node = (*node)(canvas_interface()->get_time()).get(ValueNode_Bone::Handle()))
			return bone_node;
	return node;
}

void
CanvasTreeStore::get_value_vfunc(const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const
{
	if(column==model.value.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<synfig::ValueBase> x;
		g_value_init(x.gobj(),x.value_type());

		ValueNode::Handle value_node = value_desc.get_value_node();
		
		if(!value_desc)
		{
			x.set(ValueBase());
		}
		else
		if(value_desc.is_const())
			x.set(value_desc.get_value());
		else
		if(value_node)
		{
			Type &type(value_desc.get_value_type());
			if (type == type_bone_object)
			{
				Time time(canvas_interface()->get_time());
				Bone bone((*value_node)(time).get(Bone()));
				String display(String(bone.get_name()));
				ValueNode_Bone::ConstHandle parent(bone.get_parent());
				if (!parent->is_root())
					display += " --> " + String((*parent->get_link("name"))(time).get(String()));
				x.set(display);
			}
			else
			if (type == type_bone_weight_pair)
			{
				Time time(canvas_interface()->get_time());
				BoneWeightPair bone_weight_pair((*value_node)(time).get(BoneWeightPair()));
				x.set(bone_weight_pair.get_string());
			}
			else
			if (type == type_segment
			|| type == type_list
			|| type == type_bline_point)
			{
				x.set(value_desc.get_value_type().description.local_name);
			}
			else
			{
				x.set((*value_node)(canvas_interface()->get_time()));
			}
		}
		else
		{
			synfig::error(__FILE__":%d: Unable to figure out value",__LINE__);
			return;
		}

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_value_node.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(value_desc && value_desc.is_value_node());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_shared.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(value_desc.is_value_node() && value_desc.get_value_node() && value_desc.get_value_node()->rcount()>1);

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_exported.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(value_desc.is_value_node() && value_desc.get_value_node()->is_exported());

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_canvas.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(!value_desc && (Canvas::Handle)(*iter)[model.canvas]);

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.id.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());

		if(value_desc && value_desc.is_value_node())
			x.set(value_desc.get_value_node()->get_id());
		else if(!value_desc && Canvas::Handle((*iter)[model.canvas]))
			x.set(Canvas::Handle((*iter)[model.canvas])->get_id());
		else
			return Gtk::TreeStore::get_value_vfunc(iter,column,value);

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.is_editable.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());

		x.set(
			// Temporary fix crash when trying to edit bline point
			// https://github.com/synfig/synfig/issues/264
			// TODO: move this check into a more proper place
				value_desc.get_value_type() != type_bline_point
			&&	value_desc.get_value_type() != type_width_point
			&&	value_desc.get_value_type() != type_dash_item
			&&	(
				!value_desc.is_value_node()
			||	synfigapp::is_editable(value_desc.get_value_node())
			)
		);

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.type.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
		String stype, lname;

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());

		// Set the type
		if(!value_desc)
		{
			if((*iter)[model.is_canvas])
				x.set(_("Canvas"));
		}
		else
		{
			stype=value_desc.get_value_type().description.local_name;
			if(!value_desc.is_const())
				stype+=" (" + value_desc.get_value_node()->get_local_name() + ")";
		}
		x.set(stype.c_str());
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.label.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);

		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),x.value_type());

		// Set the type
		if(!value_desc)
		{
			Canvas::Handle canvas((*iter)[model.canvas]);
			if(canvas)
			{
				if(!canvas->get_id().empty())
					x.set(canvas->get_id());
				else if(!canvas->get_name().empty())
					x.set(canvas->get_name());
				else
					x.set(_("[Unnamed]"));
				// todo: what are the previous 6 lines for if we're going to overwrite it here?
				x.set(_("Canvas"));
			}
			return Gtk::TreeStore::get_value_vfunc(iter,column,value);
		}
		else
		{
			ValueNode::Handle value_node=value_desc.get_value_node();

			// Setup the row's label
			if(value_node->get_id().empty())
				x.set(Glib::ustring((*iter)[model.name]));
			else if(Glib::ustring((*iter)[model.name]).empty())
				x.set(value_node->get_id());
			else
				x.set(Glib::ustring((*iter)[model.name])+" ("+value_node->get_id()+')');
		}

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.icon.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
		if(!value_desc)
			return Gtk::TreeStore::get_value_vfunc(iter,column,value);

		Glib::Value<Glib::RefPtr<Gdk::Pixbuf> > x;
		g_value_init(x.gobj(),x.value_type());

		x.set(get_tree_pixbuf(value_desc.get_value_type()));

		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.interpolation_icon.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
		if(!value_desc)
			return Gtk::TreeStore::get_value_vfunc(iter,column,value);
		
		Glib::Value<Glib::RefPtr<Gdk::Pixbuf> > x;
		g_value_init(x.gobj(),x.value_type());
		
		x.set(get_interpolation_pixbuf(value_desc.get_interpolation()));
		
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
		
	}
	else
	if(column==model.is_static.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
		
		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());
		
		x.set(value_desc.get_static());
		
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
	if(column==model.interpolation_icon_visible.index())
	{
		synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
		
		Glib::Value<bool> x;
		g_value_init(x.gobj(),x.value_type());
	
		bool is_visible((!value_desc.get_static())
						&& (value_desc.get_interpolation()!=INTERPOLATION_UNDEFINED)
						&& (value_desc.get_interpolation()!=INTERPOLATION_MANUAL)
						&& (value_desc.get_interpolation()!=INTERPOLATION_NIL));
		x.set(is_visible);
		g_value_init(value.gobj(),x.value_type());
		g_value_copy(x.gobj(),value.gobj());
	}
	else
		Gtk::TreeStore::get_value_vfunc(iter,column,value);
}

bool
CanvasTreeStore::find_first_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter)
{
	iter=children().begin();
	while(iter && value_desc!=(*iter)[model.value_desc])
	{
		if(!iter->children().empty())
		{
			Gtk::TreeIter iter2(iter->children().begin());
			//! \todo confirm that the && should be done before the ||
			if((iter2 && value_desc==(*iter2)[model.value_desc]) || find_next_value_desc(value_desc, iter2))
			{
				iter=iter2;
				return true;
			}
		}
		Gtk::TreeIter iter2(++iter);
		if(!iter2)
			iter=iter->parent();
		else
			iter=iter2;
	}
	return (bool)iter && value_desc==(*iter)[model.value_desc];
}

bool
CanvasTreeStore::find_next_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter)
{
	if(!iter) return find_first_value_desc(value_desc,iter);

	if(iter) do {
		if(!iter->children().empty())
		{
			Gtk::TreeIter iter2(iter->children().begin());
			//! \todo confirm that the && should be done before the ||
			if((iter2 && value_desc==(*iter2)[model.value_desc]) || find_next_value_desc(value_desc, iter2))
			{
				iter=iter2;
				return true;
			}
		}
		Gtk::TreeIter iter2(++iter);
		if(!iter2)
		{
			iter=iter->parent();
			if(iter)++iter;
		}
		else
			iter=iter2;
	} while(iter && value_desc!=(*iter)[model.value_desc]);
	return (bool)iter && value_desc==(*iter)[model.value_desc];
}

bool
CanvasTreeStore::find_first_value_node(const synfig::ValueNode::Handle& value_node, Gtk::TreeIter& iter)
{
	// maybe replace the ValueNode_Const or ValueNode_Animated with the contained ValueNode_Bone
	// todo: do we need to do this in find_next_value_node, and find_*_value_desc too?
	synfig::ValueNode::Handle node(expandable_bone_parent(value_node));

	iter=children().begin();
	while(iter && node!=(ValueNode::Handle)(*iter)[model.value_node])
	{
		if(!iter->children().empty())
		{
			Gtk::TreeIter iter2(iter->children().begin());
			//! \todo confirm that the && should be done before the ||
			if((iter2 && node==(ValueNode::Handle)(*iter2)[model.value_node]) || find_next_value_node(node, iter2))
			{
				iter=iter2;
				return true;
			}
		}
		Gtk::TreeIter iter2(++iter);
		if(!iter2)
			iter=iter->parent();
		else
			iter=iter2;
	}
	return (bool)iter && node==(ValueNode::Handle)(*iter)[model.value_node];
}

bool
CanvasTreeStore::find_next_value_node(const synfig::ValueNode::Handle& value_node, Gtk::TreeIter& iter)
{
	if(!iter) return find_first_value_node(value_node,iter);

	if(iter) do {
		if(!iter->children().empty())
		{
			Gtk::TreeIter iter2(iter->children().begin());
			//! \todo confirm that the && should be done before the ||
			if((iter2 && value_node==(ValueNode::Handle)(*iter2)[model.value_node]) || find_next_value_node(value_node, iter2))
			{
				iter=iter2;
				return true;
			}
		}
		Gtk::TreeIter iter2(++iter);
		if(!iter2)
		{
			iter=iter->parent();
			if(iter)++iter;
		}
		else
			iter=iter2;
	} while(iter && value_node!=(ValueNode::Handle)(*iter)[model.value_node]);
	return (bool)iter && value_node==(ValueNode::Handle)(*iter)[model.value_node];
}

void
CanvasTreeStore::set_row(Gtk::TreeRow row,synfigapp::ValueDesc value_desc, bool do_children)
{
	Gtk::TreeModel::Children children = row.children();
	while(!children.empty() && erase(children.begin()))
		;

	row[model.value_desc]=value_desc;
	try
	{
		//row[model.icon] = get_tree_pixbuf(value_desc.get_value_type());

		if(value_desc.is_value_node())
		{
			ValueNode::Handle value_node=value_desc.get_value_node();

			// todo: if the parent is animated and expanded, and we drag the time slider so that it changes,
			// it's not updated.  it still shows the previous bone valuenode.

			// maybe replace the ValueNode_Const or ValueNode_Animated with the contained ValueNode_Bone
			value_node = expandable_bone_parent(value_node);

			assert(value_node);

			row[model.value_node] = value_node;
			//row[model.is_canvas] = false;
			//row[model.is_value_node] = true;
			//row[model.is_editable] = synfigapp::is_editable(value_node);
			//row[model.id]=value_node->get_id();

			// Set the canvas
			if(value_desc.parent_is_canvas())
				row[model.canvas]=value_desc.get_canvas();
			else
				row[model.canvas]=canvas_interface()->get_canvas();

			LinkableValueNode::Handle linkable;
			// printf("%s:%d value_node = %s\n", __FILE__, __LINE__, value_node->get_description().c_str());
			linkable=LinkableValueNode::Handle::cast_dynamic(value_node);

			// printf("linkable: %d; do_children: %d\n", bool(linkable), bool(do_children));
			if(linkable && do_children)
			{
				row[model.link_count] = linkable->link_count();
				LinkableValueNode::Vocab vocab(linkable->get_children_vocab());
				LinkableValueNode::Vocab::iterator iter(vocab.begin());
				for(int i=0;i<linkable->link_count();i++, iter++)
				{
					if(iter->get_hidden())
						continue;
					Gtk::TreeRow child_row=*(append(row.children()));
					child_row[model.link_id] = i;
					child_row[model.canvas] = static_cast<Canvas::Handle>(row[model.canvas]);
					child_row[model.name] = linkable->link_local_name(i);
					child_row[model.tooltip] = iter->get_description();
					child_row[model.child_param_desc] = *iter;
					set_row(child_row,synfigapp::ValueDesc(linkable,i));
				}
			}
			return;
		}
		else
		{
			//row[model.is_value_node] = false;
			//row[model.is_editable] = true;
			//row[model.label] = Glib::ustring(row[model.name]);
			return;
		}
	}
	catch(synfig::Exception::IDNotFound x)
	{
		synfig::error(__FILE__":%d: IDNotFound thrown",__LINE__);
		erase(row);
		return;
	}

	// We should never get to this point
	assert(0);
}

void
CanvasTreeStore::refresh_row(Gtk::TreeModel::Row &row, bool do_children)
{
	synfigapp::ValueDesc value_desc=row[model.value_desc];

	if(value_desc)
	{
		if((bool)row[model.is_value_node] != value_desc.is_value_node() ||
			(!bool(row[model.is_value_node]) && row[model.link_count]!=0))
		{
			set_row(row,value_desc,do_children);
			return;
		}

		if(row[model.is_value_node])
		{
			ValueNode::Handle value_node(value_desc.get_value_node());

			if(ValueNode::Handle(row[model.value_node])!=value_node)
			{
				rebuild_row(row,do_children);
				return;
			}

			//row[model.id]=value_node->get_id();

			// Setup the row's label
			/*
			if(value_node->get_id().empty())
				row[model.label] = Glib::ustring(row[model.name]);
			else if(Glib::ustring(row[model.name]).empty())
				row[model.label] = value_node->get_id();
			else
				row[model.label] = Glib::ustring(row[model.name])+" ("+value_node->get_id()+')';
			*/

			LinkableValueNode::Handle linkable;
			linkable=LinkableValueNode::Handle::cast_dynamic(value_node);
			if(do_children && linkable && ((int)row[model.link_count] != linkable->link_count()))
			{
	//			Gtk::TreeModel::Children children = row.children();
	//			while(!children.empty() && erase(children.begin()));

				set_row(row,value_desc);
				return;
			}
		}
		else
		{
			//row[model.label] = Glib::ustring(row[model.name]);
			//row[model.is_value_node] = false;
			//row[model.is_editable] = true;
		}
	}
	if(!do_children)
		return;

	Gtk::TreeModel::Children children = row.children();
	Gtk::TreeModel::Children::iterator iter;

	if(!children.empty())
	for(iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row=*iter;
		refresh_row(row);
	}
}

void
CanvasTreeStore::rebuild_row(Gtk::TreeModel::Row &row, bool do_children)
{
	synfigapp::ValueDesc value_desc=(synfigapp::ValueDesc)row[model.value_desc];

	if(value_desc && value_desc.get_value_node())
	{
		ValueNode::Handle value_node;
		value_node=value_desc.get_value_node();

		assert(value_node);if(!value_node)return;

		if(value_node && value_node!=(ValueNode::Handle)row[model.value_node])
		{
//			Gtk::TreeModel::Children children = row.children();
//			while(!children.empty() && erase(children.begin()));

			set_row(row,value_desc,do_children);
			return;
		}

		LinkableValueNode::Handle linkable;
		linkable=LinkableValueNode::Handle::cast_dynamic(value_node);

		if( do_children && linkable && (int)row[model.link_count] != linkable->link_count())
		{
//			Gtk::TreeModel::Children children = row.children();
//			while(!children.empty() && erase(children.begin()));

			set_row(row,value_desc);
			return;
		}

		//if(!value_node)
		//	value_node=row[model.value_node];

		row[model.id]=value_node->get_id();

		// Setup the row's label
		if(value_node->get_id().empty())
			row[model.label] = Glib::ustring(row[model.name]);
		else if(Glib::ustring(row[model.name]).empty())
			row[model.label] = value_node->get_id();
		else
			row[model.label] = Glib::ustring(row[model.name])+" ("+value_node->get_id()+')';
	}
	else
	{
		row[model.label] = Glib::ustring(row[model.name]);
		row[model.is_value_node] = false;
		row[model.is_editable] = true;
		Gtk::TreeModel::Children children = row.children();
		while(!children.empty() && erase(children.begin()))
			;
	}
	if(!do_children)
		return;

	Gtk::TreeModel::Children children = row.children();
	Gtk::TreeModel::Children::iterator iter;
	if(!children.empty())
	for(iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row=*iter;
		rebuild_row(row);
	}
}

CellRenderer_ValueBase*
CanvasTreeStore::add_cell_renderer_value(Gtk::TreeView::Column* column)
{
	const CanvasTreeStore::Model model;

	CellRenderer_ValueBase* ret;

	ret=Gtk::manage( new CellRenderer_ValueBase() );

	column->pack_start(*ret,true);
	column->add_attribute(ret->property_value(), model.value);
	column->add_attribute(ret->property_editable(), model.is_editable);
	column->add_attribute(ret->property_canvas(), model.canvas);

	return ret;
}

CellRenderer_TimeTrack*
CanvasTreeStore::add_cell_renderer_value_node(Gtk::TreeView::Column* column)
{
	const CanvasTreeStore::Model model;

	CellRenderer_TimeTrack* ret;

	ret = Gtk::manage( new CellRenderer_TimeTrack() );

	column->pack_start(*ret,true);
	//column->add_attribute(ret->property_visible(), model.is_value_node);
	column->add_attribute(ret->property_value_desc(), model.value_desc);
	column->add_attribute(ret->property_canvas(), model.canvas);


	return ret;
}
