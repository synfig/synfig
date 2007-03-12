/* === S Y N F I G ========================================================= */
/*!	\file valuenode_stripes.h
**	\brief Template Header
**
**	$Id: valuenode_stripes.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_STRIPES_H
#define __SYNFIG_VALUENODE_STRIPES_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct ValueNode_Stripes : public LinkableValueNode
{
	typedef etl::handle<ValueNode_Stripes> Handle;
	typedef etl::handle<const ValueNode_Stripes> ConstHandle;

protected:

	ValueNode_Stripes();

private:

	ValueNode::RHandle color1_;
	ValueNode::RHandle color2_;
	ValueNode::RHandle stripes_;
	ValueNode::RHandle width_;

public:

	virtual ~ValueNode_Stripes();

//	static Handle create(ValueBase::Type id=ValueBase::TYPE_GRADIENT);

	bool set_color1(ValueNode::Handle a);
	ValueNode::Handle get_color1()const { return color1_; }

	bool set_color2(ValueNode::Handle a);
	ValueNode::Handle get_color2()const { return color2_; }

	bool set_stripes(ValueNode::Handle b);
	ValueNode::Handle get_stripes()const { return stripes_; }

	bool set_width(ValueNode::Handle x);


	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_local_name(int i)const;
	virtual String link_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

//	static bool check_type(const ValueBase::Type &type);

	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_Stripes* create(const ValueBase &x=ValueBase::TYPE_GRADIENT);
}; // END of class ValueNode_Stripes

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
