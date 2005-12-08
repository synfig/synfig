/* === S Y N F I G ========================================================= */
/*!	\file valuenode_segcalctangent.cpp
**	\brief Template File
**
**	$Id: valuenode_segcalctangent.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "valuenode_segcalctangent.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "general.h"
#include "exception.h"
#include <ETL/hermite>
#include <ETL/calculus>
#include "segment.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_SegCalcTangent::ValueNode_SegCalcTangent(const ValueBase::Type &x):
	LinkableValueNode(x)
{
	if(x!=ValueBase::TYPE_VECTOR)
		throw Exception::BadType(ValueBase::type_name(x));
	
	segment_=ValueNode_Composite::create(ValueBase::TYPE_SEGMENT);
	amount_=ValueNode_Const::create(Real(0.5));
}

ValueNode_SegCalcTangent*
ValueNode_SegCalcTangent::create(const ValueBase &x)
{
	return new ValueNode_SegCalcTangent(x.get_type());
}

ValueNode_SegCalcTangent::~ValueNode_SegCalcTangent()
{
	unlink_all();
}

ValueBase
ValueNode_SegCalcTangent::operator()(Time t)const
{
	Segment segment((*segment_)(t).get(Segment()));

	etl::hermite<Vector> curve(segment.p1,segment.p2,segment.t1,segment.t2);
	etl::derivative< etl::hermite<Vector> > deriv(curve);
	
#ifdef ETL_FIXED_DERIVATIVE
	return deriv((*amount_)(t).get(Real()))*(0.5);
#else
	return deriv((*amount_)(t).get(Real()))*(-0.5);
#endif
	
}


String
ValueNode_SegCalcTangent::get_name()const
{
	return "segcalctangent";
}

String
ValueNode_SegCalcTangent::get_local_name()const
{
	return _("SegCalcTangent");
}
		
bool
ValueNode_SegCalcTangent::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_VECTOR;
}

bool
ValueNode_SegCalcTangent::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i==0 || i==1);
	if(i==0)
	{
		segment_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	if(i==1)
	{
		amount_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_SegCalcTangent::get_link_vfunc(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return segment_;
	if(i==1)
		return amount_;

	return 0;
}

int
ValueNode_SegCalcTangent::link_count()const
{
	return 2;
}

String
ValueNode_SegCalcTangent::link_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return "segment";
	if(i==1)
		return "amount";
	return String();
}

String
ValueNode_SegCalcTangent::link_local_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return _("Segment");
	if(i==1)
		return _("Amount");
	return String();
}

int
ValueNode_SegCalcTangent::get_link_index_from_name(const String &name)const
{
	if(name=="segment")
		return 0;
	if(name=="amount")
		return 1;
	
	throw Exception::BadLinkName(name);
}

LinkableValueNode*
ValueNode_SegCalcTangent::create_new()const
{
	return new ValueNode_SegCalcTangent(ValueBase::TYPE_VECTOR);
}
