/* === S Y N F I G ========================================================= */
/*!	\file valuenode_twotone.h
**	\brief Template Header
**
**	$Id: valuenode_gradientrotate.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_VALUENODE_GRADIENTROTATE_H
#define __SYNFIG_VALUENODE_GRADIENTROTATE_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct ValueNode_GradientRotate : public LinkableValueNode
{
	typedef etl::handle<ValueNode_GradientRotate> Handle;
	typedef etl::handle<const ValueNode_GradientRotate> ConstHandle;
	
protected:

	ValueNode_GradientRotate();

private:

	ValueNode::RHandle ref_gradient;
	ValueNode::RHandle ref_offset;

public:

	virtual ~ValueNode_GradientRotate();

//	static Handle create(ValueBase::Type id=ValueBase::TYPE_GRADIENT);

	//! Sets the left-hand-side value_node
	bool set_gradient(ValueNode::Handle a);

	//! Gets the left-hand-side value_node
	ValueNode::Handle get_gradient()const { return ref_gradient; }

	//! Sets the right-hand-side value_node
	bool set_offset(ValueNode::Handle b);

	//! Gets the right-hand-side value_node
	ValueNode::Handle get_offset()const { return ref_gradient; }



	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_local_name(int i)const;
	virtual String link_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

//	static bool check_type(const ValueBase::Type &type);
protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::get_link_vfunc;

	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_GradientRotate* create(const ValueBase &x=ValueBase::TYPE_GRADIENT);
}; // END of class ValueNode_GradientRotate

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
