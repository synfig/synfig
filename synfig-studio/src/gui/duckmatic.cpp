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

#include "duckmatic.h"
#include "ducktransform_scale.h"
#include "ducktransform_translate.h"
#include "ducktransform_rotate.h"
#include <synfigapp/value_desc.h>
#include <synfigapp/canvasinterface.h>
#include <synfig/general.h>
#include <synfig/paramdesc.h>
#include <synfig/valuenode_timedswap.h>
#include <synfig/valuenode_animated.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_range.h>
#include <synfig/valuenode_scale.h>
#include <synfig/valuenode_bline.h>
#include <synfig/valuenode_wplist.h>
#include <synfig/valuenode_blinecalctangent.h>
#include <synfig/valuenode_blinecalcvertex.h>
#include <synfig/valuenode_blinecalcwidth.h>
#include <synfig/valuenode_staticlist.h>
#include <synfig/valuenode_bone.h>
#include <synfig/valuenode_boneinfluence.h>
#include <synfig/valuenode_boneweightpair.h>

#include <synfig/curve_helper.h>

#include <synfig/context.h>
#include <synfig/layer_pastecanvas.h>

#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
#include <sigc++/hide.h>
#include <sigc++/bind.h>

#include "ducktransform_matrix.h"
#include "ducktransform_rotate.h"
#include "ducktransform_translate.h"
#include "ducktransform_scale.h"
#include "ducktransform_origin.h"
#include "canvasview.h"

#include "onemoment.h"

#include "general.h"

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

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Duckmatic::Duckmatic(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface):
	canvas_interface(canvas_interface),
	type_mask(Duck::TYPE_ALL-Duck::TYPE_WIDTH-Duck::TYPE_BONE_SETUP-Duck::TYPE_BONE_RECURSIVE-Duck::TYPE_WIDTHPOINT_POSITION),
	grid_snap(false),
	guide_snap(false),
	grid_size(1.0/4.0,1.0/4.0),
	grid_color(synfig::Color(159.0/255.0,159.0/255.0,159.0/255.0)),
	show_persistent_strokes(true)
{
	axis_lock=false;
	drag_offset_=Point(0,0);
	clear_duck_dragger();
	clear_bezier_dragger();
	zoom=prev_zoom=1.0;
}

