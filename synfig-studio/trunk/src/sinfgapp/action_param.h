/* === S I N F G =========================================================== */
/*!	\file action_param.h
**	\brief Template File
**
**	$Id: action_param.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_APP_ACTION_PARAM_H
#define __SINFG_APP_ACTION_PARAM_H

/* === H E A D E R S ======================================================= */

#include <sinfg/string.h>
#include <sinfg/canvas.h>
#include <ETL/handle>
#include <ETL/stringf>
#include <ETL/trivial>

#include <map>
#include <list>

#include <sinfg/layer.h>
#include <sinfg/canvas.h>
#include <sinfg/valuenode.h>
#include <sinfgapp/value_desc.h>
#include <sinfg/value.h>
#include <sinfg/activepoint.h>
#include <sinfg/valuenode_animated.h>
#include <sinfg/string.h>
#include <sinfg/keyframe.h>
#include <sinfg/waypoint.h>

#include "editmode.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
class ProgressCallback;
class Canvas;
class RendDesc;
}; // END of namespace sinfg

namespace sinfgapp {

class CanvasInterface;
	
namespace Action {	

//! Action Parameter
class Param
{
public:
	enum Type
	{
		TYPE_NIL,
		TYPE_INTEGER,
		TYPE_REAL,
		TYPE_BOOL,
		TYPE_ACTIVEPOINT,
		TYPE_WAYPOINT,
		TYPE_WAYPOINTMODEL,
		TYPE_KEYFRAME,
		TYPE_CANVAS,
		TYPE_LAYER,
		TYPE_VALUENODE,
		TYPE_VALUEDESC,
		TYPE_VALUE,
		TYPE_STRING,
		TYPE_TIME,
		TYPE_CANVASINTERFACE,
		TYPE_EDITMODE,
		TYPE_RENDDESC,
		
		TYPE_END
	};
private:
	Type type_;

	union
	{
		etl::trivial<sinfg::Canvas::LooseHandle> canvas;
		etl::trivial<sinfg::Layer::LooseHandle> layer;
		etl::trivial<sinfg::ValueNode::LooseHandle> value_node;
		etl::trivial<sinfg::ValueBase> value;
		etl::trivial<sinfg::Activepoint> activepoint;
		etl::trivial<sinfg::ValueNode_Animated::Waypoint> waypoint;
		etl::trivial<sinfg::ValueNode_Animated::Waypoint::Model> waypoint_model;
		etl::trivial<sinfg::String> string;
		etl::trivial<sinfg::Keyframe> keyframe;
		etl::trivial<sinfg::Time> time;
		etl::trivial<sinfgapp::ValueDesc> value_desc;
		etl::trivial<etl::loose_handle<sinfgapp::CanvasInterface> > canvas_interface;
		etl::trivial<sinfg::RendDesc> rend_desc;
		int integer;
		sinfg::Real real;
		bool b;
		EditMode edit_mode;
		
	} data;
public:

	Param():type_(TYPE_NIL) { }
	Param(const Param &x);
	Param(const etl::handle<sinfgapp::CanvasInterface>& x);
	Param(const etl::loose_handle<sinfgapp::CanvasInterface>& x);
//	Param(sinfgapp::CanvasInterface* x);
	Param(const sinfg::Canvas::Handle& x);
	Param(const sinfg::Canvas::LooseHandle& x);
	Param(const sinfg::Layer::Handle& x);
	Param(const sinfg::Layer::LooseHandle& x);
	Param(const sinfg::ValueNode::Handle& x);
	Param(const sinfg::ValueNode::LooseHandle& x);
	Param(const sinfg::Activepoint& x);
	Param(const sinfg::Waypoint& x);
	Param(const sinfg::Waypoint::Model& x);
	Param(const sinfg::String& x);
	Param(const sinfg::RendDesc& x);
	Param(const char * x);
	Param(const sinfg::Keyframe& x);
	Param(const sinfgapp::ValueDesc& x);
	Param(const int& x);
	Param(const EditMode& x);
	Param(const sinfg::Real& x);
	Param(const sinfg::Time& x);
	Param(const bool& x);
	Param(const sinfg::ValueBase& x);

	~Param();
	
	Param& operator=(const Param& rhs);
	
	void clear();
	
