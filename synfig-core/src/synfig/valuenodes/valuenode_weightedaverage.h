/* === S Y N F I G ========================================================= */
/*!	\file valuenode_weightedaverage.h
**	\brief Header file for implementation of the "Weighted Average" valuenode conversion.
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
public:
	typedef std::shared_ptr<ValueNode_WeightedAverage> Handle;
	typedef std::shared_ptr<const ValueNode_WeightedAverage> ConstHandle;

	ValueNode_WeightedAverage(const ValueBase &value, std::shared_ptr<Canvas> canvas = 0);
	ValueNode_WeightedAverage(Type &type, std::shared_ptr<Canvas> canvas = 0);
	virtual ~ValueNode_WeightedAverage();

 	virtual ValueBase operator()(Time t)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

protected:
	LinkableValueNode* create_new()const;

public:
	static bool check_type(Type &type);
	static ValueNode_WeightedAverage* create(const ValueBase &value, std::shared_ptr<Canvas> canvas = 0);
}; // END of class ValueNode_WeightedAverage

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
