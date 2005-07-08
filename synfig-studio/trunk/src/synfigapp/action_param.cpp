/* === S Y N F I G ========================================================= */
/*!	\file action_param.cpp
**	\brief Template File
**
**	$Id: action_param.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#include "action_param.h"
#include "action.h"
#include "canvasinterface.h"

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === P R O C E D U R E S ================================================= */

bool
Action::canidate_check(const ParamVocab& param_vocab, const ParamList& param_list)
{
	ParamVocab::const_iterator iter;
	
	for(iter=param_vocab.begin();iter!=param_vocab.end();++iter)
	{
		int n(param_list.count(iter->get_name()));

//		if(n && !iter->get_mutual_exclusion().empty() && param_list.count(iter->get_mutual_exclusion()))
//			return false;

		if(!n && !iter->get_mutual_exclusion().empty() && param_list.count(iter->get_mutual_exclusion()))
			continue;

		if(iter->get_user_supplied() || iter->get_optional())
			continue;
							
		if(n==0)
			return false;
		if(n==1 && iter->get_requires_multiple())
			return false;
		if(n>1 && !iter->get_supports_multiple())
			return false;
		
		if(iter->get_type()!=param_list.find(iter->get_name())->second.get_type())
			return false;
	}
	return true;
}

/* === S T A T I C S ======================================================= */

struct _ParamCounter
{
	static int counter;
	~_ParamCounter()
	{
		if(counter)
			synfig::error("%d action params not yet deleted!",counter);
	}
} _param_counter;

int _ParamCounter::counter(0);

/* === M E T H O D S ======================================================= */

Param::Param(const Param &rhs):
	type_(rhs.type_)
{
	_ParamCounter::counter++;
	switch(type_)
	{
	case TYPE_ACTIVEPOINT:
		data.activepoint.construct();
		data.activepoint.get()=rhs.data.activepoint.get();
		break;
	case TYPE_WAYPOINT:
		data.waypoint.construct();
		data.waypoint.get()=rhs.data.waypoint.get();
		break;
	case TYPE_WAYPOINTMODEL:
		data.waypoint_model.construct();
		data.waypoint_model.get()=rhs.data.waypoint_model.get();
		break;
	case TYPE_KEYFRAME:
		data.keyframe.construct();
		data.keyframe.get()=rhs.data.keyframe.get();
		break;
	case TYPE_CANVAS:
		data.canvas.construct();
		data.canvas.get()=rhs.data.canvas.get();
		break;
	case TYPE_CANVASINTERFACE:
		data.canvas_interface.construct();
		data.canvas_interface.get()=rhs.data.canvas_interface.get();
		break;
	case TYPE_LAYER:
		data.layer.construct();
		data.layer.get()=rhs.data.layer.get();
		break;
	case TYPE_VALUENODE:
		data.value_node.construct();
		data.value_node.get()=rhs.data.value_node.get();
		break;
	case TYPE_VALUEDESC:
		data.value_desc.construct();
		data.value_desc.get()=rhs.data.value_desc.get();
		break;
	case TYPE_VALUE:
		data.value.construct();
		data.value.get()=rhs.data.value.get();
		break;
	case TYPE_STRING:
		data.string.construct();
		data.string.get()=rhs.data.string.get();
		break;
	case TYPE_RENDDESC:
		data.rend_desc.construct();
		data.rend_desc.get()=rhs.data.rend_desc.get();
		break;
	case TYPE_TIME:
		data.time.construct();
		data.time.get()=rhs.data.time.get();
		break;

	case TYPE_INTEGER:
		data.integer=rhs.data.integer;
		break;
	case TYPE_EDITMODE:
		data.edit_mode=rhs.data.edit_mode;
		break;
	case TYPE_REAL:
		data.real=rhs.data.real;
		break;
	case TYPE_BOOL:
		data.b=rhs.data.b;
		break;

	case TYPE_NIL:
		break;

	default:
		assert(0);
		break;
	}
}

Param::Param(const etl::handle<synfigapp::CanvasInterface>& x):
	
	type_(TYPE_CANVASINTERFACE)
{
	_ParamCounter::counter++;
	data.canvas_interface.construct();
	data.canvas_interface.get()=x;
}

