/* === S Y N F I G ========================================================= */
/*!	\file valuenode_or.cpp
**	\brief Implementation of the "Or" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

#include "valuenode_or.h"
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

REGISTER_VALUENODE(ValueNode_Or, RELEASE_VERSION_0_62_00, "or", "OR")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Or::ValueNode_Or(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	bool value(x.get(bool()));

	set_link("link1",        ValueNode_Const::create(bool(false)));
	set_link("link2",        ValueNode_Const::create(bool(false)));
	if (value)
		set_link("link1",ValueNode_Const::create(bool(true)));
}

ValueNode_Or*
ValueNode_Or::create(const ValueBase &x)
{
	return new ValueNode_Or(x);
}

LinkableValueNode*
ValueNode_Or::create_new()const
{
	return new ValueNode_Or(get_type());
}

ValueNode_Or::~ValueNode_Or()
{
	unlink_all();
}

bool
ValueNode_Or::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link1_, type_bool);
	case 1: CHECK_TYPE_AND_SET_VALUE(link2_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Or::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link1_;
	if(i==1) return link2_;
	return 0;
}

ValueBase
ValueNode_Or::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	bool link1     = (*link1_)   (t).get(bool());
	bool link2     = (*link2_)   (t).get(bool());

	return (link1 || link2);
}



bool
ValueNode_Or::check_type(Type &type)
{
	return type==type_bool;
}

LinkableValueNode::Vocab
ValueNode_Or::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link1")
		.set_local_name(_("Link1"))
		.set_description(_("Value node used for the OR boolean operation"))
	);

	ret.push_back(ParamDesc(ValueBase(),"link2")
		.set_local_name(_("Link2"))
		.set_description(_("Value node used for the OR boolean operation"))
	);

	return ret;
}