Duckmatic::~Duckmatic()
{
	clear_ducks();

	if (Duck::duck_count)
		synfig::error("%d ducks not yet deleted!", Duck::duck_count);

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


bool
Duckmatic::duck_is_selected(const etl::handle<Duck> &duck)const
{
	return duck && selected_ducks.count(duck->get_guid());
}

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
	if(value_desc.parent_is_layer_param() && type & Duck::TYPE_POSITION)
	{
		Layer::Handle layer(value_desc.get_layer());
		String layer_name(layer->get_name());

		if (layer_name == "outline" || layer_name == "region" || layer_name == "plant" ||
			layer_name == "polygon" || layer_name == "curve_gradient" || layer_name == "advanced_outline")
			return false;

		if((layer_name=="PasteCanvas"|| layer_name=="paste_canvas") &&
		   !layer->get_param("children_lock").get(bool()))
			return false;
	}
	else if (value_desc.parent_is_value_node())
	{
		if (ValueNode_BLineCalcVertex::Handle::cast_dynamic(value_desc.get_value_node()))
			return false;
		if (value_desc.parent_is_linkable_value_node())
		{
			LinkableValueNode::Handle parent_value_node(value_desc.get_parent_value_node());
			if (ValueNode_Composite::Handle::cast_dynamic(parent_value_node))
			{
				if (parent_value_node->get_type() == ValueBase::TYPE_BLINEPOINT &&
					ValueNode_BLineCalcVertex::Handle::cast_dynamic(
						parent_value_node->get_link("point")))
					return false;
				// widths ducks of the widthpoints
				// Do not avoid selection of the width ducks from widthpoints
				//if (parent_value_node->get_type() == ValueBase::TYPE_WIDTHPOINT)
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

	DuckMap::const_iterator iter;
	for(iter=duck_map.begin();iter!=duck_map.end();++iter)
	{
		Point p(iter->second->get_trans_point());
		if(p[0]<=vmax[0] && p[0]>=vmin[0] && p[1]<=vmax[1] && p[1]>=vmin[1] &&
		   is_duck_group_selectable(iter->second))
			toggle_select_duck(iter->second);
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
			if(composite && composite->get_type() == ValueBase::TYPE_BLINEPOINT && duck_value_node)
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
							BLinePoint bp=(*composite)(time);
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
			if(composite && composite->get_type() == ValueBase::TYPE_BLINEPOINT && duck_value_node)
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
							BLinePoint bp=(*composite)(time);
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
			ValueNode_BLineCalcVertex::Handle bline_vertex;
			ValueNode_Composite::Handle composite;

			if ((bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(duck->get_value_desc().get_value_node())) ||
				((composite = ValueNode_Composite::Handle::cast_dynamic(duck->get_value_desc().get_value_node())) &&
				 composite->get_type() == ValueBase::TYPE_BLINEPOINT &&
				 (bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(composite->get_link("point")))))
			{
				DuckList::iterator iter;
				for (iter=duck_list.begin(); iter!=duck_list.end(); iter++)
					if ( (*iter)->get_origin_duck()==duck  /*&& !duck_is_selected(*iter)*/ )
					{
						synfig::Real radius = 0.0;
						ValueNode_BLine::Handle bline(ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link("bline")));
						Real amount = synfig::find_closest_point((*bline)(time), duck->get_point(), radius, bline->get_loop());
						bool homogeneous((*(bline_vertex->get_link("homogeneous")))(time).get(bool()));
						if(homogeneous)
							amount=std_to_hom((*bline)(time), amount, ((*(bline_vertex->get_link("loop")))(time).get(bool())), bline->get_loop() );
						ValueNode::Handle vertex_amount_value_node(bline_vertex->get_link("amount"));


						ValueNode::Handle duck_value_node((*iter)->get_value_desc().get_value_node());
						if (ValueNode_BLineCalcTangent::Handle bline_tangent = ValueNode_BLineCalcTangent::Handle::cast_dynamic(duck_value_node))
						{
							if (bline_tangent->get_link("amount") == vertex_amount_value_node)
							{
								switch (bline_tangent->get_type())
								{
								case ValueBase::TYPE_ANGLE:
								{
									Angle angle((*bline_tangent)(time, amount).get(Angle()));
									(*iter)->set_point(Point(Angle::cos(angle).get(), Angle::sin(angle).get()));
									(*iter)->set_rotations(Angle::deg(0)); //hack: rotations are a relative value
									break;
								}
								case ValueBase::TYPE_REAL:
									(*iter)->set_point(Point((*bline_tangent)(time, amount).get(Real()), 0));
									break;
								case ValueBase::TYPE_VECTOR:
									(*iter)->set_point((*bline_tangent)(time, amount).get(Vector()));
									break;
								default:
									break;
								}
							}
						}
						else if (ValueNode_BLineCalcWidth::Handle bline_width = ValueNode_BLineCalcWidth::Handle::cast_dynamic(duck_value_node))
						{
							if (bline_width->get_link("amount") == vertex_amount_value_node)
								(*iter)->set_point(Point((*bline_width)(time, amount).get(Real()), 0));
						}
					}
			}
		}
		// We are moving a tangent handle
		else if( duck->get_type() == Duck::TYPE_TANGENT)
		{
			if(duck->get_value_desc().parent_is_linkable_value_node())
			{
				ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(duck->get_value_desc().get_parent_value_node()));
				LinkableValueNode::Handle duck_value_node(LinkableValueNode::Handle::cast_dynamic(duck->get_value_desc().get_value_node()));
				// it belongs to a composite and it is a BLinePoint
				if(composite && composite->get_type() == ValueBase::TYPE_BLINEPOINT && duck_value_node)
				{
					int index(duck->get_value_desc().get_index());
					etl::handle<Duck> origin_duck=duck->get_origin_duck();
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
								// Check if the other tangent is also selected, in that case
								// it is going to be moved itself so don't update it.
								bool selected=false;
								DuckList::const_iterator iter2;
								for(iter2=selected_ducks.begin(); iter2!=selected_ducks.end(); ++iter2)
									if(*iter == *iter2)
										selected=true;
								if(!selected)
								{
									BLinePoint bp=(*composite)(time);
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


bool
Duckmatic::end_duck_drag()
{
	if(duck_dragger_)
		return duck_dragger_->end_duck_drag(this);
	return false;
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
	else if (duck->is_linear())
	{
		Point point(duck->get_point());
		Angle constrained_angle(duck->get_linear_angle());
		Angle difference(Angle::tan(point[1], point[0])-constrained_angle);
		Real length(Angle::cos(difference).get()*point.mag());
		if (length < 0) length = 0;
		point[0] = length * Angle::cos(constrained_angle).get();
		point[1] = length * Angle::sin(constrained_angle).get();
		duck->set_point(point);

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
	smart_ptr<OneMoment> wait; if(ducks.size()>20)wait.spawn();
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
			throw;
		}
	}
	selected_ducks=old_set;
}

bool
Duckmatic::on_duck_changed(const studio::Duck &duck,const synfigapp::ValueDesc& value_desc)
{
	synfig::Point value=duck.get_point();
	switch(value_desc.get_value_type())
	{
	case ValueBase::TYPE_REAL:
		// Zoom duck value (PasteCanvas and Zoom layers) should be
		// converted back from exponent to normal
		if( duck.get_exponential() ) {
			return canvas_interface->change_value(value_desc,log(value.mag()));
		} else {
			return canvas_interface->change_value(value_desc,value.mag());
		}
	case ValueBase::TYPE_ANGLE:
		//return canvas_interface->change_value(value_desc,Angle::tan(value[1],value[0]));
		return canvas_interface->change_value(value_desc, value_desc.get_value(get_time()).get(Angle()) + duck.get_rotations());
	default:
		return canvas_interface->change_value(value_desc,value);
	}
}

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

/*	std::list<handle<Duck> >::reverse_iterator iter;

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
			else if (!equal && ret_vector.size())
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

	bezier<Point>	curve;

	Real 	d,step;
	float 	time = 0;
	float 	best_time = 0;

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
		d = (curve[1] - curve[0]).mag() + (curve[2]-curve[1]).mag()	+ (curve[3]-curve[2]).mag();

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
			*location = best_time;	// We need to square-root this because we were dealing with squared distances

		return ret;
	}

	return 0;
}

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

inline String guid_string(const synfigapp::ValueDesc& x)
{
	if(x.parent_is_layer_param())
		return strprintf("%s",x.get_layer()->get_guid().get_string().c_str())+x.get_param_name();
	//if(x.is_value_node())
		return strprintf("%s",x.get_value_node()->get_guid().get_string().c_str());
}

inline synfig::GUID calc_duck_guid(const synfigapp::ValueDesc& x,const synfig::TransformStack& transform_stack)
{
	synfig::GUID ret(0);

	if(x.parent_is_layer_param())
	{
		ret=x.get_layer()->get_guid()^synfig::GUID::hasher(x.get_param_name());
	}
	else
	{
		ret=x.get_value_node()->get_guid();
	}

	ret^=transform_stack.get_guid();
	return ret;
}

/*
Duck::Handle
Duckmatic::create_duck_from(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack, int modifier, synfig::ParamDesc *param_desc)
{
	synfig::GUID duck_guid(calc_duck_guid(value_desc,transform_stack)^synfig::GUID::hasher(modifier));
	etl::handle<Duck> duck=new Duck();

	return duck;
}
*/

void
Duckmatic::add_ducks_layers(synfig::Canvas::Handle canvas, std::set<synfig::Layer::Handle>& selected_layer_set, etl::handle<CanvasView> canvas_view, synfig::TransformStack& transform_stack)
{
	int transforms(0);
	String layer_name;

#define QUEUE_REBUILD_DUCKS		sigc::mem_fun(*canvas_view,&CanvasView::queue_rebuild_ducks)

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
		if(layer_name=="PasteCanvas")
		{
			Vector scale;
			scale[0]=scale[1]=exp(layer->get_param("zoom").get(Real()));
			Vector origin(layer->get_param("origin").get(Vector()));

			Canvas::Handle child_canvas(layer->get_param("canvas").get(Canvas::Handle()));
			Vector focus(layer->get_param("focus").get(Vector()));

			if(!scale.is_equal_to(Vector(1,1)))
				transform_stack.push(new Transform_Scale(layer->get_guid(), scale,origin+focus));
			if(!origin.is_equal_to(Vector(0,0)))
				transform_stack.push(new Transform_Translate(layer->get_guid(), origin));

			add_ducks_layers(child_canvas,selected_layer_set,canvas_view,transform_stack);

			if(!origin.is_equal_to(Vector(0,0)))
				transform_stack.pop();
			if(!scale.is_equal_to(Vector(1,1)))
				transform_stack.pop();
		}
	}
	// Remove all of the transforms we have added
	while(transforms--) { transform_stack.pop(); }

#undef QUEUE_REBUILD_DUCKS
}


bool
Duckmatic::add_to_ducks(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack, synfig::ParamDesc *param_desc, int multiple)
{
	ValueBase::Type type=value_desc.get_value_type();
#define REAL_COOKIE		reinterpret_cast<synfig::ParamDesc*>(28)
	
	switch(type)
	{
	case ValueBase::TYPE_REAL:
		if(!param_desc || param_desc==REAL_COOKIE || !param_desc->get_origin().empty())
		{
			etl::handle<Duck> duck=new Duck();
			duck->set_transform_stack(transform_stack);
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
			duck->set_name(guid_string(value_desc));
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
					String layer_name=layer->get_name();
					if(layer_name=="PasteCanvas")
					{
						Vector focus(layer->get_param("focus").get(Vector()));
						duck->set_origin(last_duck()->get_point() + focus);
					}
					else
						duck->set_origin(last_duck());
				}
				duck->set_scalar(param_desc->get_scalar());
			}

			duck->signal_edited().clear(); // value_desc.get_value_type() == ValueBase::TYPE_REAL:
			duck->signal_edited().connect(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&studio::Duckmatic::on_duck_changed),
					value_desc));
			duck->set_value_desc(value_desc);

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

			duck->set_guid(calc_duck_guid(value_desc,transform_stack)^synfig::GUID::hasher(multiple));

			add_duck(duck);

			return true;
		}
		break;

	case ValueBase::TYPE_ANGLE:

		if(!param_desc || param_desc==REAL_COOKIE || !param_desc->get_origin().empty())
		{
			etl::handle<Duck> duck=new Duck();
			duck->set_type(Duck::TYPE_ANGLE);
			duck->set_transform_stack(transform_stack);
			synfig::Angle angle;

			angle=value_desc.get_value(get_time()).get(Angle());
			duck->set_point(Point(Angle::cos(angle).get(),Angle::sin(angle).get()));
			duck->set_name(guid_string(value_desc));
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

			duck->signal_edited().clear(); // value_desc.get_value_type() == ValueBase::TYPE_ANGLE:
			duck->signal_edited().clear();
			duck->signal_edited().connect(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&studio::Duckmatic::on_duck_changed),
					value_desc));
			duck->set_value_desc(value_desc);

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
			duck->set_guid(calc_duck_guid(value_desc,transform_stack)^synfig::GUID::hasher(multiple));

			add_duck(duck);

			return true;
		}
		break;

	case ValueBase::TYPE_VECTOR:
		{
			etl::handle<Duck> duck=new Duck();
			duck->set_transform_stack(transform_stack);
			duck->set_name(guid_string(value_desc));
			ValueNode_Composite::Handle blinepoint_value_node;
			int index;
			bool done(false);
			if(value_desc.parent_is_linkable_value_node()
			   &&
			   value_desc.get_parent_value_node()->get_type() == ValueBase::TYPE_BLINEPOINT)
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
					if(duck_map.find(calc_duck_guid(value_desc_origin,transform_stack)^synfig::GUID::hasher(0))!=duck_map.end())
					{
						connect_duck=duck_map[calc_duck_guid(value_desc_origin,transform_stack)^synfig::GUID::hasher(0)];
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

			duck->signal_edited().clear(); // value_desc.get_value_type() == ValueBase::TYPE_VECTOR:
			duck->signal_edited().connect(
				sigc::bind(
					sigc::mem_fun(
						*this,
						&studio::Duckmatic::on_duck_changed),
					value_desc));
			duck->set_value_desc(value_desc);

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
			duck->set_guid(calc_duck_guid(value_desc,transform_stack)^synfig::GUID::hasher(multiple));
			add_duck(duck);

			return true;
		}
		break;
	case ValueBase::TYPE_SEGMENT:
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
				Segment segment=value_desc.get_value();
				etl::handle<Duck> duck_p,duck_c;
				synfig::String name;
				if(param_desc)
				{
					name=param_desc->get_local_name();
				}
				else
				{
					name=guid_string(value_desc);
				}

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
		break;
	case ValueBase::TYPE_BLINEPOINT:
	{
		int index;
		if(value_desc.is_value_node() &&
			ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueNode_Composite::Handle value_node;
			value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());
			index=value_node->get_link_index_from_name("p");
			if(!add_to_ducks(synfigapp::ValueDesc(value_node,index),canvas_view,transform_stack))
				return false;
			etl::handle<Duck> vertex_duck(last_duck());
			vertex_duck->set_type(Duck::TYPE_VERTEX);
			index=value_node->get_link_index_from_name("t1");
			if(!add_to_ducks(synfigapp::ValueDesc(value_node,index,-TANGENT_HANDLE_SCALE),canvas_view,transform_stack))
				return false;
			etl::handle<Duck> t1_duck(last_duck());

			t1_duck->set_origin(vertex_duck);
			t1_duck->set_scalar(-TANGENT_HANDLE_SCALE);
			t1_duck->set_tangent(true);

			etl::handle<Duck> t2_duck;

			index=value_node->get_link_index_from_name("t2");
			// If the tangents are split
			if((*value_node->get_link("split"))(get_time()).get(bool()))
			{
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,index,TANGENT_HANDLE_SCALE),canvas_view,transform_stack))
					return false;
				t2_duck=last_duck();
				t2_duck->set_origin(vertex_duck);
				t2_duck->set_scalar(TANGENT_HANDLE_SCALE);
				t2_duck->set_tangent(true);
			}
			else
			{
				if(!add_to_ducks(synfigapp::ValueDesc(value_node,index,TANGENT_HANDLE_SCALE),canvas_view,transform_stack))
					return false;
				t2_duck=last_duck();
				t2_duck->set_origin(vertex_duck);
				t2_duck->set_scalar(TANGENT_HANDLE_SCALE);
				t2_duck->set_tangent(true);
			}
			return true;
		}

	}
	break;
	case ValueBase::TYPE_LIST:
	{
		// Check for BLine
		if (value_desc.is_value_node() &&
			ValueNode_BLine::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueNode_BLine::Handle value_node;
			value_node=ValueNode_BLine::Handle::cast_dynamic(value_desc.get_value_node());

			int i,first=-1;

			etl::handle<Bezier> bezier;
			etl::handle<Duck> first_duck;
			etl::handle<Duck> duck, tduck;

			for (i = 0; i < value_node->link_count(); i++)
			{
				float amount(value_node->list[i].amount_at_time(get_time()));

				// skip vertices that aren't fully on
				if (amount < 0.9999f)
					continue;

				// remember the index of the first vertex we didn't skip
				if (first == -1)
					first = i;

				// if it's neither a BoneInfluence nor a Composite, the BLinePoint will be used
				BLinePoint bline_point((*value_node->get_link(i))(get_time()));

				// set if we are editing a boneinfluence node
				ValueNode_BoneInfluence::Handle bone_influence_vertex_value_node(
					ValueNode_BoneInfluence::Handle::cast_dynamic(value_node->get_link(i)));

				// set if we are editing a composite node or a boneinfluence node in setup mode
				ValueNode_Composite::Handle composite_vertex_value_node (
					ValueNode_Composite::Handle::cast_dynamic(value_node->get_link(i)) );

				// set if we are editing a boneinfluence node with a composite link in non-setup mode
				ValueNode_Composite::Handle composite_bone_link_value_node;
				synfig::TransformStack bone_transform_stack(transform_stack);

				if (bone_influence_vertex_value_node)
				{
					if(get_type_mask() & Duck::TYPE_BONE_SETUP)
					{
						// If in setup mode, add the original ducks prior to the bones transformation
						composite_vertex_value_node = ValueNode_Composite::Handle::cast_dynamic(
							bone_influence_vertex_value_node->get_link("link") );
					}
					else
					{
						// If not in setup mode, apply bones transformation to the ducks
						composite_bone_link_value_node = ValueNode_Composite::Handle::cast_dynamic(
							bone_influence_vertex_value_node->get_link("link") );

						if(param_desc)
						{
							if(!param_desc->get_origin().empty())
							{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								GUID guid(calc_duck_guid(value_desc_origin, transform_stack));
								bone_transform_stack.push(new Transform_Origin(guid^synfig::GUID::hasher(".o"), last_duck()));
							}
						}

						Matrix transform(bone_influence_vertex_value_node->calculate_transform(get_time()));
						GUID guid(bone_influence_vertex_value_node->get_link("bone_weight_list")->get_guid());

						bone_transform_stack.push(new Transform_Matrix(guid, transform));

						// this environmental variable affects bone functionality in core
						// \todo remove it, as it is now defunct
						assert(!getenv("SYNFIG_COMPLEX_TANGENT_BONE_INFLUENCE"));
					}
				}


				// Now add the ducks:

				// ----Bones ducks
				if (bone_influence_vertex_value_node)
				{
					// The bones ducks should be transformed to match the position of this bline,
					// and then translated along with the origin of this layer
					synfig::TransformStack layer_transform_stack(transform_stack);
					if(param_desc)
					{
						if(!param_desc->get_origin().empty())
						{
							synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
							add_to_ducks(value_desc_origin,canvas_view, transform_stack);
							GUID guid(calc_duck_guid(value_desc_origin, transform_stack));
							layer_transform_stack.push(new Transform_Origin(guid^synfig::GUID::hasher(".o"), last_duck()));
						}
					}

					add_to_ducks(synfigapp::ValueDesc(bone_influence_vertex_value_node,
									  bone_influence_vertex_value_node->get_link_index_from_name("bone_weight_list")),
								 canvas_view,layer_transform_stack);
				}

				// ----Vertex Duck
				if(composite_vertex_value_node)
				{
					int index=composite_vertex_value_node->get_link_index_from_name("p");
					if (add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,index),canvas_view,transform_stack))
					{
						duck=last_duck();
						if(i==first)
							first_duck=duck;
						duck->set_type(Duck::TYPE_VERTEX);

						duck->signal_user_click(2).clear();
						duck->signal_user_click(2).connect(
							sigc::bind(
								sigc::bind(
									sigc::bind(
										sigc::mem_fun(
											*canvas_view,
											&studio::CanvasView::popup_param_menu),
										false),
									1.0f),
								synfigapp::ValueDesc(value_node,i)));
						duck->set_value_desc(synfigapp::ValueDesc(value_node,i));

						if(param_desc)
						{
							if(!param_desc->get_origin().empty())
							{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								duck->set_origin(last_duck());
/*
								ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
								if(value.same_type_as(synfig::Point()))
									duck->set_origin(value.get(synfig::Point()));
*/
							}
						}
					}
					else
						return false;
				}
				else
				if (composite_bone_link_value_node)
				{
					int index=composite_bone_link_value_node->get_link_index_from_name("p");
					if (add_to_ducks(synfigapp::ValueDesc(composite_bone_link_value_node,index),canvas_view,bone_transform_stack))
					{
						duck=last_duck();
						if(i==first)
							first_duck=duck;
						duck->set_type(Duck::TYPE_VERTEX);

						// Do not add origin duck, as it has already been added
						// and made a part of the transformation stack
					}
					else
						return false;
				}
				// if it's not a composite or BoneInfluence with composite link
				else
				{
					duck=new Duck(bline_point.get_vertex());
					if(i==first)
						first_duck=duck;
					duck->set_transform_stack(transform_stack);
					duck->set_editable(false);
					//duck->set_name(strprintf("%x-vertex",value_node->get_link(i).get()));
					duck->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".v");

					duck->set_type(Duck::TYPE_VERTEX);
					if(param_desc)
					{
						if(!param_desc->get_origin().empty())
						{
							synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
							add_to_ducks(value_desc_origin,canvas_view, transform_stack);
							duck->set_origin(last_duck());
/*
							ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
							if(value.same_type_as(synfig::Point()))
								duck->set_origin(value.get(synfig::Point()));
*/
						}
					}
					duck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,i),transform_stack)^synfig::GUID::hasher(".v"));
					duck=add_similar_duck(duck);
				}

				// ----Width duck
				etl::handle<Duck> width;

				// Add the width duck if it is a parameter with a hint (ie. "width") or if it isn't a parameter
				//if (!   ((param_desc && !param_desc->get_hint().empty()) || !param_desc)   )
				if (param_desc && param_desc->get_hint().empty())
				{
					// if it's a parameter without a hint, then don't add the width duck
					// (This prevents width ducks from being added to region layers, and possibly other cases)
				}
				else
				if(composite_vertex_value_node)
				{
					int index=composite_vertex_value_node->get_link_index_from_name("width");
					if (add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,index),canvas_view,transform_stack,REAL_COOKIE))
					{
						width=last_duck();
						width->set_origin(duck);
						width->set_type(Duck::TYPE_WIDTH);
						width->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".w");

						// if the bline is a layer's parameter, scale the width duck by the layer's "width" parameter
						if (param_desc)
						{
							ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_hint()).get_value(get_time()));
							Real gv(value_desc.get_layer()->get_parent_canvas_grow_value());
							if(value.same_type_as(synfig::Real()))
								width->set_scalar(exp(gv)*value.get(synfig::Real())*0.5f);
							// if it doesn't have a "width" parameter, scale by 0.5f instead
							else
								width->set_scalar(0.5f);
						}
						// otherwise just present the raw unscaled width
						else
							width->set_scalar(0.5f);
					}
					else
						synfig::error("Unable to add width duck!");
				}
				else
				if (composite_bone_link_value_node)
				{
					int index=composite_bone_link_value_node->get_link_index_from_name("width");
					if (add_to_ducks(synfigapp::ValueDesc(composite_bone_link_value_node,index),canvas_view,transform_stack,REAL_COOKIE))
					{
						width=last_duck();
						width->set_origin(duck);
						width->set_type(Duck::TYPE_WIDTH);
						width->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".w");

						// if the bline is a layer's parameter, scale the width duck by the layer's "width" parameter
						if (param_desc)
						{
							ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_hint()).get_value(get_time()));
							if(value.same_type_as(synfig::Real()))
								width->set_scalar(value.get(synfig::Real())*0.5f);
							// if it doesn't have a "width" parameter, scale by 0.5f instead
							else
								width->set_scalar(0.5f);
						}
						// otherwise just present the raw unscaled width
						else
							width->set_scalar(0.5f);
					}
					else
						synfig::error("Unable to add width duck!");
				}
				else
				{
					synfig::error("Cannot add width duck to non-composite blinepoint");
				}

				// each bezier uses t2 of one point and t1 of the next
				// the first time through this loop we won't have the t2 duck from the previous vertex
				// and so we don't make a bezier.  instead we skip on to t2 for this point
				if(bezier)
				{
					// Add the tangent1 duck
					if (composite_vertex_value_node)
					{
						int index=composite_vertex_value_node->get_link_index_from_name("t1");
						if(!add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,index,-TANGENT_BEZIER_SCALE),canvas_view,transform_stack))
							return false;
						tduck=last_duck();
					}
					else
					if (composite_bone_link_value_node)
					{
						int index=composite_bone_link_value_node->get_link_index_from_name("t1");
						if(!add_to_ducks(synfigapp::ValueDesc(composite_bone_link_value_node,index,-TANGENT_BEZIER_SCALE),
										 canvas_view,bone_transform_stack))
							return false;
						tduck=last_duck();
					}
					else
					{
						tduck=new Duck(bline_point.get_tangent1());
						tduck->set_transform_stack(transform_stack);
						tduck->set_editable(false);
						tduck->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".t1");
	//					tduck->set_name(strprintf("%x-tangent1",value_node->get_link(i).get()));
						tduck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,i),transform_stack)^synfig::GUID::hasher(".t1"));
						tduck=add_similar_duck(tduck);
	//					add_duck(duck);
					}

					tduck->set_origin(duck);
					tduck->set_scalar(-TANGENT_BEZIER_SCALE);
					tduck->set_tangent(true);

					// each bezier uses t2 of one point and t1 of the next
					// we should already have a bezier, so add the t1 of this point to it

					bezier->p2=duck;
					bezier->c2=tduck;

					bezier->signal_user_click(2).connect(
						sigc::bind(
							sigc::mem_fun(
								*canvas_view,
								&studio::CanvasView::popup_param_menu_bezier),
							synfigapp::ValueDesc(value_node,i)));
