/* === S Y N F I G ========================================================= */
/*!	\file valuenode_boneinfluence.h
**	\brief Header file for implementation of the "BoneInfluence" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_VALUENODE_BONEINFLUENCE_H
#define __SYNFIG_VALUENODE_BONEINFLUENCE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>
#include <synfig/matrix.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_BoneInfluence : public LinkableValueNode
{
	ValueNode::RHandle bone_weight_list_;
	ValueNode::RHandle link_;

	mutable Matrix transform_, inverse_transform_;
	mutable bool checked_inverse_, has_inverse_;

	ValueNode_BoneInfluence(Type &x);
	ValueNode_BoneInfluence(const ValueNode::Handle &x, etl::loose_handle<Canvas> canvas);

public:
	typedef etl::handle<ValueNode_BoneInfluence> Handle;
	typedef etl::handle<const ValueNode_BoneInfluence> ConstHandle;

	static ValueNode_BoneInfluence* create(const ValueBase &x, etl::loose_handle<Canvas>);
	virtual ~ValueNode_BoneInfluence();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;

public:
	Matrix calculate_transform(Time t)const;
	Matrix& get_transform(bool rebuild=false, Time t=0)const;
	void set_transform(Matrix transform)const { transform_ = transform; checked_inverse_ = false; }
	bool has_inverse_transform()const;
	Matrix& get_inverse_transform()const;
}; // END of class ValueNode_BoneInfluence

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
