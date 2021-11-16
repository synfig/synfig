/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinecalcwidth.cpp
**	\brief Implementation of the "BLine Width" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "valuenode_blinecalcwidth.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <ETL/hermite>

#endif


/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BLineCalcWidth, RELEASE_VERSION_0_61_08, "blinecalcwidth", "Spline Width")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineCalcWidth::ValueNode_BLineCalcWidth(Type &x):
	LinkableValueNode(x)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if(x!=type_real)
		throw Exception::BadType(x.description.local_name);

	ValueNode_BLine* value_node(ValueNode_BLine::create());
	set_link("bline",value_node);
	set_link("loop",ValueNode_Const::create(bool(false)));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
	set_link("scale",ValueNode_Const::create(Real(1.0)));
	set_link("homogeneous",ValueNode_Const::create(bool(true)));
}

LinkableValueNode*
ValueNode_BLineCalcWidth::create_new()const
{
	return new ValueNode_BLineCalcWidth(type_real);
}

ValueNode_BLineCalcWidth*
ValueNode_BLineCalcWidth::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_BLineCalcWidth(x.get_type());
}

ValueNode_BLineCalcWidth::~ValueNode_BLineCalcWidth()
{
	unlink_all();
}

ValueBase
ValueNode_BLineCalcWidth::operator()(Time t, Real amount)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const ValueBase::List bline = (*bline_)(t).get_list();
	const ValueBase bline_value_node = (*bline_)(t);

	const bool looped = bline_value_node.get_loop();
	int size = (int)bline.size();
	int count = looped ? size : size - 1;
	if (count < 1) return Real();

	bool loop         = (*loop_)(t).get(bool());
	bool homogeneous  = (*homogeneous_)(t).get(bool());
	Real scale        = (*scale_)(t).get(Real());

	if (loop) amount -= floor(amount);
	if (homogeneous) amount = hom_to_std(bline, amount, loop, looped);
	if (amount < 0) amount = 0;
	if (amount > 1) amount = 1;
	amount *= count;

	int i0 = std::max(0, std::min(size-1, (int)floor(amount)));
	int i1 = (i0 + 1) % size;
	Real part = amount - i0;

	Real width0 = bline[i0].get(BLinePoint()).get_width();
	Real width1 = bline[i1].get(BLinePoint()).get_width();
	return (width0 + part*(width1 - width0))*scale;
}

ValueBase
ValueNode_BLineCalcWidth::operator()(Time t)const
{
	Real amount((*amount_)(t).get(Real()));
	return (*this)(t, amount);
}



bool
ValueNode_BLineCalcWidth::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bline_,  type_list);
	case 1: CHECK_TYPE_AND_SET_VALUE(loop_,   type_bool);
	case 2: CHECK_TYPE_AND_SET_VALUE(amount_, type_real);
	case 3: CHECK_TYPE_AND_SET_VALUE(scale_,  type_real);
	case 4: CHECK_TYPE_AND_SET_VALUE(homogeneous_,  type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BLineCalcWidth::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return bline_;
		case 1: return loop_;
		case 2: return amount_;
		case 3: return scale_;
		case 4: return homogeneous_;
	}

	return 0;
}

bool
ValueNode_BLineCalcWidth::check_type(Type &type)
{
	return type==type_real;
}

LinkableValueNode::Vocab
ValueNode_BLineCalcWidth::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bline")
		.set_local_name(_("Spline"))
		.set_description(_("The spline where the width is linked to"))
	);

	ret.push_back(ParamDesc(ValueBase(),"loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked, the amount would loop"))
	);

	ret.push_back(ParamDesc(ValueBase(),"amount")
		.set_local_name(_("Amount"))
		.set_description(_("The position of the linked width on the spline (0,1]"))
	);

	ret.push_back(ParamDesc(ValueBase(),"scale")
		.set_local_name(_("Scale"))
		.set_description(_("Scale of the width"))
	);

	ret.push_back(ParamDesc(ValueBase(),"homogeneous")
		.set_local_name(_("Homogeneous"))
		.set_description(_("When checked, the width is spline length based"))
	);
	return ret;
}

