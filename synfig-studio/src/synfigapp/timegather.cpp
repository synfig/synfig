/* === S Y N F I G ========================================================= */
/*!	\file timegather.cpp
**	\brief Time Gather File
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "timegather.h"
#include "value_desc.h"

#include <synfig/general.h>

#include <synfig/layers/layer_pastecanvas.h>

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//! Definitions for build a list of accurate valuenode references

void synfigapp::timepoints_ref::insert(synfig::ValueNode_Animated::Handle v, synfig::Waypoint w, synfig::Real time_dilation)
{
	ValueBaseTimeInfo	vt;
	vt.val = v;
	vt.time_dilation = time_dilation;

	waytracker::iterator i = waypointbiglist.find(vt);

	if(i != waypointbiglist.end())
	{
		i->waypoints.insert(w);
	}else
	{
		vt.waypoints.insert(w);
		waypointbiglist.insert(vt);
	}
}

void synfigapp::timepoints_ref::insert(synfigapp::ValueDesc v, synfig::Activepoint a, synfig::Real time_dilation)
{
	ActiveTimeInfo	vt;
	vt.val = v;
	vt.time_dilation = time_dilation;

	acttracker::iterator i = actpointbiglist.find(vt);

	if(i != actpointbiglist.end())
	{
		i->activepoints.insert(a);
		/*{ //if it fails...
			synfig::info("!!!!For some reason it wasn't able to insert the activepoint in the list (%s,%.4lg)",
							a.state?"true":"false", (double)a.time);
		}*/
	}else
	{
		vt.activepoints.insert(a);
		actpointbiglist.insert(vt);
		//synfig::info("Insert new activept list for valdesc");
	}
}

//recursion functions
void synfigapp::recurse_canvas(synfig::Canvas::Handle h, const std::set<Time> &tlist,
								timepoints_ref &vals, synfig::Time time_offset, synfig::Real time_dilation)
{

	//synfig::info("Canvas...\n Recurse through layers");
	// iterate through the layers

	synfig::Canvas::iterator i = h->begin(), end = h->end();

	for(; i != end; ++i)
	{
		const Node::time_set &tset = (*i)->get_times();
		if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end(),time_offset,time_dilation))
		{
			recurse_layer(*i,tlist,vals,time_offset,time_dilation);
		}
	}
}

void synfigapp::recurse_layer(synfig::Layer::Handle h, const std::set<Time> &tlist,
								timepoints_ref &vals, synfig::Time time_offset, synfig::Real time_dilation)
{
	// iterate through the layers
	//check for special case of paste canvas
	etl::handle<synfig::Layer_PasteCanvas> p = etl::handle<synfig::Layer_PasteCanvas>::cast_dynamic(h);

	//synfig::info("Layer...");

	if(p)
	{
		//synfig::info("We are a paste canvas so go into that");
		//recurse into the canvas
		const synfig::Node::time_set &tset = p->get_sub_canvas()->get_times();
		synfig::Real subcanvas_time_dilation(p->get_time_dilation());
		synfig::Time subcanvas_time_offset(time_offset * subcanvas_time_dilation + p->get_time_offset());
		subcanvas_time_dilation *= time_dilation;

		if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end(),subcanvas_time_offset,subcanvas_time_dilation))
			recurse_canvas(p->get_sub_canvas(),tlist,vals,subcanvas_time_offset,subcanvas_time_dilation);
	}

	//check all the valuenodes regardless...
	//synfig::info("Recurse all valuenodes");
	synfig::Layer::DynamicParamList::const_iterator 	i = h->dynamic_param_list().begin(),
													end = h->dynamic_param_list().end();
	for(; i != end; ++i)
	{
		const synfig::Node::time_set &tset = i->second->get_times();

		if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end(),time_offset,time_dilation))
		{
			recurse_valuedesc(ValueDesc(h,i->first),tlist,vals,time_offset,time_dilation);
		}
	}
}

template < typename IT, typename CMP >
static bool sorted(IT i,IT end, const CMP &cmp = CMP())
{
	if(i == end) return true;

	for(IT last = i++; i != end; last = i++)
	{
		if(!cmp(*last,*i))
			return false;
	}

	return true;
}

