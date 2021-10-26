/* === S Y N F I G ========================================================= */
/*!	\file valuenode_segcalctangent.cpp
**	\brief Implementation of the "Segment Tangent" valuenode conversion.
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

#include "valuenode_segcalctangent.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <ETL/hermite>
#include <ETL/calculus>
#include <synfig/segment.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_SegCalcTangent, RELEASE_VERSION_0_61_06, "segcalctangent", "Segment Tangent")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_SegCalcTangent::ValueNode_SegCalcTangent(Type &x):
	LinkableValueNode(x)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if(x!=type_vector)
		throw Exception::BadType(x.description.local_name);

	set_link("segment",ValueNode_Const::create(type_segment));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
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
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Segment segment((*segment_)(t).get(Segment()));

	etl::hermite<Vector> curve(segment.p1,segment.p2,segment.t1,segment.t2);
	etl::derivative< etl::hermite<Vector> > deriv(curve);

	return deriv((*amount_)(t).get(Real()));
}




bool
ValueNode_SegCalcTangent::check_type(Type &type)
{
	return type==type_vector;
}

bool
ValueNode_SegCalcTangent::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(segment_, type_segment);
	case 1: CHECK_TYPE_AND_SET_VALUE(amount_,  type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_SegCalcTangent::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return segment_;
	if(i==1)
		return amount_;

	return 0;
}

LinkableValueNode*
ValueNode_SegCalcTangent::create_new()const
{
	return new ValueNode_SegCalcTangent(type_vector);
}

LinkableValueNode::Vocab
ValueNode_SegCalcTangent::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"segment")
		.set_local_name(_("Segment"))
		.set_description(_("The Segment where the tangent is linked to"))
	);

	ret.push_back(ParamDesc(ValueBase(),"amount")
		.set_local_name(_("Amount"))
		.set_description(_("The position of the linked tangent on the Segment (0,1]"))
	);
	return ret;
}
