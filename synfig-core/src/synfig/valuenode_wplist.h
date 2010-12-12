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

#include "valuenode.h"
#include "time.h"
#include "uniqueid.h"
#include "widthpoint.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class ValueNode_WPList
**	\brief This class implements a list of Width Points
* 	It will provide methods to calculate the width at any intermediate position
* 	and the insertion and remove of width points.
* 	It shouldn't allow to remove all the width points to still allowing to the
* 	user to add new ones.
*/
class ValueNode_WPList : public ValueNode_DynamicList
{
public:

	typedef etl::handle<ValueNode_WPList> Handle;
	typedef etl::handle<const ValueNode_WPList> ConstHandle;
	ValueNode_WPList();

public:

 	virtual ValueBase operator()(Time t)const;
	virtual ~ValueNode_WPList();
	virtual String link_local_name(int i)const;
	virtual String get_name()const;
	virtual String get_local_name()const;
	// Inserts a new entry at index with time and origin used for
	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5);

protected:

	LinkableValueNode* create_new()const;

public:

	static bool check_type(ValueBase::Type type);
	// Creates a Value Node Width Point List from another compatible list
	static ValueNode_WPList* create(const ValueBase &x=ValueBase::TYPE_LIST);
}; // END of class ValueNode_WPList

typedef ValueNode_WPList::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
