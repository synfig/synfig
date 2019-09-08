/* === S Y N F I G ========================================================= */
/*!	\file duckmatic.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009, 2011 Nikita Kitaev
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2015 Blanchi Jérôme
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
#include <fstream>
#include <iostream>
#include <algorithm>

#include <ETL/hermite>

#include <synfig/general.h>

#include "duckmatic.h"
#include "ducktransform_scale.h"
#include "ducktransform_translate.h"
#include "ducktransform_rotate.h"
#include <synfigapp/value_desc.h>
#include <synfigapp/canvasinterface.h>
#include <synfig/paramdesc.h>
#include <synfig/valuenodes/valuenode_timedswap.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_range.h>
#include <synfig/valuenodes/valuenode_scale.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_blinecalctangent.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_blinecalcwidth.h>
#include <synfig/valuenodes/valuenode_staticlist.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_boneinfluence.h>
#include <synfig/valuenodes/valuenode_boneweightpair.h>
#include <synfig/segment.h>
#include <synfig/pair.h>

#include <synfig/curve_helper.h>

#include <synfig/context.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/layers/layer_filtergroup.h>

#include "ducktransform_matrix.h"
#include "ducktransform_rotate.h"
#include "ducktransform_translate.h"
#include "ducktransform_scale.h"
#include "ducktransform_origin.h"
#include "canvasview.h"

#include "onemoment.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* 0.33333333333333333 makes for nice short tangent handles,
   1.0 makes them draw as their real length */
#define TANGENT_HANDLE_SCALE 0.33333333333333333

/* leave this alone or the bezier won't lie on top of the bline */
#define TANGENT_BEZIER_SCALE 0.33333333333333333

/* === I N L I N E ======================================================= */
synfig::GUID calc_duck_guid(const synfigapp::ValueDesc& value_desc, const synfig::TransformStack& transform_stack);
void set_duck_value_desc(Duck& duck, const synfigapp::ValueDesc& value_desc, const synfig::TransformStack& transform_stack);
void set_duck_value_desc(Duck& duck, const synfigapp::ValueDesc& value_desc, const synfig::String& sub_name, const synfig::TransformStack& transform_stack);

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Duckmatic::Duckmatic(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface):
	canvas_interface(canvas_interface),
	type_mask(Duck::TYPE_ALL-Duck::TYPE_WIDTH-Duck::TYPE_BONE_RECURSIVE-Duck::TYPE_WIDTHPOINT_POSITION),
	type_mask_state(Duck::TYPE_NONE),
	alternative_mode_(false),
	lock_animation_mode_(false),
	grid_snap(false),
	guide_snap(false),
	grid_size(1.0/4.0,1.0/4.0),
	grid_color(synfig::Color(159.0/255.0,159.0/255.0,159.0/255.0)),
	guides_color(synfig::Color(111.0/255.0,111.0/255.0,1.0)),
	zoom(1.0),
	prev_zoom(1.0),
	show_persistent_strokes(true),
	axis_lock(false),
	drag_offset_(0, 0)
{
	clear_duck_dragger();
	clear_bezier_dragger();
}

Duckmatic::~Duckmatic()
{
	clear_ducks();

	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("Duckmatic::~Duckmatic(): Deleted");
}

void
Duckmatic::clear_ducks()
{
	for(;!duck_changed_connections.empty();duck_changed_connections.pop_back())duck_changed_connections.back().disconnect();

	duck_data_share_map.clear();
	duck_map.clear();

	//duck_list_.clear();
	bezier_list_.clear();
	stroke_list_.clear();

	if(show_persistent_strokes)
		stroke_list_=persistent_stroke_list_;
}


/*
-- ** -- D U C K  M A N I P U L A T I O N  M E T H O D S-----------------------
*/

bool
Duckmatic::duck_is_selected(const etl::handle<Duck> &duck)const
{
    return duck && selected_ducks.count(duck->get_guid());
}

void
Duckmatic::clear_selected_ducks()
{
	selected_ducks.clear();
	signal_duck_selection_changed_();
}

etl::handle<Duckmatic::Duck>
Duckmatic::get_selected_duck()const
{
	if(selected_ducks.empty() || duck_map.empty())
		return 0;
	return duck_map.find(*selected_ducks.begin())->second;
}

etl::handle<Duckmatic::Bezier>
Duckmatic::get_selected_bezier()const
{
	return selected_bezier;
}

void
Duckmatic::refresh_selected_ducks()
{
/*
	std::set<etl::handle<Duck> >::iterator iter;
	std::set<etl::handle<Duck> > new_set;
	if(duck_list().empty())
	{
		selected_duck_list.clear();
		signal_duck_selection_changed_();
		return;
	}

	for(iter=selected_duck_list.begin();iter!=selected_duck_list.end();++iter)
	{
		etl::handle<Duck> similar(find_similar_duck(*iter));
		if(similar)
		{
			new_set.insert(similar);
		}
	}
	selected_duck_list=new_set;
*/
	GUIDSet old_set(selected_ducks);
	GUIDSet::const_iterator iter;

	for(iter=old_set.begin();iter!=old_set.end();++iter)
	{
		if(duck_map.count(*iter)==0)
			selected_ducks.erase(*iter);
	}

	signal_duck_selection_changed_();
}

bool
Duckmatic::is_duck_group_selectable(const etl::handle<Duck>& x)const
{
	const Type type(get_type_mask());

	if (((x->get_type() && (!(type & x->get_type()))) ||
		 !x->get_editable()))
		return false;

	synfigapp::ValueDesc value_desc(x->get_value_desc());
	if(value_desc.parent_is_layer() && (type & Duck::TYPE_POSITION))
	{
		Layer::Handle layer(value_desc.get_layer());
		String layer_name(layer->get_name());

		if (layer_name == "outline" || layer_name == "region" || layer_name == "plant" ||
			layer_name == "polygon" || layer_name == "curve_gradient" || layer_name == "advanced_outline")
			return false;

		if(etl::handle<Layer_PasteCanvas>::cast_dynamic(layer) &&
		   !layer->get_param("children_lock").get(bool()))
			return false;
	}
	else if (value_desc.parent_is_value_node())
	{
		if (ValueNode_BLineCalcVertex::Handle::cast_dynamic(value_desc.get_value_node()))
			return false;
		if (value_desc.parent_is_linkable_value_node())
		{
			ValueNode::Handle parent_value_node(value_desc.get_parent_value_node());
			if (ValueNode_Composite::Handle composite = ValueNode_Composite::Handle::cast_dynamic(parent_value_node))
			{
				if (parent_value_node->get_type() == type_bline_point &&
					ValueNode_BLineCalcVertex::Handle::cast_dynamic(
							composite->get_link("point")))
					return false;
				// widths ducks of the widthpoints
				// Do not avoid selection of the width ducks from widthpoints
				//if (parent_value_node->get_type() == type_width_point)
				//	return false;
			}
			else if (ValueNode_BLine::Handle::cast_dynamic(parent_value_node))
			{
				ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(
														  value_desc.get_value_node()));
				if (composite &&
					ValueNode_BLineCalcVertex::Handle::cast_dynamic(composite->get_link("point")))
					return false;
			}
			// position ducks of the widthpoints
			else if (ValueNode_WPList::Handle::cast_dynamic(parent_value_node))
				return false;

		}
	}
	return true;
}

void
Duckmatic::select_all_ducks()
{
	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
		if(is_duck_group_selectable(iter->second))
			select_duck(iter->second);
		else
			unselect_duck(iter->second);
}

void
Duckmatic::unselect_all_ducks()
{
	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
		unselect_duck(iter->second);
}

void
Duckmatic::toggle_select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br)
{
	Vector vmin, vmax;
	vmin[0]=std::min(tl[0],br[0]);
	vmin[1]=std::min(tl[1],br[1]);
	vmax[0]=std::max(tl[0],br[0]);
	vmax[1]=std::max(tl[1],br[1]);

	{
	    DuckMap::const_iterator iter;
        for(iter=duck_map.begin();iter!=duck_map.end();++iter)
        {
            Point p(iter->second->get_trans_point());
            if(p[0]<=vmax[0] && p[0]>=vmin[0] && p[1]<=vmax[1] && p[1]>=vmin[1] &&
               is_duck_group_selectable(iter->second))
                toggle_select_duck(iter->second);
        }
	}
}

void
Duckmatic::select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br)
{
	Vector vmin, vmax;
	vmin[0]=std::min(tl[0],br[0]);
	vmin[1]=std::min(tl[1],br[1]);
	vmax[0]=std::max(tl[0],br[0]);
	vmax[1]=std::max(tl[1],br[1]);

//	Type type(get_type_mask());

	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
	{
		Point p(iter->second->get_trans_point());
		if(p[0]<=vmax[0] && p[0]>=vmin[0] && p[1]<=vmax[1] && p[1]>=vmin[1])
		{
			if(is_duck_group_selectable(iter->second))
				select_duck(iter->second);
		}
	}
}

int
Duckmatic::count_selected_ducks()const
{
	return selected_ducks.size();
}

void
Duckmatic::select_duck(const etl::handle<Duck> &duck)
{
	if(duck)
	{
		selected_ducks.insert(duck->get_guid());
		signal_duck_selection_changed_();

		if(selected_ducks.size() == 1)
		{
		    signal_duck_selection_single_(duck);
		}
	}
}

DuckList
Duckmatic::get_selected_ducks()const
{
	DuckList ret;
	GUIDSet::const_iterator iter;
	const Type type(get_type_mask());

	for(iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		const DuckMap::const_iterator d_iter(duck_map.find(*iter));

		if(d_iter==duck_map.end())
			continue;

		if(( d_iter->second->get_type() && (!(type & d_iter->second->get_type())) ) )
			continue;

		ret.push_back(d_iter->second);
	}
	return ret;
}


DuckList
Duckmatic::get_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br)const
{
    Vector vmin, vmax;
    vmin[0]=std::min(tl[0],br[0]);
    vmin[1]=std::min(tl[1],br[1]);
    vmax[0]=std::max(tl[0],br[0]);
    vmax[1]=std::max(tl[1],br[1]);

    DuckList ret;

//  Type type(get_type_mask());

    DuckMap::const_iterator iter;
    for(iter=duck_map.begin();iter!=duck_map.end();++iter)
    {
        Point p(iter->second->get_trans_point());
        if(p[0]<=vmax[0] && p[0]>=vmin[0] && p[1]<=vmax[1] && p[1]>=vmin[1])
        {
          //  if(is_duck_group_selectable(iter->second))
            ret.push_back(iter->second);
        }
    }
    return ret;
}

DuckList
Duckmatic::get_duck_list()const
{
	DuckList ret;
	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter) if (iter->second->get_type()&Duck::TYPE_POSITION) ret.push_back(iter->second);
	for(iter=duck_map.begin();iter!=duck_map.end();++iter) if (iter->second->get_type()&Duck::TYPE_VERTEX  ) ret.push_back(iter->second);
	for(iter=duck_map.begin();iter!=duck_map.end();++iter) if (iter->second->get_type()&Duck::TYPE_TANGENT ) ret.push_back(iter->second);
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
		if (!(iter->second->get_type()&Duck::TYPE_POSITION) &&
			!(iter->second->get_type()&Duck::TYPE_VERTEX) &&
			!(iter->second->get_type()&Duck::TYPE_TANGENT))
			ret.push_back(iter->second);
	return ret;
}

void
Duckmatic::unselect_duck(const etl::handle<Duck> &duck)
{
	if(duck && selected_ducks.count(duck->get_guid()))
	{
		selected_ducks.erase(duck->get_guid());
		signal_duck_selection_changed_();
	}
}

void
Duckmatic::toggle_select_duck(const etl::handle<Duck> &duck)
{
	if(duck_is_selected(duck))
		unselect_duck(duck);
	else
		select_duck(duck);
}

void
Duckmatic::translate_selected_ducks(const synfig::Vector& vector)
{
	if(duck_dragger_)
		duck_dragger_->duck_drag(this,vector);
}

void
Duckmatic::start_duck_drag(const synfig::Vector& offset)
{
	if(duck_dragger_)
		duck_dragger_->begin_duck_drag(this,offset);

	//drag_offset_=offset;
	drag_offset_=find_duck(offset)->get_trans_point();
}

bool
Duckmatic::end_duck_drag()
{
    if(duck_dragger_)
        return duck_dragger_->end_duck_drag(this);
    return false;
}

