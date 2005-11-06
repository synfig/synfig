/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: valuenode_const.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode_const.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Const::ValueNode_Const()
{
	DCAST_HACK_ENABLE();
}


ValueNode_Const::ValueNode_Const(const ValueBase &x):
	ValueNode	(x.get_type()),
	value		(x)
{
	DCAST_HACK_ENABLE();
}


ValueNode_Const*
ValueNode_Const::create(const ValueBase &x)
{
	return new ValueNode_Const(x);
}


ValueNode*
ValueNode_Const::clone(const GUID& deriv_guid)const
{
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
	ValueNode* ret(new ValueNode_Const(value));
	ret->set_guid(get_guid()^deriv_guid);
	return ret;
}


ValueNode_Const::~ValueNode_Const()
{
}


ValueBase
ValueNode_Const::operator()(Time t)const
{
	return value;
}


const ValueBase &
ValueNode_Const::get_value()const
{
	return value;
}

ValueBase &
ValueNode_Const::get_value()
{
	return value;
}

void
ValueNode_Const::set_value(const ValueBase &data)
{
	if(data!=value)
	{
		value=data;
		changed();
	}
}


String
ValueNode_Const::get_name()const
{
	return "constant";
}

String
ValueNode_Const::get_local_name()const
{
	return _("Constant");
}

void ValueNode_Const::get_times_vfunc(Node::time_set &set) const
{
}
