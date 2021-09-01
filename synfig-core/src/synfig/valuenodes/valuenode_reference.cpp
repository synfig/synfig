/* === S Y N F I G ========================================================= */
/*!	\file valuenode_reference.cpp
**	\brief Implementation of the "Reference" valuenode conversion.
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

#include "valuenode_reference.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Reference, RELEASE_VERSION_0_61_06, "reference", "Reference")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Reference::ValueNode_Reference(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Reference::ValueNode_Reference(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("link",x);
}

ValueNode_Reference*
ValueNode_Reference::create(const ValueBase &x)
{
	return new ValueNode_Reference(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_Reference::create_new()const
{
	return new ValueNode_Reference(get_type());
}

ValueNode_Reference::~ValueNode_Reference()
{
	unlink_all();
}

bool
ValueNode_Reference::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_, get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Reference::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	return link_;
}

ValueBase
ValueNode_Reference::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return (*link_)(t);
}




bool
ValueNode_Reference::check_type(Type &type)
{
	if(type != type_nil)
		return true;
	return false;
}

LinkableValueNode::Vocab
ValueNode_Reference::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The referenced value"))
	);

	return ret;
}