/*
Param::Param(synfigapp::CanvasInterface* x):
	
	type_(TYPE_CANVASINTERFACE)
{
	_ParamCounter::counter++;
	data.canvas_interface.construct();
	data.canvas_interface=x;
}
*/

Param::Param(const etl::loose_handle<synfigapp::CanvasInterface>& x):
	
	type_(TYPE_CANVASINTERFACE)
{
	_ParamCounter::counter++;
	data.canvas_interface.construct();
	data.canvas_interface.get()=x;
}

Param::Param(const synfig::Canvas::Handle& x):	
	type_(TYPE_CANVAS)
{
	_ParamCounter::counter++;
	data.canvas.construct();
	data.canvas.get()=x;
}

Param::Param(const synfig::Canvas::LooseHandle& x):	
	type_(TYPE_CANVAS)
{
	_ParamCounter::counter++;
	data.canvas.construct();
	data.canvas.get()=x;
}

Param::Param(const synfig::Layer::Handle& x):
	
	type_(TYPE_LAYER)
{
	_ParamCounter::counter++;
	data.layer.construct();
	data.layer.get()=x;
}

Param::Param(const synfig::Layer::LooseHandle& x):
	
	type_(TYPE_LAYER)
{
	_ParamCounter::counter++;
	data.layer.construct();
	data.layer.get()=x;
}

Param::Param(const synfig::ValueNode::Handle& x):
	
	type_(TYPE_VALUENODE)
{
	_ParamCounter::counter++;
	data.value_node.construct();
	data.value_node.get()=x;
}

Param::Param(const synfig::ValueNode::LooseHandle& x):
	
	type_(TYPE_VALUENODE)
{
	_ParamCounter::counter++;
	data.value_node.construct();
	data.value_node.get()=x;
}

Param::Param(const synfig::ValueBase& x):
	
	type_(TYPE_VALUE)
{
	_ParamCounter::counter++;
	data.value.construct();
	data.value.get()=x;
}

Param::Param(const synfig::RendDesc& x):	
	type_(TYPE_RENDDESC)
{
	_ParamCounter::counter++;
	data.rend_desc.construct();
	data.rend_desc.get()=x;
}

Param::Param(const synfig::Time& x):
	type_(TYPE_TIME)
{
	_ParamCounter::counter++;
	data.time.construct();
	data.time.get()=x;
}

Param::Param(const synfig::Activepoint& x):
	
	type_(TYPE_ACTIVEPOINT)
{
	_ParamCounter::counter++;
	data.activepoint.construct();
	data.activepoint.get()=x;
}

Param::Param(const synfig::Waypoint& x):	
	type_(TYPE_WAYPOINT)
{
	_ParamCounter::counter++;
	data.waypoint.construct();
	data.waypoint.get()=x;
}

Param::Param(const synfig::Waypoint::Model& x):	
	type_(TYPE_WAYPOINTMODEL)
{
	_ParamCounter::counter++;
	data.waypoint_model.construct();
	data.waypoint_model.get()=x;
}

Param::Param(const synfig::String& x):
	type_(TYPE_STRING)
{
	_ParamCounter::counter++;
	data.string.construct();
	data.string.get()=x;
}

Param::Param(const char * x):
	type_(TYPE_STRING)
{
	_ParamCounter::counter++;
	data.string.construct();
	data.string.get()=x;
}

Param::Param(const synfig::Keyframe& x):
	
	type_(TYPE_KEYFRAME)
{
	_ParamCounter::counter++;
	data.keyframe.construct();
	data.keyframe.get()=x;
}

Param::Param(const synfigapp::ValueDesc& x):
	
	type_(TYPE_VALUEDESC)
{
	_ParamCounter::counter++;
	data.value_desc.construct();
	data.value_desc.get()=x;
}

Param::Param(const int& x):
	type_(TYPE_INTEGER)
{
	_ParamCounter::counter++;
	data.integer=x;
}

Param::Param(const EditMode& x):
	type_(TYPE_EDITMODE)
{
	_ParamCounter::counter++;
	data.edit_mode=x;
}

