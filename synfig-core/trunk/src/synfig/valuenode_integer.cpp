/* === S Y N F I G ========================================================= */
/*!	\file valuenode_integer.cpp
**	\brief Implementation of the "Integer" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "valuenode_integer.h"
#include "valuenode_const.h"
#include "general.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Integer::ValueNode_Integer(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Integer::ValueNode_Integer(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	switch(x.get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("integer", ValueNode_Const::create(round_to_int(Angle::deg(x.get(Angle())).get())));
		break;
	case ValueBase::TYPE_BOOL:
		set_link("integer", ValueNode_Const::create(int(x.get(bool()))));
		break;
	case ValueBase::TYPE_REAL:
		set_link("integer", ValueNode_Const::create(round_to_int(x.get(Real()))));
		break;
	case ValueBase::TYPE_TIME:
		set_link("integer", ValueNode_Const::create(round_to_int(x.get(Time()))));
		break;
	default:
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+ValueBase::type_local_name(x.get_type()));
	}
}

ValueNode_Integer*
ValueNode_Integer::create(const ValueBase &x)
{
	return new ValueNode_Integer(x);
}

LinkableValueNode*
ValueNode_Integer::create_new()const
{
	return new ValueNode_Integer(get_type());
}

ValueNode_Integer::~ValueNode_Integer()
{
	unlink_all();
}

bool
ValueNode_Integer::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0:  integer_ = x; break;
	default: return false;
	}

	signal_child_changed()(i);
	signal_value_changed()();
	return true;
}

ValueNode::LooseHandle
ValueNode_Integer::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return integer_;

	return 0;
}

int
ValueNode_Integer::link_count()const
{
	return 1;
}

String
ValueNode_Integer::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return _("Integer");
	return String();
}

String
ValueNode_Integer::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return "integer";
	return String();
}

int
ValueNode_Integer::get_link_index_from_name(const String &name)const
{
	if(name=="integer") return 0;

	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_Integer::operator()(Time t)const
{
	int integer = (*integer_)(t).get(int());

	switch (get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return Angle::deg(integer);
	case ValueBase::TYPE_BOOL:
		return bool(integer);
	case ValueBase::TYPE_REAL:
		return Real(integer);
	case ValueBase::TYPE_TIME:
		return Time(integer);
	default:
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+ValueBase::type_local_name(get_type()));
	}
}

String
ValueNode_Integer::get_name()const
{
	return "fromint";
}

String
ValueNode_Integer::get_local_name()const
{
	return _("From Integer");
}

// don't show this to the user at the moment - maybe it's not very useful
bool
ValueNode_Integer::check_type(ValueBase::Type type)
{
	return false;
//	return
//		type==ValueBase::TYPE_ANGLE ||
//		type==ValueBase::TYPE_BOOL  ||
//		type==ValueBase::TYPE_REAL  ||
//		type==ValueBase::TYPE_TIME;
}