void
Duckmatic::update_ducks()
{
	Time time(get_time());
	DuckList duck_list(get_duck_list());
	const DuckList selected_ducks(get_selected_ducks());
	DuckList::const_iterator selected_iter;
	if(get_selected_bezier())
	{
		etl::handle<Duck> c1(get_selected_bezier()->c1);
		etl::handle<Duck> c2(get_selected_bezier()->c2);
		if(c1->get_value_desc().parent_is_linkable_value_node())
		{
			ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(c1->get_value_desc().get_parent_value_node()));
			LinkableValueNode::Handle duck_value_node(LinkableValueNode::Handle::cast_dynamic(c1->get_value_desc().get_value_node()));
			// it belongs to a composite and it is a BLinePoint
			if(composite && composite->get_type() == type_bline_point && duck_value_node)
			{
				int index(c1->get_value_desc().get_index());
				etl::handle<Duck> origin_duck=c1->get_origin_duck();
				// Search all the rest of ducks
				DuckList::iterator iter;
				for (iter=duck_list.begin(); iter!=duck_list.end(); iter++)
					// if the other duck has the same origin and it is tangent type
					if ( (*iter)->get_origin_duck()==origin_duck && (*iter)->get_type() == Duck::TYPE_TANGENT)
					{
						ValueNode_Composite::Handle iter_composite;
						iter_composite=ValueNode_Composite::Handle::cast_dynamic((*iter)->get_value_desc().get_parent_value_node());
						// and their parent valuenode are the same
						if(iter_composite.get() == composite.get())
						{
							BLinePoint bp=(*composite)(time).get(BLinePoint());
							int t1_index=composite->get_link_index_from_name("t1");
							int t2_index=composite->get_link_index_from_name("t2");
							if(index==t1_index && (*iter)->get_value_desc().get_index()!=t1_index)
							{
								bp.set_tangent1(c1->get_point());
								Vector t2(bp.get_tangent2());
								(*iter)->set_point(Point(t2));
							}
							else if(index==t2_index && (*iter)->get_value_desc().get_index()!=t2_index)
							{
								// Create a new BLinePoint
								BLinePoint nbp;
								// Terporary set the flags for the new BLinePoint to all split
								nbp.set_split_tangent_both(true);
								// Now we can set the tangents. Tangent2 won't be modified by tangent1
								nbp.set_tangent1(c1->get_point());
								nbp.set_tangent2(bp.get_tangent1());
								// Now update the flags
								nbp.set_split_tangent_radius(bp.get_split_tangent_radius());
								nbp.set_split_tangent_angle(bp.get_split_tangent_angle());
								// Now retrieve the updated tangent2 (which will be stored as t1, see below)
								Vector t1(nbp.get_tangent2());
								(*iter)->set_point(Point(t1));
							}
						}
					}
			}
		}
		if(c2->get_value_desc().parent_is_linkable_value_node())
		{
			ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(c2->get_value_desc().get_parent_value_node()));
			LinkableValueNode::Handle duck_value_node(LinkableValueNode::Handle::cast_dynamic(c2->get_value_desc().get_value_node()));
			// it belongs to a composite and it is a BLinePoint
			if(composite && composite->get_type() == type_bline_point && duck_value_node)
			{
				int index(c2->get_value_desc().get_index());
				etl::handle<Duck> origin_duck=c2->get_origin_duck();
				// Search all the rest of ducks
				DuckList::iterator iter;
				for (iter=duck_list.begin(); iter!=duck_list.end(); iter++)
					// if the other duck has the same origin and it is tangent type
					if ( (*iter)->get_origin_duck()==origin_duck && (*iter)->get_type() == Duck::TYPE_TANGENT)
					{
						ValueNode_Composite::Handle iter_composite;
						iter_composite=ValueNode_Composite::Handle::cast_dynamic((*iter)->get_value_desc().get_parent_value_node());
						// and their parent valuenode are the same
						if(iter_composite.get() == composite.get())
						{
							BLinePoint bp=(*composite)(time).get(BLinePoint());
							int t1_index=composite->get_link_index_from_name("t1");
							int t2_index=composite->get_link_index_from_name("t2");
							if(index==t1_index && (*iter)->get_value_desc().get_index()!=t1_index)
							{
								bp.set_tangent1(c2->get_point());
								Vector t2(bp.get_tangent2());
								(*iter)->set_point(Point(t2));
							}
							else if(index==t2_index && (*iter)->get_value_desc().get_index()!=t2_index)
							{
								// Create a new BLinePoint
								BLinePoint nbp;
								// Terporary set the flags for the new BLinePoint to all split
								nbp.set_split_tangent_both(true);
								// Now we can set the tangents. Tangent2 won't be modified by tangent1
								nbp.set_tangent1(c2->get_point());
								nbp.set_tangent2(bp.get_tangent1());
								// Now update the flags
								nbp.set_split_tangent_radius(bp.get_split_tangent_radius());
								nbp.set_split_tangent_angle(bp.get_split_tangent_angle());
								// Now retrieve the updated tangent2 (which will be stored as t1, see below)
								Vector t1(nbp.get_tangent2());
								(*iter)->set_point(Point(t1));
							}
						}
					}
			}
		}
	}
	for (selected_iter=selected_ducks.begin(); selected_iter!=selected_ducks.end(); ++selected_iter)
	{
		etl::handle<Duck> duck(*selected_iter);
		if(!duck)
			return;
		if (duck->get_type() == Duck::TYPE_VERTEX || duck->get_type() == Duck::TYPE_POSITION)
		{
			ValueNode_BLineCalcVertex::Handle bline_vertex =
				ValueNode_BLineCalcVertex::Handle::cast_dynamic(duck->get_value_desc().get_value_node());
			if (!bline_vertex && duck->get_value_desc().parent_is_value_desc()) {
				ValueNode_Composite::Handle composite =
					ValueNode_Composite::Handle::cast_dynamic(duck->get_value_desc().get_value_node());
				if (composite)
					bline_vertex =
						ValueNode_BLineCalcVertex::Handle::cast_dynamic(
							composite->get_link(
								duck->get_value_desc().get_sub_name() ));
			}

			if (bline_vertex)
			{
				synfig::Real radius = 0.0;
				synfig::Point point(0.0, 0.0);
				ValueNode_BLine::Handle bline(ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link("bline")));
				Real amount = synfig::find_closest_point((*bline)(time), duck->get_point(), radius, bline->get_loop(), &point);
				bool homogeneous((*(bline_vertex->get_link("homogeneous")))(time).get(bool()));
				if(homogeneous)
					amount=std_to_hom((*bline)(time), amount, ((*(bline_vertex->get_link("loop")))(time).get(bool())), bline->get_loop() );
				ValueNode::Handle vertex_amount_value_node(bline_vertex->get_link("amount"));
				duck->set_point(point);

				DuckList::iterator iter;
				for (iter=duck_list.begin(); iter!=duck_list.end(); iter++)
				{
					if ( (*iter)->get_origin_duck()==duck /*&& !duck_is_selected(*iter)*/ )
					{
						ValueNode::Handle duck_value_node = (*iter)->get_value_desc().get_value_node();
						if (duck_value_node)
						{
							ValueNode_Composite::Handle duck_value_node_composite = ValueNode_Composite::Handle::cast_dynamic(duck_value_node);
							ValueNode::Handle sub_duck_value_node =
									duck_value_node_composite && (*iter)->get_value_desc().parent_is_value_desc()
								  ? ValueNode::Handle(duck_value_node_composite->get_link( (*iter)->get_value_desc().get_sub_name() ))
								  : duck_value_node;
							if (sub_duck_value_node)
							{
								if ( ValueNode_BLineCalcTangent::Handle bline_tangent =
										ValueNode_BLineCalcTangent::Handle::cast_dynamic(sub_duck_value_node) )
								{
									if (bline_tangent->get_link("amount") == vertex_amount_value_node)
									{
										synfig::Type &type(bline_tangent->get_type());
										if (type == type_angle)
										{
											Angle angle((*bline_tangent)(time, amount).get(Angle()));
											(*iter)->set_point(Point(Angle::cos(angle).get(), Angle::sin(angle).get()));
											(*iter)->set_rotations(Angle::deg(0)); //hack: rotations are a relative value
										}
										else
										if (type == type_real)
											(*iter)->set_point(Point((*bline_tangent)(time, amount).get(Real()), 0));
										else
										if (type == type_vector)
											(*iter)->set_point((*bline_tangent)(time, amount).get(Vector()));
									}
								} else
								if ( ValueNode_BLineCalcWidth::Handle bline_width =
										ValueNode_BLineCalcWidth::Handle::cast_dynamic(sub_duck_value_node) )
								{
									if (bline_width->get_link("amount") == vertex_amount_value_node)
										(*iter)->set_point(Point((*bline_width)(time, amount).get(Real()), 0));
								}
							}
						}
					}
				}
			}
		}
		// We are moving a tangent handle
		else
		if(duck->get_type() == Duck::TYPE_TANGENT)
		{
			if (duck->get_value_desc().parent_is_value_desc()
			 && duck->get_value_desc().is_value_node() )
			{
				ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(duck->get_value_desc().get_value_node()));
				if (composite && composite->get_type() == type_bline_point)
				{
					ValueNode::Handle duck_value_node(composite->get_link(duck->get_value_desc().get_sub_name()));
					// it belongs to a composite and it is a BLinePoint
					if (duck_value_node)
					{
						int index(duck->get_value_desc().get_index());
						etl::handle<Duck> origin_duck=duck->get_origin_duck();
						// Search all the rest of ducks
						DuckList::iterator iter;
						for (iter=duck_list.begin(); iter!=duck_list.end(); iter++)
						{
							// if the other duck has the same origin and it is tangent type
							if ( (*iter)->get_origin_duck()==origin_duck && (*iter)->get_type() == Duck::TYPE_TANGENT)
							{
								ValueNode_Composite::Handle iter_composite;
								iter_composite=ValueNode_Composite::Handle::cast_dynamic((*iter)->get_value_desc().get_parent_value_node());
								// and their parent valuenode are the same
								if(iter_composite.get() == composite.get())
								{
									// Check if the other tangent is also selected, in that case
									// it is going to be moved itself so don't update it.
									bool selected=false;
									DuckList::const_iterator iter2;
									for(iter2=selected_ducks.begin(); iter2!=selected_ducks.end(); ++iter2)
										if(*iter == *iter2)
											selected=true;
									if(!selected)
									{
										BLinePoint bp=(*composite)(time).get(BLinePoint());
										int t1_index=composite->get_link_index_from_name("t1");
										int t2_index=composite->get_link_index_from_name("t2");
										if(index==t1_index && (*iter)->get_value_desc().get_index()!=t1_index)
										{
											bp.set_tangent1(duck->get_point());
											Vector t2(bp.get_tangent2());
											(*iter)->set_point(Point(t2));
										}
										else if(index==t2_index && (*iter)->get_value_desc().get_index()!=t2_index)
										{
											// Create a new BLinePoint
											BLinePoint nbp;
											// Terporary set the flags for the new BLinePoint to all split
											nbp.set_split_tangent_both(true);
											// Now we can set the tangents. Tangent2 won't be modified by tangent1
											nbp.set_tangent1(duck->get_point());
											nbp.set_tangent2(bp.get_tangent1());
											// Now update the flags
											nbp.set_split_tangent_radius(bp.get_split_tangent_radius());
											nbp.set_split_tangent_angle(bp.get_split_tangent_angle());
											// Now retrieve the updated tangent2 (which will be stored as t1, see below)
											Vector t1(nbp.get_tangent2());
											(*iter)->set_point(Point(t1));
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void
Duckmatic::start_bezier_drag(const synfig::Vector& offset, float bezier_click_pos)
{
	if(bezier_dragger_)
		bezier_dragger_->begin_bezier_drag(this,offset,bezier_click_pos);
}

void
Duckmatic::translate_selected_bezier(const synfig::Vector& vector)
{
	if(bezier_dragger_)
		bezier_dragger_->bezier_drag(this,vector);
}

bool
Duckmatic::end_bezier_drag()
{
	if(bezier_dragger_)
		return bezier_dragger_->end_bezier_drag(this);
	return false;
}

/*
-- ** -- grid and guide M E T H O D S----------------------------
*/

void
Duckmatic::set_grid_size(const synfig::Vector &s)
{
    if(grid_size!=s)
    {
        grid_size=s;
        signal_grid_changed();
    }
}

void
Duckmatic::set_grid_color(const synfig::Color &c)
{
    if(grid_color!=c)
    {
        grid_color=c;
        signal_grid_changed();
    }
}

void
Duckmatic::set_grid_snap(bool x)
{
    if(grid_snap!=x)
    {
        grid_snap=x;
        signal_grid_changed();
    }
}

void
Duckmatic::set_guide_snap(bool x)
{
    if(guide_snap!=x)
    {
        guide_snap=x;
        signal_grid_changed();
    }
}

void
Duckmatic::set_guides_color(const synfig::Color &c)
{
    if(guides_color!=c)
    {
        guides_color=c;
        signal_grid_changed();
    }
}

Duckmatic::GuideList::iterator
Duckmatic::find_guide_x(synfig::Point pos, float radius)
{
    GuideList::iterator iter,best(guide_list_x_.end());
    float dist(radius);
    for(iter=guide_list_x_.begin();iter!=guide_list_x_.end();++iter)
    {
        float amount(abs(*iter-pos[0]));
        if(amount<dist)
        {
            dist=amount;
            best=iter;
        }
    }
    return best;
}

Duckmatic::GuideList::iterator
Duckmatic::find_guide_y(synfig::Point pos, float radius)
{
    GuideList::iterator iter,best(guide_list_y_.end());
    float dist(radius);
    for(iter=guide_list_y_.begin();iter!=guide_list_y_.end();++iter)
    {
        float amount(abs(*iter-pos[1]));
        if(amount<=dist)
        {
            dist=amount;
            best=iter;
        }
    }
    return best;
}

Point
Duckmatic::snap_point_to_grid(const synfig::Point& x)const
{
	Point ret(x);
	float radius(0.1/zoom);

	GuideList::const_iterator guide_x,guide_y;
	bool has_guide_x(false), has_guide_y(false);

	guide_x=find_guide_x(ret,radius);
	if(guide_x!=guide_list_x_.end())
		has_guide_x=true;

	guide_y=find_guide_y(ret,radius);
	if(guide_y!=guide_list_y_.end())
		has_guide_y=true;

	if(get_grid_snap())
	{
		Point snap(
			floor(ret[0]/get_grid_size()[0]+0.5)*get_grid_size()[0],
			floor(ret[1]/get_grid_size()[1]+0.5)*get_grid_size()[1]);

		if(abs(snap[0]-ret[0])<=radius && (!has_guide_x || abs(snap[0]-ret[0])<=abs(*guide_x-ret[0])))
			ret[0]=snap[0],has_guide_x=false;
		if(abs(snap[1]-ret[1])<=radius && (!has_guide_y || abs(snap[1]-ret[1])<=abs(*guide_y-ret[1])))
			ret[1]=snap[1],has_guide_y=false;
	}

	if(guide_snap)
	{
		if(has_guide_x)
			ret[0]=*guide_x;
		if(has_guide_y)
			ret[1]=*guide_y;
	}

	if(axis_lock)
	{
		ret-=drag_offset_;
		if(abs(ret[0])<abs(ret[1]))
			ret[0]=0;
		else
			ret[1]=0;
		ret+=drag_offset_;
	}

	return ret;
}

/*
-- ** -- S I G N A L S  M E T H O D S----------------------------
*/
void
Duckmatic::signal_user_click_selected_ducks(int button)
{
    const DuckList ducks(get_selected_ducks());
    DuckList::const_iterator iter;

    for(iter=ducks.begin();iter!=ducks.end();++iter)
    {
        (*iter)->signal_user_click(button)();
    }
}

void
Duckmatic::signal_edited_duck(const etl::handle<Duck> &duck, bool moving)
{
    if (moving && !duck->get_edit_immediatelly()) return;

    if (duck->get_type() == Duck::TYPE_ANGLE)
    {
        if(!duck->signal_edited()(*duck))
        {
            throw String("Bad edit");
        }
    }
    else if (App::restrict_radius_ducks &&
             duck->is_radius())
    {
        Point point(duck->get_point());
        bool changed = false;

        if (point[0] < 0)
        {
            point[0] = 0;
            changed = true;
        }
        if (point[1] < 0)
        {
            point[1] = 0;
            changed = true;
        }

        if (changed) duck->set_point(point);

        if(!duck->signal_edited()(*duck))
        {
            throw String("Bad edit");
        }
    }
    else
    {
        if(!duck->signal_edited()(*duck))
        {
            throw String("Bad edit");
        }
    }
}


void
Duckmatic::signal_edited_selected_ducks(bool moving)
{
    const DuckList ducks(get_selected_ducks());
    DuckList::const_iterator iter;

    synfig::GUIDSet old_set(selected_ducks);

    // If we have more than 20 things to move, then display
    // something to explain that it may take a moment
    //smart_ptr<OneMoment> wait; if(ducks.size()>20)wait.spawn();
    for(iter=ducks.begin();iter!=ducks.end();++iter)
    {
        try
        {
            if (!moving || (*iter)->get_edit_immediatelly())
                signal_edited_duck(*iter);
        }
        catch (String)
        {
            selected_ducks=old_set;
            // signals must not throw exceptions!!!
            //throw;
        }
    }
    selected_ducks=old_set;
}

bool
Duckmatic::on_duck_changed(const studio::Duck &duck,const synfigapp::ValueDesc& value_desc)
{
    bool lock_animation = get_lock_animation_mode();
    synfig::Point value=duck.get_point();
    synfig::Type &type(value_desc.get_value_type());
    if (type == type_real)
    {
        if (value_desc.parent_is_value_node())
        {
            etl::handle<ValueNode_Bone> bone_node =
                etl::handle<ValueNode_Bone>::cast_dynamic(
                    value_desc.get_parent_value_node());
            if (bone_node)
            {
                int index1 = bone_node->get_link_index_from_name("scalex");
                int index2 = bone_node->get_link_index_from_name("scalelx");
                int angleIndex = bone_node->get_link_index_from_name("angle");
                if (value_desc.get_index() == index1
                 || value_desc.get_index() == index2)
                {
                    //Bone bone((*bone_node)(get_time()).get(Bone()));
                    //Real prev_duck_length = bone.get_length() * bone.get_scalex() * bone.get_scalex();
                    //Real duck_length = duck.get_point().mag();
                    //Real prev_length = value_desc.get_value(get_time()).get(Real());
                    //Real new_length = prev_length == 0.f || prev_duck_length == 0.f
                    //                ? duck_length
                    //                : prev_length * duck_length / prev_duck_length;
                    Real new_length = duck.get_point().mag();
                    Angle angle = (*bone_node->get_link(angleIndex))(get_time()).get(Angle());
                    angle += duck.get_rotations();
                    return canvas_interface->change_value(synfigapp::ValueDesc(bone_node, angleIndex, value_desc.get_parent_desc()), angle, lock_animation)
                        && canvas_interface->change_value(value_desc, new_length, lock_animation);
                }
            }
        }

        // Zoom duck value (PasteCanvas and Zoom layers) should be
        // converted back from exponent to normal
        if( duck.get_exponential() ) {
            return canvas_interface->change_value(value_desc,log(value.mag()),lock_animation);
        } else {
            return canvas_interface->change_value(value_desc,value.mag(),lock_animation);
        }
    }
    else
    if (type == type_angle)
        //return canvas_interface->change_value(value_desc,Angle::tan(value[1],value[0]),lock_animation);
        return canvas_interface->change_value(value_desc, value_desc.get_value(get_time()).get(Angle()) + duck.get_rotations(),lock_animation);
    else
    if (type == type_transformation)
    {
        if (get_alternative_mode()
         && duck.get_alternative_editable()
         && duck.get_alternative_value_desc().is_valid()
         && duck.get_alternative_value_desc().parent_is_layer()
         && etl::handle<Layer_PasteCanvas>::cast_dynamic(duck.get_alternative_value_desc().get_layer())
         && duck.get_alternative_value_desc().get_param_name() == "origin")
        {
            Point origin = duck.get_alternative_value_desc().get_value(get_time()).get(Point());
            Transformation transformation = duck.get_value_desc().get_value(get_time()).get(Transformation());
            Point delta_offset = value - transformation.offset;
            Point delta_origin = transformation.back_transform(delta_offset, false);
            transformation.offset += delta_offset;
            origin += delta_origin;
            return canvas_interface->change_value(duck.get_alternative_value_desc(), origin, lock_animation)
                && canvas_interface->change_value(duck.get_value_desc(), transformation, lock_animation);
        }
        else
        {
            Transformation transformation = value_desc.get_value(get_time()).get(Transformation());
            Point axis_x_one(1, transformation.angle);
            Point axis_y_one(1, transformation.angle + Angle::deg(90.f) + transformation.skew_angle);

            switch(duck.get_type()) {
            case Duck::TYPE_POSITION:
                transformation.offset = value;
                break;
            case Duck::TYPE_ANGLE:
                transformation.angle += duck.get_rotations();
                break;
            case Duck::TYPE_SKEW:
                transformation.skew_angle += duck.get_rotations();
                break;
            case Duck::TYPE_SCALE:
                transformation.scale = transformation.scale.multiply_coords(duck.get_point());
                break;
            case Duck::TYPE_SCALE_X:
                transformation.scale[0] *= duck.get_point()[0];
                break;
            case Duck::TYPE_SCALE_Y:
                transformation.scale[1] *= duck.get_point()[0];
                break;
            default:
                break;
            }

            return canvas_interface->change_value(value_desc, transformation, lock_animation);
        }
        return false;
    }
    else
    if (type == type_bline_point)
    {
        BLinePoint point = value_desc.get_value(get_time()).get(BLinePoint());
        switch(duck.get_type()) {
        case Duck::TYPE_VERTEX:
            point.set_vertex(duck.get_point());
            break;
        case Duck::TYPE_WIDTH:
            point.set_width(duck.get_point().mag());
            break;
        case Duck::TYPE_TANGENT:
            if (duck.get_scalar() < 0.f)
                point.set_tangent1(duck.get_point());
            else
            if (point.get_merge_tangent_both())
                point.set_tangent1(duck.get_point());
            else
            if (point.get_split_tangent_both())
                point.set_tangent2(duck.get_point());
            else
            if (point.get_split_tangent_angle())
            {
                point.set_tangent1( Point(duck.get_point().mag(), point.get_tangent1().angle()) );
                point.set_tangent2(duck.get_point());
            }
            else
            {
                point.set_tangent1( Point(point.get_tangent1().mag(), duck.get_point().angle()) );
                point.set_tangent2(duck.get_point());
            }
            break;
        default:
            break;
        }

        return canvas_interface->change_value(value_desc, point, lock_animation);
    }

    return canvas_interface->change_value(value_desc,value,lock_animation);
}

void
Duckmatic::connect_signals(const Duck::Handle &duck, const synfigapp::ValueDesc& value_desc, CanvasView &canvas_view)
{
    duck->signal_edited().connect(
        sigc::bind(
            sigc::mem_fun(
                *this,
                &studio::Duckmatic::on_duck_changed),
            value_desc));
    duck->signal_user_click(2).connect(
        sigc::bind(
            sigc::bind(
                sigc::bind(
                    sigc::mem_fun(
                        canvas_view,
                        &studio::CanvasView::popup_param_menu),
                    false),
                1.0f),
            value_desc));
}

/*
-- ** -- DUCK BEZIER STOKE ADD/ERASE/FIND...  M E T H O D S----------------------------
*/
void
Duckmatic::add_duck(const etl::handle<Duck> &duck)
{
    //if(!duck_map.count(duck->get_guid()))
    {
        if(duck_data_share_map.count(duck->get_data_guid()))
        {
            duck->set_shared_point(duck_data_share_map[duck->get_data_guid()]);
        }
        else
        {
            etl::smart_ptr<synfig::Point> point(new Point(duck->get_point()));
            duck->set_shared_point(point);
            duck_data_share_map[duck->get_data_guid()]=point;
        }

        duck_map.insert(duck);
    }

    last_duck_guid=duck->get_guid();
}

void
Duckmatic::add_bezier(const etl::handle<Bezier> &bezier)
{
    bezier_list_.push_back(bezier);
}

void
Duckmatic::add_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color)
{
    assert(stroke_point_list);

    std::list<etl::handle<Stroke> >::iterator iter;

    for(iter=stroke_list_.begin();iter!=stroke_list_.end();++iter)
    {
        if((*iter)->stroke_data==stroke_point_list)
            return;
    }

    etl::handle<Stroke> stroke(new Stroke());

    stroke->stroke_data=stroke_point_list;
    stroke->color=color;

    stroke_list_.push_back(stroke);
}

void
Duckmatic::add_persistent_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color)
{
    add_stroke(stroke_point_list,color);
    persistent_stroke_list_.push_back(stroke_list_.back());
}

void
Duckmatic::clear_persistent_strokes()
{
    persistent_stroke_list_.clear();
}

void
Duckmatic::set_show_persistent_strokes(bool x)
{
    if(x!=show_persistent_strokes)
    {
        show_persistent_strokes=x;
        if(x)
            stroke_list_=persistent_stroke_list_;
        else
            stroke_list_.clear();
    }
}

void
Duckmatic::erase_duck(const etl::handle<Duck> &duck)
{
    duck_map.erase(duck->get_guid());
}

etl::handle<Duckmatic::Duck>
Duckmatic::find_similar_duck(etl::handle<Duck> duck)
{
    DuckMap::const_iterator iter(duck_map.find(duck->get_guid()));
    if(iter!=duck_map.end())
        return iter->second;
    return 0;

/*  std::list<handle<Duck> >::reverse_iterator iter;

    for(iter=duck_list_.rbegin();iter!=duck_list_.rend();++iter)
    {
        if(*iter!=duck && **iter==*duck)
        {
            //synfig::info("Found similar duck! (iter:%08x vs. duck:%08x)",iter->get(), duck.get());
            return *iter;
        }
    }
    return 0;
*/
}

etl::handle<Duckmatic::Duck>
Duckmatic::add_similar_duck(etl::handle<Duck> duck)
{
    etl::handle<Duck> similar(find_similar_duck(duck));
    if(!similar)
    {
        add_duck(duck);
        return duck;
    }
    return similar;
}

void
Duckmatic::erase_bezier(const etl::handle<Bezier> &bezier)
{
    std::list<handle<Bezier> >::iterator iter;

    for(iter=bezier_list_.begin();iter!=bezier_list_.end();++iter)
    {
        if(*iter==bezier)
        {
            bezier_list_.erase(iter);
            return;
        }
    }
    synfig::warning("Unable to find bezier to erase!");
}

etl::handle<Duckmatic::Duck>
Duckmatic::last_duck()const
{
    DuckMap::const_iterator iter(duck_map.find(last_duck_guid));
    if(iter!=duck_map.end())
        return iter->second;
    return 0;
}

etl::handle<Duckmatic::Bezier>
Duckmatic::last_bezier()const
{
    return bezier_list_.back();
}

etl::handle<Duckmatic::Duck>
Duckmatic::find_duck(synfig::Point point, synfig::Real radius, Duck::Type type)
{
    if(radius==0)radius=10000000;

    if(type==Duck::TYPE_DEFAULT)
        type=get_type_mask();

    Real closest(10000000);
    etl::handle<Duck> ret;
    std::vector< etl::handle<Duck> > ret_vector;

    DuckMap::const_iterator iter;

    for(iter=duck_map.begin();iter!=duck_map.end();++iter)
    {
        const Duck::Handle& duck(iter->second);

        if(duck->get_ignore() ||
           (duck->get_type() && !(type & duck->get_type())))
            continue;

        Real dist((duck->get_trans_point()-point).mag_squared());

        bool equal;
        equal=fabs(dist-closest)<0.0000001?true:false;
        if(dist<closest || equal)
        {
            // if there are two ducks at the "same" position, keep track of them
            if(equal)
            {
                // if we haven't any duck stored keep track of last found
                if(!ret_vector.size())
                    ret_vector.push_back(ret);
                // and also keep track of the one on the same place
                ret_vector.push_back(duck);
            }
            // we have another closer duck then discard the stored
            else if (ret_vector.size())
                ret_vector.clear();
            closest=dist;
            ret=duck;
        }
    }

    // Priorization of duck selection when are in the same place.
    bool found(false);
    if(ret_vector.size())
    {
        unsigned int i;
        for(i=0; i<ret_vector.size();i++)
            if(ret_vector[i]->get_type() & Duck::TYPE_WIDTHPOINT_POSITION)
            {
                ret=ret_vector[i];
                found=true;
                break;
            }
        if(!found)
            for(i=0; i<ret_vector.size();i++)
                if(ret_vector[i]->get_type() & Duck::TYPE_WIDTH)
                {
                    ret=ret_vector[i];
                    found=true;
                    break;
                }
        if(!found)
            for(i=0; i<ret_vector.size();i++)
                if(ret_vector[i]->get_type() & Duck::TYPE_RADIUS)
                {
                    ret=ret_vector[i];
                    found=true;
                    break;
                }
        if(!found)
            for(i=0; i<ret_vector.size();i++)
                if(ret_vector[i]->get_type() & Duck::TYPE_VERTEX)
                {
                    ret=ret_vector[i];
                    found=true;
                    break;
                }
        if(!found)
            for(i=0; i<ret_vector.size();i++)
                if(ret_vector[i]->get_type() & Duck::TYPE_TANGENT)
                {
                    ret=ret_vector[i];
                    found=true;
                    break;
                }
        if(!found)
            for(i=0; i<ret_vector.size();i++)
                if(ret_vector[i]->get_type() & Duck::TYPE_POSITION)
                {
                    ret=ret_vector[i];
                    found=true;
                    break;
                }
    }
    if(radius==0 || closest<radius*radius)
        return ret;

    return 0;
}

etl::handle<Duckmatic::Bezier>
Duckmatic::find_bezier(synfig::Point point, synfig::Real radius,float* location)
{
    return find_bezier(point,radius,radius,location);
}

etl::handle<Duckmatic::Bezier>
Duckmatic::find_bezier(synfig::Point pos, synfig::Real scale, synfig::Real radius, float* location)
{
    if(radius==0)radius=10000000;
    Real closest(10000000);
    etl::handle<Bezier> ret;

    bezier<Point>   curve;

    Real    d,step;
    float   time = 0;
    float   best_time = 0;

    for(std::list<handle<Bezier> >::const_iterator iter=bezier_list().begin();iter!=bezier_list().end();++iter)
    {
        curve[0] = (*iter)->p1->get_trans_point();
        curve[1] = (*iter)->c1->get_trans_point();
        curve[2] = (*iter)->c2->get_trans_point();
        curve[3] = (*iter)->p2->get_trans_point();
        curve.sync();

#if 0
        // I don't know why this doesn't work
        time=curve.find_closest(pos,6);
        d=((curve(time)-pos).mag_squared());

#else
        //set the step size based on the size of the picture
        d = (curve[1] - curve[0]).mag() + (curve[2]-curve[1]).mag() + (curve[3]-curve[2]).mag();

        step = d/(2*scale); //want to make the distance between lines happy

        step = max(step,0.01); //100 samples should be plenty
        step = min(step,0.1); //10 is minimum

        d = find_closest(curve,pos,step,&closest,&time);
#endif

        if(d < closest)
        {
            closest = d;
            ret = *iter;
            best_time=time;
        }
    }

    if(closest < radius*radius)
    {
        if(location)
            *location = best_time;  // We need to square-root this because we were dealing with squared distances

        return ret;
    }

    return 0;
}


/*
-- ** -- Duckmatic sketch M E T H O D S----------------------------
*/
bool
Duckmatic::save_sketch(const synfig::String& filename)const
{
    ChangeLocale change_locale(LC_NUMERIC, "C");
    std::ofstream file(filename.c_str());

    if(!file)return false;

    file<<"SKETCH"<<endl;

    std::list<etl::handle<Stroke> >::const_iterator iter;

    for(iter=persistent_stroke_list_.begin();iter!=persistent_stroke_list_.end();++iter)
    {
        file<<"C "
            <<(*iter)->color.get_r()<<' '
            <<(*iter)->color.get_g()<<' '
            <<(*iter)->color.get_b()
        <<endl;
        std::list<synfig::Point>::const_iterator viter;
        for(viter=(*iter)->stroke_data->begin();viter!=(*iter)->stroke_data->end();++viter)
        {
            file<<"V "
                <<(*viter)[0]<<' '
                <<(*viter)[1]
            <<endl;
        }
    }
    if(!file)return false;
    sketch_filename_=filename;
    signal_sketch_saved_();
    return true;
}

bool
Duckmatic::load_sketch(const synfig::String& filename)
{
    ChangeLocale change_locale(LC_NUMERIC, "C");
    std::ifstream file(filename.c_str());

    if(!file)
        return false;

    std::string line;
    getline(file,line);

    if(line!="SKETCH")
    {
        synfig::error("Not a sketch");
        return false;
    }

    etl::smart_ptr<std::list<synfig::Point> > stroke_data;

    while(file)
    {
        getline(file,line);

        if(line.empty())
            continue;

        switch(line[0])
        {
        case 'C':
        case 'c':
            {
                stroke_data.spawn();
                float r,g,b;
                if(!strscanf(line,"C %f %f %f",&r, &g, &b))
                {
                    synfig::warning("Bad color line \"%s\"",line.c_str());
                    r=0;g=0;b=0;
                }
                add_persistent_stroke(stroke_data, synfig::Color(r,g,b));
            }
            break;
        case 'V':
        case 'v':
            if(!stroke_data)
            {
                stroke_data.spawn();
                add_persistent_stroke(stroke_data, synfig::Color(0,0,0));
            }
            float x,y;
            if(!strscanf(line,"V %f %f",&x, &y))
                synfig::warning("Bad vertex \"%s\"",line.c_str());
            else
                stroke_data->push_back(synfig::Vector(x,y));
            break;
        default:
            synfig::warning("Unexpected sketch token '%c'",line[0]);
            break;
        }
    }

    sketch_filename_=filename;
    return true;
}


void
Duckmatic::add_ducks_layers(synfig::Canvas::Handle canvas, std::set<synfig::Layer::Handle>& selected_layer_set, etl::handle<CanvasView> canvas_view, synfig::TransformStack& transform_stack, int *out_transform_count)
{
    int transforms(0);
    String layer_name;

#define QUEUE_REBUILD_DUCKS     sigc::mem_fun(*canvas_view,&CanvasView::queue_rebuild_ducks)

    if(!canvas)
    {
        synfig::warning("Duckmatic::add_ducks_layers(): Layer doesn't have canvas set");
        return;
    }
    for(Canvas::iterator iter(canvas->begin());iter!=canvas->end();++iter)
    {
        Layer::Handle layer(*iter);

        if(selected_layer_set.count(layer))
        {
            if(!curr_transform_stack_set)
            {
                curr_transform_stack_set=true;
                curr_transform_stack=transform_stack;
            }

            // This layer is currently selected.
            duck_changed_connections.push_back(layer->signal_changed().connect(QUEUE_REBUILD_DUCKS));

            // do the bounding box thing
            synfig::Rect& bbox = canvas_view->get_bbox();

            // special calculations for Layer_PasteCanvas
            etl::handle<Layer_PasteCanvas> layer_pastecanvas( etl::handle<Layer_PasteCanvas>::cast_dynamic(layer) );
            synfig::Rect layer_bounds = layer_pastecanvas
                                      ? layer_pastecanvas->get_bounding_rect_context_dependent(canvas_view->get_context_params())
                                      : layer->get_bounding_rect();

            bbox|=transform_stack.perform(layer_bounds);

            // Grab the layer's list of parameters
            Layer::ParamList paramlist(layer->get_param_list());

            // Grab the layer vocabulary
            Layer::Vocab vocab=layer->get_param_vocab();
            Layer::Vocab::iterator iter;

            for(iter=vocab.begin();iter!=vocab.end();iter++)
            {
                if(!iter->get_hidden() && !iter->get_invisible_duck())
                {
                    synfigapp::ValueDesc value_desc(layer,iter->get_name());
                    add_to_ducks(value_desc,canvas_view,transform_stack,&*iter);
                    if(value_desc.is_value_node())
                        duck_changed_connections.push_back(value_desc.get_value_node()->signal_changed().connect(QUEUE_REBUILD_DUCKS));
                }
            }
        }

        layer_name=layer->get_name();

        if(layer->active())
        {
            Transform::Handle trans(layer->get_transform());
            if(trans)
            {
                transform_stack.push(trans);
                transforms++;
            }
        }

        // If this is a paste canvas layer, then we need to
        // descend into it
        if(etl::handle<Layer_PasteCanvas> layer_pastecanvas = etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
        {
            transform_stack.push_back(
                new Transform_Matrix(
                    layer->get_guid(),
                    layer_pastecanvas->get_summary_transformation().get_matrix()
                )
            );

            Canvas::Handle child_canvas(layer->get_param("canvas").get(Canvas::Handle()));

            // keep stack
            if ( etl::handle<Layer_FilterGroup>::cast_dynamic(layer_pastecanvas)
              && layer_pastecanvas->get_amount() > 0.5 )
            {
            	transforms++;
                add_ducks_layers(child_canvas,selected_layer_set,canvas_view,transform_stack, &transforms);
            }
            else
            {
                add_ducks_layers(child_canvas,selected_layer_set,canvas_view,transform_stack);
            	transform_stack.pop();
            }
        }
    }

    if (out_transform_count)
    {
    	// keep stack and return count transforms ...
    	*out_transform_count += transforms;
    }
    else
    {
        // ... or remove all of the transforms we have added
    	while(transforms--) { transform_stack.pop(); }
    }

#undef QUEUE_REBUILD_DUCKS
}

/*
-- ** -- add_to_ducks GIANT  M E T H O D S-------------------------------------
-- ** -- -----------------------------------------------------------------------
-- ** -- S H O U L D  B E  S I M P L I F  I E D---------------------------------
-- ** -- -----------------------------------------------------------------------
---**-----------------------TODO-Must be simplified!(1500 lines length)-------
-- ** -- -----------------------------------------------------------------------
*/
bool
Duckmatic::add_to_ducks(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack, synfig::ParamDesc *param_desc)
{
    synfig::Type &type=value_desc.get_value_type();
#define REAL_COOKIE     reinterpret_cast<synfig::ParamDesc*>(28)

    if (type == type_real)
    {
        if(!param_desc || param_desc==REAL_COOKIE || !param_desc->get_origin().empty())
        {
            etl::handle<Duck> duck=new Duck();
            set_duck_value_desc(*duck, value_desc, transform_stack);
            duck->set_radius(true);
            duck->set_type(Duck::TYPE_RADIUS);

            // put the duck on the right hand side of the center
            // Zoom parameter value (PasteCanvas and Zoom layers)
            // should be represented as exponent
            if ( param_desc && param_desc!=REAL_COOKIE && param_desc->get_exponential() )
            {
                duck->set_point(Point(exp(value_desc.get_value(get_time()).get(Real())), 0));
                duck->set_exponential(param_desc->get_exponential());
            } else {
                duck->set_point(Point(value_desc.get_value(get_time()).get(Real()), 0));
                duck->set_exponential(false);
            }

            if(value_desc.is_value_node())
            {
                // If the ValueNode can be directly manipulated,
                // then set it as so.
                duck->set_editable(synfigapp::is_editable(value_desc.get_value_node()));
            }
            else
            {
                duck->set_editable(true);
            }

            if(param_desc && param_desc!=REAL_COOKIE)
            {
                if(!param_desc->get_origin().empty())
                {
                    synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                    /*
                    duck->set_origin(value_desc_origin.get_value(get_time()).get(synfig::Point()));
                    */
                    add_to_ducks(value_desc_origin,canvas_view, transform_stack);

                    Layer::Handle layer=value_desc.get_layer();
                    if(etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
                    {
                        Vector focus(layer->get_param("focus").get(Vector()));
                        duck->set_origin(last_duck()->get_point() + focus);
                    }
                    else
                        duck->set_origin(last_duck());
                }
                duck->set_scalar(param_desc->get_scalar());
            }

            duck->signal_edited().clear(); // value_desc.get_value_type() == type_real:
            duck->signal_edited().connect(
                sigc::bind(
                    sigc::mem_fun(
                        *this,
                        &studio::Duckmatic::on_duck_changed),
                    value_desc));

            duck->signal_user_click(2).connect(
                sigc::bind(
                    sigc::bind(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu),
                            false),
                        0.0f),
                    value_desc));

            add_duck(duck);

            return true;
        }
    }
    else
    if (type == type_angle)
    {
        if(!param_desc || param_desc==REAL_COOKIE || !param_desc->get_origin().empty())
        {
            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_ANGLE);
            set_duck_value_desc(*duck, value_desc, transform_stack);
            synfig::Angle angle;

            angle=value_desc.get_value(get_time()).get(Angle());
            duck->set_point(Point(Angle::cos(angle).get(),Angle::sin(angle).get()));
            if(value_desc.is_value_node())
            {
                ValueNode::Handle value_node=value_desc.get_value_node();
                //duck->set_name(strprintf("%x",value_node.get()));

                // If the ValueNode can be directly manipulated,
                // then set it as so.
                duck->set_editable(synfigapp::is_editable(value_desc.get_value_node()));
            }
            else
            {
                //angle=(value_desc.get_value().get(Angle()));
                //duck->set_point(Point(Angle::cos(angle).get(),Angle::sin(angle).get()));
                //duck->set_name(strprintf("%x",value_desc.get_layer().get())+value_desc.get_param_name());
                duck->set_editable(true);
            }

            if(param_desc && param_desc!=REAL_COOKIE)
            {
                if(!param_desc->get_origin().empty())
                {
                    synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                    /*
                    duck->set_origin(value_desc_origin.get_value(get_time()).get(synfig::Point()));
                    */
                    add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                    duck->set_origin(last_duck());
                }
                duck->set_scalar(param_desc->get_scalar());
            }

            duck->signal_edited().clear(); // value_desc.get_value_type() == type_angle:
            duck->signal_edited().connect(
                sigc::bind(
                    sigc::mem_fun(
                        *this,
                        &studio::Duckmatic::on_duck_changed),
                    value_desc));

            duck->signal_user_click(2).connect(
                sigc::bind(
                    sigc::bind(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu),
                            false),
                        0.0f),
                    value_desc));

            add_duck(duck);

            return true;
        }
    }
    else
    if (type == type_vector)
    {
        etl::handle<Layer_PasteCanvas> layer;
        if (value_desc.parent_is_layer())
            layer = etl::handle<Layer_PasteCanvas>::cast_dynamic(value_desc.get_layer());
        if (!layer) {
            etl::handle<Duck> duck=new Duck();
            set_duck_value_desc(*duck, value_desc, transform_stack);
            ValueNode_Composite::Handle blinepoint_value_node;
            int index;
            bool done(false);
            if(value_desc.parent_is_linkable_value_node()
               &&
               value_desc.get_parent_value_node()->get_type() == type_bline_point)
            {
                blinepoint_value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node());
                if(blinepoint_value_node)
                {
                    index=blinepoint_value_node->get_link_index_from_name("t2");
                    if(index==value_desc.get_index())
                    {
                        BLinePoint bp=(*blinepoint_value_node)(get_time()).get(BLinePoint());
                        Vector t2=bp.get_tangent2();
                        duck->set_point(t2);
                        done=true;
                    }
                }
            }
            if(!done)
                duck->set_point(value_desc.get_value(get_time()).get(Point()));

            if(value_desc.is_value_node())
            {
                // if the vertex is converted to 'bone influence', add the bones' ducks
                if (ValueNode_BoneInfluence::Handle bone_influence_vertex_value_node =
                    ValueNode_BoneInfluence::Handle::cast_dynamic(value_desc.get_value_node()))
                    add_to_ducks(synfigapp::ValueDesc(bone_influence_vertex_value_node,
                                                      bone_influence_vertex_value_node->get_link_index_from_name("bone_weight_list")),
                                 canvas_view, transform_stack);

                // If the ValueNode can be directly manipulated,
                // then set it as so.
                duck->set_editable(synfigapp::is_editable(value_desc.get_value_node()));
            }
            else
            {
                //duck->set_point(value_desc.get_value().get(Point()));
                //duck->set_name(strprintf("%x",value_desc.get_layer().get())+value_desc.get_param_name());
                duck->set_editable(true);
            }

            // If we were passed a parameter description
            if(param_desc)
            {
                if(!param_desc->get_connect().empty())
                {
                    synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_connect());
                    Duck::Handle connect_duck;
                    if(duck_map.find(calc_duck_guid(value_desc_origin, transform_stack))!=duck_map.end())
                    {
                        connect_duck=duck_map[calc_duck_guid(value_desc_origin, transform_stack)];
                    }
                    else
                    {
                        add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                        connect_duck=last_duck();
                    }
                    duck->set_connect_duck(connect_duck);
                }
                if(!param_desc->get_box().empty())
                {
                    synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_box());
                    add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                    duck->set_box_duck(last_duck());
                }

                // If we have an origin
                if(!param_desc->get_origin().empty())
                {
                    synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                    /*
                    duck->set_origin(value_desc_origin.get_value(get_time()).get(synfig::Point()));
                    */
                    add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                    duck->set_origin(last_duck());
                    duck->set_type(Duck::TYPE_VERTEX);
                }
                else
                    duck->set_type(Duck::TYPE_POSITION);

                duck->set_scalar(param_desc->get_scalar());
            }
            else
                duck->set_type(Duck::TYPE_POSITION);

            duck->signal_edited().clear(); // value_desc.get_value_type() == type_vector:
            duck->signal_edited().connect(
                sigc::bind(
                    sigc::mem_fun(
                        *this,
                        &studio::Duckmatic::on_duck_changed),
                    value_desc));

            duck->signal_user_click(2).connect(
                sigc::bind(
                    sigc::bind(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu),
                            false),
                        1.0f),
                    value_desc));

            add_duck(duck);

            return true;
        }
    }
    else
    if (type == type_transformation)
    {
        if (value_desc.parent_is_layer() && param_desc != NULL)
        {
            etl::handle<Layer_PasteCanvas> layer = etl::handle<Layer_PasteCanvas>::cast_dynamic(value_desc.get_layer());
            if (layer)
            {
                synfigapp::ValueDesc alternative_value_desc(value_desc.get_layer(), "origin");
                Transformation transformation = value_desc.get_value(get_time()).get(Transformation());

                bool editable = !value_desc.is_value_node()
                    || synfigapp::is_editable(value_desc.get_value_node());
                bool alternative_editable = !alternative_value_desc.is_value_node()
                    || synfigapp::is_editable(alternative_value_desc.get_value_node());
                alternative_editable = alternative_editable && editable;
                Point axis_x(1, transformation.angle);
                Point axis_y(1, transformation.angle + Angle::deg(90.f) + transformation.skew_angle);

                Point screen_offset = transform_stack.perform(transformation.offset);
                Point screen_axis_x = transform_stack.perform(transformation.offset + axis_x) - screen_offset;
                Point screen_axis_y = transform_stack.perform(transformation.offset + axis_y) - screen_offset;
                Real scalar_x = screen_axis_x.mag();
                if (scalar_x > 0.0) scalar_x = 1.0/scalar_x;
                Real scalar_y = screen_axis_y.mag();
                if (scalar_y > 0.0) scalar_y = 1.0/scalar_y;
                scalar_x /= zoom;
                scalar_y /= zoom;
                Real pw = canvas_interface->get_canvas()->rend_desc().get_pw();
                Real ph = canvas_interface->get_canvas()->rend_desc().get_ph();
                scalar_x *= 75.0 * fabs(pw);
                scalar_y *= 75.0 * fabs(ph);

                Duck::Handle duck;

                // add offset duck
                duck=new Duck();
                set_duck_value_desc(*duck, value_desc, "offset", transform_stack);
                duck->set_point(transformation.offset);
                duck->set_editable(editable);
                duck->set_alternative_editable(alternative_editable);
                duck->set_type(Duck::TYPE_POSITION);
                duck->set_alternative_value_desc(alternative_value_desc);
                connect_signals(duck, duck->get_value_desc(), *canvas_view);
                add_duck(duck);

                etl::handle<Duck> origin_duck = duck;

                // add angle duck
                duck=new Duck();
                duck->set_type(Duck::TYPE_ANGLE);
                set_duck_value_desc(*duck, value_desc, "angle", transform_stack);
                duck->set_point(Point(0.8,transformation.angle));
                duck->set_scalar(scalar_x);
                duck->set_editable(editable);
                duck->set_origin(origin_duck);
                connect_signals(duck, duck->get_value_desc(), *canvas_view);
                add_duck(duck);

                etl::handle<Duck> angle_duck = duck;

                // add skew duck
                duck=new Duck();
                duck->set_type(Duck::TYPE_SKEW);
                set_duck_value_desc(*duck, value_desc, "skew_angle", transform_stack);
                duck->set_point(Point(0.8,transformation.skew_angle));
                duck->set_scalar(scalar_y);
                duck->set_editable(editable);
                duck->set_origin(origin_duck);
                duck->set_axis_x_angle(angle_duck, Angle::deg(90));
                duck->set_axis_y_angle(angle_duck, Angle::deg(180));
                connect_signals(duck, duck->get_value_desc(), *canvas_view);
                add_duck(duck);

                etl::handle<Duck> skew_duck = duck;

                // add scale-x duck
                duck=new Duck();
                duck->set_type(Duck::TYPE_SCALE_X);
                set_duck_value_desc(*duck, value_desc.get_sub_value("scale").get_sub_value("x"), transform_stack);
                duck->set_point(Point(1,0));
                duck->set_scalar(scalar_x);
                duck->set_editable(editable);
                duck->set_origin(origin_duck);
                duck->set_linear(true, angle_duck);
                connect_signals(duck, duck->get_value_desc(), *canvas_view);
                add_duck(duck);

                etl::handle<Duck> scale_x_duck = duck;

                // add scale-y duck
                duck=new Duck();
                duck->set_type(Duck::TYPE_SCALE_Y);
                set_duck_value_desc(*duck, value_desc.get_sub_value("scale").get_sub_value("y"), transform_stack);
                duck->set_point(Point(1,0));
                duck->set_scalar(scalar_y);
                duck->set_editable(editable);
                duck->set_origin(origin_duck);
                duck->set_linear(true, skew_duck);
                connect_signals(duck, duck->get_value_desc(), *canvas_view);
                add_duck(duck);

                etl::handle<Duck> scale_y_duck = duck;

                // add scale duck
                duck=new Duck();
                duck->set_type(Duck::TYPE_SCALE);
                set_duck_value_desc(*duck, value_desc, "scale", transform_stack);
                duck->set_point(Point(1,1));
                duck->set_lock_aspect(true);
                duck->set_editable(editable);
                duck->set_origin(origin_duck);
                duck->set_axis_x_angle(scale_x_duck);
                duck->set_axis_x_mag(scale_x_duck);
                duck->set_axis_y_angle(scale_y_duck);
                duck->set_axis_y_mag(scale_y_duck);
                duck->set_track_axes(true);
                connect_signals(duck, duck->get_value_desc(), *canvas_view);
                add_duck(duck);

                return true;
            }
        }
    }
    else
    if (type == type_segment)
    {
        int index;
        etl::handle<Bezier> bezier(new Bezier());
        ValueNode_Composite::Handle value_node;

        if(value_desc.is_value_node() &&
            (value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())))
        {
            index=value_node->get_link_index_from_name("p1");
            if(!add_to_ducks(synfigapp::ValueDesc(value_node,index),canvas_view,transform_stack))
                return false;
            bezier->p1=last_duck();
            bezier->p1->set_type(Duck::TYPE_VERTEX);

            index=value_node->get_link_index_from_name("t1");
            if(!add_to_ducks(synfigapp::ValueDesc(value_node,index),canvas_view,transform_stack))
                return false;
            bezier->c1=last_duck();
            bezier->c1->set_type(Duck::TYPE_TANGENT);
            bezier->c1->set_origin(bezier->p1);
            bezier->c1->set_scalar(TANGENT_BEZIER_SCALE);
            bezier->c1->set_tangent(true);

            index=value_node->get_link_index_from_name("p2");
            if(!add_to_ducks(synfigapp::ValueDesc(value_node,index),canvas_view,transform_stack))
                return false;
            bezier->p2=last_duck();
            bezier->p2->set_type(Duck::TYPE_VERTEX);

            index=value_node->get_link_index_from_name("t2");
            if(!add_to_ducks(synfigapp::ValueDesc(value_node,index),canvas_view,transform_stack))
                return false;
            bezier->c2=last_duck();
            bezier->c2->set_type(Duck::TYPE_TANGENT);
            bezier->c2->set_origin(bezier->p2);
            bezier->c2->set_scalar(-TANGENT_BEZIER_SCALE);
            bezier->c2->set_tangent(true);

            bezier->signal_user_click(2).connect(
                sigc::bind(
                    sigc::mem_fun(
                        *canvas_view,
                        &studio::CanvasView::popup_param_menu_bezier),
                    value_desc));

            add_bezier(bezier);
        }
        else if(value_desc.get_value().is_valid())
        {
            Segment segment=value_desc.get_value().get(Segment());
            etl::handle<Duck> duck_p,duck_c;
            synfig::String name;
            if(param_desc)
                name=param_desc->get_local_name();
            else
                name=value_desc.get_guid_string();

            duck_p=new Duck(segment.p1);
            duck_p->set_name(name+".P1");
            duck_p->set_type(Duck::TYPE_VERTEX);
            add_duck(duck_p);

            duck_c=new Duck(segment.t1);
            duck_c->set_name(name+".T1");
            duck_c->set_type(Duck::TYPE_TANGENT);
            add_duck(duck_c);
            duck_c->set_origin(duck_p);
            duck_c->set_scalar(TANGENT_HANDLE_SCALE);
            duck_c->set_tangent(true);

            bezier->p1=duck_p;
            bezier->c1=duck_c;

            duck_p=new Duck(segment.p2);
            duck_p->set_name(name+".P2");
            duck_p->set_type(Duck::TYPE_VERTEX);
            add_duck(duck_p);

            duck_c=new Duck(segment.t2);
            duck_c->set_type(Duck::TYPE_TANGENT);
            duck_c->set_name(name+".T2");
            add_duck(duck_c);
            duck_c->set_origin(duck_p);
            duck_c->set_scalar(-TANGENT_HANDLE_SCALE);
            duck_c->set_tangent(true);

            bezier->p2=duck_p;
            bezier->c2=duck_c;
            add_bezier(bezier);
        }

        return true;
    }
    else
    if (type == type_bline_point)
    {
        bool editable = !value_desc.is_value_node() || synfigapp::is_editable(value_desc.get_value_node());
        BLinePoint point = value_desc.get_value(get_time()).get(BLinePoint());

        Duck::Handle duck;

        // add vertex duck
        duck=new Duck();
        set_duck_value_desc(*duck, value_desc, "point", transform_stack);
        duck->set_point(point.get_vertex());
        duck->set_editable(editable);
        duck->set_type(Duck::TYPE_VERTEX);
        connect_signals(duck, duck->get_value_desc(), *canvas_view);
        add_duck(duck);

        etl::handle<Duck> vertex_duck = duck;

        // add tangent1 duck
        duck=new Duck();
        duck->set_type(Duck::TYPE_TANGENT);
        set_duck_value_desc(*duck, value_desc, "t1", transform_stack);
        duck->set_point(point.get_tangent1());
        duck->set_editable(editable);
        duck->set_origin(vertex_duck);
        connect_signals(duck, duck->get_value_desc(), *canvas_view);
        add_duck(duck);

        // add tangent2 duck
        duck=new Duck();
        duck->set_type(Duck::TYPE_TANGENT);
        set_duck_value_desc(*duck, value_desc, "t2", transform_stack);
        duck->set_point(point.get_tangent2());
        duck->set_editable(editable);
        duck->set_origin(vertex_duck);
        duck->set_scalar(-1);
        connect_signals(duck, duck->get_value_desc(), *canvas_view);
        add_duck(duck);

        return true;
    }
    else
    if (type == type_list)
    {
        // Check for BLine
        if (value_desc.is_value_node() &&
            ValueNode_BLine::Handle::cast_dynamic(value_desc.get_value_node()))
        {
            ValueNode_BLine::Handle value_node;
            value_node=ValueNode_BLine::Handle::cast_dynamic(value_desc.get_value_node());

            int i,first=-1;

            etl::handle<Bezier> bezier;
            etl::handle<Duck> first_duck, first_tangent2_duck;

            for (i = 0; i < value_node->link_count(); i++)
            {
                float amount(value_node->list[i].amount_at_time(get_time()));

                // skip vertices that aren't fully on
                if (amount < 0.9999f)
                    continue;

                // remember the index of the first vertex we didn't skip
                if (first == -1)
                    first = i;

                ValueNode::Handle sub_node = value_node->get_link(i);
                bool editable = synfigapp::is_editable(sub_node);
                BLinePoint bline_point((*value_node->get_link(i))(get_time()).get(BLinePoint()));
                synfigapp::ValueDesc sub_value_desc(value_node, i, value_desc);

                // Now add the ducks:

                Duck::Handle duck;

                // ----Vertex Duck

                duck=new Duck(bline_point.get_vertex());
                set_duck_value_desc(*duck, sub_value_desc, "point", transform_stack);
                duck->set_editable(editable);
                duck->set_type(Duck::TYPE_VERTEX);
                if(param_desc)
                {
                    if(!param_desc->get_origin().empty())
                    {
                        synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                        add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                        duck->set_origin(last_duck());
                    }
                }
                duck=add_similar_duck(duck);
                if(i==first) first_duck=duck;
                connect_signals(duck, duck->get_value_desc(), *canvas_view);

                Duck::Handle vertex_duck = duck;

                // ----Width duck

                Duck::Handle width;

                // Add the width duck if it is a parameter with a hint (ie. "width") or if it isn't a parameter
                //if (!   ((param_desc && !param_desc->get_hint().empty()) || !param_desc)   )
                if (param_desc && param_desc->get_hint().empty())
                {
                    // if it's a parameter without a hint, then don't add the width duck
                    // (This prevents width ducks from being added to region layers, and possibly other cases)
                }
                else
                {
                    // add width duck

                    duck=new Duck();
                    set_duck_value_desc(*duck, sub_value_desc, "width", transform_stack);
                    duck->set_radius(true);
                    duck->set_point(Point(bline_point.get_width(), 0));
                    duck->set_editable(editable);
                    duck->set_type(Duck::TYPE_WIDTH);
                    duck->set_origin(vertex_duck);
                    connect_signals(duck, duck->get_value_desc(), *canvas_view);

                    // if the bline is a layer's parameter, scale the width duck by the layer's "width" parameter
                    if (param_desc)
                    {
                        ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_hint()).get_value(get_time()));
                        Real gv(value_desc.get_layer()->get_outline_grow_mark());
                        if(value.same_type_as(synfig::Real()))
                            duck->set_scalar(exp(gv)*value.get(synfig::Real())*0.5f);
                        // if it doesn't have a "width" parameter, scale by 0.5f instead
                        else
                            duck->set_scalar(0.5f);
                    }
                    // otherwise just present the raw unscaled width
                    else
                        duck->set_scalar(0.5f);

                    add_duck(duck);
                    width = duck;
                }

                // each bezier uses t2 of one point and t1 of the next
                // the first time through this loop we won't have the t2 duck from the previous vertex
                // and so we don't make a bezier.  instead we skip on to t2 for this point
                Duck::Handle tangent1_duck;
                if(bezier)
                {
                    // Add the tangent1 duck
                    duck=new Duck(bline_point.get_tangent1());
                    set_duck_value_desc(*duck, sub_value_desc, "t1", transform_stack);
                    duck->set_editable(editable);
                    duck=add_similar_duck(duck);

                    duck->set_origin(vertex_duck);
                    duck->set_scalar(-TANGENT_BEZIER_SCALE);
                    duck->set_tangent(true);
                    duck->set_shared_point(etl::smart_ptr<Point>());
                    duck->set_shared_angle(etl::smart_ptr<Angle>());
                    duck->set_shared_mag(etl::smart_ptr<Real>());
                    connect_signals(duck, duck->get_value_desc(), *canvas_view);

                    // each bezier uses t2 of one point and t1 of the next
                    // we should already have a bezier, so add the t1 of this point to it

                    bezier->p2=vertex_duck;
                    bezier->c2=duck;

                    bezier->signal_user_click(2).connect(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu_bezier),
                            synfigapp::ValueDesc(value_node,i)));

                    add_bezier(bezier);
                    bezier=0;
                    tangent1_duck = duck;
                }

                // don't start a new bezier for the last point in the line if we're not looped
                if ((i+1>=value_node->link_count() && !value_node->get_loop()))
                    continue;

                bezier=new Bezier();

                // Add the tangent2 duck
                Duck::Handle tangent2_duck;
                duck=new Duck(bline_point.get_tangent2());
                set_duck_value_desc(*duck, sub_value_desc, "t2", transform_stack);
                duck->set_editable(editable);

                duck=add_similar_duck(duck);
                duck->set_origin(vertex_duck);
                duck->set_scalar(TANGENT_BEZIER_SCALE);
                duck->set_tangent(true);
                duck->set_shared_point(etl::smart_ptr<Point>());
                duck->set_shared_angle(etl::smart_ptr<Angle>());
                duck->set_shared_mag(etl::smart_ptr<Real>());
                connect_signals(duck, duck->get_value_desc(), *canvas_view);

                bezier->p1=vertex_duck;
                bezier->c1=duck;
                tangent2_duck = duck;
                if (i == first) first_tangent2_duck = tangent2_duck;

                // link tangents
                if (tangent1_duck && tangent2_duck && !bline_point.get_split_tangent_both()) {
                    if (bline_point.get_merge_tangent_both())
                    {
                        etl::smart_ptr<synfig::Point> point(new Point(tangent1_duck->get_point()));
                        tangent1_duck->set_shared_point(point);
                        tangent2_duck->set_shared_point(point);
                    }
                    else
                    if (!bline_point.get_split_tangent_angle())
                    {
                        etl::smart_ptr<synfig::Angle> angle(new Angle(tangent1_duck->get_point().angle()));
                        tangent1_duck->set_shared_angle(angle);
                        tangent2_duck->set_shared_angle(angle);
                    }
                    else
                    if (!bline_point.get_split_tangent_radius())
                    {
                        etl::smart_ptr<synfig::Real> mag(new Real(tangent1_duck->get_point().mag()));
                        tangent1_duck->set_shared_mag(mag);
                        tangent2_duck->set_shared_mag(mag);
                    }
                }
            }

            // Loop if necessary
            if(bezier && value_node->get_loop())
            {
                ValueNode::Handle sub_node = value_node->get_link(first);
                bool editable = synfigapp::is_editable(sub_node);
                bool is_bline_point = sub_node->get_type() == type_bline_point;
                BLinePoint bline_point;
                if (is_bline_point) bline_point = (*sub_node)(get_time()).get(BLinePoint());

                ValueNode_BoneInfluence::Handle bone_influence_vertex_value_node(
                    ValueNode_BoneInfluence::Handle::cast_dynamic(value_node->get_link(first)));
                ValueNode_Composite::Handle composite_bone_link_value_node;
                synfig::TransformStack bone_transform_stack(transform_stack);
                if (bone_influence_vertex_value_node)
                {
                    // apply bones transformation to the ducks
                    composite_bone_link_value_node = ValueNode_Composite::Handle::cast_dynamic(
                        bone_influence_vertex_value_node->get_link("link") );

                    if(param_desc)
                    {
                        if(!param_desc->get_origin().empty())
                        {
                            synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                            add_to_ducks(value_desc_origin, canvas_view, transform_stack);
                            synfig::GUID guid(calc_duck_guid(value_desc_origin, transform_stack));
                            bone_transform_stack.push(new Transform_Origin(guid^synfig::GUID::hasher("origin"), last_duck()));
                        }
                    }

                    Matrix transform(bone_influence_vertex_value_node->calculate_transform(get_time()));
                    synfig::GUID guid(bone_influence_vertex_value_node->get_link("bone_weight_list")->get_guid());

                    bone_transform_stack.push(new Transform_Matrix(guid, transform));
                }

                // Add the vertex duck
                Duck::Handle duck;
                Duck::Handle vertex_duck(first_duck);
                Duck::Handle tangent2_duck(first_tangent2_duck);
                synfigapp::ValueDesc sub_value_desc(value_node,first,value_desc);

                // Add the tangent1 duck
                duck=new Duck(bline_point.get_tangent1());
                set_duck_value_desc(*duck, sub_value_desc, "t1", transform_stack);
                duck->set_editable(editable);

                duck=add_similar_duck(duck);
                duck->set_origin(vertex_duck);
                duck->set_scalar(-TANGENT_BEZIER_SCALE);
                duck->set_tangent(true);
                duck->set_shared_point(etl::smart_ptr<Point>());
                duck->set_shared_angle(etl::smart_ptr<Angle>());
                duck->set_shared_mag(etl::smart_ptr<Real>());
                connect_signals(duck, duck->get_value_desc(), *canvas_view);

                bezier->p2=vertex_duck;
                bezier->c2=duck;

                bezier->signal_user_click(2).connect(
                    sigc::bind(
                        sigc::mem_fun(
                            *canvas_view,
                            &studio::CanvasView::popup_param_menu_bezier),
                        synfigapp::ValueDesc(value_node,first)));

                add_bezier(bezier);
                bezier=0;
                Duck::Handle tangent1_duck = duck;

                // link tangents
                if (tangent1_duck && tangent2_duck && !bline_point.get_split_tangent_both()) {
                    if (bline_point.get_merge_tangent_both())
                    {
                        etl::smart_ptr<synfig::Point> point(new Point(tangent1_duck->get_point()));
                        tangent1_duck->set_shared_point(point);
                        tangent2_duck->set_shared_point(point);
                    }
                    else
                    if (!bline_point.get_split_tangent_angle())
                    {
                        etl::smart_ptr<synfig::Angle> angle(new Angle(tangent1_duck->get_point().angle()));
                        tangent1_duck->set_shared_angle(angle);
                        tangent2_duck->set_shared_angle(angle);
                    }
                    else
                    if (!bline_point.get_split_tangent_radius())
                    {
                        etl::smart_ptr<synfig::Real> mag(new Real(tangent1_duck->get_point().mag()));
                        tangent1_duck->set_shared_mag(mag);
                        tangent2_duck->set_shared_mag(mag);
                    }
                }
            }
            return true;
        }

        else // Check for StaticList
        if(value_desc.is_value_node() &&
            ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_value_node()))
        {
            ValueNode_StaticList::Handle value_node;
            value_node=ValueNode_StaticList::Handle::cast_dynamic(value_desc.get_value_node());
            int i;

            synfig::Type &contained_type(value_node->get_contained_type());
            if (contained_type == type_vector)
            {
                Bezier bezier;
                etl::handle<Duck> first_duck, duck;
                int first = -1;
                for(i=0;i<value_node->link_count();i++)
                {
                    if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
                        return false;
                    duck = last_duck();

                    // remember the index of the first vertex we didn't skip
                    if (first == -1)
                    {
                        first = i;
                        first_duck = duck;
                    }

                    if(param_desc && !param_desc->get_origin().empty())
                    {
                        synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                        add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                        duck->set_origin(last_duck());
/*
                        ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
                        if(value.same_type_as(synfig::Point()))
                            duck->set_origin(value.get(synfig::Point()));
*/
//                      if(!param_desc->get_origin().empty())
//                          last_duck()->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
                    }
                    duck->set_type(Duck::TYPE_VERTEX);
                    bezier.p1=bezier.p2;bezier.c1=bezier.c2;
                    bezier.p2=bezier.c2=duck;

                    if (first != i)
                    {
                        handle<Bezier> bezier_(new Bezier());
                        bezier_->p1=bezier.p1;
                        bezier_->c1=bezier.c1;
                        bezier_->p2=bezier.p2;
                        bezier_->c2=bezier.c2;
                        add_bezier(bezier_);
                        last_bezier()->signal_user_click(2).connect(
                            sigc::bind(
                                sigc::mem_fun(
                                    *canvas_view,
                                    &studio::CanvasView::popup_param_menu_bezier),
                                synfigapp::ValueDesc(value_node,i)));
                    }
                }

                if (value_node->get_loop() && first != -1 && first_duck != duck)
                {
                    duck = first_duck;

                    bezier.p1=bezier.p2;bezier.c1=bezier.c2;
                    bezier.p2=bezier.c2=duck;

                    handle<Bezier> bezier_(new Bezier());
                    bezier_->p1=bezier.p1;
                    bezier_->c1=bezier.c1;
                    bezier_->p2=bezier.p2;
                    bezier_->c2=bezier.c2;
                    add_bezier(bezier_);
                    last_bezier()->signal_user_click(2).connect(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu_bezier),
                            synfigapp::ValueDesc(value_node,first)));
                }
            }
            else
            if (contained_type == type_segment)
            {
                for(i=0;i<value_node->link_count();i++)
                {
                    if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
                        return false;
                }
            }
            else
            if (contained_type == type_bone_object)
            {
                printf("%s:%d adding ducks\n", __FILE__, __LINE__);
                for(i=0;i<value_node->link_count();i++)
                    if(!add_to_ducks(synfigapp::ValueDesc(value_node,i,value_desc),canvas_view,transform_stack))
                        return false;
                printf("%s:%d adding ducks done\n\n", __FILE__, __LINE__);
            }
            else
            if (contained_type == type_bone_weight_pair)
            {
                for(i=0;i<value_node->link_count();i++)
                    if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
                        return false;
            }
            else
            if (value_node->get_contained_type() == types_namespace::TypePair<Bone, Bone>::instance)
            {
                bool edit_second = value_desc.parent_is_layer() && value_desc.get_layer()->active();
                for(i=0;i<value_node->link_count();i++)
                {
                    ValueNode_Composite::Handle value_node_composite =
                        ValueNode_Composite::Handle::cast_dynamic(
                            value_node->get_link(i) );
                    if (value_node_composite)
                    {
                        if (!add_to_ducks(
                            synfigapp::ValueDesc(
                                value_node_composite,
                                value_node_composite->get_link_index_from_name(edit_second ? "second" : "first"),
                                synfigapp::ValueDesc(value_node,i,value_desc) ),
                            canvas_view,
                            transform_stack ))
                                    return false;
                    }
                }
            }
            else
                return false;
        }

        else // Check for WPList
        if(value_desc.is_value_node() &&
            ValueNode_WPList::Handle::cast_dynamic(value_desc.get_value_node()))
        {
            ValueNode_WPList::Handle value_node;
            bool homogeneous=true; // if we have an exported WPList without a layer consider it homogeneous
            value_node=ValueNode_WPList::Handle::cast_dynamic(value_desc.get_value_node());
            if(!value_node)
            {
                error("expected a ValueNode_WPList");
                assert(0);
            }
            ValueNode::Handle bline(value_node->get_bline());
            // it is not possible to place any widthpoint's duck if there is
            // not associated bline.
            if(!bline)
                return false;
            // Retrieve the homogeneous layer parameter
            Layer::Handle layer_parent;
            if(value_desc.parent_is_layer())
                layer_parent=value_desc.get_layer();
            if(layer_parent)
                {
                    String layer_name(layer_parent->get_name());
                    if(layer_name=="advanced_outline")
                        homogeneous=layer_parent->get_param("homogeneous").get(bool());
                }
            int i;
            for (i = 0; i < value_node->link_count(); i++)
            {
                float amount(value_node->list[i].amount_at_time(get_time()));
                // skip width points that aren't fully on
                if (amount < 0.9999f)
                    continue;
                WidthPoint width_point((*value_node->get_link(i))(get_time()).get(WidthPoint()));
                // try casting the width point to Composite - this tells us whether it is composite or not
                ValueNode_Composite::Handle composite_width_point_value_node(
                    ValueNode_Composite::Handle::cast_dynamic(value_node->get_link(i)));
                if(composite_width_point_value_node) // Add the position
                {
                    etl::handle<Duck> pduck=new Duck();
                    synfigapp::ValueDesc wpoint_value_desc(value_node, i); // The i-widthpoint on WPList
                    pduck->set_type(Duck::TYPE_WIDTHPOINT_POSITION);
                    set_duck_value_desc(*pduck, wpoint_value_desc, transform_stack);
                    // This is a quick hack to obtain the ducks position.
                    // The position by amount and the amount by position
                    // has to be written considering the bline length too
                    // optionally
                    ValueNode_BLineCalcVertex::LooseHandle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
                    bline_calc_vertex->set_link("bline", bline);
                    bline_calc_vertex->set_link("loop", ValueNode_Const::create(false));
                    bline_calc_vertex->set_link("amount", ValueNode_Const::create(width_point.get_norm_position(value_node->get_loop())));
                    bline_calc_vertex->set_link("homogeneous", ValueNode_Const::create(homogeneous));
                    pduck->set_point((*bline_calc_vertex)(get_time()).get(Vector()));
                    // hack end
                    pduck->set_editable(synfigapp::is_editable(wpoint_value_desc.get_value_node()));
                    pduck->signal_edited().clear();
                    pduck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), wpoint_value_desc));
                    pduck->signal_user_click(2).clear();
                    pduck->signal_user_click(2).connect(
                        sigc::bind(
                            sigc::bind(
                                sigc::bind(
                                    sigc::mem_fun(
                                        *canvas_view,
                                        &studio::CanvasView::popup_param_menu),
                                    false),
                                1.0f),
                            wpoint_value_desc));
                    add_duck(pduck);
                    if(param_desc)
                    {
                        if(!param_desc->get_origin().empty())
                        {
                            synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                            add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                            pduck->set_origin(last_duck());
                        }
                    }
                    // add the width duck
                    int index=composite_width_point_value_node->get_link_index_from_name("width");
                    if (add_to_ducks(synfigapp::ValueDesc(composite_width_point_value_node,index),canvas_view,transform_stack))
                    {
                        etl::handle<Duck> wduck;
                        wduck=last_duck();
                        wduck->set_origin(pduck);
                        wduck->set_type(Duck::TYPE_WIDTH);
                        // if the composite comes from a layer get the layer's "width" parameter and scale the
                        // duck by that value.
                        if (param_desc)
                        {
                            ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),"width").get_value(get_time()));
                            Real gv(value_desc.get_layer()->get_outline_grow_mark());
                            if(value.same_type_as(synfig::Real()))
                                wduck->set_scalar(exp(gv)*(value.get(synfig::Real())*0.5f));
                            // if it doesn't have a "width" parameter, scale by 0.5f instead
                            else
                                wduck->set_scalar(0.5f);
                        }
                        // otherwise just present the raw unscaled width
                        else
                            wduck->set_scalar(0.5f);
                    }
                    else
                        return false;
                }
            }
            return true;
        }
        else // Check for DynamicList
        if(value_desc.is_value_node() &&
            ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_value_node()))
        {
            ValueNode_DynamicList::Handle value_node;
            value_node=ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_value_node());
            int i;

            if(value_node->get_contained_type()==type_vector)
            {
                Bezier bezier;
                etl::handle<Duck> first_duck, duck;
                int first = -1;
                for(i=0;i<value_node->link_count();i++)
                {
                    if(!value_node->list[i].status_at_time(get_time()))
                        continue;
                    if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
                        return false;
                    duck = last_duck();

                    // remember the index of the first vertex we didn't skip
                    if (first == -1)
                    {
                        first = i;
                        first_duck = duck;
                    }

                    if(param_desc && !param_desc->get_origin().empty())
                    {
                        synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
                        add_to_ducks(value_desc_origin,canvas_view, transform_stack);
                        duck->set_origin(last_duck());
/*
                        ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
                        if(value.same_type_as(synfig::Point()))
                            duck->set_origin(value.get(synfig::Point()));
*/
//                      if(!param_desc->get_origin().empty())
//                          last_duck()->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
                    }
                    duck->set_type(Duck::TYPE_VERTEX);
                    bezier.p1 = bezier.p2;
                    bezier.c1 = bezier.c2;
                    bezier.p2 = duck;
                    bezier.c2 = duck;

                    if (first != i)
                    {
                        handle<Bezier> bezier_(new Bezier());
                        bezier_->p1=bezier.p1;
                        bezier_->c1=bezier.c1;
                        bezier_->p2=bezier.p2;
                        bezier_->c2=bezier.c2;
                        add_bezier(bezier_);
                        last_bezier()->signal_user_click(2).connect(
                            sigc::bind(
                                sigc::mem_fun(
                                    *canvas_view,
                                    &studio::CanvasView::popup_param_menu_bezier),
                                synfigapp::ValueDesc(value_node,i)));
                    }
                }

                if (value_node->get_loop() && first != -1 && first_duck != duck)
                {
                    duck = first_duck;

                    bezier.p1 = bezier.p2;
                    bezier.c1 = bezier.c2;
                    bezier.p2 = duck;
                    bezier.c2 = duck;

                    handle<Bezier> bezier_(new Bezier());
                    bezier_->p1 = bezier.p1;
                    bezier_->c1 = bezier.c1;
                    bezier_->p2 = bezier.p2;
                    bezier_->c2 = bezier.c2;
                    add_bezier(bezier_);
                    last_bezier()->signal_user_click(2).connect(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu_bezier),
                            synfigapp::ValueDesc(value_node,first)));
                }
            }
            else if(value_node->get_contained_type()==type_segment)
            {
                for(i=0;i<value_node->link_count();i++)
                {
                    if(!value_node->list[i].status_at_time(get_time()))
                        continue;
                    if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
                        return false;
                }
            }
            else
                return false;
        }
        else
        {
            // WRITEME
        }

        return true;
    }
    else
    if (type == type_bone_object
     || type == type_bone_valuenode)
    {
        const synfigapp::ValueDesc &orig_value_desc = value_desc;
        ValueNode::Handle value_node(value_desc.get_value_node());

        if (type == type_bone_valuenode)
        {
            assert(value_desc.parent_is_value_node());
            value_node = (*value_node)(get_time()).get(ValueNode_Bone::Handle());
        }
        else
            assert(value_desc.parent_is_linkable_value_node() || value_desc.parent_is_canvas());

        Duck::Handle fake_duck;
        Duck::Handle tip_duck;
        synfig::TransformStack origin_transform_stack(transform_stack), bone_transform_stack;
        bool recursive(get_type_mask() & Duck::TYPE_BONE_RECURSIVE);

        ValueNode_Bone::Handle bone_value_node;
        if (!(bone_value_node = ValueNode_Bone::Handle::cast_dynamic(value_node)))
        {
            error("expected a ValueNode_Bone");
            assert(0);
        }

        synfig::GUID guid(bone_value_node->get_guid());
        Time time(get_time());
        Bone bone((*bone_value_node)(time).get(Bone()));
        bool invertible(true);
        Angle angle;
        Angle::deg parent_angle(0);

        {
            Matrix transform;
            bool has_parent(!bone.is_root());
            if (has_parent)
            {
                Bone parent_bone((*bone.get_parent())(time).get(Bone()));

                // add the parent's ducks too
                add_to_ducks(synfigapp::ValueDesc(bone_value_node, bone_value_node->get_link_index_from_name("parent"), value_desc),canvas_view,transform_stack);

                transform = parent_bone.get_animated_matrix();
                origin_transform_stack.push(new Transform_Matrix(guid, transform));
                bone_transform_stack = origin_transform_stack;
                invertible = transform.is_invertible();

                Vector scale(parent_bone.get_local_scale());
                bone_transform_stack.push(new Transform_Translate(guid, Point((scale[0])*bone.get_origin()[0],
                                                                              (scale[1])*bone.get_origin()[1])));
                origin_transform_stack.push(new Transform_Scale(guid, scale));

#ifdef TRY_TO_ALIGN_WIDTH_DUCKS
                // this stuff doesn't work very well - we can find out
                // the cumulative angle and so place the duck on the
                // right of the circle, but recursive scales in the
                // parent can cause the radius to look wrong, and the
                // duck to appear off-center
                printf("%s:%d bone %s:\n", __FILE__, __LINE__, bone.get_name().c_str());
                while (true) {
                    printf("%s:%d parent_angle = %5.2f + %5.2f = %5.2f\n", __FILE__, __LINE__,
                           Angle::deg(parent_angle).get(), Angle::deg(parent_bone.get_angle()).get(),
                           Angle::deg(parent_angle + (parent_bone.get_angle())).get());
                    parent_angle += parent_bone.get_angle();
                    if (parent_bone.is_root()) break;
                    parent_bone = (*parent_bone.get_parent())(time).get(Bone());
                }
                printf("%s:%d finally %5.2f\n\n", __FILE__, __LINE__, Angle::deg(parent_angle).get());
#endif
            }
            else
            {
                bone_transform_stack = origin_transform_stack;
                bone_transform_stack.push(new Transform_Translate(guid, bone.get_origin()));
            }
        }

        bone_transform_stack.push(new Transform_Rotate(guid, bone.get_angle()));

        // origin
        {
            synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name("origin"), orig_value_desc);

            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_POSITION);
            set_duck_value_desc(*duck, value_desc, origin_transform_stack);
            duck->set_point(value_desc.get_value(time).get(Point()));

            // if the ValueNode can be directly manipulated, then set it as so
            duck->set_editable(!invertible ? false :
                               !value_desc.is_value_node() ? true :
                               synfigapp::is_editable(value_desc.get_value_node()));

            duck->signal_edited().clear();
            duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
            duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
                                                                                              &studio::CanvasView::popup_param_menu),
                                                                                false), // bezier
                                                                     0.0f),             // location
                                                          value_desc));                 // value_desc
            add_duck(duck);
        }

        // fake
        {
            synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name("name"), orig_value_desc);

            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_NONE);
            set_duck_value_desc(*duck, value_desc, bone_transform_stack);
            duck->set_point(Point(0, 0));

            duck->set_ignore(true);
            add_duck(duck);
            fake_duck = last_duck();
        }

        // angle
        {
            synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name("angle"), orig_value_desc);

            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_ANGLE);
            set_duck_value_desc(*duck, value_desc, bone_transform_stack);

            angle = value_desc.get_value(time).get(Angle());
            Real length(bone.get_length() * (bone.get_scalex() * bone.get_scalelx()));
            duck->set_point(Point(length*0.9, 0));

            // if the ValueNode can be directly manipulated, then set it as so
            duck->set_editable(!value_desc.is_value_node() ? true :
                               synfigapp::is_editable(value_desc.get_value_node()));

            duck->signal_edited().clear();
            duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
            duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
                                                                                              &studio::CanvasView::popup_param_menu),
                                                                                false), // bezier
                                                                     0.0f),             // location
                                                          value_desc));                 // value_desc
            duck->set_origin(fake_duck);
            add_duck(duck);
        }

        // tip
        {
            synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name(recursive ? "scalex" : "scalelx"), orig_value_desc);

            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_VERTEX);
            set_duck_value_desc(*duck, value_desc, bone_transform_stack);
            //Real length = bone.get_length()*bone.get_scalex()*bone.get_scalelx();
            Real length = value_desc.get_value(time).get(Real());
            duck->set_point(Vector(length, 0.0));

            // if the ValueNode can be directly manipulated, then set it as so
            duck->set_editable(!invertible ? false :
                               !value_desc.is_value_node() ? true :
                               synfigapp::is_editable(value_desc.get_value_node()));

            duck->signal_edited().clear();
            duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
            duck->signal_user_click(2).connect(
                sigc::bind(
                    sigc::bind(
                        sigc::bind(
                            sigc::mem_fun(
                                *canvas_view,
                                &studio::CanvasView::popup_param_menu
                            ),
                            false // bezier
                        ),
                        0.0f // location
                    ),
                    value_desc
                )
            );
            duck->set_origin(fake_duck);
            add_duck(duck);
            tip_duck = last_duck();
        }

        // origin width
        {

            synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name("width"), orig_value_desc);

            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_WIDTH);
            set_duck_value_desc(*duck, value_desc, bone_transform_stack);
            duck->set_radius(true);
            duck->set_scalar(1);
            duck->set_point(Point(0, value_desc.get_value(time).get(Real())));

            // if the ValueNode can be directly manipulated, then set it as so
            duck->set_editable(!invertible ? false :
                               !value_desc.is_value_node() ? true :
                               synfigapp::is_editable(value_desc.get_value_node()));

            duck->signal_edited().clear();
            duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
            duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
                            &studio::CanvasView::popup_param_menu),
                        false), // bezier
                    0.0f),              // location
                value_desc));                   // value_desc
            duck->set_origin(fake_duck);
            add_duck(duck);
        }

        // tip width
        {

            synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name("tipwidth"), orig_value_desc);

            etl::handle<Duck> duck=new Duck();
            duck->set_type(Duck::TYPE_WIDTH);
            set_duck_value_desc(*duck, value_desc, bone_transform_stack);
            duck->set_radius(true);
            duck->set_scalar(1);
            duck->set_point(Point(0, value_desc.get_value(time).get(Real())));

            // if the ValueNode can be directly manipulated, then set it as so
            duck->set_editable(!invertible ? false :
                               !value_desc.is_value_node() ? true :
                               synfigapp::is_editable(value_desc.get_value_node()));

            duck->signal_edited().clear();
            duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
            duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
                            &studio::CanvasView::popup_param_menu),
                        false), // bezier
                    0.0f),              // location
                value_desc));                   // value_desc
            duck->set_origin(tip_duck);
            add_duck(duck);
        }

        return true;
    }
    else
    if (type == type_bone_weight_pair)
    {
        ValueNode_BoneWeightPair::Handle value_node;
        if(value_desc.is_value_node() &&
           (value_node=ValueNode_BoneWeightPair::Handle::cast_dynamic(value_desc.get_value_node())))
            add_to_ducks(synfigapp::ValueDesc(value_node, value_node->get_link_index_from_name("bone"), value_desc), canvas_view, transform_stack);
    }

    return false;
}


