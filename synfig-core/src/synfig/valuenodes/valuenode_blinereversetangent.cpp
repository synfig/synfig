/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinereversetangent.cpp
**	\brief Implementation of the "Reverse Tangent" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_blinereversetangent.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <ETL/hermite>
#include <ETL/calculus>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BLineRevTangent, RELEASE_VERSION_0_61_08, "blinerevtangent", "Reverse Tangent")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineRevTangent::ValueNode_BLineRevTangent(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_BLineRevTangent::ValueNode_BLineRevTangent(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if(x->get_type()!=type_bline_point)
		throw Exception::BadType(x->get_type().description.local_name);

	set_link("reference",x);
	set_link("reverse",ValueNode_Const::create(bool(false)));
}

ValueNode_BLineRevTangent*
ValueNode_BLineRevTangent::create(const ValueBase &x)
{
	return new ValueNode_BLineRevTangent(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_BLineRevTangent::create_new()const
{
	return new ValueNode_BLineRevTangent(get_type());
}

ValueNode_BLineRevTangent::~ValueNode_BLineRevTangent()
{
	unlink_all();
}

ValueBase
ValueNode_BLineRevTangent::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if ((*reverse_)(t).get(bool()))
	{
		BLinePoint reference((*reference_)(t).get(BLinePoint()));
		BLinePoint ret(reference);
		ret.reverse();
		return ret;
	}
	else
		return (*reference_)(t);
}



bool
ValueNode_BLineRevTangent::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(reference_, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(reverse_,   type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BLineRevTangent::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return reference_;
		case 1: return reverse_;
	}

	return 0;
}

bool
ValueNode_BLineRevTangent::check_type(Type &type)
{
	return (type==type_bline_point);
}

LinkableValueNode::Vocab
ValueNode_BLineRevTangent::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"reference")
		.set_local_name(_("Reference"))
		.set_description(_("The referenced tangent to reverse"))
	);

	ret.push_back(ParamDesc(ValueBase(),"reverse")
		.set_local_name(_("Reverse"))
		.set_description(_("When checked, the reference is reversed"))
	);

	return ret;
}
