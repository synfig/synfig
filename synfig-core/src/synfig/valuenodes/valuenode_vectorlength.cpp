/* === S Y N F I G ========================================================= */
/*!	\file valuenode_vectorlength.cpp
**	\brief Implementation of the "Vector Length" valuenode conversion.
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

#include "valuenode_vectorlength.h"
#include "valuenode_const.h"
#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_VectorLength::ValueNode_VectorLength(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_real)
		set_link("vector",ValueNode_Const::create(Vector(value.get(Real()), 0)));
	else
		throw Exception::BadType(value.get_type().description.local_name);
}

LinkableValueNode*
ValueNode_VectorLength::create_new()const
{
	return new ValueNode_VectorLength(get_type());
}

ValueNode_VectorLength*
ValueNode_VectorLength::create(const ValueBase &x)
{
	return new ValueNode_VectorLength(x);
}

ValueNode_VectorLength::~ValueNode_VectorLength()
{
	unlink_all();
}

ValueBase
ValueNode_VectorLength::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return (*vector_)(t).get(Vector()).mag();
}


bool
ValueNode_VectorLength::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(vector_, type_vector);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_VectorLength::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return vector_;
	return 0;
}

String
ValueNode_VectorLength::get_name()const
{
	return "vectorlength";
}

String
ValueNode_VectorLength::get_local_name()const
{
	return _("Vector Length");
}

bool
ValueNode_VectorLength::check_type(Type &type)
{
	return type==type_real;
}

LinkableValueNode::Vocab
ValueNode_VectorLength::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"vector")
		.set_local_name(_("Vector"))
		.set_description(_("The vector where the length is calculated from"))
	);

	return ret;
}