/*
-- ** -- DuckDrag_Translate M E T H O D S----------------------------
*/
void
DuckDrag_Translate::begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& offset)
{
	is_moving = false;

	last_translate_=Vector(0,0);
	{
		drag_offset_=duckmatic->find_duck(offset)->get_trans_point();

		snap=Vector(0,0);
	}

	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	positions.clear();
	for(iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		Point p((*iter)->get_trans_point());
		positions.push_back(p);
	}
}

bool
DuckDrag_Translate::end_duck_drag(Duckmatic* duckmatic)
{
	if(is_moving)
	{
		duckmatic->signal_edited_selected_ducks();
		return true;
	}
	else
	{
		duckmatic->signal_user_click_selected_ducks(0);
		return false;
	}
}

void
DuckDrag_Translate::duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	const DuckList selected_ducks(duckmatic->get_selected_ducks());
	DuckList::const_iterator iter;

	synfig::Vector vect(duckmatic->snap_point_to_grid(vector)-drag_offset_);
	int i;
	Time time(duckmatic->get_time());

	// drag the vertex and position ducks first
	for (i=0,iter=selected_ducks.begin(); iter!=selected_ducks.end(); ++iter,i++)
		if((*iter)->get_type() == Duck::TYPE_VERTEX || (*iter)->get_type() == Duck::TYPE_POSITION)
			(*iter)->set_trans_point(positions[i]+vect, time);

	// then drag the others
	for (i=0,iter=selected_ducks.begin(); iter!=selected_ducks.end(); ++iter,i++)
		if ((*iter)->get_type() != Duck::TYPE_VERTEX && (*iter)->get_type() != Duck::TYPE_POSITION)
			(*iter)->set_trans_point(positions[i]+vect, time);

	last_translate_=vect;

	if(last_translate_.mag()>0.0001)
		is_moving = true;

	if (is_moving)
		duckmatic->signal_edited_selected_ducks(true);

	// then patch up the tangents for the vertices we've moved
	duckmatic->update_ducks();
}

