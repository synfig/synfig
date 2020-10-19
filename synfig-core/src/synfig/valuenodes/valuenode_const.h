/* === S Y N F I G ========================================================= */
/*!	\file valuenode_const.h
**	\brief Header file for implementation of the "Constant" valuenode conversion.
**
**	$Id$
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

#ifndef __SYNFIG_VALUENODE_CONST_H
#define __SYNFIG_VALUENODE_CONST_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Const : public ValueNode
{
public:
	typedef etl::handle<ValueNode_Const> Handle;
	typedef etl::handle<const ValueNode_Const> ConstHandle;

private:
	ValueBase value;

	ValueNode_Const();
	ValueNode_Const(const ValueBase &x, etl::loose_handle<Canvas> canvas = 0);

public:

	virtual ValueBase operator()(Time t)const;
	virtual ~ValueNode_Const();

	const ValueBase &get_value()const;
	ValueBase &get_value();
	void set_value(const ValueBase &data);

	bool get_static()const {return get_value().get_static();}
	void set_static(bool x) { get_value().set_static(x); }
	virtual Interpolation get_interpolation()const {return get_value().get_interpolation();}
	virtual void set_interpolation(Interpolation x) { get_value().set_interpolation(x); }
	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;
#ifdef _DEBUG
	String get_string()const;
#endif	// _DEBUG
public:
	// create a new ValueNode_Const object with the given value.
	// Unless the given value is a Bone, in which case make a ValueNode_Bone.
	static ValueNode* create(const ValueBase &x=ValueBase(), etl::loose_handle<Canvas> canvas = 0);

protected:
	virtual void get_times_vfunc(Node::time_set &set) const;
	virtual void get_values_vfunc(std::map<Time, ValueBase> &x) const;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
