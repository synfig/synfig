/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinecalcvertex.h
**	\brief Header file for implementation of the "BLine Vertex" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_VALUENODE_BLINECALCVERTEX_H
#define __SYNFIG_VALUENODE_BLINECALCVERTEX_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_BLineCalcVertex : public LinkableValueNode
{
	ValueNode::RHandle bline_;
	ValueNode::RHandle loop_;
	ValueNode::RHandle amount_;
	ValueNode::RHandle homogeneous_;

	ValueNode_BLineCalcVertex(Type &x=type_vector);

public:
	typedef etl::handle<ValueNode_BLineCalcVertex> Handle;
	typedef etl::handle<const ValueNode_BLineCalcVertex> ConstHandle;

	static ValueNode_BLineCalcVertex* create(const ValueBase& x=type_vector, etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_BLineCalcVertex();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	ValueNode::LooseHandle get_bline_handle(bool& bline_loop) {
		ValueNode::LooseHandle bline = this->get_link("bline");
		if (bline) {
			ValueBase v = (*bline)(Time(0));

			if (v.get_contained_type() == type_bline_point)
				bline_loop = v.get_loop();
		}
		return bline;
	}

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_BLineCalcVertex

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
