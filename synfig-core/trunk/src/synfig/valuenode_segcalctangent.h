/* === S Y N F I G ========================================================= */
/*!	\file valuenode_segcalctangent.h
**	\brief Template Header
**
**	$Id: valuenode_segcalctangent.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_VALUENODE_SEGCALCTANGENT_H
#define __SYNFIG_VALUENODE_SEGCALCTANGENT_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_SegCalcTangent : public LinkableValueNode
{
	ValueNode::RHandle segment_;
	ValueNode::RHandle amount_;
	
	ValueNode_SegCalcTangent(const ValueBase::Type &x=ValueBase::TYPE_VECTOR);

public:

	typedef etl::handle<ValueNode_SegCalcTangent> Handle;
	typedef etl::handle<const ValueNode_SegCalcTangent> ConstHandle;

	//static Handle create(const ValueBase::Type &x=ValueBase::TYPE_VECTOR);


	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_SegCalcTangent();

	virtual String get_name()const;
	virtual String get_local_name()const;


	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;
	virtual int link_count()const;
	virtual String link_name(int i)const;

	virtual String link_local_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_SegCalcTangent* create(const ValueBase &x=ValueBase::TYPE_VECTOR);
}; // END of class ValueNode_SegCalcTangent

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