/*
-- ** -- BezierDrag_Default M E T H O D S----------------------------
*/
void
BezierDrag_Default::begin_bezier_drag(Duckmatic* duckmatic, const synfig::Vector& offset, float bezier_click_pos)
{
	is_moving = false;
	drag_offset_=offset;
	click_pos_=bezier_click_pos;

	etl::handle<Duck> c1(duckmatic->get_selected_bezier()->c1);
	etl::handle<Duck> c2(duckmatic->get_selected_bezier()->c2);

	c1_initial = c1->get_trans_point();
	c2_initial = c2->get_trans_point();
	last_translate_ = synfig::Vector(0,0);

	if (c1 == duckmatic->get_selected_bezier()->p1
		&& c2 == duckmatic->get_selected_bezier()->p2)
	{
		// This is a polygon segment
		// We can't bend the curve, so drag it instead
		c1_ratio = 1.0;
		c2_ratio = 1.0;

	}
	else
	{
		// This is a bline segment, so we can bend the curve

		// Magic Bezier Drag Equations follow! (stolen from Inkscape)
		// "weight" describes how the influence of the drag should be
		// distributed among the handles;
		// 0 = front handle only, 1 = back handle only.

		float t = bezier_click_pos;
		float weight;
		if (t <= 1.0/6.0 ) weight=0;
		else if (t <= 0.5 ) weight = (pow((6 * t - 1) / 2.0, 3)) / 2;
		else if (t <= 5.0 / 6.0) weight = (1 - pow((6 * (1-t) - 1) / 2.0, 3)) / 2 + 0.5;
		else weight = 1;

		c1_ratio = (1-weight)/(3*t*(1-t)*(1-t));
		c2_ratio = weight/(3*t*t*(1-t));
	}
}

