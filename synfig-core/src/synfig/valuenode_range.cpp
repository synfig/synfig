/* === S Y N F I G ========================================================= */
/*!	\file valuenode_range.cpp
**	\brief Implementation of the "Range" valuenode conversion.
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

#include "general.h"
#include "valuenode_range.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "vector.h"
#include "angle.h"
#include "real.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Range::ValueNode_Range(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	ValueBase::Type id(value.get_type());

	switch(id)
	{
	case ValueBase::TYPE_ANGLE:
		set_link("min",ValueNode_Const::create(value.get(Angle())));
		set_link("max",ValueNode_Const::create(value.get(Angle())));
		set_link("link",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("min",ValueNode_Const::create(value.get(int())));
		set_link("max",ValueNode_Const::create(value.get(int())));
		set_link("link",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("min",ValueNode_Const::create(value.get(Real())));
		set_link("max",ValueNode_Const::create(value.get(Real())));
		set_link("link",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("min",ValueNode_Const::create(value.get(Time())));
		set_link("max",ValueNode_Const::create(value.get(Time())));
		set_link("link",ValueNode_Const::create(value.get(Time())));
		break;
	default:
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+ValueBase::type_local_name(id));
	}

	assert(min_->get_type()==id);
	assert(max_->get_type()==id);
	assert(link_->get_type()==id);
	assert(get_type()==id);
}

LinkableValueNode*
ValueNode_Range::create_new()const
{
	return new ValueNode_Range(get_type());
}

ValueNode_Range*
ValueNode_Range::create(const ValueBase& value)
{
	return new ValueNode_Range(value);
}

synfig::ValueNode_Range::~ValueNode_Range()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Range::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!min_ || !max_ || !link_)
		throw runtime_error(strprintf("ValueNode_Range: %s",_("Some of my parameters aren't set!")));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
	{
		Angle minimum = (* min_)(t).get(Angle());
		Angle maximum = (* max_)(t).get(Angle());
		Angle link    = (*link_)(t).get(Angle());
// This code was removed because it didn't work with link < minimum
// It is sane to completely delete it if the replacement code is fine.
/* ***********************************************
		// if link is between min and max, use it
		if (Angle::deg((link-minimum).mod()).get() < Angle::deg((maximum-minimum).mod()).get())
			return link;
		// otherwise use whichever of min and max is closest to link
		else if (link.dist(minimum).abs() < link.dist(maximum).abs())
			return minimum;
		else
			return maximum;
*********************************************** */
		if(Angle::rad(maximum).get()>=Angle::rad(link).get() && Angle::rad(link).get()>=Angle::rad(minimum).get())
			return link;
		else if (Angle::rad(minimum).get()>Angle::rad(link).get())
			return minimum;
		else
			return maximum;
	}
	case ValueBase::TYPE_INTEGER:
		return std::max((*min_)(t).get(int()),  std::min((*max_)(t).get(int()),  (*link_)(t).get(int())));
	case ValueBase::TYPE_REAL:
		return std::max((*min_)(t).get(Real()), std::min((*max_)(t).get(Real()), (*link_)(t).get(Real())));
	case ValueBase::TYPE_TIME:
		return std::max((*min_)(t).get(Time()), std::min((*max_)(t).get(Time()), (*link_)(t).get(Time())));
	default:
		assert(0);
		break;
	}
	return ValueBase();
}

synfig::ValueBase
synfig::ValueNode_Range::get_inverse(Time t, const synfig::Vector &target_value) const
{
	switch (get_type())
	{
		case ValueBase::TYPE_INTEGER:
			{
			int max_value((*max_)(t).get(int()));
			int min_value((*min_)(t).get(int()));
			return std::max(min_value, std::min(max_value, int(target_value.mag())));
			}
		case ValueBase::TYPE_REAL:
			{
			Real max_value((*max_)(t).get(Real()));
			Real min_value((*min_)(t).get(Real()));
			return std::max(min_value, std::min(max_value, target_value.mag()));
			}
		case ValueBase::TYPE_ANGLE:
			{
			Angle max_value((*max_)(t).get(Angle()));
			Angle min_value((*min_)(t).get(Angle()));
			Angle target_angle(Angle::tan(target_value[1],target_value[0]));
			return target_angle>max_value?max_value:target_angle<min_value?min_value:target_angle;
			}
		case ValueBase::TYPE_TIME:
			{
			Real max_value((*max_)(t).get(Time()));
			Real min_value((*min_)(t).get(Time()));
			return std::max(min_value, std::min(max_value, target_value.mag()));
			}
		default:
			return target_value;
	}
	return ValueBase();
}

synfig::ValueBase
synfig::ValueNode_Range::get_inverse(Time t, const synfig::Angle &target_value) const
{
	Angle minimum = (* min_)(t).get(Angle());
	Angle maximum = (* max_)(t).get(Angle());
	// Angle link = (*link_)(t).get(Angle());
		switch (get_type())
		{
			default:

		if(Angle::rad(maximum).get()>=Angle::rad(target_value).get() && Angle::rad(target_value).get()>=Angle::rad(minimum).get())
			return target_value;
		else if (Angle::rad(minimum).get()>Angle::rad(target_value).get())
			return minimum;
		else
			return maximum;
		}
	return ValueBase();
}


bool
ValueNode_Range::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(min_,  get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(max_,  get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(link_, get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Range::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return min_;
		case 1: return max_;
		case 2: return link_;
		default: return 0;
	}
}

String
ValueNode_Range::get_name()const
{
	return "range";
}

String
ValueNode_Range::get_local_name()const
{
	return _("Range");
}

bool
ValueNode_Range::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_ANGLE
		|| type==ValueBase::TYPE_INTEGER
		|| type==ValueBase::TYPE_REAL
		|| type==ValueBase::TYPE_TIME;
}

LinkableValueNode::Vocab
ValueNode_Range::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"min")
		.set_local_name(_("Min"))
		.set_description(_("Returned value when 'Link' is smaller"))
	);

	ret.push_back(ParamDesc(ValueBase(),"max")
		.set_local_name(_("Max"))
		.set_description(_("Returned value when 'Link' is greater"))
	);

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node to limit its range"))
	);

	return ret;
}
