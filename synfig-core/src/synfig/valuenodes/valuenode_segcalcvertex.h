/* === S Y N F I G ========================================================= */
/*!	\file valuenode_segcalcvertex.h
**	\brief Header file for implementation of the "Segment Vertex" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_SEGCALCVERTEX_H
#define __SYNFIG_VALUENODE_SEGCALCVERTEX_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_SegCalcVertex : public LinkableValueNode
{
	ValueNode::RHandle segment_;
	ValueNode::RHandle amount_;

	ValueNode_SegCalcVertex(Type &x=type_vector);

public:

	typedef std::shared_ptr<ValueNode_SegCalcVertex> Handle;
	typedef std::shared_ptr<const ValueNode_SegCalcVertex> ConstHandle;

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_SegCalcVertex();

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
	static ValueNode_SegCalcVertex* create(const ValueBase &x=type_vector);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_SegCalcVertex

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
