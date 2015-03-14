/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinecalcvertex.cpp
**	\brief Implementation of the "BLine Vertex" valuenode conversion.
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

#include "valuenode_blinecalcvertex.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/general.h>
#include <synfig/exception.h>
#include <ETL/hermite>

#endif


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineCalcVertex::ValueNode_BLineCalcVertex(Type &x):
	LinkableValueNode(x)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if(x!=type_vector)
		throw Exception::BadType(x.description.local_name);

	ValueNode_BLine* value_node(new ValueNode_BLine());
	set_link("bline",value_node);
	set_link("loop",ValueNode_Const::create(bool(false)));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
	set_link("homogeneous", ValueNode_Const::create(bool(true)));
}

LinkableValueNode*
ValueNode_BLineCalcVertex::create_new()const
{
	return new ValueNode_BLineCalcVertex(type_vector);
}

ValueNode_BLineCalcVertex*
ValueNode_BLineCalcVertex::create(const ValueBase &x)
{
	return new ValueNode_BLineCalcVertex(x.get_type());
}

ValueNode_BLineCalcVertex::~ValueNode_BLineCalcVertex()
{
	unlink_all();
}

ValueBase
ValueNode_BLineCalcVertex::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const std::vector<ValueBase> bline((*bline_)(t).get_list());
	handle<ValueNode_BLine> bline_value_node(bline_);
	const bool looped(bline_value_node->get_loop());
	int size = bline.size(), from_vertex;
	bool loop((*loop_)(t).get(bool()));
	bool homogeneous((*homogeneous_)(t).get(bool()));
	Real amount((*amount_)(t).get(Real()));
	if(homogeneous)
	{
		amount=hom_to_std(bline, amount, loop, looped);
	}
	BLinePoint blinepoint0, blinepoint1;

	if (!looped) size--;
	if (size < 1) return Vector();
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
	blinepoint0 = from_vertex ? (next+from_vertex-1)->get(BLinePoint()) : iter->get(BLinePoint());
	blinepoint1 = (next+from_vertex)->get(BLinePoint());

	etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							   blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
	return curve(amount-from_vertex);
}







String
ValueNode_BLineCalcVertex::get_name()const
{
	return "blinecalcvertex";
}

String
ValueNode_BLineCalcVertex::get_local_name()const
{
	return _("Spline Vertex");
}

bool
ValueNode_BLineCalcVertex::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bline_,  type_list);
	case 1: CHECK_TYPE_AND_SET_VALUE(loop_,   type_bool);
	case 2: CHECK_TYPE_AND_SET_VALUE(amount_, type_real);
	case 3: CHECK_TYPE_AND_SET_VALUE(homogeneous_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BLineCalcVertex::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return bline_;
		case 1: return loop_;
		case 2: return amount_;
		case 3: return homogeneous_;
	}

	return 0;
}

bool
ValueNode_BLineCalcVertex::check_type(Type &type)
{
	return type==type_vector;
}

LinkableValueNode::Vocab
ValueNode_BLineCalcVertex::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bline")
		.set_local_name(_("Spline"))
		.set_description(_("The spline where the vertex is linked to"))
	);

	ret.push_back(ParamDesc(ValueBase(),"loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked, the amount would loop"))
	);

	ret.push_back(ParamDesc(ValueBase(),"amount")
		.set_local_name(_("Amount"))
		.set_description(_("The position of the linked vertex on the Spline (0,1]"))
	);

	ret.push_back(ParamDesc(ValueBase(),"homogeneous")
		.set_local_name(_("Homogeneous"))
		.set_description(_("When checked, the position is Spline length based"))
	);
	return ret;
}
