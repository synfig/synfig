/* === S Y N F I G ========================================================= */
/*!	\file valuenode_not.cpp
**	\brief Implementation of the "Not" valuenode conversion.
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

#include "valuenode_not.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Not, RELEASE_VERSION_0_62_00, "not", "NOT")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Not::ValueNode_Not(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	bool value(x.get(bool()));

	set_link("link",         ValueNode_Const::create(!value));
}

ValueNode_Not*
ValueNode_Not::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_Not(x);
}

LinkableValueNode*
ValueNode_Not::create_new()const
{
	return new ValueNode_Not(get_type());
}

ValueNode_Not::~ValueNode_Not()
{
	unlink_all();
}

bool
ValueNode_Not::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Not::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;
	return 0;
}

ValueBase
ValueNode_Not::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	bool link      = (*link_)    (t).get(bool());

	return !link;
}



bool
ValueNode_Not::check_type(Type &type)
{
	return type==type_bool;
}

LinkableValueNode::Vocab
ValueNode_Not::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("Value node used to do the NOT operation"))
	);

	return ret;
}
