/* === S Y N F I G ========================================================= */
/*!	\file valuenode_duplicate.cpp
**	\brief Implementation of the "Duplicate" valuenode conversion.
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

#include "valuenode_duplicate.h"
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

REGISTER_VALUENODE(ValueNode_Duplicate, RELEASE_VERSION_0_61_08, "duplicate", "Duplicate")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Duplicate::ValueNode_Duplicate(Type &x):
	LinkableValueNode(x),
	index()
{
}

ValueNode_Duplicate::ValueNode_Duplicate(const ValueBase &x):
	LinkableValueNode(x.get_type()),
	index(1.0)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("from", ValueNode_Const::create(Real(1.0)));
	set_link("to",   ValueNode_Const::create(x.get(Real())));
	set_link("step", ValueNode_Const::create(Real(1.0)));
}

ValueNode_Duplicate*
ValueNode_Duplicate::create(const ValueBase &x)
{
	return new ValueNode_Duplicate(x);
}

LinkableValueNode*
ValueNode_Duplicate::create_new()const
{
	return new ValueNode_Duplicate(get_type());
}

ValueNode_Duplicate::~ValueNode_Duplicate()
{
	unlink_all();
}

bool
ValueNode_Duplicate::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(from_, type_real);
	case 1: CHECK_TYPE_AND_SET_VALUE(to_,   type_real);
	case 2: CHECK_TYPE_AND_SET_VALUE(step_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Duplicate::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return from_;
	if(i==1) return to_;
	if(i==2) return step_;

	return 0;
}

void
ValueNode_Duplicate::reset_index(Time t)const
{
	Real from = (*from_)(t).get(Real());
	index = from;
}

bool
ValueNode_Duplicate::step(Time t)const
{
	Real from = (*from_)(t).get(Real());
	Real to   = (*to_  )(t).get(Real());
	Real step = (*step_)(t).get(Real());
	Real prev = index;

	if (step == 0) return false;

	step = std::fabs(step);

	if (from < to)
	{
		if ((index += step) <= to) return true;
	}
	else
		if ((index -= step) >= to) return true;

	// at the end of the loop, leave the index at the last value that was used
	index = prev;
	return false;
}

int
ValueNode_Duplicate::count_steps(Time t)const
{
	Real from = (*from_)(t).get(Real());
	Real to   = (*to_  )(t).get(Real());
	Real step = (*step_)(t).get(Real());

	if (step == 0) return 1;

	return std::fabs((from - to) / step) + 1;
}

ValueBase
ValueNode_Duplicate::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return index;
}



bool
ValueNode_Duplicate::check_type(Type &type)
{
	// never offer this as a choice.  it's used automatically by the 'Duplicate' layer.
	return false;
}

LinkableValueNode::Vocab
ValueNode_Duplicate::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"from")
		.set_local_name(_("From"))
		.set_description(_("Initial value of the index "))
	);

	ret.push_back(ParamDesc(ValueBase(),"to")
		.set_local_name(_("To"))
		.set_description(_("Final value of the index"))
	);

	ret.push_back(ParamDesc(ValueBase(),"step")
		.set_local_name(_("Step"))
		.set_description(_("Amount increment of the index"))
	);

	return ret;
}
