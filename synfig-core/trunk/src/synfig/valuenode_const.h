/* === S I N F G =========================================================== */
/*!	\file valuenode_const.h
**	\brief Template Header
**
**	$Id: valuenode_const.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SINFG_VALUENODE_CONST_H
#define __SINFG_VALUENODE_CONST_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class ValueNode_Const : public ValueNode
{
public:
	typedef etl::handle<ValueNode_Const> Handle;
	typedef etl::handle<const ValueNode_Const> ConstHandle;

private:
	ValueBase value;
	
	ValueNode_Const();
	ValueNode_Const(const ValueBase &x);

public:
	
	virtual ValueBase operator()(Time t)const;
	virtual ~ValueNode_Const();

	const ValueBase &get_value()const;
	ValueBase &get_value();
	void set_value(const ValueBase &data);
	

	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode* clone(const GUID& deriv_guid=GUID())const;
	
public:
	static ValueNode_Const* create(const ValueBase &x=ValueBase());

protected:
	virtual void get_times_vfunc(Node::time_set &set) const;
};

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