//
					duck->signal_user_click(2).clear();
					duck->signal_user_click(2).connect(
						sigc::bind(
							sigc::bind(
								sigc::bind(
									sigc::mem_fun(
										*canvas_view,
										&studio::CanvasView::popup_param_menu),
									false),
								1.0f),
							synfigapp::ValueDesc(value_node,i)));
					duck->set_value_desc(synfigapp::ValueDesc(value_node,i));
//
					add_bezier(bezier);
					bezier=0;
				}

				// don't start a new bezier for the last point in the line if we're not looped
				if ((i+1>=value_node->link_count() && !value_node->get_loop()))
					continue;

				bezier=new Bezier();

				// Add the tangent2 duck
				if (composite_vertex_value_node)
				{
					int i=composite_vertex_value_node->get_link_index_from_name("t2");
					if(!add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,i,TANGENT_BEZIER_SCALE),canvas_view,transform_stack,0,2))
						return false;
					tduck=last_duck();
				}
				else
				if (composite_bone_link_value_node)
				{
					int i=composite_bone_link_value_node->get_link_index_from_name("t2");
					if(!add_to_ducks(synfigapp::ValueDesc(composite_bone_link_value_node,i,TANGENT_BEZIER_SCALE),
									 canvas_view,bone_transform_stack,0,2))
						return false;
					tduck=last_duck();
				}
				else
				{
					tduck=new Duck(bline_point.get_tangent2());
					tduck->set_transform_stack(transform_stack);
					tduck->set_name(guid_string(synfigapp::ValueDesc(value_node,i))+".t2");
					tduck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,i),transform_stack)^synfig::GUID::hasher(".t2"));
					tduck->set_editable(false);
					tduck=add_similar_duck(tduck);
