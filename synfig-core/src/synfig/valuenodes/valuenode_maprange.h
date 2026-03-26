/* === S Y N F I G ========================================================= */
/*!	\file valuenode_maprange.h
**	\brief Header file for implementation of the "Map Range" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2025 Synfig Contributors
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

#ifndef SYNFIG_VALUENODE_MAPRANGE_H
#define SYNFIG_VALUENODE_MAPRANGE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_MapRange : public LinkableValueNode
{
	enum Interpolation {
		LINEAR = 0,
	};

	ValueNode::RHandle interpolation_;
	ValueNode::RHandle clamp_;
	ValueNode::RHandle link_;
	ValueNode::RHandle from_min_;
	ValueNode::RHandle from_max_;
	ValueNode::RHandle to_min_;
	ValueNode::RHandle to_max_;

	ValueNode_MapRange(const ValueBase& value);

public:
	typedef etl::handle<ValueNode_MapRange> Handle;
	typedef etl::handle<const ValueNode_MapRange> ConstHandle;

	static ValueNode_MapRange* create(const ValueBase& value = ValueBase(), etl::loose_handle<Canvas> canvas = nullptr);
	virtual ~ValueNode_MapRange();

	String get_name() const override;
	String get_local_name() const override;
	static bool check_type(Type& type);

	ValueBase operator()(Time t) const override;

	/**
	 * Checks if it is possible to call get_inverse() for target_value at time t.
	 * If so, return the link_index related to the return value provided by get_inverse()
	 */
	InvertibleStatus is_invertible(const Time& t, const ValueBase& target_value, int* link_index = nullptr) const override;
	/** Returns the modified Link to match the target value at time t */
	ValueBase get_inverse(const Time& t, const synfig::ValueBase& target_value) const override;

protected:
	LinkableValueNode* create_new() const override;

	bool set_link_vfunc(int i, ValueNode::Handle value) override;
	ValueNode::LooseHandle get_link_vfunc(int i) const override;

	Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_Add

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif // SYNFIG_VALUENODE_MAPRANGE_H