void
BezierDrag_Default::bezier_drag(Duckmatic* duckmatic, const synfig::Vector& vector)
{
	synfig::Vector vect(duckmatic->snap_point_to_grid(vector)-drag_offset_);
	Time time(duckmatic->get_time());

	synfig::Vector c1_offset(vect[0]*c1_ratio, vect[1]*c1_ratio);
	synfig::Vector c2_offset(vect[0]*c2_ratio, vect[1]*c2_ratio);

	etl::handle<Duck> c1(duckmatic->get_selected_bezier()->c1);
	etl::handle<Duck> c2(duckmatic->get_selected_bezier()->c2);

	c1->set_trans_point(c1_initial+c1_offset, time);
	c2->set_trans_point(c2_initial+c2_offset, time);

	last_translate_=vect;
	if(last_translate_.mag()>0.0001)
		is_moving = true;

	if (is_moving) {
		duckmatic->signal_edited_duck(c1, true);
		duckmatic->signal_edited_duck(c2, true);
	}

	duckmatic->update_ducks();
}

bool
BezierDrag_Default::end_bezier_drag(Duckmatic* duckmatic)
{
	if(is_moving)
	{
		etl::handle<Duck> c1(duckmatic->get_selected_bezier()->c1);
		etl::handle<Duck> c2(duckmatic->get_selected_bezier()->c2);

		duckmatic->signal_edited_duck(c1);
		duckmatic->signal_edited_duck(c2);

		return true;
	}
	else
	{
		return false;
	}
}

