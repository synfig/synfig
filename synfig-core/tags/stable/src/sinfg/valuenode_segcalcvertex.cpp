/* === S I N F G =========================================================== */
/*!	\file valuenode_segcalcvertex.cpp
**	\brief Template File
**
**	$Id: valuenode_segcalcvertex.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "valuenode_segcalcvertex.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "general.h"
#include "exception.h"
#include <ETL/hermite>
#include "segment.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_SegCalcVertex::ValueNode_SegCalcVertex(const ValueBase::Type &x):
	LinkableValueNode(x)
{
	if(x!=ValueBase::TYPE_VECTOR)
		throw Exception::BadType(ValueBase::type_name(x));
	
	segment_=ValueNode_Composite::create(ValueBase::TYPE_SEGMENT);
	amount_=ValueNode_Const::create(Real(0.5));
	
}

ValueNode_SegCalcVertex*
ValueNode_SegCalcVertex::create(const ValueBase &x)
{
	return new ValueNode_SegCalcVertex(x.get_type());
}

ValueNode_SegCalcVertex::~ValueNode_SegCalcVertex()
{
	unlink_all();
}

ValueBase
ValueNode_SegCalcVertex::operator()(Time t)const
{
	Segment segment((*segment_)(t).get(Segment()));

	etl::hermite<Vector> curve(segment.p1,segment.p2,segment.t1,segment.t2);
	
	return curve((*amount_)(t).get(Real()));
}


String
ValueNode_SegCalcVertex::get_name()const
{
	return "segcalcvertex";
}

String
ValueNode_SegCalcVertex::get_local_name()const
{
	return _("SegCalcVertex");
}
		
bool
ValueNode_SegCalcVertex::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_VECTOR;
}

bool
ValueNode_SegCalcVertex::set_link_vfunc(int i,ValueNode::Handle x)
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
ValueNode_SegCalcVertex::get_link_vfunc(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return segment_;
	if(i==1)
		return amount_;

	return 0;
}

int
ValueNode_SegCalcVertex::link_count()const
{
	return 2;
}

String
ValueNode_SegCalcVertex::link_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return "segment";
	if(i==1)
		return "amount";
	return String();
}

String
ValueNode_SegCalcVertex::link_local_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return _("Segment");
	if(i==1)
		return _("Amount");
	return String();
}

int
ValueNode_SegCalcVertex::get_link_index_from_name(const String &name)const
{
	if(name=="segment")
		return 0;
	if(name=="amount")
		return 1;
	
	throw Exception::BadLinkName(name);
}

LinkableValueNode*
ValueNode_SegCalcVertex::create_new()const
{
	return new ValueNode_SegCalcVertex(ValueBase::TYPE_VECTOR);
}
