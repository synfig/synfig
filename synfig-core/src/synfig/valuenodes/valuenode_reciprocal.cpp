/* === S Y N F I G ========================================================= */
/*!	\file valuenode_reciprocal.cpp
**	\brief Implementation of the "Reciprocal" valuenode conversion.
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

#include "valuenode_reciprocal.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Reciprocal, RELEASE_VERSION_0_61_08, "reciprocal", "Reciprocal")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Reciprocal::ValueNode_Reciprocal(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Real value(x.get(Real()));
	Real infinity(999999.0);
	Real epsilon(0.000001);

	if (value == 0)
		value = infinity;
	else
		value = 1.0/value;

	set_link("link",     ValueNode_Const::create(Real(value)));
	set_link("epsilon",  ValueNode_Const::create(Real(epsilon)));
	set_link("infinite", ValueNode_Const::create(Real(infinity)));
}

ValueNode_Reciprocal*
ValueNode_Reciprocal::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_Reciprocal(x);
}

LinkableValueNode*
ValueNode_Reciprocal::create_new()const
{
	return new ValueNode_Reciprocal(get_type());
}

ValueNode_Reciprocal::~ValueNode_Reciprocal()
{
	unlink_all();
}

bool
ValueNode_Reciprocal::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,     type_real);
	case 1: CHECK_TYPE_AND_SET_VALUE(epsilon_,  type_real);
	case 2: CHECK_TYPE_AND_SET_VALUE(infinite_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Reciprocal::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;
	if(i==1) return epsilon_;
	if(i==2) return infinite_;

	return 0;
}

ValueBase
ValueNode_Reciprocal::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Real link     = (*link_)    (t).get(Real());
	Real epsilon  = (*epsilon_) (t).get(Real());
	Real infinite = (*infinite_)(t).get(Real());

	if (epsilon < 0.00000001)
		epsilon = 0.00000001;

	if (std::fabs(link) < epsilon)
		if (link < 0)
			return -infinite;
		else
			return infinite;
	else
		return 1.0f / link;
}



bool
ValueNode_Reciprocal::check_type(Type &type)
{
	return type==type_real;
}

LinkableValueNode::Vocab
ValueNode_Reciprocal::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node used to calculate its reciprocal"))
	);

	ret.push_back(ParamDesc(ValueBase(),"epsilon")
		.set_local_name(_("Epsilon"))
		.set_description(_("The value used to decide whether 'Link' is too small to obtain its reciprocal"))
	);

		ret.push_back(ParamDesc(ValueBase(),"infinite")
		.set_local_name(_("Infinite"))
		.set_description(_("The resulting value when 'Link' < 'Epsilon'"))
	);

	return ret;
}
