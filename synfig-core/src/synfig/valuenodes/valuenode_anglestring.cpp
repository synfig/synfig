/* === S Y N F I G ========================================================= */
/*!	\file valuenode_anglestring.cpp
**	\brief Implementation of the "AngleString" valuenode conversion.
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

#include "valuenode_anglestring.h"
#include "valuenode_const.h"
#include <synfig/canvas.h>
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

REGISTER_VALUENODE(ValueNode_AngleString, RELEASE_VERSION_0_61_09, "anglestring", "Angle String")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_AngleString::ValueNode_AngleString(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_string)
	{
		set_link("angle",ValueNode_Const::create(Angle::deg(0)));
		set_link("width",ValueNode_Const::create(int(0)));
		set_link("precision",ValueNode_Const::create(int(3)));
		set_link("zero_pad",ValueNode_Const::create(bool(false)));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_AngleString::create_new()const
{
	return new ValueNode_AngleString(get_type());
}

ValueNode_AngleString*
ValueNode_AngleString::create(const ValueBase &x)
{
	return new ValueNode_AngleString(x);
}

ValueNode_AngleString::~ValueNode_AngleString()
{
	unlink_all();
}

ValueBase
ValueNode_AngleString::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Real angle(Angle::deg((*angle_)(t).get(Angle())).get());
	int width((*width_)(t).get(int()));
	int precision((*precision_)(t).get(int()));
	int zero_pad((*zero_pad_)(t).get(bool()));

	if(precision<0) precision=0;

	if (get_type() == type_string)
		return strprintf(strprintf("%%%s%d.%df",
								   zero_pad ? "0" : "",
								   width,
								   precision).c_str(), angle)+"°";

	assert(0);
	return ValueBase();
}



bool
ValueNode_AngleString::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(angle_, type_angle);
	case 1: CHECK_TYPE_AND_SET_VALUE(width_, type_integer);
	case 2: CHECK_TYPE_AND_SET_VALUE(precision_, type_integer);
	case 3: CHECK_TYPE_AND_SET_VALUE(zero_pad_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_AngleString::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return angle_;
	case 1: return width_;
	case 2: return precision_;
	case 3: return zero_pad_;
	}

	return 0;
}

bool
ValueNode_AngleString::check_type(Type &type)
{
	return
		type==type_string;
}

LinkableValueNode::Vocab
ValueNode_AngleString::get_children_vocab_vfunc()const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"angle")
		.set_local_name(_("Angle"))
		.set_description(_("Value to convert to string"))
	);

	ret.push_back(ParamDesc(ValueBase(),"width")
		.set_local_name(_("Width"))
		.set_description(_("Width of the string"))
	);

	ret.push_back(ParamDesc(ValueBase(),"precision")
		.set_local_name(_("Precision"))
		.set_description(_("Number of decimal places"))
	);

	ret.push_back(ParamDesc(ValueBase(),"zero_pad")
		.set_local_name(_("Zero Padded"))
		.set_description(_("When checked, the string is left filled with zeros to match the width"))
	);

	return ret;
}
