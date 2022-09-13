/* === S Y N F I G ========================================================= */
/*!	\file valuenode_average.cpp
**	\brief Implementation of the "Average" valuenode conversion.
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode_average.h"

#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#include "valuenode_const.h"

#include <synfig/canvas.h>
#include <synfig/valueoperations.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Average, RELEASE_VERSION_1_0, "average", N_("Average"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Average::ValueNode_Average(const ValueBase &value, Canvas::LooseHandle canvas):
	ValueNode_DynamicList(value.get_type(), value.get_type(), canvas)
{
	if (!check_type(value.get_type()))
	{
		assert(0);
		throw std::runtime_error(get_local_name()+_(":Bad type ")+value.get_type().description.local_name);
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
		throw std::runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}
}

ValueNode_Average::~ValueNode_Average() { }

ValueNode_Average*
ValueNode_Average::create(const ValueBase& value, Canvas::LooseHandle canvas)
	{ return new ValueNode_Average(value, canvas); }

ValueBase
ValueNode_Average::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);
	return ValueAverage::average( ValueNode_DynamicList::operator()(t), ValueBase(), ValueBase(get_type()));
}



LinkableValueNode*
ValueNode_Average::create_new()const
	{ return new ValueNode_Average(get_type(), nullptr); }

bool
ValueNode_Average::check_type(Type &type)
	{ return ValueAverage::check_type(type); }
