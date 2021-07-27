/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animated.h
**	\brief Header file for Valuenode_Animated.
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

#ifndef __SYNFIG_VALUENODE_ANIMATED_H
#define __SYNFIG_VALUENODE_ANIMATED_H

/* === H E A D E R S ======================================================= */

#include <synfig/canvas.h>

#include <synfig/valuenodes/valuenode_animatedinterface.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class ValueNode_Animated
 *  \brief Virtual class for the ValueNode Animated implementation.
*/
class ValueNode_Animated : public ValueNode, public ValueNode_AnimatedInterface
{
public:
	typedef std::shared_ptr<ValueNode_Animated> Handle;
	typedef std::shared_ptr<const ValueNode_Animated> ConstHandle;

	ValueNode::Handle clone(Canvas::LooseHandle canvas, const synfig::GUID& deriv_guid)const;

	//! Virtual member to be filled by inherited classes
	virtual String get_name()const;
	//! Virtual member to be filled by inherited classes
	virtual String get_local_name()const;

	String get_string()const;

	//! Creates a Valuenode_Animated by type
	static Handle create(Type &type);
	//! Creates a Valuenode_Animated by ValueBase and Time
	static Handle create(const ValueBase& value, const Time& time);
	//! Creates a Valuenode_Animated by ValueNode and Time
	static Handle create(ValueNode::Handle value_node, const Time& time);

	virtual ValueBase operator()(Time t) const;
	virtual void get_values_vfunc(std::map<Time, ValueBase> &x) const;

	virtual Interpolation get_interpolation()const
		{ return ValueNode_AnimatedInterfaceConst::get_interpolation(); }
	virtual void set_interpolation(Interpolation i)
		{ ValueNode_AnimatedInterfaceConst::set_interpolation(i); }

protected:
	ValueNode_Animated(Type &type);

	virtual void on_changed();
	virtual void get_times_vfunc(Node::time_set &set) const;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
