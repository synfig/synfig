/* === S Y N F I G ========================================================= */
/*!	\file valuenode_average.cpp
**	\brief Implementation of the "Average" valuenode conversion.
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "valuenode_average.h"
#include "valuenode_const.h"
#include <synfig/valueoperations.h>
#include <synfig/canvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Average::ValueNode_Average(const ValueBase &value, Canvas::LooseHandle canvas):
	ValueNode_DynamicList(value.get_type(), value.get_type(), canvas)
{
	if (!check_type(value.get_type()))
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+value.get_type().description.local_name);
	}

	ref();
	add(ValueNode::Handle(ValueNode_Const::create(value, canvas)));
	unref_inactive();
}

ValueNode_Average::ValueNode_Average(Type &type, Canvas::LooseHandle canvas):
	ValueNode_DynamicList(type, type, canvas)
{
	if (!check_type(type))
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}
}

ValueNode_Average::~ValueNode_Average() { }

ValueNode_Average*
ValueNode_Average::create(const ValueBase &value, Canvas::LooseHandle canvas)
	{ return new ValueNode_Average(value, canvas); }

ValueBase
ValueNode_Average::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);
	return ValueAverage::average( ValueNode_DynamicList::operator()(t), ValueBase(), ValueBase(get_type()));
}

String
ValueNode_Average::get_name()const
	{ return "average"; }

String
ValueNode_Average::get_local_name()const
	{ return _("Average"); }

LinkableValueNode*
ValueNode_Average::create_new()const
	{ return new ValueNode_Average(get_type()); }

bool
ValueNode_Average::check_type(Type &type)
	{ return ValueAverage::check_type(type); }
