/* === S Y N F I G ========================================================= */
/*!	\file valuenode_weightedaverage.cpp
**	\brief Implementation of the "Weighted Average" valuenode conversion.
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

#include "valuenode_weightedaverage.h"

#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#include "valuenode_const.h"

#include <synfig/canvas.h>
#include <synfig/valueoperations.h>
#include <synfig/weightedvalue.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_WeightedAverage, RELEASE_VERSION_1_0, "weighted_average", "Weighted Average")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_WeightedAverage::ValueNode_WeightedAverage(const ValueBase &value, Canvas::LooseHandle canvas):
	ValueNode_DynamicList(ValueAverage::convert_to_weighted_type(value.get_type()), value.get_type(), canvas)
{
	if (!check_type(value.get_type()))
	{
		assert(0);
		throw std::runtime_error(get_local_name()+_(":Bad type ")+value.get_type().description.local_name);
	}
}

ValueNode_WeightedAverage::ValueNode_WeightedAverage(Type &type, Canvas::LooseHandle canvas):
	ValueNode_DynamicList(ValueAverage::convert_to_weighted_type(type), type, canvas)
{
	if (!check_type(type))
	{
		assert(0);
		throw std::runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}
}

ValueNode_WeightedAverage::~ValueNode_WeightedAverage() { }

ValueNode_WeightedAverage*
ValueNode_WeightedAverage::create(const ValueBase &value, Canvas::LooseHandle canvas)
{ 
	ValueNode_WeightedAverage* value_node(new ValueNode_WeightedAverage(value, canvas));
	
	types_namespace::TypeWeightedValueBase *t = ValueAverage::get_weighted_type_for(value_node->get_type());
	assert(t != NULL);

	value_node->ref();
	value_node->add(ValueNode::Handle(ValueNode_Const::create(t->create_weighted_value(1, value), canvas)));
	value_node->unref_inactive();
	
	return value_node;
}

ValueBase
ValueNode_WeightedAverage::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);
	return ValueAverage::average_weighted(ValueNode_DynamicList::operator()(t), ValueBase(get_type()));
}



LinkableValueNode*
ValueNode_WeightedAverage::create_new()const
	{ return new ValueNode_WeightedAverage(get_type()); }

bool
ValueNode_WeightedAverage::check_type(Type &type)
	{ return ValueAverage::check_type(type); }
