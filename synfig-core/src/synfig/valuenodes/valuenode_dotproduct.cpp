/* === S Y N F I G ========================================================= */
/*!	\file valuenode_dotproduct.cpp
**	\brief Implementation of the "DotProduct" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_dotproduct.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_DotProduct, RELEASE_VERSION_0_61_09, "dotproduct", "Dot Product")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_DotProduct::ValueNode_DotProduct(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(value.get_type());
	if (type == type_real)
	{
		set_link("lhs",ValueNode_Const::create(Vector(value.get(Real()),0)));
		set_link("rhs",ValueNode_Const::create(Vector(1,0)));
	}
	else
	if (type == type_angle)
	{
		set_link("lhs",ValueNode_Const::create(Vector(Angle::cos(value.get(Angle())).get(), Angle::sin(value.get(Angle())).get())));
		set_link("rhs",ValueNode_Const::create(Vector(1,0)));
	}
	else
		throw Exception::BadType(value.get_type().description.local_name);
}

LinkableValueNode*
ValueNode_DotProduct::create_new()const
{
	return new ValueNode_DotProduct(get_type());
}

ValueNode_DotProduct*
ValueNode_DotProduct::create(const ValueBase &x)
{
	return new ValueNode_DotProduct(x);
}

ValueNode_DotProduct::~ValueNode_DotProduct()
{
	unlink_all();
}

ValueBase
ValueNode_DotProduct::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Vector lhs((*lhs_)(t).get(Vector()));
	Vector rhs((*rhs_)(t).get(Vector()));

	Type &type(get_type());
	if (type == type_angle)
		return Angle::cos(lhs * rhs / lhs.mag() / rhs.mag()).mod();
	if (type == type_real)
		return lhs * rhs;

	assert(0);
	return ValueBase();
}



bool
ValueNode_DotProduct::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(lhs_, type_vector);
	case 1: CHECK_TYPE_AND_SET_VALUE(rhs_, type_vector);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_DotProduct::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return lhs_;
	case 1: return rhs_;
	}

	return nullptr;
}

bool
ValueNode_DotProduct::check_type(Type &type)
{
	return
		type==type_angle ||
		type==type_real;
}

LinkableValueNode::Vocab
ValueNode_DotProduct::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"lhs")
		.set_local_name(_("LHS"))
		.set_description(_("The left side of the dot product"))
	);

	ret.push_back(ParamDesc(ValueBase(),"rhs")
		.set_local_name(_("RHS"))
		.set_description(_("The right side of the dot product"))
	);

	return ret;
}
