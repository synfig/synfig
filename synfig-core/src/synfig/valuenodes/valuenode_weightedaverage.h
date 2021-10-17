/* === S Y N F I G ========================================================= */
/*!	\file valuenode_weightedaverage.h
**	\brief Header file for implementation of the "Weighted Average" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_WEIGHTEDAVERAGE_H
#define __SYNFIG_VALUENODE_WEIGHTEDAVERAGE_H

/* === H E A D E R S ======================================================= */

#include "valuenode_dynamiclist.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace types_namespace { class TypeWeightedValueBase; }

class ValueNode_WeightedAverage : public ValueNode_DynamicList
{
	ValueNode_WeightedAverage(const ValueBase &value, etl::loose_handle<Canvas> canvas = 0);
public:
	typedef etl::handle<ValueNode_WeightedAverage> Handle;
	typedef etl::handle<const ValueNode_WeightedAverage> ConstHandle;

	ValueNode_WeightedAverage(Type &type, etl::loose_handle<Canvas> canvas = 0);
	static ValueNode_WeightedAverage* create(const ValueBase& value, etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_WeightedAverage();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

protected:
	LinkableValueNode* create_new() const override;
}; // END of class ValueNode_WeightedAverage

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
