/* === S Y N F I G ========================================================= */
/*!	\file valuenode_random.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_VALUENODE_INTERPOLATION_RANDOM_H
#define __SYNFIG_VALUENODE_INTERPOLATION_RANDOM_H

/* === H E A D E R S ======================================================= */

#include "synfig/valuenode.h"
#include "random_noise.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Random : public LinkableValueNode
{
	ValueNode::RHandle link_;
	ValueNode::RHandle radius_;
	ValueNode::RHandle seed_;
	ValueNode::RHandle speed_;
	ValueNode::RHandle smooth_;
	ValueNode::RHandle loop_;

	mutable RandomNoise random;
	RandomNoise::SmoothType smooth;
	ValueNode_Random(const ValueBase &value);

public:

	typedef etl::handle<ValueNode_Random> Handle;
	typedef etl::handle<const ValueNode_Random> ConstHandle;

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_Random();

	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	void randomize_seed();

protected:
	LinkableValueNode* create_new()const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

public:
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_Random* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Random

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
