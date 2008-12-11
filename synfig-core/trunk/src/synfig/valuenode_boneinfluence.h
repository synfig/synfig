/* === S Y N F I G ========================================================= */
/*!	\file valuenode_boneinfluence.h
**	\brief Header file for implementation of the "BoneInfluence" valuenode conversion.
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_BONEINFLUENCE_H
#define __SYNFIG_VALUENODE_BONEINFLUENCE_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_BoneInfluence : public LinkableValueNode
{
	ValueNode::RHandle vertex_free_;
	ValueNode::RHandle vertex_setup_;
	ValueNode::RHandle bone_weight_list_;
public:
	typedef etl::handle<ValueNode_BoneInfluence> Handle;
	typedef etl::handle<const ValueNode_BoneInfluence> ConstHandle;

	ValueNode_BoneInfluence(const ValueBase::Type &x);

	ValueNode_BoneInfluence(const ValueNode::Handle &x);

//	static Handle create(const ValueBase::Type &x);
//	static Handle create(const ValueNode::Handle &x);


	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_name(int i)const;

	virtual String link_local_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_BoneInfluence();

	virtual String get_name()const;

	virtual String get_local_name()const;

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_BoneInfluence* create(const ValueBase &x);
}; // END of class ValueNode_BoneInfluence

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
