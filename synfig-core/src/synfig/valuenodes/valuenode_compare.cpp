/* === S Y N F I G ========================================================= */
/*!	\file valuenode_compare.cpp
**	\brief Implementation of the "Compare" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_compare.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Compare, RELEASE_VERSION_0_62_00, "compare", N_("Compare"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Compare::ValueNode_Compare(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	bool value(x.get(bool()));

	set_link("lhs",          ValueNode_Const::create(Real(0)));
	set_link("rhs",          ValueNode_Const::create(Real(0)));
	set_link("greater",      ValueNode_Const::create(bool(false)));
	if (value)
		set_link("equal",ValueNode_Const::create(bool(true)));
	else
		set_link("equal",ValueNode_Const::create(bool(false)));
	set_link("less",         ValueNode_Const::create(bool(false)));
}

ValueNode_Compare*
ValueNode_Compare::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_Compare(x);
}

LinkableValueNode*
ValueNode_Compare::create_new()const
{
	return new ValueNode_Compare(get_type());
}

ValueNode_Compare::~ValueNode_Compare()
{
	unlink_all();
}

bool
ValueNode_Compare::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(lhs_,      type_real);
	case 1: CHECK_TYPE_AND_SET_VALUE(rhs_,      type_real);
	case 2: CHECK_TYPE_AND_SET_VALUE(greater_,  type_bool);
	case 3: CHECK_TYPE_AND_SET_VALUE(equal_,    type_bool);
	case 4: CHECK_TYPE_AND_SET_VALUE(less_,     type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Compare::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return lhs_;
	if(i==1) return rhs_;
	if(i==2) return greater_;
	if(i==3) return equal_;
	if(i==4) return less_;
	return 0;
}

ValueBase
ValueNode_Compare::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Real lhs      = (*lhs_)     (t).get(Real());
	Real rhs      = (*rhs_)     (t).get(Real());
	Real greater  = (*greater_) (t).get(bool());
	Real equal    = (*equal_)   (t).get(bool());
	Real less     = (*less_)    (t).get(bool());

	if (greater && lhs > rhs)
		return true;
	if (equal && lhs == rhs)
		return true;
	if (less && lhs < rhs)
		return true;

	return false;
}



bool
ValueNode_Compare::check_type(Type &type)
{
	return type==type_bool;
}

LinkableValueNode::Vocab
ValueNode_Compare::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"lhs")
		.set_local_name(_("LHS"))
		.set_description(_("The left side of the comparison"))
	);

	ret.push_back(ParamDesc(ValueBase(),"rhs")
		.set_local_name(_("RHS"))
		.set_description(_("The right side of the comparison"))
	);

	ret.push_back(ParamDesc(ValueBase(),"greater")
		.set_local_name(_("Greater"))
		.set_description(_("When checked, returns true if LHS > RHS"))
	);

	ret.push_back(ParamDesc(ValueBase(),"equal")
		.set_local_name(_("Equal"))
		.set_description(_("When checked, returns true if LHS = RHS"))
	);

	ret.push_back(ParamDesc(ValueBase(),"less")
		.set_local_name(_("Less"))
		.set_description(_("When checked, returns true if LHS < RHS"))
	);

	return ret;
}
