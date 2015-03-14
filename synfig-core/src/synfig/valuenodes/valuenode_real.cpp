/* === S Y N F I G ========================================================= */
/*!	\file ValueNode_Real.cpp
**	\brief Implementation of the "Real" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2013 Konstantin Dmitriev
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

#include "valuenode_real.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Real::ValueNode_Real(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Real::ValueNode_Real(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(x.get_type());
	if (type == type_angle)
		set_link("link", ValueNode_Const::create(Angle::deg(x.get(Angle())).get()));
	else
	if (type == type_bool)
		set_link("link", ValueNode_Const::create(float(x.get(bool()))));
	else
	if (type == type_integer)
		set_link("link", ValueNode_Const::create(float(x.get(int()))));
	else
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+x.get_type().description.local_name);
	}
}

ValueNode_Real*
ValueNode_Real::create(const ValueBase &x)
{
	return new ValueNode_Real(x);
}

LinkableValueNode*
ValueNode_Real::create_new()const
{
	return new ValueNode_Real(get_type());
}

ValueNode_Real::~ValueNode_Real()
{
	unlink_all();
}

bool
ValueNode_Real::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(real_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Real::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return real_;

	return 0;
}

ValueBase
ValueNode_Real::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	float real = (*real_)(t).get(float());

	Type &type(get_type());
	if (type == type_angle)
		return Angle::deg(real);
	if (type == type_bool)
		return bool(real);
	if (type == type_integer)
		return int(real);

	assert(0);
	throw runtime_error(get_local_name()+_(":Bad type ")+get_type().description.local_name);
}

synfig::ValueBase
synfig::ValueNode_Real::get_inverse(Time /*t*/, const synfig::Angle &target_value) const
{
	return (float)Angle::deg(target_value).get();
}



String
ValueNode_Real::get_name()const
{
	return "fromreal";
}

String
ValueNode_Real::get_local_name()const
{
	return _("Real");
}

bool
ValueNode_Real::check_type(Type &type __attribute__ ((unused)))
{
	return
		type==type_angle ||
		type==type_bool  ||
		type==type_integer;
}

LinkableValueNode::Vocab
ValueNode_Real::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The real value to be converted"))
	);

	return ret;
}
