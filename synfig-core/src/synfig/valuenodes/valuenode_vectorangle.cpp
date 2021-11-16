/* === S Y N F I G ========================================================= */
/*!	\file valuenode_vectorangle.cpp
**	\brief Implementation of the "Vector Angle" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_vectorangle.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_VectorAngle, RELEASE_VERSION_0_61_09, "vectorangle", "Vector Angle")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_VectorAngle::ValueNode_VectorAngle(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_angle)
		set_link("vector",ValueNode_Const::create(Vector(Angle::cos(value.get(Angle())).get(),
														 Angle::sin(value.get(Angle())).get())));
	else
		throw Exception::BadType(value.get_type().description.local_name);
}

LinkableValueNode*
ValueNode_VectorAngle::create_new()const
{
	return new ValueNode_VectorAngle(get_type());
}

ValueNode_VectorAngle*
ValueNode_VectorAngle::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_VectorAngle(x);
}

ValueNode_VectorAngle::~ValueNode_VectorAngle()
{
	unlink_all();
}

ValueBase
ValueNode_VectorAngle::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return (*vector_)(t).get(Vector()).angle();
}


bool
ValueNode_VectorAngle::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(vector_, type_vector);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_VectorAngle::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return vector_;
	return 0;
}



bool
ValueNode_VectorAngle::check_type(Type &type)
{
	return type==type_angle;
}


LinkableValueNode::Vocab
ValueNode_VectorAngle::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"vector")
		.set_local_name(_("Vector"))
		.set_description(_("The vector where the angle is calculated from"))
	);

	return ret;
}
