/* === S Y N F I G ========================================================= */
/*!	\file valuenode_reference.cpp
**	\brief Template File
**
**	$Id: valuenode_reference.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#include "valuenode_reference.h"
#include "valuenode_const.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Reference::ValueNode_Reference(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Reference::ValueNode_Reference(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	set_link("link",x);
}

ValueNode_Reference*
ValueNode_Reference::create(const ValueBase &x)
{
	return new ValueNode_Reference(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_Reference::create_new()const
{
	return new ValueNode_Reference(get_type());
}

ValueNode_Reference::~ValueNode_Reference()
{
	unlink_all();
}

bool
ValueNode_Reference::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i==0);
	if(x->get_type()!=get_type() && !PlaceholderValueNode::Handle::cast_dynamic(x))
		return false;
	link_=x;
	signal_child_changed()(i);signal_value_changed()();
	return true;
}
	
ValueNode::LooseHandle
ValueNode_Reference::get_link_vfunc(int i)const
{
	assert(i==0);
	return link_;
}

int
ValueNode_Reference::link_count()const
{
	return 1;
}

String
ValueNode_Reference::link_local_name(int i)const
{
	assert(i==0);
	return _("Link");
}	

String
ValueNode_Reference::link_name(int i)const
{
	return "link";
}

int
ValueNode_Reference::get_link_index_from_name(const String &name)const
{
	if(name=="link")
		return 0;

	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_Reference::operator()(Time t)const
{
	return (*link_)(t);
}


String
ValueNode_Reference::get_name()const
{
	return "reference";
}

String
ValueNode_Reference::get_local_name()const
{
	return _("Reference");
}

bool
ValueNode_Reference::check_type(ValueBase::Type type)
{
	if(type)
		return true;
	return false;
}
