/* === S Y N F I G ========================================================= */
/*!	\file valuenode_average.h
**	\brief Header file for implementation of the "Average" valuenode conversion.
**
**	\legal
**	......... ... 2014 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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
	ValueNode_Average(const ValueBase &value, etl::loose_handle<Canvas> canvas);

public:
	typedef etl::handle<ValueNode_Average> Handle;
	typedef etl::handle<const ValueNode_Average> ConstHandle;

	ValueNode_Average(Type &type, etl::loose_handle<Canvas> canvas);
	static ValueNode_Average* create(const ValueBase& value, etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_Average();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

protected:
	LinkableValueNode* create_new() const override;
}; // END of class ValueNode_Average

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
