/* === S I N F G =========================================================== */
/*!	\file valuenode_composite.h
**	\brief Template Header
**
**	$Id: valuenode_composite.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SINFG_VALUENODE_COMPOSITE_H
#define __SINFG_VALUENODE_COMPOSITE_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class ValueNode_Composite : public LinkableValueNode
{
	ValueNode::RHandle components[6];
	ValueNode_Composite(const ValueBase &value);
	
public:
	typedef etl::handle<ValueNode_Composite> Handle;
	typedef etl::handle<const ValueNode_Composite> ConstHandle;


	~ValueNode_Composite();

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;
	virtual int link_count()const;
	virtual String link_name(int i)const;
	virtual String link_local_name(int i)const;
	virtual ValueBase operator()(Time t)const;


	virtual String get_name()const;
	virtual String get_local_name()const;
	virtual int get_link_index_from_name(const String &name)const;
	
protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	
	LinkableValueNode* create_new()const;

public:
	using sinfg::LinkableValueNode::set_link_vfunc;
	using sinfg::LinkableValueNode::get_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_Composite* create(const ValueBase &x);
}; // END of class ValueNode_Composite

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
