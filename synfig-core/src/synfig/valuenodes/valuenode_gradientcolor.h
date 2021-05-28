/* === S Y N F I G ========================================================= */
/*!	\file valuenode_gradientcolor.h
**	\brief Header file for implementation of the "Gradient Color" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_GRADIENTCOLOR_H
#define __SYNFIG_VALUENODE_GRADIENTCOLOR_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_GradientColor : public LinkableValueNode
{
	ValueNode::RHandle gradient_;
	ValueNode::RHandle index_;
	ValueNode::RHandle loop_;

	ValueNode_GradientColor(const ValueBase &value);

public:

	typedef etl::handle<ValueNode_GradientColor> Handle;
	typedef etl::handle<const ValueNode_GradientColor> ConstHandle;


	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_GradientColor();

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
	static ValueNode_GradientColor* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_GradientColor

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
