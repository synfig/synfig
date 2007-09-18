/* === S Y N F I G ========================================================= */
/*!	\file valuenode_subtract.h
**	\brief Template Header
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

#ifndef __SYNFIG_VALUENODE_SUBTRACT_H
#define __SYNFIG_VALUENODE_SUBTRACT_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct ValueNode_Subtract : public LinkableValueNode
{
	typedef etl::handle<ValueNode_Subtract> Handle;
	typedef etl::handle<const ValueNode_Subtract> ConstHandle;

protected:

	ValueNode_Subtract(const ValueBase &value);

private:

	ValueNode::RHandle ref_a;
	ValueNode::RHandle ref_b;
	ValueNode::RHandle scalar;

public:

	virtual ~ValueNode_Subtract();

//	static Handle create(ValueBase::Type id=ValueBase::TYPE_NIL);

	//! Sets the left-hand-side value_node
	bool set_lhs(ValueNode::Handle a);

	//! Gets the left-hand-side value_node
	ValueNode::Handle get_lhs()const { return ref_a; }

	//! Sets the right-hand-side value_node
	bool set_rhs(ValueNode::Handle b);

	//! Gets the right-hand-side value_node
	ValueNode::Handle get_rhs()const { return ref_b; }

	//! Sets the scalar value_node
	bool set_scalar(ValueNode::Handle x);

	//! Gets the scalar value_node
	ValueNode::Handle get_scalar()const { return scalar; }

	void set_scalar(Real x);

	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_local_name(int i)const;
	virtual String link_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

//	static bool check_type(const ValueBase::Type &type);

	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_Subtract* create(const ValueBase &value=ValueBase());
}; // END of class ValueNode_Subtract

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
