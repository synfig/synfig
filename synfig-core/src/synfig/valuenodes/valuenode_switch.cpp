/* === S Y N F I G ========================================================= */
/*!	\file valuenode_switch.cpp
**	\brief Implementation of the "Switch" valuenode conversion.
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

#include "valuenode_switch.h"
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

REGISTER_VALUENODE(ValueNode_Switch, RELEASE_VERSION_0_61_08, "switch", "Switch")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Switch::ValueNode_Switch(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Switch::ValueNode_Switch(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("link_off",ValueNode_Const::create(x));
	set_link("link_on",ValueNode_Const::create(x));
	set_link("switch",ValueNode_Const::create(bool(false)));
}

ValueNode_Switch*
ValueNode_Switch::create(const ValueBase &x)
{
	return new ValueNode_Switch(x);
}

LinkableValueNode*
ValueNode_Switch::create_new()const
{
	return new ValueNode_Switch(get_type());
}

ValueNode_Switch::~ValueNode_Switch()
{
	unlink_all();
}

bool
ValueNode_Switch::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_off_, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(link_on_,  get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(switch_,   type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Switch::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return link_off_;
	case 1: return link_on_;
	case 2: return switch_;
	}
	return 0;
}

ValueBase
ValueNode_Switch::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return (*switch_)(t).get(bool()) ? (*link_on_)(t) : (*link_off_)(t);
}




bool
ValueNode_Switch::check_type(Type &type)
{
	if(type != type_nil)
		return true;
	return false;
}

LinkableValueNode::Vocab
ValueNode_Switch::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link_off")
		.set_local_name(_("Link Off"))
		.set_description(_("The value node returned when the switch is off"))
	);

	ret.push_back(ParamDesc(ValueBase(),"link_on")
		.set_local_name(_("Link On"))
		.set_description(_("The value node returned when the switch is on"))
	);

	ret.push_back(ParamDesc(ValueBase(),"switch")
		.set_local_name(_("Switch"))
		.set_description(_("When checked, returns 'Link On', otherwise returns 'Link Off'"))
	);

	return ret;
}