void synfigapp::recurse_valuedesc(synfigapp::ValueDesc h, const std::set<Time> &tlist,
								timepoints_ref &vals, synfig::Time time_offset, synfig::Real time_dilation)
{
	//special cases for Animated, DynamicList, and Linkable

	//synfig::info("ValueBasenode... %p, %s", h.get_value_node().get(),typeid(*h.get_value_node()).name());


	//animated case
	{
		synfig::ValueNode_Animated::Handle p = synfig::ValueNode_Animated::Handle::cast_dynamic(h.get_value_node());

		if(p)
		{
			//loop through and determine which waypoint we will need to reference
			const synfig::WaypointList &w = p->waypoint_list();

			synfig::WaypointList::const_iterator i = w.begin(),
												end = w.end();

			std::set<Time>::const_iterator		j = tlist.begin(),
												jend = tlist.end();
			for(; i != end && j != jend;)
			{
				//synfig::info("tpair t(%.3f) = %.3f", (float)*j, (float)(i->get_time()));

				if((*j*time_dilation+time_offset).is_equal(i->get_time()))
				{
					vals.insert(p,*i,time_dilation);
					++i,++j;
				}else if(*i < *j*time_dilation+time_offset)
				{
					++i;
				}else ++j;
			}
			return;
		}
	}

	//parent dynamiclist case - just for active points for that object...
	if(h.parent_is_value_node())
	{
		synfig::ValueNode_DynamicList::Handle p = synfig::ValueNode_DynamicList::Handle::cast_dynamic(h.get_parent_value_node());

		if(p)
		{
			int index = h.get_index();

			//check all the active points in each list...
			const synfig::ActivepointList &a = p->list[index].timing_info;

			//synfig::info("Our parent = dynamic list, searching in %d activepts",a.size());

			std::set<Time>::const_iterator			i = tlist.begin(),
													end = tlist.end();

			synfig::ActivepointList::const_iterator 	j = a.begin(),
													jend = a.end();

			for(; j != jend && i != end;)
			{
				double it = *i*time_dilation+time_offset;
				double jt = j->get_time();
				double diff = (double)(it - jt);

				//synfig::info("\ttpair match(%.4lg) - %.4lg (diff = %lg",it,jt,diff);

				//
				if(std::fabs(diff) < (double)Time::epsilon())
				{
					//synfig::info("\tActivepoint to add being referenced (%x,%s,%.4lg)",
					//				(int)j->get_uid(),j->state?"true":"false", (double)j->time);
					vals.insert(ValueDesc(p,index),*j,time_dilation);
					++i,++j;
				}else if(it < jt)
				{
					++i;
					//synfig::info("\tIncrementing time");
				}
				else
				{
					++j;
					//synfig::info("\tIncrementing actpt");
				}
			}
		}
	}

	//dynamiclist case - we must still make sure that we read from the list entries the time values
	//						because just the linked valuenodes will not do that
	{
		synfig::ValueNode_DynamicList::Handle p = synfig::ValueNode_DynamicList::Handle::cast_dynamic(h.get_value_node());

		if(p)
		{
			//synfig::info("Process dynamic list valuenode");
			int index = 0;

			std::vector<synfig::ValueNode_DynamicList::ListEntry>::const_iterator
							i = p->list.begin(),
							end = p->list.end();

			for(; i != end; ++i, ++index)
			{
				const Node::time_set &tset = i->get_times();

				if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end(),time_offset,time_dilation))
				{
					recurse_valuedesc(ValueDesc(p,index),tlist,vals,time_offset,time_dilation);
				}
			}
			return;
		}
	}

	//the linkable case...
	{
		etl::handle<synfig::LinkableValueNode> p = etl::handle<synfig::LinkableValueNode>::cast_dynamic(h.get_value_node());

		if(p)
		{
			//synfig::info("Process Linkable ValueBasenode");
			int i = 0, size = p->link_count();

			for(; i < size; ++i)
			{
				ValueNode::Handle v = p->get_link(i);
				const Node::time_set &tset = v->get_times();

				if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end(),time_offset,time_dilation))
				{
					recurse_valuedesc(ValueDesc(p,i),tlist,vals,time_offset,time_dilation);
				}
			}
		}
	}
}
