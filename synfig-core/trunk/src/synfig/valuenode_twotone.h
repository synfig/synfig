/* === S I N F G =========================================================== */
/*!	\file valuenode_twotone.h
**	\brief Template Header
**
**	$Id: valuenode_twotone.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_VALUENODE_TWOTONE_H
#define __SINFG_VALUENODE_TWOTONE_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

struct ValueNode_TwoTone : public LinkableValueNode
{
	typedef etl::handle<ValueNode_TwoTone> Handle;
	typedef etl::handle<const ValueNode_TwoTone> ConstHandle;
	
protected:

	ValueNode_TwoTone();

private:

	ValueNode::RHandle ref_a;
	ValueNode::RHandle ref_b;

public:

	virtual ~ValueNode_TwoTone();

//	static Handle create(ValueBase::Type id=ValueBase::TYPE_GRADIENT);

	//! Sets the left-hand-side value_node
	bool set_lhs(ValueNode::Handle a);

	//! Gets the left-hand-side value_node
	ValueNode::Handle get_lhs()const { return ref_a; }

	//! Sets the right-hand-side value_node
	bool set_rhs(ValueNode::Handle b);

	//! Gets the right-hand-side value_node
	ValueNode::Handle get_rhs()const { return ref_b; }


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
	using sinfg::LinkableValueNode::get_link_vfunc;

	using sinfg::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_TwoTone* create(const ValueBase &x=ValueBase::TYPE_GRADIENT);
}; // END of class ValueNode_TwoTone

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
