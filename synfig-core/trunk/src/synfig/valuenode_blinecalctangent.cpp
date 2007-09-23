/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinecalctangent.cpp
**	\brief Template File
**
**	$Id$
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

#include "valuenode_blinecalctangent.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "general.h"
#include "exception.h"
#include <ETL/hermite>
#include <ETL/calculus>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineCalcTangent::ValueNode_BLineCalcTangent(const ValueBase::Type &x):
	LinkableValueNode(x)
{
	if(x!=ValueBase::TYPE_ANGLE && x!=ValueBase::TYPE_VECTOR)
		throw Exception::BadType(ValueBase::type_name(x));

	ValueNode_BLine* value_node(new ValueNode_BLine());
	set_link("bline",value_node);
	set_link("loop",ValueNode_Const::create(bool(false)));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
}

LinkableValueNode*
ValueNode_BLineCalcTangent::create_new()const
{
	return new ValueNode_BLineCalcTangent(get_type());
}

ValueNode_BLineCalcTangent*
ValueNode_BLineCalcTangent::create(const ValueBase &x)
{
	return new ValueNode_BLineCalcTangent(x.get_type());
}

ValueNode_BLineCalcTangent::~ValueNode_BLineCalcTangent()
{
	unlink_all();
}

ValueBase
ValueNode_BLineCalcTangent::operator()(Time t)const
{
	const std::vector<ValueBase> bline((*bline_)(t));
	handle<ValueNode_BLine> bline_value_node(bline_);
	const bool looped(bline_value_node->get_loop());
	int size = bline.size(), from_vertex;
	bool loop((*loop_)(t).get(bool()));
	Real amount((*amount_)(t).get(Real()));
	BLinePoint blinepoint0, blinepoint1;

	if (!looped) size--;
	if (size < 1)
		switch (get_type())
		{
			case ValueBase::TYPE_ANGLE:  return Angle();
			case ValueBase::TYPE_VECTOR: return Vector();
			default: assert(0); return ValueBase();
		}
	if (loop)
	{
		amount = amount - int(amount);
		if (amount < 0) amount++;
	}
	else
	{
		if (amount < 0) amount = 0;
		if (amount > 1) amount = 1;
	}

	vector<ValueBase>::const_iterator iter, next(bline.begin());

	iter = looped ? --bline.end() : next++;
	amount = amount * size;
	from_vertex = int(amount);
	if (from_vertex > size-1) from_vertex = size-1;
	blinepoint0 = from_vertex ? *(next+from_vertex-1) : *iter;
	blinepoint1 = *(next+from_vertex);

	etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							   blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
	etl::derivative< etl::hermite<Vector> > deriv(curve);

#ifdef ETL_FIXED_DERIVATIVE
	switch (get_type())
	{
		case ValueBase::TYPE_ANGLE:  return (deriv(amount-from_vertex)*(0.5)).angle();
		case ValueBase::TYPE_VECTOR: return deriv(amount-from_vertex)*(0.5);
		default: assert(0); return ValueBase();
	}
#else
	switch (get_type())
	{
		case ValueBase::TYPE_ANGLE:  return (deriv(amount-from_vertex)*(-0.5)).angle();
		case ValueBase::TYPE_VECTOR: return deriv(amount-from_vertex)*(-0.5);
		default: assert(0); return ValueBase();
	}
#endif
}

String
ValueNode_BLineCalcTangent::get_name()const
{
	return "blinecalctangent";
}

String
ValueNode_BLineCalcTangent::get_local_name()const
{
	return _("BLine Tangent");
}

bool
ValueNode_BLineCalcTangent::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:
			bline_=x;
			signal_child_changed()(i);signal_value_changed()();
			return true;
		case 1:
			loop_=x;
			signal_child_changed()(i);signal_value_changed()();
			return true;
		case 2:
			amount_=x;
			signal_child_changed()(i);signal_value_changed()();
			return true;
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BLineCalcTangent::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0: return bline_;
		case 1: return loop_;
		case 2: return amount_;
	}

	return 0;
}

int
ValueNode_BLineCalcTangent::link_count()const
{
	return 3;
}

String
ValueNode_BLineCalcTangent::link_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0: return "bline";
		case 1: return "loop";
		case 2: return "amount";
	}
	return String();
}

String
ValueNode_BLineCalcTangent::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0: return _("BLine");
		case 1: return _("Loop");
		case 2: return _("Amount");
	}
	return String();
}

int
ValueNode_BLineCalcTangent::get_link_index_from_name(const String &name)const
{
	if(name=="bline")  return 0;
	if(name=="loop")   return 1;
	if(name=="amount") return 2;
	throw Exception::BadLinkName(name);
}

bool
ValueNode_BLineCalcTangent::check_type(ValueBase::Type type)
{
	return (type==ValueBase::TYPE_ANGLE ||
			type==ValueBase::TYPE_VECTOR);
}
