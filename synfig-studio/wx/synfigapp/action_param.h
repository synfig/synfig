/* === S Y N F I G ========================================================= */
/*!	\file action_param.h
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_ACTION_PARAM_H
#define __SYNFIG_APP_ACTION_PARAM_H

/* === H E A D E R S ======================================================= */

#include <synfig/string.h>
#include <synfig/canvas.h>
#include <ETL/handle>
#include <ETL/stringf>
#include <ETL/trivial>

#include <map>
#include <list>

#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfig/valuenode.h>
#include <synfigapp/value_desc.h>
#include <synfig/value.h>
#include <synfig/activepoint.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/string.h>
#include <synfig/keyframe.h>
#include <synfig/waypoint.h>

#include "editmode.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
class ProgressCallback;
class Canvas;
class RendDesc;
}; // END of namespace synfig

namespace synfigapp {

class CanvasInterface;

namespace Action {

//! Action Parameter
class Param
{
public:
	enum Type
	{
		TYPE_NIL,				// 0
		TYPE_INTEGER,
		TYPE_REAL,
		TYPE_BOOL,
		TYPE_ACTIVEPOINT,		// 4
		TYPE_WAYPOINT,
		TYPE_WAYPOINTMODEL,
		TYPE_KEYFRAME,
		TYPE_CANVAS,			// 8
		TYPE_LAYER,
		TYPE_VALUENODE,
		TYPE_VALUEDESC,
		TYPE_VALUE,				// 12
		TYPE_STRING,
		TYPE_TIME,
		TYPE_CANVASINTERFACE,
		TYPE_EDITMODE,			// 16
		TYPE_RENDDESC,
		TYPE_INTERPOLATION,

		TYPE_END				// 19
	};
private:
	Type type_;

	union
	{
		etl::trivial<synfig::Canvas::LooseHandle> canvas;
		etl::trivial<synfig::Layer::LooseHandle> layer;
		etl::trivial<synfig::ValueNode::LooseHandle> value_node;
		etl::trivial<synfig::ValueBase> value;
		etl::trivial<synfig::Activepoint> activepoint;
		etl::trivial<synfig::ValueNode_Animated::Waypoint> waypoint;
		etl::trivial<synfig::ValueNode_Animated::Waypoint::Model> waypoint_model;
		etl::trivial<synfig::String> string;
		etl::trivial<synfig::Keyframe> keyframe;
		etl::trivial<synfig::Time> time;
		etl::trivial<synfigapp::ValueDesc> value_desc;
		etl::trivial<etl::loose_handle<synfigapp::CanvasInterface> > canvas_interface;
		etl::trivial<synfig::RendDesc> rend_desc;
		int integer;
		synfig::Real real;
		bool b;
		EditMode edit_mode;
		synfig::Interpolation interpolation;

	} data;
public:

	Param():type_(TYPE_NIL) { }
	Param(const Param &x);
	Param(const etl::handle<synfigapp::CanvasInterface>& x);
	Param(const etl::loose_handle<synfigapp::CanvasInterface>& x);
//	Param(synfigapp::CanvasInterface* x);
	Param(const synfig::Canvas::Handle& x);
	Param(const synfig::Canvas::LooseHandle& x);
	Param(const synfig::Layer::Handle& x);
	Param(const synfig::Layer::LooseHandle& x);
	Param(const synfig::ValueNode::Handle& x);
	Param(const synfig::ValueNode::LooseHandle& x);
	Param(const synfig::Activepoint& x);
	Param(const synfig::Waypoint& x);
	Param(const synfig::Waypoint::Model& x);
	Param(const synfig::String& x);
	Param(const synfig::RendDesc& x);
	Param(const char * x);
	Param(const synfig::Keyframe& x);
	Param(const synfigapp::ValueDesc& x);
	Param(const int& x);
	Param(const EditMode& x);
	Param(const synfig::Real& x);
	Param(const synfig::Time& x);
	Param(const bool& x);
	Param(const synfig::ValueBase& x);
	Param(const synfig::Interpolation& x);

	~Param();

	Param& operator=(const Param& rhs);

	void clear();

