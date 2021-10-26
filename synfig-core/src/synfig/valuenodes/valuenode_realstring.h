/* === S Y N F I G ========================================================= */
/*!	\file valuenode_realstring.h
**	\brief Header file for implementation of the "Real String" valuenode conversion.
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_REALSTRING_H
#define __SYNFIG_VALUENODE_REALSTRING_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_RealString : public LinkableValueNode
{
	ValueNode::RHandle real_;
	ValueNode::RHandle width_;
	ValueNode::RHandle precision_;
	ValueNode::RHandle zero_pad_;

	ValueNode_RealString(const ValueBase &value);

public:

	typedef etl::handle<ValueNode_RealString> Handle;
	typedef etl::handle<const ValueNode_RealString> ConstHandle;


	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_RealString();

	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

protected:
	LinkableValueNode* create_new()const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

public:
	using synfig::LinkableValueNode::get_link_vfunc;

	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_RealString* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_RealString

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
