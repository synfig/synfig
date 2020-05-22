/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bone.h
**	\brief Header file for implementation of the "Bone" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_BONE_H
#define __SYNFIG_VALUENODE_BONE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>
#include <synfig/bone.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Bone : public LinkableValueNode
{
	ValueNode::RHandle name_;
	ValueNode::RHandle origin_;
	ValueNode::RHandle angle_;
	ValueNode::RHandle scalelx_;
	ValueNode::RHandle scalex_;
	ValueNode::RHandle length_;
	ValueNode::RHandle width_;
	ValueNode::RHandle tipwidth_;
	ValueNode::RHandle depth_;
	ValueNode::RHandle parent_;

protected:
	ValueNode_Bone();
	ValueNode_Bone(const ValueBase &value, etl::loose_handle<Canvas> canvas = nullptr);

public:

	typedef etl::handle<ValueNode_Bone> Handle;
	typedef etl::handle<const ValueNode_Bone> ConstHandle;
	typedef etl::loose_handle<ValueNode_Bone> LooseHandle;
	typedef std::map<synfig::GUID, LooseHandle> BoneMap;
	typedef std::map<etl::loose_handle<const Canvas>, BoneMap> CanvasMap;
	typedef std::set<LooseHandle> BoneSet;
	typedef std::list<LooseHandle> BoneList;

	virtual ValueBase operator()(Time t)const;

	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	virtual ~ValueNode_Bone();
	virtual void set_guid(const GUID& new_guid);
	virtual void set_root_canvas(etl::loose_handle<Canvas> canvas);

	virtual String get_name()const;
	virtual String get_local_name()const;
	virtual String get_bone_name(Time t)const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;
	
	// checks if point belongs to the range of influence of current bone
	bool have_influence_on(Time t, const Vector &x)const
		{ return (*this)(t).get(Bone()).have_influence_on(x); }

protected:
	LinkableValueNode* create_new()const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	virtual void on_changed();

public:
	using synfig::LinkableValueNode::get_link_vfunc;

	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_Bone* create(const ValueBase &x, etl::loose_handle<Canvas> canvas = nullptr);
	virtual Vocab get_children_vocab_vfunc()const;
	ValueNode_Bone::LooseHandle find(String name)const;
	String unique_name(String name)const;
	static void show_bone_map(etl::loose_handle<Canvas> canvas, const char *file, int line, String text, Time t=0);
	static BoneMap get_bone_map(etl::handle<const Canvas> canvas);
	static BoneList get_ordered_bones(etl::handle<const Canvas> canvas);

	ValueNode_Bone::ConstHandle is_ancestor_of(ValueNode_Bone::ConstHandle bone, Time t)const;
	virtual bool is_root()const { return false; }

	// return a set of the bones that affect the given valuenode
	//   recurses through the valuenodes in the waypoints if it's animated,
	//   through the subnodes if it's linkable,
	//   and through the bone itself it's a bone constant
	static BoneSet get_bones_referenced_by(ValueNode::Handle value_node, bool recursive = true);

	// return a set of the bones that would be affected if the given ValueNode were edited
	// value_node is either a ValueNode_Const or a ValueNode_Animated, of type VALUENODE_BONE
	static BoneSet get_bones_affected_by(ValueNode::Handle value_node);

	// return a set of the bones that can be parents of the given ValueNode without causing loops
	static BoneSet get_possible_parent_bones(ValueNode::Handle value_node);

	static ValueNode_Bone::Handle get_root_bone();

#ifdef _DEBUG
	virtual void ref()const;
	virtual bool unref()const;
	virtual void rref()const;
	virtual void runref()const;
#endif

private:
	virtual Matrix get_animated_matrix(Time t, Point child_origin)const;
	Matrix get_animated_matrix(Time t, Real scalex, Real scaley, Angle angle, Point origin, ValueNode_Bone::ConstHandle parent)const;
	ValueNode_Bone::ConstHandle get_parent(Time t)const;

}; // END of class ValueNode_Bone

class ValueNode_Bone_Root : public ValueNode_Bone
{
public:

	ValueNode_Bone_Root();
	virtual ~ValueNode_Bone_Root();

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;
	virtual String get_bone_name(Time t)const;

	virtual void set_guid(const GUID& new_guid);
	virtual void set_root_canvas(etl::loose_handle<Canvas> canvas);

	virtual int link_count()const;
	virtual bool is_root()const { return true; }

private:
	Matrix get_animated_matrix(Time t, Point child_origin)const;

protected:
	LinkableValueNode* create_new()const;

public:
	static bool check_type(Type &type);
	static ValueNode_Bone* create(const ValueBase &x);

#ifdef _DEBUG
	virtual void ref()const;
	virtual bool unref()const;
	virtual void rref()const;
	virtual void runref()const;
#endif

}; // END of class ValueNode_Bone_Root

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
