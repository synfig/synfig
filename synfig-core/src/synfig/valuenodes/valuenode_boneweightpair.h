/* === S Y N F I G ========================================================= */
/*!	\file valuenode_boneweightpair.h
**	\brief Header file for implementation of the "BoneWeightPair" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2008 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_BONEWEIGHTPAIR_H
#define __SYNFIG_VALUENODE_BONEWEIGHTPAIR_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_BoneWeightPair : public LinkableValueNode
{
	ValueNode::RHandle bone_;
	ValueNode::RHandle weight_;

	ValueNode_BoneWeightPair(const ValueBase &value, etl::loose_handle<Canvas> = 0);

public:

	typedef etl::handle<ValueNode_BoneWeightPair> Handle;
	typedef etl::handle<const ValueNode_BoneWeightPair> ConstHandle;


	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_BoneWeightPair();

	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

protected:
	LinkableValueNode* create_new()const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

public:
	using synfig::LinkableValueNode::get_link_vfunc;

	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_BoneWeightPair* create(const ValueBase &x, etl::loose_handle<Canvas>);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_BoneWeightPair

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