	const sinfg::Canvas::LooseHandle& get_canvas()const { assert(type_==TYPE_CANVAS); return data.canvas.get(); }
	const etl::loose_handle<sinfgapp::CanvasInterface>& get_canvas_interface()const { assert(type_==TYPE_CANVASINTERFACE); return data.canvas_interface.get(); }
	const sinfg::Layer::LooseHandle& get_layer()const { assert(type_==TYPE_LAYER); return data.layer.get(); }
	const sinfg::ValueNode::LooseHandle& get_value_node()const { assert(type_==TYPE_VALUENODE); return data.value_node.get(); }
	const sinfg::ValueBase& get_value()const { assert(type_==TYPE_VALUE); return data.value.get(); }
	const sinfg::Activepoint& get_activepoint()const { assert(type_==TYPE_ACTIVEPOINT); return data.activepoint.get(); }
	const sinfg::Waypoint& get_waypoint()const { assert(type_==TYPE_WAYPOINT); return data.waypoint.get(); }
	const sinfg::Waypoint::Model& get_waypoint_model()const { assert(type_==TYPE_WAYPOINTMODEL); return data.waypoint_model.get(); }
	const sinfg::String& get_string()const { assert(type_==TYPE_STRING); return data.string.get(); }
	const sinfg::Keyframe& get_keyframe()const { assert(type_==TYPE_KEYFRAME); return data.keyframe.get(); }
	const sinfgapp::ValueDesc& get_value_desc()const { assert(type_==TYPE_VALUEDESC); return data.value_desc.get(); }
	const sinfg::Real& get_real()const { assert(type_==TYPE_REAL); return data.real; }
	const sinfg::Time& get_time()const { assert(type_==TYPE_TIME); return data.time.get(); }
	const sinfg::RendDesc& get_rend_desc()const { assert(type_==TYPE_RENDDESC); return data.rend_desc.get(); }
	int get_integer()const { assert(type_==TYPE_INTEGER); return data.integer; }
	EditMode get_edit_mode()const { assert(type_==TYPE_EDITMODE); return data.edit_mode; }
	bool get_bool()const { assert(type_==TYPE_BOOL); return data.b; }


	const Type& get_type()const { return type_; }
}; // END of class Param

class ParamList : public std::multimap<sinfg::String,Param>
{
public:
	ParamList& add(const sinfg::String& name, const Param &x) { insert(std::pair<sinfg::String,Param>(name,x)); return *this; }
	ParamList& add(const ParamList& x) { insert(x.begin(),x.end()); return *this; }
}; // END of class ParamList

class ParamDesc
{
private:
	sinfg::String	name_;
	sinfg::String	local_name_;
	sinfg::String	desc_;
	sinfg::String	mutual_exclusion_;
	Param::Type	type_;
	bool	user_supplied_;
	bool	supports_multiple_;
	bool	requires_multiple_;
	bool	optional_;

public:
	ParamDesc(const sinfg::String &name, Param::Type type):
		name_(name),
		local_name_(name),
		type_(type),
		user_supplied_(false),
		supports_multiple_(false),
		requires_multiple_(false),
		optional_(false)
	{ }
	
	const sinfg::String& get_name()const { return name_; }
	const sinfg::String& get_desc()const { return desc_; }
	const sinfg::String& get_mutual_exclusion()const { return mutual_exclusion_; }
	const sinfg::String& get_local_name()const { return local_name_; }
	const Param::Type& get_type()const { return type_; }
	bool get_user_supplied()const { return user_supplied_; }
	bool get_supports_multiple()const { return supports_multiple_||requires_multiple_; }
	bool get_requires_multiple()const { return requires_multiple_; }
	bool get_optional()const { return optional_; }

	ParamDesc& set_local_name(const sinfg::String& x) { local_name_=x; return *this; }
	ParamDesc& set_desc(const sinfg::String& x) { desc_=x; return *this; }
	ParamDesc& set_mutual_exclusion(const sinfg::String& x) { mutual_exclusion_=x; return *this; }
	ParamDesc& set_user_supplied(bool x=true) { user_supplied_=x; return *this; }
	ParamDesc& set_supports_multiple(bool x=true) { supports_multiple_=x; return *this; }
	ParamDesc& set_requires_multiple(bool x=true) { requires_multiple_=x; if(x)supports_multiple_=true; return *this; }
	ParamDesc& set_optional(bool x=true) { optional_=x; return *this; }
}; // END of class ParamDesc

class ParamVocab : public std::list< ParamDesc > { };

bool canidate_check(const ParamVocab& param_vocab, const ParamList& param_list);

}; // END of namespace Action

}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
