/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: timepointcollect.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#include "timepointcollect.h"
#include "valuenode_animated.h"
#include "layer.h"
#include "canvas.h"
#include "value.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

//! \writeme
int
sinfg::waypoint_collect(set<Waypoint, std::less<UniqueID> >& waypoint_set,const Time& time, const etl::handle<Node>& node)
{	
	const TimePointSet& timepoint_set(node->get_times());

	// Check to see if there is anything in here at the given time
	if(timepoint_set.find(time)==timepoint_set.end())
		return 0;

	// Check if we are a linkable value node
	LinkableValueNode::Handle linkable_value_node;
	linkable_value_node=linkable_value_node.cast_dynamic(node);
	if(linkable_value_node)
	{
		const int link_count(linkable_value_node->link_count());
		int i,ret(0);
		for(i=0;i<link_count;i++)
		{
			ret+=waypoint_collect(waypoint_set,time,linkable_value_node->get_link(i).get());
		}
		return ret;
	}

	// Check if we are a layer
	Layer::Handle layer;
	layer=layer.cast_dynamic(node);
	if(layer)
	{
		const Layer::DynamicParamList& dyn_param_list(layer->dynamic_param_list());
		Layer::DynamicParamList::const_iterator iter;
		int ret(0);
		for(iter=dyn_param_list.begin();iter!=dyn_param_list.end();++iter)
		{
			ret+=waypoint_collect(waypoint_set,time,iter->second);
		}
		ValueBase canvas_value(layer->get_param("canvas"));
		if(canvas_value.get_type()==ValueBase::TYPE_CANVAS)
		{
			ret+=waypoint_collect(waypoint_set,time,Canvas::Handle(canvas_value.get(Canvas::Handle())));
		}
		return ret;
	}

	// Check if we are a canvas
	Canvas::Handle canvas;
	canvas=canvas.cast_dynamic(node);
	if(canvas)
	{
		Canvas::const_iterator iter;
		int ret(0);
		for(iter=canvas->begin();iter!=canvas->end();++iter)
			ret+=waypoint_collect(waypoint_set,time,*iter);
		return ret;
	}

	// Check if we are an animated value node
	ValueNode_Animated::Handle value_node_animated;
	value_node_animated=value_node_animated.cast_dynamic(node);
	if(value_node_animated)
	{
		try{
			Waypoint waypoint=*value_node_animated->find(time);
			
			// If it is already in the waypoint set, then
			// don't bother adding it again
			if(waypoint_set.find(waypoint)!=waypoint_set.end())
			{
				return 0;
			}
			waypoint_set.insert(waypoint);
			return 1;
		}catch(...)
		{
			return 0;
		}
	}

	return 0;
}

//! \writeme
int
sinfg::activepoint_collect(set<Activepoint, std::less<UniqueID> >& activepoint_set,const Time& time, const etl::handle<Node>& node)
{
	const TimePointSet& timepoint_set(node->get_times());

	// Check to see if there is anything in here at the given time
	if(timepoint_set.find(time)==timepoint_set.end());
		return 0;

	return 0;
}
