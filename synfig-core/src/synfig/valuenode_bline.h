/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.h
**	\brief Header file for implementation of the "BLine" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

//! Finds the closest point to pos in bline
Real find_closest_point(const ValueBase &bline, const Point &pos, Real &radius, bool loop, Point *out_point = 0);

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

	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5);

protected:

	LinkableValueNode* create_new()const;

public:
	//using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_BLine* create(const ValueBase &x=ValueBase::TYPE_LIST);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_BLine

typedef ValueNode_BLine::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
