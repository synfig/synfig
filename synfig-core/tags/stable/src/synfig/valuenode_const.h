/* === S Y N F I G ========================================================= */
/*!	\file valuenode_const.h
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

#ifndef __SYNFIG_VALUENODE_CONST_H
#define __SYNFIG_VALUENODE_CONST_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

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
	ValueNode_Const(const ValueBase &x);

public:

	virtual ValueBase operator()(Time t)const;
	virtual ~ValueNode_Const();

	const ValueBase &get_value()const;
	ValueBase &get_value();
	void set_value(const ValueBase &data);


	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode* clone(const GUID& deriv_guid=GUID())const;

public:
	static ValueNode_Const* create(const ValueBase &x=ValueBase());

protected:
	virtual void get_times_vfunc(Node::time_set &set) const;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
