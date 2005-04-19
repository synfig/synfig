/* === S I N F G =========================================================== */
/*!	\file valuenode_scale.h
**	\brief Template Header
**
**	$Id: valuenode_scale.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SINFG_VALUENODE_SCALE_H
#define __SINFG_VALUENODE_SCALE_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

struct ValueNode_Scale : public LinkableValueNode
{
	typedef etl::handle<ValueNode_Scale> Handle;
	typedef etl::handle<const ValueNode_Scale> ConstHandle;
	
private:
	ValueNode::RHandle value_node;
	ValueNode::RHandle scalar;

	ValueNode_Scale();

public:

	//static Handle create(ValueBase::Type id=ValueBase::TYPE_NIL);

	//static Handle create(ValueNode::Handle value_node, Real scalar);

	virtual ~ValueNode_Scale();

	//! \writeme
	bool set_value_node(const ValueNode::Handle &a);

	//! \writeme
	ValueNode::Handle get_value_node()const;

	void set_scalar(Real x);

	bool set_scalar(const ValueNode::Handle &x);

	ValueNode::Handle get_scalar()const;




	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	
	virtual String get_local_name()const;

	virtual String link_local_name(int i)const;

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	
	virtual LinkableValueNode* create_new()const;

public:
	using sinfg::LinkableValueNode::get_link_vfunc;
	using sinfg::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_Scale* create(const ValueBase &x);
}; // END of class ValueNode_Scale

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
