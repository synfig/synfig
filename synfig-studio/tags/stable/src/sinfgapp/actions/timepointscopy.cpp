/* === S I N F G =========================================================== */
/*!	\file timepointscopy.cpp
**	\brief Copy the Time Points File
**
**	$Id: timepointscopy.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "timepointscopy.h"
#include <sinfg/layer_pastecanvas.h>
#include <sinfgapp/canvasinterface.h>
#include <sinfg/valuenode_dynamiclist.h>
#include <sinfg/valuenode_animated.h>

#include "activepointsimpleadd.h"
#include "waypointsimpleadd.h"
#include <sinfgapp/timegather.h>

#include <typeinfo>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::TimepointsCopy);
ACTION_SET_NAME(Action::TimepointsCopy,"timepoint_copy");
ACTION_SET_LOCAL_NAME(Action::TimepointsCopy,"Copy Time Points");
ACTION_SET_TASK(Action::TimepointsCopy,"copy");
ACTION_SET_CATEGORY(Action::TimepointsCopy,Action::CATEGORY_WAYPOINT|Action::CATEGORY_ACTIVEPOINT);
ACTION_SET_PRIORITY(Action::TimepointsCopy,0);
ACTION_SET_VERSION(Action::TimepointsCopy,"0.0");
ACTION_SET_CVS_ID(Action::TimepointsCopy,"$Id: timepointscopy.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::TimepointsCopy::TimepointsCopy()
{
	timedelta = 0;
	set_dirty(false);
}

Action::ParamVocab
Action::TimepointsCopy::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	
	ret.push_back(ParamDesc("addlayer",Param::TYPE_VALUE)
		.set_local_name(_("New Selected Layer"))
		.set_desc(_("A layer to add to our selected list"))
		.set_supports_multiple()
		.set_optional()
	);
	
	ret.push_back(ParamDesc("addcanvas",Param::TYPE_CANVAS)
		.set_local_name(_("New Selected Canvas"))
		.set_desc(_("A canvas to add to our selected list"))
		.set_supports_multiple()
		.set_optional()
	);
	
	ret.push_back(ParamDesc("addvaluedesc",Param::TYPE_VALUEDESC)
		.set_local_name(_("New Selected ValueBase"))
		.set_desc(_("A valuenode's description to add to our selected list"))
		.set_supports_multiple()
		.set_optional()
	);
	
	ret.push_back(ParamDesc("addtime",Param::TYPE_TIME)
		.set_local_name(_("New Selected Time Point"))
		.set_desc(_("A time point to add to our selected list"))
		.set_supports_multiple()
	);
	
	ret.push_back(ParamDesc("deltatime",Param::TYPE_TIME)
		.set_local_name(_("Time adjustment"))
		.set_desc(_("The amount of time to adjust all the selected points"))
	);

	return ret;
}

bool
Action::TimepointsCopy::is_canidate(const ParamList &x)
{
	if(!canidate_check(get_param_vocab(),x)) 
		return false;
	
	if(	x.find("addlayer") == x.end() && 
		x.find("addcanvas") == x.end() && 
		x.find("addvaluedesc") == x.end())
		return false;
	return true;
}

bool
Action::TimepointsCopy::set_param(const sinfg::String& name, const Action::Param &param)
{
	if(name=="addlayer" && param.get_type()==Param::TYPE_LAYER)
	{
		//add a layer to the list
		sel_layers.push_back(param.get_layer());
		//sinfg::info("action got layer");
		
		return true;
	}
	
	if(name=="addcanvas" && param.get_type()==Param::TYPE_CANVAS)
	{
		//add a layer to the list
		sel_canvases.push_back(param.get_canvas());
		//sinfg::info("action got canvas");
		
		return true;
	}
	
	if(name=="addvaluedesc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		//add a layer to the list
		sel_values.push_back(param.get_value_desc());
		//sinfg::info("action got valuedesc");
		
		return true;
	}
	
	if(name=="addtime" && param.get_type()==Param::TYPE_TIME)
	{
		//add a layer to the list
		sel_times.insert(param.get_time());
		//sinfg::info("action got time");
		
		return true;
	}
	
	if(name=="deltatime" && param.get_type()==Param::TYPE_TIME)
	{
		timedelta = param.get_time();
		//sinfg::info("action got time to move");
		
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::TimepointsCopy::is_ready()const
{
	if((sel_layers.empty() && sel_canvases.empty() && sel_values.empty()) || sel_times.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::TimepointsCopy::prepare()
{
	clear();
	
	//sinfg::info("Preparing TimepointsCopy by %f secs",(float)timemove);
	
	if(sel_times.empty()) return;
	
	//all our lists should be set correctly...

	//build our sub-action list
	// 	and yes we do need to store it temporarily so we don't duplicate 
	//		an operation on a specific valuenode, etc....
	timepoints_ref	match;
	
	Time fps = get_canvas()->rend_desc().get_frame_rate();
	
	//std::vector<sinfg::Layer::Handle>
	//sinfg::info("Layers %d", sel_layers.size());
	{
		std::vector<sinfg::Layer::Handle>::iterator i = sel_layers.begin(),
													end = sel_layers.end();
		
		for(; i != end; ++i)
		{
			//sinfg::info("Recurse through a layer");
			recurse_layer(*i,sel_times,match);
		}
	}
	
	//std::vector<sinfg::Canvas::Handle>	sel_canvases;
	//sinfg::info("Canvases %d", sel_canvases.size());
	{
		std::vector<sinfg::Canvas::Handle>::iterator 	i = sel_canvases.begin(),
														end = sel_canvases.end();
		
		for(; i != end; ++i)
		{
			//sinfg::info("Recurse through a canvas");
			recurse_canvas(*i,sel_times,match);
		}
	}
	
	//std::vector<sinfgapp::ValueDesc>
	//sinfg::info("ValueBasedescs %d", sel_values.size());
	{
		std::vector<sinfgapp::ValueDesc>::iterator 	i = sel_values.begin(),
													end = sel_values.end();
		
		for(; i != end; ++i)
		{
			//sinfg::info("Recurse through a valuedesc");
			recurse_valuedesc(*i,sel_times,match);
		}
	}
	
	//sinfg::info("built list of waypoints/activepoints to modify");
	//sinfg::info("\t There are %d waypoint sets and %d activepointsets", 
	//				match.waypointbiglist.size(), match.actpointbiglist.size());
	//process the hell out of em...
	{
		//must build from both lists
		timepoints_ref::waytracker::const_iterator 	i = match.waypointbiglist.begin(),
													end = match.waypointbiglist.end();
		for(; i != end; ++i)
		{
			//iterate through each waypoint for this specific valuenode
			std::set<sinfg::Waypoint>::const_iterator 	j = i->waypoints.begin(),
														end = i->waypoints.end();			
			for(; j != end; ++j)
			{
				Action::Handle action(WaypointSimpleAdd::create());
		
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_node",ValueNode::Handle(i->val));
				
				//sinfg::info("add waypoint mod...");
				//NOTE: We may want to store the old time for undoing the action...
				Waypoint neww;
				Waypoint w = *j;
				w.set_time((w.get_time() + timedelta).round(fps));
				w.mimic(neww); //make sure the new waypoint has a new id
				
				action->set_param("waypoint",w);
				
				//run the action now that we've added everything
				assert(action->is_ready());
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				
				add_action_front(action);
			}
		}
	}
	{
		//must build from both lists
		timepoints_ref::acttracker::const_iterator 	i = match.actpointbiglist.begin(),
													end = match.actpointbiglist.end();
		for(; i != end; ++i)
		{
			//iterate through each activepoint for this specific valuenode
			std::set<sinfg::Activepoint>::const_iterator 	j = i->activepoints.begin(),
															jend = i->activepoints.end();
			for(; j != jend; ++j)
			{
				Action::Handle action(ActivepointSimpleAdd::create());
					
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc",i->val);
				
				//NOTE: We may want to store the old time for undoing the action...
				Activepoint newa;
				Activepoint a = *j;
				a.set_time((a.get_time() + timedelta).round(fps));
				a.mimic(newa); //make sure the new activepoint has a new id
				
				action->set_param("activepoint",a);
				
				assert(action->is_ready());
				if(!action->is_ready())
				{
					throw Error(Error::TYPE_NOTREADY);
				}
			
				add_action_front(action);
			}
		}
	}
}

void
Action::TimepointsCopy::perform()
{
	Action::Super::perform();
}
