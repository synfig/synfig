/* === S Y N F I G ========================================================= */
/*!	\file valuenode_const.cpp
**	\brief Implementation of the "Constant" valuenode conversion.
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode_const.h"
#include "valuenode_bone.h"
#include "valuenode_boneweightpair.h"
#include "valuenode_composite.h"
#include <synfig/canvas.h>
#include <synfig/localization.h>
#include <synfig/pair.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Const::ValueNode_Const()
{
}


ValueNode_Const::ValueNode_Const(const ValueBase &x, Canvas::LooseHandle canvas):
	ValueNode	(x.get_type()),
	value		(x)
{
	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set parent canvas for const %p to %p\n", __FILE__, __LINE__, this, canvas.get());

	if (x.get_type() == type_bone_valuenode)
		add_child(x.get(ValueNode_Bone::Handle()).get());

	set_parent_canvas(canvas);
}


ValueNode*
ValueNode_Const::create(const ValueBase& x, Canvas::LooseHandle canvas)
{
	// this is nasty - shouldn't it be done somewhere else?
	if (x.get_type() == type_bone_object)
	{
		printf("%s:%d forcing convert to ValueNode_Bone\n", __FILE__, __LINE__);
		return ValueNode_Bone::create(x, canvas);
	}

	// this too
	if (x.get_type() == type_bone_weight_pair)
	{
		printf("%s:%d forcing convert to ValueNode_BoneWeightPair\n", __FILE__, __LINE__);
		return ValueNode_BoneWeightPair::create(x, canvas);
	}

	// and this
	if (dynamic_cast<types_namespace::TypePairBase*>(&x.get_type()))
	{
		printf("%s:%d forcing convert to ValueNode_Composite\n", __FILE__, __LINE__);
		return ValueNode_Composite::create(x, canvas);
	}

	return new ValueNode_Const(x, canvas);
}


ValueNode::Handle
ValueNode_Const::clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid)const
{
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
	ValueNode* ret(new ValueNode_Const(value));
	ret->set_guid(get_guid()^deriv_guid);
	ret->set_parent_canvas(canvas);
	return ret;
}


ValueNode_Const::~ValueNode_Const()
{
	if (get_value().get_type() == type_bone_valuenode)
		remove_child(get_value().get(ValueNode_Bone::Handle()).get());
}


ValueBase
ValueNode_Const::operator()(Time /*t*/)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

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
		if (value.get_type() == type_bone_valuenode)
			remove_child(value.get(ValueNode_Bone::Handle()).get());

		if (data.get_type() == type_bone_valuenode)
			add_child(data.get(ValueNode_Bone::Handle()).get());

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
#ifdef _DEBUG
String
ValueNode_Const::get_string()const
{
	return strprintf("ValueNode_Const (%s)", get_value().get_string().c_str());
}
#endif	// _DEBUG
void ValueNode_Const::get_times_vfunc(Node::time_set &/*set*/) const
{
}

void ValueNode_Const::get_values_vfunc(std::map<Time, ValueBase> &x) const
{
	add_value_to_map(x, 0, value);
}