/*
-- ** -- Duckmatic::Push M E T H O D S----------------------------
*/
Duckmatic::Push::Push(Duckmatic *duckmatic_):
	duckmatic_(duckmatic_)
{
	duck_map=duckmatic_->duck_map;
	bezier_list_=duckmatic_->bezier_list_;
	duck_data_share_map=duckmatic_->duck_data_share_map;
	stroke_list_=duckmatic_->stroke_list_;
	duck_dragger_=duckmatic_->duck_dragger_;
	needs_restore=true;
}

Duckmatic::Push::~Push()
{
	if(needs_restore)
		restore();
}

void
Duckmatic::Push::restore()
{
	duckmatic_->duck_map=duck_map;
	duckmatic_->bezier_list_=bezier_list_;
	duckmatic_->duck_data_share_map=duck_data_share_map;
	duckmatic_->stroke_list_=stroke_list_;
	duckmatic_->duck_dragger_=duck_dragger_;
	needs_restore=false;
}

/*
-- ** -- inline M E T H O D S----------------------------
*/
inline synfig::GUID calc_duck_guid(const synfigapp::ValueDesc& value_desc, const synfig::TransformStack& transform_stack)
{
	return value_desc.get_guid() % transform_stack.get_guid();
}

//! sets duck name, value_desc, transform_stack and GUID
inline void set_duck_value_desc(Duck& duck, const synfigapp::ValueDesc& value_desc, const synfig::TransformStack& transform_stack)
{
	duck.set_name(value_desc.get_guid_string());
	duck.set_value_desc(value_desc);
	duck.set_transform_stack(transform_stack);
	duck.set_guid(calc_duck_guid(value_desc, transform_stack));
}

//! sets duck name, value_desc and GUID
inline void set_duck_value_desc(Duck& duck, const synfigapp::ValueDesc& value_desc, const synfig::String& sub_name, const synfig::TransformStack& transform_stack)
{
	set_duck_value_desc(duck, value_desc.get_sub_value(sub_name), transform_stack);
}
