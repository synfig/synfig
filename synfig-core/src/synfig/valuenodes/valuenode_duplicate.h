/* === S Y N F I G ========================================================= */
/*!	\file valuenode_duplicate.h
**	\brief Header file for implementation of the "Duplicate" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_VALUENODE_DUPLICATE_H
#define __SYNFIG_VALUENODE_DUPLICATE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Duplicate : public LinkableValueNode
{
	ValueNode::RHandle from_;
	ValueNode::RHandle to_;
	ValueNode::RHandle step_;
	mutable Real index;

	ValueNode_Duplicate(Type &x);
	ValueNode_Duplicate(const ValueBase &x);

public:
	typedef etl::handle<ValueNode_Duplicate> Handle;
	typedef etl::handle<const ValueNode_Duplicate> ConstHandle;

	static ValueNode_Duplicate* create(const ValueBase &x);
	virtual ~ValueNode_Duplicate();

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	virtual ValueBase operator()(Time t) const override;

	void reset_index(Time t) const;
	bool step(Time t) const;
	int count_steps(Time t) const;

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_Duplicate

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
