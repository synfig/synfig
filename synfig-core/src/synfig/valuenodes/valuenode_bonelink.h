/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bonelink.h
**	\brief Header file for implementation of the "BoneLink" valuenode conversion.
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#ifndef __SYNFIG_VALUENODE_BONELINK_H
#define __SYNFIG_VALUENODE_BONELINK_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_BoneLink : public LinkableValueNode
{
	ValueNode::RHandle bone_;
	ValueNode::RHandle base_value_;
	ValueNode::RHandle translate_;
	ValueNode::RHandle rotate_;
	ValueNode::RHandle skew_;
	ValueNode::RHandle scale_x_;
	ValueNode::RHandle scale_y_;

public:
	typedef etl::handle<ValueNode_BoneLink> Handle;
	typedef etl::handle<const ValueNode_BoneLink> ConstHandle;

	ValueNode_BoneLink(const ValueBase &x);

	Transformation get_bone_transformation(Time t)const;
	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_BoneLink();

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
	static ValueNode_BoneLink* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
	virtual void set_root_canvas(etl::loose_handle<Canvas> canvas);
}; // END of class ValueNode_Pow

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
