/* === S I N F G =========================================================== */
/*!	\file timegather.cpp
**	\brief Time Gather File
**
**	$Id: timegather.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#include "timegather.h"
#include "value_desc.h"

#include <sinfg/layer_pastecanvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//! Definitions for build a list of accurate valuenode references

void sinfgapp::timepoints_ref::insert(sinfg::ValueNode_Animated::Handle v, sinfg::Waypoint w)
{
	ValueBaseTimeInfo	vt;
	vt.val = v;
	
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

void sinfgapp::timepoints_ref::insert(sinfgapp::ValueDesc v, sinfg::Activepoint a)
{
	ActiveTimeInfo	vt;
	vt.val = v;
	
	acttracker::iterator i = actpointbiglist.find(vt);
	
	if(i != actpointbiglist.end())
	{
		i->activepoints.insert(a);
		/*{ //if it fails...
			sinfg::info("!!!!For some reason it wasn't able to insert the activepoint in the list (%s,%.4lg)",
							a.state?"true":"false", (double)a.time);
		}*/
	}else
	{
		vt.activepoints.insert(a);
		actpointbiglist.insert(vt);
		//sinfg::info("Insert new activept list for valdesc");
	}
}

//recursion functions
void sinfgapp::recurse_canvas(sinfg::Canvas::Handle h, const std::set<Time> &tlist, 
								timepoints_ref &vals)
{
	
	//sinfg::info("Canvas...\n Recurse through layers");
	// iterate through the layers

	sinfg::Canvas::iterator i = h->begin(), end = h->end();

	for(; i != end; ++i)
	{
		const Node::time_set &tset = (*i)->get_times();
		if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end()))
		{
			recurse_layer(*i,tlist,vals);
		}
	}
}

void sinfgapp::recurse_layer(sinfg::Layer::Handle h, const std::set<Time> &tlist, 
								timepoints_ref &vals)
{
	// iterate through the layers
	//check for special case of paste canvas
	etl::handle<sinfg::Layer_PasteCanvas> p = etl::handle<sinfg::Layer_PasteCanvas>::cast_dynamic(h);
	
	//sinfg::info("Layer...");
	
	if(p)
	{
		//sinfg::info("We are a paste canvas so go into that");
		//recurse into the canvas
		const sinfg::Node::time_set &tset = p->get_sub_canvas()->get_times();
		
		if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end()))
		{
			//we have to offset the times so it won't wreck havoc if the canvas is imported more than once...
			// and so we get correct results when offsets are present
			std::set<Time>	tlistoff;			
			std::set<Time>::iterator i = tlist.begin(), end = tlist.end();
			for(; i != end; ++i)
			{
				tlistoff.insert(*i - p->get_time_offset());
			}
			
			recurse_canvas(p->get_sub_canvas(),tlist,vals);
		}
	}

	//check all the valuenodes regardless...
	//sinfg::info("Recurse all valuenodes");
	sinfg::Layer::DynamicParamList::const_iterator 	i = h->dynamic_param_list().begin(),
													end = h->dynamic_param_list().end();
	for(; i != end; ++i)
	{
		const sinfg::Node::time_set &tset = i->second->get_times();
		
		if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end()))
		{
			recurse_valuedesc(ValueDesc(h,i->first),tlist,vals);
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

void sinfgapp::recurse_valuedesc(sinfgapp::ValueDesc h, const std::set<Time> &tlist, 
								timepoints_ref &vals)
{
	//special cases for Animated, DynamicList, and Linkable
	
	//sinfg::info("ValueBasenode... %p, %s", h.get_value_node().get(),typeid(*h.get_value_node()).name());
	
	
	//animated case
	{
		sinfg::ValueNode_Animated::Handle p = sinfg::ValueNode_Animated::Handle::cast_dynamic(h.get_value_node());
		
		if(p)
		{
			//loop through and determine which waypoint we will need to reference
			const sinfg::WaypointList &w = p->waypoint_list();
			
			sinfg::WaypointList::const_iterator i = w.begin(),
												end = w.end();
			
			std::set<Time>::const_iterator		j = tlist.begin(),
												jend = tlist.end();
			for(; i != end && j != jend;) 
			{
				//sinfg::info("tpair t(%.3f) = %.3f", (float)*j, (float)(i->get_time()));
				
				if(j->is_equal(i->get_time()))
				{
					vals.insert(p,*i);
					++i,++j;
				}else if(*i < *j) 
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
		sinfg::ValueNode_DynamicList::Handle p = sinfg::ValueNode_DynamicList::Handle::cast_dynamic(h.get_parent_value_node());						
				
		if(p)
		{
			int index = h.get_index();
			
			//check all the active points in each list...
			const sinfg::ActivepointList &a = p->list[index].timing_info;
			
			//sinfg::info("Our parent = dynamic list, searching in %d activepts",a.size());
						
			std::set<Time>::const_iterator			i = tlist.begin(),
													end = tlist.end();
			
			sinfg::ActivepointList::const_iterator 	j = a.begin(),
													jend = a.end();
			
			for(; j != jend && i != end;)
			{
				double it = *i;
				double jt = j->get_time();
				double diff = (double)(it - jt);
				
				//sinfg::info("\ttpair match(%.4lg) - %.4lg (diff = %lg",it,jt,diff);				
				
				//
				if(abs(diff) < (double)Time::epsilon())
				{
					//sinfg::info("\tActivepoint to add being referenced (%x,%s,%.4lg)",
					//				(int)j->get_uid(),j->state?"true":"false", (double)j->time);
					vals.insert(ValueDesc(p,index),*j);
					++i,++j;						
				}else if(it < jt)
				{
					++i;
					//sinfg::info("\tIncrementing time");
				}
				else 
				{
					++j;
					//sinfg::info("\tIncrementing actpt");
				}
			}
		}
	}
	
	//dynamiclist case - we must still make sure that we read from the list entries the time values
	//						because just the linked valuenodes will not do that
	{
		sinfg::ValueNode_DynamicList::Handle p = sinfg::ValueNode_DynamicList::Handle::cast_dynamic(h.get_value_node());
		
		if(p)
		{
			//sinfg::info("Process dynamic list valuenode");
			int index = 0;
			
			std::vector<sinfg::ValueNode_DynamicList::ListEntry>::const_iterator 	
							i = p->list.begin(),
							end = p->list.end();
			
			for(; i != end; ++i, ++index)
			{
				const Node::time_set &tset = i->get_times();
				
				if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end()))
				{
					recurse_valuedesc(ValueDesc(p,index),tlist,vals);
				}
			}
			return;
		}
	}
	
	//the linkable case...
	{
		etl::handle<sinfg::LinkableValueNode> p = etl::handle<sinfg::LinkableValueNode>::cast_dynamic(h.get_value_node());
		
		if(p)
		{
			//sinfg::info("Process Linkable ValueBasenode");
			int i = 0, size = p->link_count();
			
			for(; i < size; ++i)
			{
				ValueNode::Handle v = p->get_link(i);
				const Node::time_set &tset = v->get_times();
				
				if(check_intersect(tset.begin(),tset.end(),tlist.begin(),tlist.end()))
				{
					recurse_valuedesc(ValueDesc(p,i),tlist,vals);
				}
			}
		}
	}
}