	const synfig::Canvas::LooseHandle& get_canvas()const { assert(type_==TYPE_CANVAS); return data.canvas.get(); }
	const etl::loose_handle<synfigapp::CanvasInterface>& get_canvas_interface()const { assert(type_==TYPE_CANVASINTERFACE); return data.canvas_interface.get(); }
	const synfig::Layer::LooseHandle& get_layer()const { assert(type_==TYPE_LAYER); return data.layer.get(); }
	const synfig::ValueNode::LooseHandle& get_value_node()const { assert(type_==TYPE_VALUENODE); return data.value_node.get(); }
	const synfig::ValueBase& get_value()const { assert(type_==TYPE_VALUE); return data.value.get(); }
	const synfig::Activepoint& get_activepoint()const { assert(type_==TYPE_ACTIVEPOINT); return data.activepoint.get(); }
	const synfig::Waypoint& get_waypoint()const { assert(type_==TYPE_WAYPOINT); return data.waypoint.get(); }
	const synfig::Waypoint::Model& get_waypoint_model()const { assert(type_==TYPE_WAYPOINTMODEL); return data.waypoint_model.get(); }
	const synfig::String& get_string()const { assert(type_==TYPE_STRING); return data.string.get(); }
	const synfig::Keyframe& get_keyframe()const { assert(type_==TYPE_KEYFRAME); return data.keyframe.get(); }
	const synfigapp::ValueDesc& get_value_desc()const { assert(type_==TYPE_VALUEDESC); return data.value_desc.get(); }
	const synfig::Real& get_real()const { assert(type_==TYPE_REAL); return data.real; }
	const synfig::Time& get_time()const { assert(type_==TYPE_TIME); return data.time.get(); }
	const synfig::RendDesc& get_rend_desc()const { assert(type_==TYPE_RENDDESC); return data.rend_desc.get(); }
	int get_integer()const { assert(type_==TYPE_INTEGER); return data.integer; }
	EditMode get_edit_mode()const { assert(type_==TYPE_EDITMODE); return data.edit_mode; }
	bool get_bool()const { assert(type_==TYPE_BOOL); return data.b; }
	const synfig::Interpolation& get_interpolation()const { assert(type_==TYPE_INTERPOLATION); return data.interpolation; }


	const Type& get_type()const { return type_; }
}; // END of class Param

class ParamList : public std::multimap<synfig::String,Param>
{
public:
	ParamList& add(const synfig::String& name, const Param &x) { insert(std::pair<synfig::String,Param>(name,x)); return *this; }
	ParamList& add(const ParamList& x) { insert(x.begin(),x.end()); return *this; }
	ParamList& remove_all(const synfig::String& name) { erase(name); return *this; }
}; // END of class ParamList

class ParamDesc
{
private:
	synfig::String	name_;
	synfig::String	local_name_;
	synfig::String	desc_;
	synfig::String	mutual_exclusion_;
	Param::Type	type_;
	bool	user_supplied_;
	bool	supports_multiple_;
	bool	requires_multiple_;
	bool	optional_;
	bool	value_provided_;

public:
	ParamDesc(const synfig::String &name, Param::Type type):
		name_(name),
		local_name_(name),
		type_(type),
		user_supplied_(false),
		supports_multiple_(false),
		requires_multiple_(false),
		optional_(false),
		value_provided_(false)
	{ }

	const synfig::String& get_name()const { return name_; }
	const synfig::String& get_desc()const { return desc_; }
	const synfig::String& get_mutual_exclusion()const { return mutual_exclusion_; }
	const synfig::String& get_local_name()const { return local_name_; }
	const Param::Type& get_type()const { return type_; }
	bool get_user_supplied()const { return user_supplied_; }
	bool get_supports_multiple()const { return supports_multiple_||requires_multiple_; }
	bool get_requires_multiple()const { return requires_multiple_; }
	bool get_optional()const { return optional_; }
	bool get_value_provided()const { return value_provided_; }

	ParamDesc& set_local_name(const synfig::String& x) { local_name_=x; return *this; }
	ParamDesc& set_desc(const synfig::String& x) { desc_=x; return *this; }
	ParamDesc& set_mutual_exclusion(const synfig::String& x) { mutual_exclusion_=x; return *this; }
	ParamDesc& set_user_supplied(bool x=true) { user_supplied_=x; return *this; }
	ParamDesc& set_supports_multiple(bool x=true) { supports_multiple_=x; return *this; }
	ParamDesc& set_requires_multiple(bool x=true) { requires_multiple_=x; if(x)supports_multiple_=true; return *this; }
	ParamDesc& set_optional(bool x=true) { optional_=x; return *this; }
	ParamDesc& set_value_provided(bool x=true) { value_provided_=x; return *this; }
}; // END of class ParamDesc

class ParamVocab : public std::list< ParamDesc > { };

bool candidate_check(const ParamVocab& param_vocab, const ParamList& param_list);

}; // END of namespace Action

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
