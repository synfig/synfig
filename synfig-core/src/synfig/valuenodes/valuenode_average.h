/* === S Y N F I G ========================================================= */
/*!	\file valuenode_average.h
**	\brief Header file for implementation of the "Average" valuenode conversion.
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_VALUENODE_AVERAGE_H
#define __SYNFIG_VALUENODE_AVERAGE_H

/* === H E A D E R S ======================================================= */

#include "valuenode_dynamiclist.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Average : public ValueNode_DynamicList
{
public:

	typedef etl::handle<ValueNode_Average> Handle;
	typedef etl::handle<const ValueNode_Average> ConstHandle;


	ValueNode_Average(const ValueBase &value, etl::loose_handle<Canvas> canvas = 0);
	ValueNode_Average(Type &type, etl::loose_handle<Canvas> canvas = 0);
	virtual ~ValueNode_Average();

 	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

protected:
	LinkableValueNode* create_new()const;

public:
	static bool check_type(Type &type);
	static ValueNode_Average* create(const ValueBase &value, etl::loose_handle<Canvas> canvas = 0);
}; // END of class ValueNode_Average

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
