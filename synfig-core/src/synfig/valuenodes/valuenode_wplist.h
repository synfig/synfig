/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.h
**	\brief Header file for implementation of the "Width Point List" valuenode
**	conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Carlos López
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

#ifndef __SYNFIG_VALUENODE_WPLIST_H
#define __SYNFIG_VALUENODE_WPLIST_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <list>

#include <synfig/valuenode.h>
#include <synfig/time.h>
#include <synfig/uniqueid.h>
#include <synfig/widthpoint.h>
#include "valuenode_dynamiclist.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
//! Converts a ValueNode_BLine into a WidthPoint list
synfig::ValueBase convert_bline_to_wplist(const ValueBase& bline);
//! Interpolated width
//! \param prev the previous width point to evaluate the width
//! \param next the next width point to evaluate the width
//! \param position the position to interpolate between prev and next
//! \param smoothness a [0,1] value that defines how is calculated the interpolation
//! \return the interpolated width based on surrounding width points and its values
synfig::Real widthpoint_interpolate(const WidthPoint& prev, const WidthPoint& next, const Real position, const Real smoothness=0.0);

/*! \class ValueNode_WPList
**	\brief This class implements a list of Width Points
* 	It will provide methods to calculate the width at any intermediate position
* 	and the insertion and remove of width points.
* 	It shouldn't allow to remove all the width points to still allowing to the
* 	user to add new ones.
*/
class ValueNode_WPList : public ValueNode_DynamicList
{
private:
	ValueNode::RHandle bline_;

public:

	typedef etl::handle<ValueNode_WPList> Handle;
	typedef etl::handle<const ValueNode_WPList> ConstHandle;
	typedef etl::handle<const ValueNode_WPList> LooseHandle;
	ValueNode_WPList();

public:

 	virtual ValueBase operator()(Time t)const;
	virtual ~ValueNode_WPList();
	virtual String link_local_name(int i)const;
	virtual String get_name()const;
	virtual String get_local_name()const;
	//! Inserts a new entry between the previous found
	//! widthpoint and the one where the action was called
	//! with an average width and a middle position.
	//! \param index the index of the entry where the action is done
	//! \param time the time when inserted in animation mode
	//! \param origin unused. Always is in the middle.
	//! \return the new List Entry
	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5);
	//! Finds a fully on width point at given time and after the given position
	//! \param position the position where to start to seek from
	//! \param time the time when things are evaluated
	//! \return a width point reference with the proper values
	WidthPoint find_next_valid_entry_by_position(Real position, Time time=0)const;
	//! Finds a fully on width point at given time and before the given position
	//! \param position the position where to start to seek from
	//! \param time the time when things are evaluated
	//! \return a width point reference with the proper values
	WidthPoint find_prev_valid_entry_by_position(Real position, Time time=0)const;
	//! Interpolated width at a a given time based on surrounding full 'on' width points
	//! \param position the position where to evaluate the width
	//! \param time the time when evaluates
	//! \return the interpolated width
	Real interpolated_width(Real position, Time time)const;
	//! Gets the bline RHandle
	ValueNode::LooseHandle get_bline()const;
	//! Sets the bline RHandle
	void set_bline(ValueNode::Handle b);

protected:

	LinkableValueNode* create_new()const;

public:

	static bool check_type(Type &type);
	// Creates a Value Node Width Point List from another compatible list
	static ValueNode_WPList* create(const ValueBase &x=type_list);
}; // END of class ValueNode_WPList

typedef ValueNode_WPList::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
