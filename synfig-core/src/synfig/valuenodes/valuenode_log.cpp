/* === S Y N F I G ========================================================= */
/*!	\file valuenode_log.cpp
**	\brief Implementation of the "Natural Logarithm" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008, 2011 Carlos López
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

#include "valuenode_log.h"
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

REGISTER_VALUENODE(ValueNode_Logarithm, RELEASE_VERSION_0_61_09, "logarithm", "Logarithm")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Logarithm::ValueNode_Logarithm(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Real value(x.get(Real()));
	Real infinity(999999.0);
	Real epsilon(0.000001);

	value = exp(value);

	set_link("link",     ValueNode_Const::create(Real(value)));
	set_link("epsilon",  ValueNode_Const::create(Real(epsilon)));
	set_link("infinite", ValueNode_Const::create(Real(infinity)));
}

ValueNode_Logarithm*
ValueNode_Logarithm::create(const ValueBase &x)
{
	return new ValueNode_Logarithm(x);
}

LinkableValueNode*
ValueNode_Logarithm::create_new()const
{
	return new ValueNode_Logarithm(get_type());
}

ValueNode_Logarithm::~ValueNode_Logarithm()
{
	unlink_all();
}

bool
ValueNode_Logarithm::set_link_vfunc(int i,ValueNode::Handle value)
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
ValueNode_Logarithm::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;
	if(i==1) return epsilon_;
	if(i==2) return infinite_;

	return 0;
}

ValueBase
ValueNode_Logarithm::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Real link     = (*link_)    (t).get(Real());
	Real epsilon  = (*epsilon_) (t).get(Real());
	Real infinite = (*infinite_)(t).get(Real());

	if (epsilon < 0.00000001)
		epsilon = 0.00000001;

	if (link < epsilon)
			return -infinite;
	else
		return log(link);
}



bool
ValueNode_Logarithm::check_type(Type &type)
{
	return type==type_real;
}

LinkableValueNode::Vocab
ValueNode_Logarithm::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("Value node used to calculate the Neperian logarithm"))
	);

		ret.push_back(ParamDesc(ValueBase(),"epsilon")
		.set_local_name(_("Epsilon"))
		.set_description(_("Value used to compare 'Link' with zero "))
	);

		ret.push_back(ParamDesc(ValueBase(),"infinite")
		.set_local_name(_("Infinite"))
		.set_description(_("Returned value when result tends to infinite"))
	);

	return ret;
}

