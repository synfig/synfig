/* === S Y N F I G ========================================================= */
/*!	\file valuenode_exp.cpp
**	\brief Implementation of the "Exponential" valuenode conversion.
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

#include "valuenode_exp.h"
#include "valuenode_const.h"
#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Exp::ValueNode_Exp(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_real)
	{
		set_link("exp",ValueNode_Const::create(Real(0)));
		set_link("scale",ValueNode_Const::create(value.get(Real())));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_Exp::create_new()const
{
	return new ValueNode_Exp(get_type());
}

ValueNode_Exp*
ValueNode_Exp::create(const ValueBase &x)
{
	return new ValueNode_Exp(x);
}

ValueNode_Exp::~ValueNode_Exp()
{
	unlink_all();
}

ValueBase
ValueNode_Exp::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return (exp((*exp_)(t).get(Real())) *
			(*scale_)(t).get(Real()));
}

String
ValueNode_Exp::get_name()const
{
	return "exp";
}

String
ValueNode_Exp::get_local_name()const
{
	return _("Exponential");
}

bool
ValueNode_Exp::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(exp_,   type_real);
	case 1: CHECK_TYPE_AND_SET_VALUE(scale_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Exp::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return exp_;
	if(i==1)
		return scale_;

	return 0;
}

bool
ValueNode_Exp::check_type(Type &type)
{
	return type==type_real;
}

LinkableValueNode::Vocab
ValueNode_Exp::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"exp")
		.set_local_name(_("Exponent"))
		.set_description(_("The value to raise the constant 'e'"))
	);

	ret.push_back(ParamDesc(ValueBase(),"scale")
		.set_local_name(_("Scale"))
		.set_description(_("Multiplier of the resulting exponent"))
	);

	return ret;
}
