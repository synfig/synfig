/* === S Y N F I G ========================================================= */
/*!	\file valuenode_gradientcolor.cpp
**	\brief Implementation of the "Gradient Color" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_gradientcolor.h"
#include "valuenode_const.h"
#include "gradient.h"
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

ValueNode_GradientColor::ValueNode_GradientColor(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_COLOR:
		set_link("gradient", ValueNode_Const::create(Gradient(value.get(Color()),value.get(Color()))));
		set_link("index",    ValueNode_Const::create(Real(0.5)));
		set_link("loop",    ValueNode_Const::create(bool(false)));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}
}

LinkableValueNode*
ValueNode_GradientColor::create_new()const
{
	return new ValueNode_GradientColor(get_type());
}

ValueNode_GradientColor*
ValueNode_GradientColor::create(const ValueBase &x)
{
	return new ValueNode_GradientColor(x);
}

ValueNode_GradientColor::~ValueNode_GradientColor()
{
	unlink_all();
}

ValueBase
ValueNode_GradientColor::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Real index((*index_)(t).get(Real()));
	bool loop((*loop_)(t).get(bool()));
	if (loop) index -= floor(index);
	return (*gradient_)(t).get(Gradient())(index);
}


bool
ValueNode_GradientColor::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(gradient_,	ValueBase::TYPE_GRADIENT);
	case 1: CHECK_TYPE_AND_SET_VALUE(index_,	ValueBase::TYPE_REAL);
	case 2: CHECK_TYPE_AND_SET_VALUE(loop_,		ValueBase::TYPE_BOOL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_GradientColor::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return gradient_;
		case 1: return index_;
		case 2: return loop_;
	}

	return 0;
}

int
ValueNode_GradientColor::link_count()const
{
	return 3;
}

String
ValueNode_GradientColor::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return _("Gradient");
		case 1: return _("Index");
		case 2: return _("Loop");
	}
	return String();
}

String
ValueNode_GradientColor::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return "gradient";
		case 1: return "index";
		case 2: return "loop";
	}
	return String();
}

int
ValueNode_GradientColor::get_link_index_from_name(const String &name)const
{
	if (name=="gradient") return 0;
	if (name=="index")	  return 1;
	if (name=="loop")	  return 2;
	throw Exception::BadLinkName(name);
}

String
ValueNode_GradientColor::get_name()const
{
	return "gradientcolor";
}

String
ValueNode_GradientColor::get_local_name()const
{
	return _("Gradient Color");
}

bool
ValueNode_GradientColor::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_COLOR;
}
