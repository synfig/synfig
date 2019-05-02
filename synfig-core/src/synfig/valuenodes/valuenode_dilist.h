/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.h
**	\brief Header file for implementation of the "Dash Item List" valuenode
**	conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_VALUENODE_DILIST_H
#define __SYNFIG_VALUENODE_DILIST_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <list>

#include <synfig/valuenode.h>
#include <synfig/time.h>
#include <synfig/uniqueid.h>
#include <synfig/dashitem.h>
#include "valuenode_dynamiclist.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
//! Converts a ValueNode_DIList into a DashItem list
// TODO synfig::ValueBase convert_bline_to_DIList(const ValueBase& bline);

/*! \class ValueNode_DIList
**	\brief This class implements a list of Dash Items
*/
class ValueNode_DIList : public ValueNode_DynamicList
{
private:
	ValueNode::RHandle bline_;
public:

	typedef etl::handle<ValueNode_DIList> Handle;
	typedef etl::handle<const ValueNode_DIList> ConstHandle;
	typedef etl::handle<const ValueNode_DIList> LooseHandle;
	ValueNode_DIList();

public:

 	virtual ValueBase operator()(Time t)const;
	virtual ~ValueNode_DIList();
	virtual String link_local_name(int i)const;
	virtual String get_name()const;
	virtual String get_local_name()const;
	//! Inserts a new entry between the previous found
	//! dashitem and the one where the action was called
	//! \param index the index of the entry where the action is done
	//! \param time the time when inserted in animation mode
	//! \param origin unused. Always is in the middle.
	//! \return the new List Entry
	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5);
	//! Gets the bline RHandle
	ValueNode::LooseHandle get_bline()const;
	//! Sets the bline RHandle
	void set_bline(ValueNode::Handle b);

protected:

	LinkableValueNode* create_new()const;

public:

	static bool check_type(Type &type);
	// Creates a Value Node Width Point List from another compatible list
	static ValueNode_DIList* create(const ValueBase &x=type_list);
}; // END of class ValueNode_DIList

typedef ValueNode_DIList::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
