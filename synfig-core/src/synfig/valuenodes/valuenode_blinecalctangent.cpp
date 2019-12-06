/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinecalctangent.cpp
**	\brief Implementation of the "BLine Tangent" valuenode conversion.
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

#include "valuenode_blinecalctangent.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <ETL/hermite>
#include <ETL/calculus>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BLineCalcTangent, RELEASE_VERSION_0_61_07, "blinecalctangent", "Spline Tangent")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineCalcTangent::ValueNode_BLineCalcTangent(Type &x):
	LinkableValueNode(x)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if(x!=type_angle && x!=type_real && x!=type_vector)
		throw Exception::BadType(x.description.local_name);

	ValueNode_BLine* value_node(new ValueNode_BLine());
	set_link("bline",value_node);
	set_link("loop",ValueNode_Const::create(bool(false)));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
	set_link("offset",ValueNode_Const::create(Angle::deg(0)));
	set_link("scale",ValueNode_Const::create(Real(1.0)));
	set_link("fixed_length",ValueNode_Const::create(bool(false)));
	set_link("homogeneous",ValueNode_Const::create(bool(true)));
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
ValueNode_BLineCalcTangent::operator()(Time t, Real amount)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const ValueBase::List bline = (*bline_)(t).get_list();
	handle<ValueNode_BLine> bline_value_node( handle<ValueNode_BLine>::cast_dynamic(bline_) );
	assert(bline_value_node);

	const bool looped = bline_value_node->get_loop();
	int size = (int)bline.size();
	int count = looped ? size : size - 1;
	if (count < 1) return Vector();

	bool loop         = (*loop_)(t).get(bool());
	bool homogeneous  = (*homogeneous_)(t).get(bool());
	Angle offset      = (*offset_)(t).get(Angle());
	Real scale        = (*scale_)(t).get(Real());
	bool fixed_length = (*fixed_length_)(t).get(bool());

	if (loop) amount -= floor(amount);
	if (homogeneous) amount = hom_to_std(bline, amount, loop, looped);
	if (amount < 0) amount = 0;
	if (amount > 1) amount = 1;
	amount *= count;

	int i0 = std::max(0, std::min(size-1, (int)floor(amount)));
	int i1 = (i0 + 1) % size;
	Real part = amount - i0;

	const BLinePoint &blinepoint0 = bline[i0].get(BLinePoint());
	const BLinePoint &blinepoint1 = bline[i1].get(BLinePoint());

	etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							   blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
	etl::derivative< etl::hermite<Vector> > deriv(curve);

	Vector tangent = deriv(part);

	Type &type(get_type());
	if (type == type_angle)
		return tangent.angle() + offset;
	if (type == type_real) {
		if (fixed_length) return scale;
		return tangent.mag() * scale;
	}
	if (type == type_vector) {
		Angle angle(tangent.angle() + offset);
		Real mag = fixed_length ? scale : (tangent.mag() * scale);
		return Vector( Angle::cos(angle).get()*mag,
					   Angle::sin(angle).get()*mag );
	}

	assert(0);
	return ValueBase();
}

ValueBase
ValueNode_BLineCalcTangent::operator()(Time t)const
{
	Real amount((*amount_)(t).get(Real()));
	return (*this)(t, amount);
}



bool
ValueNode_BLineCalcTangent::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bline_,		type_list);
	case 1: CHECK_TYPE_AND_SET_VALUE(loop_,			type_bool);
	case 2: CHECK_TYPE_AND_SET_VALUE(amount_,		type_real);
	case 3: CHECK_TYPE_AND_SET_VALUE(offset_,		type_angle);
	case 4: CHECK_TYPE_AND_SET_VALUE(scale_,		type_real);
	case 5: CHECK_TYPE_AND_SET_VALUE(fixed_length_,	type_bool);
	case 6: CHECK_TYPE_AND_SET_VALUE(homogeneous_,	type_bool);
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
		case 3: return offset_;
		case 4: return scale_;
		case 5: return fixed_length_;
		case 6: return homogeneous_;
	}

	return 0;
}

bool
ValueNode_BLineCalcTangent::check_type(Type &type)
{
	return (type==type_angle ||
			type==type_real  ||
			type==type_vector);
}

LinkableValueNode::Vocab
ValueNode_BLineCalcTangent::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bline")
		.set_local_name(_("Spline"))
		.set_description(_("The Spline where the tangent is linked to"))
	);

	ret.push_back(ParamDesc(ValueBase(),"loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked, the amount would loop"))
	);

	ret.push_back(ParamDesc(ValueBase(),"amount")
		.set_local_name(_("Amount"))
		.set_description(_("The position of the linked tangent on the Spline (0,1]"))
	);

	ret.push_back(ParamDesc(ValueBase(),"offset")
		.set_local_name(_("Offset"))
		.set_description(_("Angle offset of the tangent"))
	);

	ret.push_back(ParamDesc(ValueBase(),"scale")
		.set_local_name(_("Scale"))
		.set_description(_("Scale of the tangent"))
	);

	ret.push_back(ParamDesc(ValueBase(),"fixed_length")
		.set_local_name(_("Fixed Length"))
		.set_description(_("When checked, the tangent's length is fixed"))
	);

	ret.push_back(ParamDesc(ValueBase(),"homogeneous")
		.set_local_name(_("Homogeneous"))
		.set_description(_("When checked, the tangent is Spline length based"))
	);
	return ret;
}
