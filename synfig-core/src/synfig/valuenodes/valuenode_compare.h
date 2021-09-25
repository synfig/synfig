/* === S Y N F I G ========================================================= */
/*!	\file valuenode_compare.h
**	\brief Header file for implementation of the "Compare" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

#ifndef __SYNFIG_VALUENODE_COMPARE_H
#define __SYNFIG_VALUENODE_COMPARE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Compare : public LinkableValueNode
{
	ValueNode::RHandle lhs_;
	ValueNode::RHandle rhs_;
	ValueNode::RHandle greater_;
	ValueNode::RHandle equal_;
	ValueNode::RHandle less_;

	ValueNode_Compare(const ValueBase &x);

public:
	typedef etl::handle<ValueNode_Compare> Handle;
	typedef etl::handle<const ValueNode_Compare> ConstHandle;

	static ValueNode_Compare* create(const ValueBase &x);
	virtual ~ValueNode_Compare();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_Compare

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
