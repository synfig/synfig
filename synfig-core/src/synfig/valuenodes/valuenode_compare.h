/* === S Y N F I G ========================================================= */
/*!	\file valuenode_compare.h
**	\brief Header file for implementation of the "Compare" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
**  Copyright (c) 2011 Carlos López
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

public:
	typedef etl::handle<ValueNode_Compare> Handle;
	typedef etl::handle<const ValueNode_Compare> ConstHandle;

	ValueNode_Compare(const ValueBase &x);

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_Compare();

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
	static ValueNode_Compare* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Compare

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
