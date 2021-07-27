/* === S Y N F I G ========================================================= */
/*!	\file action_param.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "action_param.h"
#include "action.h"
#include "canvasinterface.h"

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === P R O C E D U R E S ================================================= */

bool
Action::candidate_check(const ParamVocab& param_vocab, const ParamList& param_list)
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

struct ParamCounter
{
	static int counter;
	~ParamCounter()
	{
		if(counter)
			synfig::error("%d action params not yet deleted!",counter);
	}
};

int ParamCounter::counter(0);

/* === M E T H O D S ======================================================= */

// We using placement new here `new(&obj)` to call the object constructor
// without allocating memory. Memory is already allocated by anonymous union
// But as a drawback, we SHOULD call destructor manually (see `clear()` method and `destruct` function)
template<typename T, typename X>
void inline construct(T* field, X& value) {
	new(field) T(value);
}

// This function is used to automatically detect destructor name
template<typename T>
void inline destruct(T& field) {
	field.~T();
}

Param::Param(const Param &rhs)
{
	++ParamCounter::counter;
	set(rhs);
}

Param::Param(const std::shared_ptr<synfigapp::CanvasInterface>& x):

	type_(TYPE_CANVASINTERFACE)
{
	++ParamCounter::counter;
	construct(&canvas_interface_, x);
}

Param::Param(const std::shared_ptr<synfigapp::CanvasInterface>& x):

	type_(TYPE_CANVASINTERFACE)
{
	++ParamCounter::counter;
	construct(&canvas_interface_, x);
}

Param::Param(const synfig::Canvas::Handle& x):
	type_(TYPE_CANVAS)
{
	++ParamCounter::counter;
	construct(&canvas_, x);
}

Param::Param(const synfig::Canvas::LooseHandle& x):
	type_(TYPE_CANVAS)
{
	++ParamCounter::counter;
	construct(&canvas_, x);
}

Param::Param(const synfig::Layer::Handle& x):

	type_(TYPE_LAYER)
{
	++ParamCounter::counter;
	construct(&layer_, x);
}

Param::Param(const synfig::Layer::LooseHandle& x):

	type_(TYPE_LAYER)
{
	++ParamCounter::counter;
	construct(&layer_, x);
}

Param::Param(const synfig::ValueNode::Handle& x):

	type_(TYPE_VALUENODE)
{
	++ParamCounter::counter;
	construct(&value_node_, x);
}

Param::Param(const synfig::ValueNode::LooseHandle& x):

	type_(TYPE_VALUENODE)
{
	++ParamCounter::counter;
	construct(&value_node_, x);
}

Param::Param(const synfig::ValueBase& x):

	type_(TYPE_VALUE)
{
	++ParamCounter::counter;
	construct(&value_, x);
}

Param::Param(const synfig::RendDesc& x):
	type_(TYPE_RENDDESC)
{
	++ParamCounter::counter;
	construct(&rend_desc_, x);
}

Param::Param(const synfig::Time& x):
	type_(TYPE_TIME)
{
	++ParamCounter::counter;
	construct(&time_, x);
}

Param::Param(const synfig::Activepoint& x):

	type_(TYPE_ACTIVEPOINT)
{
	++ParamCounter::counter;
	construct(&activepoint_, x);
}

Param::Param(const synfig::Waypoint& x):
	type_(TYPE_WAYPOINT)
{
	++ParamCounter::counter;
	construct(&waypoint_, x);
}

Param::Param(const synfig::Waypoint::Model& x):
	type_(TYPE_WAYPOINTMODEL)
{
	++ParamCounter::counter;
	construct(&waypoint_model_, x);
}

Param::Param(const synfig::String& x):
	type_(TYPE_STRING)
{
	++ParamCounter::counter;
	construct(&string_, x);
}

Param::Param(const char * x):
	type_(TYPE_STRING)
{
	++ParamCounter::counter;
	construct(&string_, x);
}

Param::Param(const synfig::Keyframe& x):

	type_(TYPE_KEYFRAME)
{
	++ParamCounter::counter;
	construct(&keyframe_, x);
}

Param::Param(const synfigapp::ValueDesc& x):

	type_(TYPE_VALUEDESC)
{
	++ParamCounter::counter;
	construct(&value_desc_, x);
}

