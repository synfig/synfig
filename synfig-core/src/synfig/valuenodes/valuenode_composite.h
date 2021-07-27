/* === S Y N F I G ========================================================= */
/*!	\file valuenode_composite.h
**	\brief Header file for implementation of the "Composite" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_COMPOSITE_H
#define __SYNFIG_VALUENODE_COMPOSITE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

#define MAX_LINKS 8

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Composite : public LinkableValueNode
{
	ValueNode::RHandle components[MAX_LINKS];
	ValueNode_Composite(const ValueBase &value, std::shared_ptr<Canvas> canvas = 0);

public:
	typedef etl::handle<ValueNode_Composite> Handle;
	typedef etl::handle<const ValueNode_Composite> ConstHandle;


	~ValueNode_Composite();

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;
	virtual String link_name(int i)const;
	virtual ValueBase operator()(Time t)const;
	virtual String get_name()const;
	virtual String get_local_name()const;
	virtual int get_link_index_from_name(const String &name)const;

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::set_link_vfunc;
	using synfig::LinkableValueNode::get_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_Composite* create(const ValueBase &x, std::shared_ptr<Canvas> canvas = 0);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Composite

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
