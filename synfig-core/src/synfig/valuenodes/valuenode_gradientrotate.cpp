/* === S Y N F I G ========================================================= */
/*!	\file valuenode_gradientrotate.cpp
**	\brief Implementation of the "Gradient Rotate" valuenode conversion.
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include "valuenode_gradientrotate.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/gradient.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_GradientRotate, RELEASE_VERSION_0_61_06, "gradient_rotate", N_("Gradient Rotate"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_GradientRotate::ValueNode_GradientRotate(const Gradient& x):
	LinkableValueNode(synfig::type_gradient)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("gradient",ValueNode_Const::create(x));
	set_link("offset",ValueNode_Const::create(Real(0)));
}

LinkableValueNode*
ValueNode_GradientRotate::create_new()const
{
	return new ValueNode_GradientRotate(Gradient());
}

ValueNode_GradientRotate*
ValueNode_GradientRotate::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	Type &type(x.get_type());
	if(type!=type_gradient)
	{
		assert(0);
		throw std::runtime_error(String(_("Gradient Rotate"))+_(":Bad type ")+type.description.local_name);
	}

	ValueNode_GradientRotate* value_node=new ValueNode_GradientRotate(x.get(Gradient()));

	assert(value_node->get_type()==type);

	return value_node;
}

synfig::ValueNode_GradientRotate::~ValueNode_GradientRotate()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_GradientRotate::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Gradient gradient;
	gradient=(*ref_gradient)(t).get(gradient);
	Real offset((*ref_offset)(t).get(Real()));
	Gradient::iterator iter;
	for(iter=gradient.begin();iter!=gradient.end();++iter)
		iter->pos+=offset;

	return gradient;
}

bool
ValueNode_GradientRotate::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(ref_gradient, type_gradient);
	case 1: CHECK_TYPE_AND_SET_VALUE(ref_offset,   type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_GradientRotate::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:
			return ref_gradient;
		case 1:
			return ref_offset;
	}
	return 0;
}



bool
ValueNode_GradientRotate::check_type(Type &type)
{
	return type==type_gradient;
}

LinkableValueNode::Vocab
ValueNode_GradientRotate::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("The source gradient to rotate"))
	);

	ret.push_back(ParamDesc(ValueBase(),"offset")
		.set_local_name(_("Offset"))
		.set_description(_("The amount to offset the gradient"))
	);

	return ret;
}