Param::Param(const int& x):
	type_(TYPE_INTEGER)
{
	++ParamCounter::counter;
	integer_ = x;
}

Param::Param(const EditMode& x):
	type_(TYPE_EDITMODE)
{
	++ParamCounter::counter;
	edit_mode_ = x;
}

Param::Param(const synfig::Real& x):

	type_(TYPE_REAL)
{
	++ParamCounter::counter;
	real_ = x;
}

Param::Param(const bool& x):

	type_(TYPE_BOOL)
{
	++ParamCounter::counter;
	bool_ = x;
}

Param::Param(const synfig::Interpolation& x):
	type_(TYPE_INTERPOLATION)
{
	++ParamCounter::counter;
	interpolation_ = x;
}

Param::~Param()
{
	clear();
	--ParamCounter::counter;
}

void Param::set(const Param& rhs) {
	type_ = rhs.type_;

	switch(type_)
	{
		case TYPE_ACTIVEPOINT:
			construct(&activepoint_, rhs.activepoint_);
			break;
		case TYPE_WAYPOINT:
			construct(&waypoint_, rhs.waypoint_);
			break;
		case TYPE_WAYPOINTMODEL:
			construct(&waypoint_model_, rhs.waypoint_model_);
			break;
		case TYPE_KEYFRAME:
			construct(&keyframe_, rhs.keyframe_);
			break;
		case TYPE_CANVAS:
			construct(&canvas_, rhs.canvas_);
			break;
		case TYPE_CANVASINTERFACE:
			construct(&canvas_interface_, rhs.canvas_interface_);
			break;
		case TYPE_TIME:
			construct(&time_, rhs.time_);
			break;
		case TYPE_LAYER:
			construct(&layer_, rhs.layer_);
			break;
		case TYPE_VALUENODE:
			construct(&value_node_, rhs.value_node_);
			break;
		case TYPE_VALUEDESC:
			construct(&value_desc_, rhs.value_desc_);
			break;
		case TYPE_VALUE:
			construct(&value_, rhs.value_);
			break;
		case TYPE_RENDDESC:
			construct(&rend_desc_, rhs.rend_desc_);
			break;
		case TYPE_STRING:
			construct(&string_, rhs.string_);
			break;

		// Trivially constructed
		case TYPE_INTEGER:
			integer_ = rhs.integer_;
			break;
		case TYPE_EDITMODE:
			edit_mode_ = rhs.edit_mode_;
			break;
		case TYPE_REAL:
			real_ = rhs.real_;
			break;
		case TYPE_BOOL:
			bool_ = rhs.bool_;
			break;
		case TYPE_INTERPOLATION:
			interpolation_ = rhs.interpolation_;
			break;

		case TYPE_NIL:
			break;

		default:
			assert(0);
			break;
	}
}

Param&
Param::operator=(const Param& rhs)
{
	clear();
	set(rhs);
	return *this;
}

void
Param::clear()
{
	switch(type_)
	{
	case TYPE_ACTIVEPOINT:
		destruct(activepoint_);
		break;
	case TYPE_WAYPOINT:
		destruct(waypoint_);
		break;
	case TYPE_WAYPOINTMODEL:
		destruct(waypoint_model_);
		break;
	case TYPE_KEYFRAME:
		destruct(keyframe_);
		break;
	case TYPE_CANVAS:
		destruct(canvas_);
		break;
	case TYPE_CANVASINTERFACE:
		destruct(canvas_interface_);
		break;
	case TYPE_LAYER:
		destruct(layer_);
		break;
	case TYPE_TIME:
		destruct(time_);
		break;
	case TYPE_VALUENODE:
		destruct(value_node_);
		break;
	case TYPE_VALUEDESC:
		destruct(value_desc_);
		break;
	case TYPE_VALUE:
		destruct(value_);
		break;
	case TYPE_RENDDESC:
		destruct(rend_desc_);
		break;
	case TYPE_STRING:
		destruct(string_);
		break;

	case TYPE_NIL:
	case TYPE_EDITMODE:
	case TYPE_INTEGER:
	case TYPE_REAL:
	case TYPE_BOOL:
	case TYPE_INTERPOLATION:
		break;

	default:
		assert(0);
		break;
	}
	type_=TYPE_NIL;
}
