/* === S Y N F I G ========================================================= */
/*!	\file valuenode_reference.h
**	\brief Template Header
**
**	$Id: valuenode_reference.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_VALUENODE_REFERENCE_H
#define __SYNFIG_VALUENODE_REFERENCE_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Reference : public LinkableValueNode
{
	ValueNode::RHandle link_;
public:
	typedef etl::handle<ValueNode_Reference> Handle;
	typedef etl::handle<const ValueNode_Reference> ConstHandle;

	ValueNode_Reference(const ValueBase::Type &x);

	ValueNode_Reference(const ValueNode::Handle &x);

//	static Handle create(const ValueBase::Type &x);
//	static Handle create(const ValueNode::Handle &x);


	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_name(int i)const;

	virtual String link_local_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_Reference();

	virtual String get_name()const;

	virtual String get_local_name()const;

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	
	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_Reference* create(const ValueBase &x);
}; // END of class ValueNode_Reference

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