//					add_duck(duck);
					if(param_desc)
					{
						synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
						add_to_ducks(value_desc_origin,canvas_view, transform_stack);
						duck->set_origin(last_duck());
/*
						ValueBase value(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()));
						if(value.same_type_as(synfig::Point()))
							duck->set_origin(value.get(synfig::Point()));
*/
//						if(!param_desc->get_origin().empty())
//							duck->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
					}
					duck->signal_user_click(2).clear();
					duck->signal_user_click(2).connect(
						sigc::bind(
							sigc::bind(
								sigc::bind(
									sigc::mem_fun(
										*canvas_view,
										&studio::CanvasView::popup_param_menu),
									false),
								1.0f),
							synfigapp::ValueDesc(value_node,i)));
					duck->set_value_desc(synfigapp::ValueDesc(value_node,i));
				}

				tduck->set_origin(duck);
				tduck->set_scalar(TANGENT_BEZIER_SCALE);
				tduck->set_tangent(true);

				bezier->p1=duck;
				bezier->c1=tduck;
			}

			// Loop if necessary
			if(bezier && value_node->get_loop())
			{
				BLinePoint bline_point((*value_node->get_link(first))(get_time()));

				ValueNode_Composite::Handle composite_vertex_value_node(
					ValueNode_Composite::Handle::cast_dynamic(
						value_node->get_link(first)));
				ValueNode_BoneInfluence::Handle bone_influence_vertex_value_node(
					ValueNode_BoneInfluence::Handle::cast_dynamic(value_node->get_link(first)));
				ValueNode_Composite::Handle composite_bone_link_value_node;
				synfig::TransformStack bone_transform_stack(transform_stack);
				if (bone_influence_vertex_value_node)
				{
					if(get_type_mask() & Duck::TYPE_BONE_SETUP)
					{
						// If in setup mode, add the original ducks prior to the bones transformation
						composite_vertex_value_node = ValueNode_Composite::Handle::cast_dynamic(
							bone_influence_vertex_value_node->get_link("link") );
					}
					else
					{
						// If not in setup mode, apply bones transformation to the ducks
						composite_bone_link_value_node = ValueNode_Composite::Handle::cast_dynamic(
							bone_influence_vertex_value_node->get_link("link") );

						if(param_desc)
						{
							if(!param_desc->get_origin().empty())
							{
								synfigapp::ValueDesc value_desc_origin(value_desc.get_layer(),param_desc->get_origin());
								add_to_ducks(value_desc_origin,canvas_view, transform_stack);
								GUID guid(calc_duck_guid(value_desc_origin, transform_stack));
								bone_transform_stack.push(new Transform_Origin(guid^synfig::GUID::hasher(".o"), last_duck()));
							}
						}

						Matrix transform(bone_influence_vertex_value_node->calculate_transform(get_time()));
						GUID guid(bone_influence_vertex_value_node->get_link("bone_weight_list")->get_guid());

						bone_transform_stack.push(new Transform_Matrix(guid, transform));
					}
				}
				// Add the vertex duck
				duck=first_duck;

				// Add the tangent1 duck
				if(composite_vertex_value_node)
				{
					int index=composite_vertex_value_node->get_link_index_from_name("t1");
					if(!add_to_ducks(synfigapp::ValueDesc(composite_vertex_value_node,index),canvas_view,transform_stack))
						return false;
					tduck=last_duck();
				}
				else
				if (composite_bone_link_value_node)
				{
					int index=composite_bone_link_value_node->get_link_index_from_name("t1");
					if(!add_to_ducks(synfigapp::ValueDesc(composite_bone_link_value_node,index,-TANGENT_BEZIER_SCALE),
									 canvas_view,bone_transform_stack))
						return false;
					tduck=last_duck();
				}
				else
				{
					tduck=new Duck(bline_point.get_tangent1());
					tduck->set_transform_stack(transform_stack);
					tduck->set_editable(false);
					tduck->set_name(guid_string(synfigapp::ValueDesc(value_node,first))+".t1");
					//tduck->set_name(strprintf("%x-tangent1",value_node->get_link(first).get()));
					tduck=add_similar_duck(tduck);
					tduck->set_guid(calc_duck_guid(synfigapp::ValueDesc(value_node,first),transform_stack)^synfig::GUID::hasher(".t1"));
					//add_duck(duck);
				}

				tduck->set_origin(duck);
				tduck->set_scalar(-TANGENT_BEZIER_SCALE);
				tduck->set_tangent(true);

				bezier->p2=duck;
				bezier->c2=tduck;

				bezier->signal_user_click(2).connect(
					sigc::bind(
						sigc::mem_fun(
							*canvas_view,
							&studio::CanvasView::popup_param_menu_bezier),
						synfigapp::ValueDesc(value_node,first)));

				duck->signal_user_click(2).clear();
				duck->signal_user_click(2).connect(
					sigc::bind(
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(
									*canvas_view,
									&studio::CanvasView::popup_param_menu),
								false),
							1.0f),
						synfigapp::ValueDesc(value_node,first)));
				duck->set_value_desc(synfigapp::ValueDesc(value_node,first));

				add_bezier(bezier);
				bezier=0;
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

			switch(value_node->get_contained_type())
			{
			case ValueBase::TYPE_VECTOR:
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
//						if(!param_desc->get_origin().empty())
//							last_duck()->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
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
				break;
			}

			case ValueBase::TYPE_SEGMENT:
				for(i=0;i<value_node->link_count();i++)
				{
					if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
						return false;
				}
				break;

			case ValueBase::TYPE_BONE:
				printf("%s:%d adding ducks\n", __FILE__, __LINE__);
				for(i=0;i<value_node->link_count();i++)
					if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
						return false;
				printf("%s:%d adding ducks done\n\n", __FILE__, __LINE__);
				break;

			case ValueBase::TYPE_BONE_WEIGHT_PAIR:
				for(i=0;i<value_node->link_count();i++)
					if(!add_to_ducks(synfigapp::ValueDesc(value_node,i),canvas_view,transform_stack))
						return false;
				break;

			default:
				return false;
			}
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
			if(value_desc.parent_is_layer_param())
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
				WidthPoint width_point((*value_node->get_link(i))(get_time()));
				// try casting the width point to Composite - this tells us whether it is composite or not
				ValueNode_Composite::Handle composite_width_point_value_node(
					ValueNode_Composite::Handle::cast_dynamic(value_node->get_link(i)));
				if(composite_width_point_value_node) // Add the position
				{
					etl::handle<Duck> pduck=new Duck();
					synfigapp::ValueDesc wpoint_value_desc(value_node, i); // The i-widthpoint on WPList
					pduck->set_type(Duck::TYPE_WIDTHPOINT_POSITION);
					pduck->set_transform_stack(transform_stack);
					pduck->set_name(guid_string(wpoint_value_desc));
					pduck->set_value_desc(wpoint_value_desc);
					// This is a quick hack to obtain the ducks position.
					// The position by amount and the amount by position
					// has to be written considering the bline length too
					// optionally
					ValueNode_BLineCalcVertex::LooseHandle bline_calc_vertex(ValueNode_BLineCalcVertex::create(Vector(0,0)));
					bline_calc_vertex->set_link("bline", bline);
					bline_calc_vertex->set_link("loop", ValueNode_Const::create(false));
					bline_calc_vertex->set_link("amount", ValueNode_Const::create(width_point.get_norm_position(value_node->get_loop())));
					bline_calc_vertex->set_link("homogeneous", ValueNode_Const::create(homogeneous));
					pduck->set_point((*bline_calc_vertex)(get_time()));
					// hack end
					pduck->set_guid(calc_duck_guid(wpoint_value_desc,transform_stack)^synfig::GUID::hasher(".wpoint"));
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
							Real gv(value_desc.get_layer()->get_parent_canvas_grow_value());
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

			if(value_node->get_contained_type()==ValueBase::TYPE_VECTOR)
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
//						if(!param_desc->get_origin().empty())
//							last_duck()->set_origin(synfigapp::ValueDesc(value_desc.get_layer(),param_desc->get_origin()).get_value(get_time()).get(synfig::Point()));
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
			else if(value_node->get_contained_type()==ValueBase::TYPE_SEGMENT)
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
	break;
	case ValueBase::TYPE_BONE:
	case ValueBase::TYPE_VALUENODE_BONE:
	{
		ValueNode::Handle value_node(value_desc.get_value_node());

		if (type == ValueBase::TYPE_VALUENODE_BONE)
		{
			assert(value_desc.parent_is_value_node());
			value_node = (*value_node)(get_time()).get(ValueNode_Bone::Handle());
		}
		else
			assert(value_desc.parent_is_linkable_value_node() || value_desc.parent_is_canvas());

		Duck::Handle fake_duck;
		synfig::TransformStack origin_transform_stack(transform_stack), bone_transform_stack;
		bool setup(get_type_mask() & Duck::TYPE_BONE_SETUP);
		bool recursive(get_type_mask() & Duck::TYPE_BONE_RECURSIVE);

		ValueNode_Bone::Handle bone_value_node;
		if (!(bone_value_node = ValueNode_Bone::Handle::cast_dynamic(value_node)))
		{
			error("expected a ValueNode_Bone");
			assert(0);
		}

		GUID guid(bone_value_node->get_guid());
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
				add_to_ducks(synfigapp::ValueDesc(bone_value_node, bone_value_node->get_link_index_from_name("parent")),canvas_view,transform_stack);

				if (setup)
				{
					transform = parent_bone.get_setup_matrix().invert();
					origin_transform_stack.push(new Transform_Matrix(guid, transform));
					bone_transform_stack = origin_transform_stack;
					bone_transform_stack.push(new Transform_Translate(guid, bone.get_origin0()));
				}
				else
				{
					transform = parent_bone.get_animated_matrix();
					origin_transform_stack.push(new Transform_Matrix(guid, transform));
					bone_transform_stack = origin_transform_stack;
					invertible = transform.is_invertible();

					Vector scale(parent_bone.get_local_scale());
					bone_transform_stack.push(new Transform_Translate(guid, Point((scale[0])*bone.get_origin()[0],
																				  (scale[1])*bone.get_origin()[1])));
					origin_transform_stack.push(new Transform_Scale(guid, scale));
				}

#ifdef TRY_TO_ALIGN_WIDTH_DUCKS
				// this stuff doesn't work very well - we can find out
				// the cumulative angle and so place the duck on the
				// right of the circle, but recursive scales in the
				// parent can cause the radius to look wrong, and the
				// duck to appear off-center
				printf("%s:%d bone %s:\n", __FILE__, __LINE__, bone.get_name().c_str());
				while (true) {
					printf("%s:%d parent_angle = %5.2f + %5.2f = %5.2f\n", __FILE__, __LINE__,
						   Angle::deg(parent_angle).get(), Angle::deg(setup ? parent_bone.get_angle0() : parent_bone.get_angle()).get(),
						   Angle::deg(parent_angle + (setup ? parent_bone.get_angle0() : parent_bone.get_angle())).get());
					parent_angle += setup ? parent_bone.get_angle0() : parent_bone.get_angle();
					if (parent_bone.is_root()) break;
					parent_bone = (*parent_bone.get_parent())(time).get(Bone());
				}
				printf("%s:%d finally %5.2f\n\n", __FILE__, __LINE__, Angle::deg(parent_angle).get());
#endif
			}
			else
			{
				bone_transform_stack = origin_transform_stack;
				bone_transform_stack.push(new Transform_Translate(guid, setup ? bone.get_origin0() : bone.get_origin()));
			}
		}

		bone_transform_stack.push(new Transform_Rotate(guid, setup ? bone.get_angle0() : bone.get_angle()));

		// origin
		{
			synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name(setup ? "origin0" : "origin"));

			etl::handle<Duck> duck=new Duck();
			duck->set_type(Duck::TYPE_POSITION);
			duck->set_transform_stack(origin_transform_stack);
			duck->set_name(guid_string(value_desc));
			duck->set_value_desc(value_desc);
			duck->set_point(value_desc.get_value(time).get(Point()));

			// duck->set_guid(calc_duck_guid(value_desc,origin_transform_stack)^synfig::GUID::hasher(multiple));
			duck->set_guid(calc_duck_guid(value_desc,origin_transform_stack)^synfig::GUID::hasher(".origin"));

			// if the ValueNode can be directly manipulated, then set it as so
			duck->set_editable(!invertible ? false :
							   !value_desc.is_value_node() ? true :
							   synfigapp::is_editable(value_desc.get_value_node()));

			duck->signal_edited().clear();
			duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
			duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
																							  &studio::CanvasView::popup_param_menu),
																				false), // bezier
																	 0.0f),				// location
														  value_desc));					// value_desc
			add_duck(duck);
		}

		// fake
		{
			synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name("name"));

			etl::handle<Duck> duck=new Duck();
			duck->set_type(Duck::TYPE_NONE);
			duck->set_transform_stack(bone_transform_stack);
			duck->set_name(guid_string(value_desc));
			duck->set_value_desc(value_desc);
			duck->set_point(Point(0, 0));

			// duck->set_guid(calc_duck_guid(value_desc,bone_transform_stack)^synfig::GUID::hasher(multiple));
			duck->set_guid(calc_duck_guid(value_desc,bone_transform_stack)^synfig::GUID::hasher(".fake"));

			duck->set_ignore(true);
			add_duck(duck);
			fake_duck = last_duck();
		}

		// width
		if (!setup)
		{
			synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name(recursive ? "scaley" : "scalely"));

			etl::handle<Duck> duck=new Duck();
			duck->set_type(Duck::TYPE_WIDTH);
			duck->set_transform_stack(bone_transform_stack);
			duck->set_name(guid_string(value_desc));
			duck->set_value_desc(value_desc);
			duck->set_radius(true);
			duck->set_scalar(1);
			duck->set_point(Point(value_desc.get_value(time).get(Real()), 0));

			// duck->set_guid(calc_duck_guid(value_desc,bone_transform_stack)^synfig::GUID::hasher(multiple));
			duck->set_guid(calc_duck_guid(value_desc,bone_transform_stack)^synfig::GUID::hasher(".width"));

			// if the ValueNode can be directly manipulated, then set it as so
			duck->set_editable(!invertible ? false :
							   !value_desc.is_value_node() ? true :
							   synfigapp::is_editable(value_desc.get_value_node()));

			duck->signal_edited().clear();
			duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
			duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
																							  &studio::CanvasView::popup_param_menu),
																				false), // bezier
																	 0.0f),				// location
														  value_desc));					// value_desc
			duck->set_origin(fake_duck);
			add_duck(duck);
		}

		// angle
		{
			synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name(setup ? "angle0" : "angle"));

			etl::handle<Duck> duck=new Duck();
			duck->set_type(Duck::TYPE_ANGLE);
			duck->set_transform_stack(bone_transform_stack);
			duck->set_name(guid_string(value_desc));
			duck->set_value_desc(value_desc);

			angle = value_desc.get_value(time).get(Angle());
			Real length(bone.get_length() * (setup ? 1 : bone.get_scalex() * bone.get_scalelx()));
			duck->set_point(Point(length*0.9, 0));

			// duck->set_guid(calc_duck_guid(value_desc,bone_transform_stack)^synfig::GUID::hasher(multiple));
			duck->set_guid(calc_duck_guid(value_desc,bone_transform_stack)^synfig::GUID::hasher(".angle"));

			// if the ValueNode can be directly manipulated, then set it as so
			duck->set_editable(!value_desc.is_value_node() ? true :
							   synfigapp::is_editable(value_desc.get_value_node()));

			duck->signal_edited().clear();
			duck->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &studio::Duckmatic::on_duck_changed), value_desc));
			duck->signal_user_click(2).connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*canvas_view,
																							  &studio::CanvasView::popup_param_menu),
																				false), // bezier
																	 0.0f),				// location
														  value_desc));					// value_desc
			duck->set_origin(fake_duck);
			add_duck(duck);
		}

		// tip
		{
			synfigapp::ValueDesc value_desc(bone_value_node, bone_value_node->get_link_index_from_name(setup ? "length" : recursive ? "scalex" : "scalelx"));

			etl::handle<Duck> duck;
			if (add_to_ducks(value_desc,canvas_view,bone_transform_stack,REAL_COOKIE))
			{
				duck=last_duck();
				duck->set_origin(fake_duck);
				duck->set_type(Duck::TYPE_RADIUS);
				duck->set_name(guid_string(value_desc));
				duck->set_linear(true, Angle::deg(0));

				Real scale();
				duck->set_scalar(setup     ? 1 :
								 recursive ? bone.get_length()*bone.get_scalelx() :
											 bone.get_length()*bone.get_scalex());
			}
		}

		return true;
	}
	break;
	case ValueBase::TYPE_BONE_WEIGHT_PAIR:
	{
		ValueNode_BoneWeightPair::Handle value_node;
		if(value_desc.is_value_node() &&
		   (value_node=ValueNode_BoneWeightPair::Handle::cast_dynamic(value_desc.get_value_node())))
			add_to_ducks(synfigapp::ValueDesc(value_node, value_node->get_link_index_from_name("bone")), canvas_view, transform_stack);
		break;
	}
	default:
		break;
	}
	return false;
}
