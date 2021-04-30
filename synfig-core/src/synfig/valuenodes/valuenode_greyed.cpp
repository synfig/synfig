/* === S Y N F I G ========================================================= */
/*!	\file valuenode_greyed.cpp
**	\brief Implementation of the "Greyed" valuenode conversion.
**
**	$Id$
**
**	\legal
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

#include "valuenode_greyed.h"

#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#include "valuenode_const.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Greyed, RELEASE_VERSION_0_62_00, "greyed", "Greyed")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Greyed::ValueNode_Greyed(Type &x):
	ValueNode_Reference(x)
{
}

ValueNode_Greyed::ValueNode_Greyed(const ValueNode::Handle &x):
	ValueNode_Reference(x->get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("link",x);
}

ValueNode_Greyed*
ValueNode_Greyed::create(const ValueBase &x)
{
	return new ValueNode_Greyed(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_Greyed::create_new()const
{
	return new ValueNode_Greyed(get_type());
}



LinkableValueNode::Vocab
ValueNode_Greyed::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The greyed value"))
	);

	return ret;
}
