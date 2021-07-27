/* === S Y N F I G ========================================================= */
/*!	\file timepointcollect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "timepointcollect.h"
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/layers/layer_pastecanvas.h>
#include "layer.h"
#include "canvas.h"
#include "value.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

//! \writeme
int
synfig::waypoint_collect(set<Waypoint, std::less<UniqueID> >	&waypoint_set,
						 const Time								&time,
						 const std::shared_ptr<Node>				&node)
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
			ret+=waypoint_collect(waypoint_set,time,linkable_value_node->get_link(i).get());

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
			ret+=waypoint_collect(waypoint_set,time,iter->second);

		ValueBase canvas_value(layer->get_param("canvas"));
		if(canvas_value.get_type()==type_canvas)
		{
			std::shared_ptr<Layer_PasteCanvas> p = std::shared_ptr<Layer_PasteCanvas>::cast_dynamic(layer);
			if (p)
				ret+=waypoint_collect(waypoint_set, time + p->get_time_offset(),
									  Canvas::Handle(canvas_value.get(Canvas::Handle())));
			else
				ret+=waypoint_collect(waypoint_set, time,
									  Canvas::Handle(canvas_value.get(Canvas::Handle())));
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
				return 0;

			waypoint_set.insert(waypoint);
			return 1;
		}catch(...)
		{
			return 0;
		}
	}

	return 0;
}

bool
synfig::waypoint_search(Waypoint& waypoint, const UniqueID &uid, const std::shared_ptr<Node> &node)
{
	// Check if we are a linkable value node
	LinkableValueNode::Handle linkable_value_node;
	linkable_value_node=linkable_value_node.cast_dynamic(node);
	if(linkable_value_node)
	{
		const int link_count(linkable_value_node->link_count());
		for(int i=0; i < link_count; i++) {
			bool ret = waypoint_search(waypoint, uid, linkable_value_node->get_link(i).get());
			if (ret)
				return true;
		}
		return false;
	}

	// Check if we are a layer
	Layer::Handle layer;
	layer=layer.cast_dynamic(node);
	if(layer)
	{
		const Layer::DynamicParamList& dyn_param_list(layer->dynamic_param_list());

		for(Layer::DynamicParamList::const_iterator iter=dyn_param_list.begin(); iter!=dyn_param_list.end(); ++iter) {
			bool ret = waypoint_search(waypoint, uid, iter->second);
			if (ret)
				return true;
		}

		ValueBase canvas_value(layer->get_param("canvas"));
		if(canvas_value.get_type()==type_canvas)
		{
			bool ret = waypoint_search(waypoint, uid,
									  Canvas::Handle(canvas_value.get(Canvas::Handle())));
			if (ret)
				return true;
		}
		return false;
	}

	// Check if we are a canvas
	Canvas::Handle canvas;
	canvas=canvas.cast_dynamic(node);
	if(canvas)
	{
		for(Canvas::const_iterator iter = canvas->begin(); iter!=canvas->end(); ++iter) {
			bool ret = waypoint_search(waypoint, uid, *iter);
			if (ret)
				return true;
		}
		return false;
	}

	// Check if we are an animated value node
	ValueNode_Animated::Handle value_node_animated;
	value_node_animated=value_node_animated.cast_dynamic(node);
	if(value_node_animated)
	{
		ValueNode_AnimatedInterfaceConst::const_findresult result = value_node_animated->find_uid(uid);
		if (!result.second)
			return false;

		waypoint = *result.first;
		return true;
	}

	return false;
}

//! \writeme
int
synfig::activepoint_collect(set<Activepoint, std::less<UniqueID> >& /*activepoint_set*/,const Time& time, const std::shared_ptr<Node>& node)
{
	const TimePointSet& timepoint_set(node->get_times());

	// Check to see if there is anything in here at the given time
	if(timepoint_set.find(time)==timepoint_set.end())
		return 0;

	return 0;
}
