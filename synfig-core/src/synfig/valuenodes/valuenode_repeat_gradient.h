/* === S Y N F I G ========================================================= */
/*!	\file valuenode_repeat_gradient.h
**	\brief Header file for implementation of the "Repeat Gradient" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_VALUENODE_REPEAT_GRADIENT_H
#define __SYNFIG_VALUENODE_REPEAT_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct ValueNode_Repeat_Gradient : public LinkableValueNode
{
	typedef etl::handle<ValueNode_Repeat_Gradient> Handle;
	typedef etl::handle<const ValueNode_Repeat_Gradient> ConstHandle;

protected:

	ValueNode_Repeat_Gradient(const Gradient& x);

private:

	ValueNode::RHandle gradient_;
	ValueNode::RHandle count_;
	ValueNode::RHandle width_;
	ValueNode::RHandle specify_start_;
	ValueNode::RHandle specify_end_;
	ValueNode::RHandle start_color_;
	ValueNode::RHandle end_color_;

public:

	virtual ~ValueNode_Repeat_Gradient();

	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

//	static bool check_type(Type &type);

	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_Repeat_Gradient* create(const ValueBase &x=type_gradient);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Repeat_Gradient

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
