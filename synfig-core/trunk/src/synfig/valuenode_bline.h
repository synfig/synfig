/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.h
**	\brief Template Header
**
**	$Id: valuenode_bline.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_VALUENODE_BLINE_H
#define __SYNFIG_VALUENODE_BLINE_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <list>

#include "valuenode.h"
#include "time.h"
#include "uniqueid.h"
#include "blinepoint.h"
#include "valuenode_dynamiclist.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {


//! Converts a list of bline points into a list of segments
ValueBase convert_bline_to_segment_list(const ValueBase &bline);

//! Converts a list of bline points into a list of widths
ValueBase convert_bline_to_width_list(const ValueBase &bline);
	
/*! \class ValueNode_BLine
**	\brief \writeme
*/
class ValueNode_BLine : public ValueNode_DynamicList
{
public:

	typedef etl::handle<ValueNode_BLine> Handle;
	typedef etl::handle<const ValueNode_BLine> ConstHandle;
	

	ValueNode_BLine();

public:
	

	
 	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_BLine();

	virtual String link_local_name(int i)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode* clone(const GUID& deriv_guid=GUID())const;
	
	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5);

protected:
	
	LinkableValueNode* create_new()const;

public:
	//using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_BLine* create(const ValueBase &x=ValueBase::TYPE_LIST);
}; // END of class ValueNode_BLine

typedef ValueNode_BLine::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