Param::Param(const synfig::Real& x):
	
	type_(TYPE_REAL)
{
	_ParamCounter::counter++;
	data.real=x;
}

Param::Param(const bool& x):
	
	type_(TYPE_BOOL)
{
	_ParamCounter::counter++;
	data.b=x;
}

Param::~Param()
{
	clear();
	_ParamCounter::counter--;
}

Param&
Param::operator=(const Param& rhs)
{
	clear();
	type_=rhs.type_;
	
	switch(type_)
	{
	case TYPE_ACTIVEPOINT:
		data.activepoint.construct();
		data.activepoint.get()=rhs.data.activepoint.get();
		break;
	case TYPE_WAYPOINT:
		data.waypoint.construct();
		data.waypoint.get()=rhs.data.waypoint.get();
		break;
	case TYPE_WAYPOINTMODEL:
		data.waypoint_model.construct();
		data.waypoint_model.get()=rhs.data.waypoint_model.get();
		break;
	case TYPE_KEYFRAME:
		data.keyframe.construct();
		data.keyframe.get()=rhs.data.keyframe.get();
		break;
	case TYPE_CANVAS:
		data.canvas.construct();
		data.canvas.get()=rhs.data.canvas.get();
		break;
	case TYPE_CANVASINTERFACE:
		data.canvas_interface.construct();
		data.canvas_interface.get()=rhs.data.canvas_interface.get();
		break;
	case TYPE_TIME:
		data.time.construct();
		data.time.get()=rhs.data.time.get();
		break;
	case TYPE_LAYER:
		data.layer.construct();
		data.layer.get()=rhs.data.layer.get();
		break;
	case TYPE_VALUENODE:
		data.value_node.construct();
		data.value_node.get()=rhs.data.value_node.get();
		break;
	case TYPE_VALUEDESC:
		data.value_desc.construct();
		data.value_desc.get()=rhs.data.value_desc.get();
		break;
	case TYPE_VALUE:
		data.value.construct();
		data.value.get()=rhs.data.value.get();
		break;
	case TYPE_RENDDESC:
		data.rend_desc.construct();
		data.rend_desc.get()=rhs.data.rend_desc.get();
		break;
	case TYPE_STRING:
		data.string.construct();
		data.string.get()=rhs.data.string.get();
		break;

	case TYPE_INTEGER:
		data.integer=rhs.data.integer;
		break;
	case TYPE_EDITMODE:
		data.integer=rhs.data.integer;
		break;
	case TYPE_REAL:
		data.real=rhs.data.real;
		break;
	case TYPE_BOOL:
		data.b=rhs.data.b;
		break;

	case TYPE_NIL:
		break;

	default:
		assert(0);
		break;
	}
	return *this;
}

void
Param::clear()
{
	switch(type_)
	{
	case TYPE_ACTIVEPOINT:
		data.activepoint.destruct();
		break;
	case TYPE_WAYPOINT:
		data.waypoint.destruct();
		break;
	case TYPE_WAYPOINTMODEL:
		data.waypoint_model.destruct();
		break;
	case TYPE_KEYFRAME:
		data.keyframe.destruct();
		break;
	case TYPE_CANVAS:
		data.canvas.destruct();
		break;
	case TYPE_CANVASINTERFACE:
		data.canvas_interface.destruct();
		break;
	case TYPE_LAYER:
		data.layer.destruct();
		break;
	case TYPE_TIME:
		data.time.destruct();
		break;
	case TYPE_VALUENODE:
		data.value_node.destruct();
		break;
	case TYPE_VALUEDESC:
		data.value_desc.destruct();
		break;
	case TYPE_VALUE:
		data.value.destruct();
		break;
	case TYPE_RENDDESC:
		data.rend_desc.destruct();
		break;
	case TYPE_STRING:
		data.string.destruct();
		break;

	case TYPE_NIL:
	case TYPE_EDITMODE:
	case TYPE_INTEGER:
	case TYPE_REAL:
	case TYPE_BOOL:
		break;

	default:
		assert(0);
		break;
	}
	type_=TYPE_NIL;
}
