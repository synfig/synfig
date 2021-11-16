/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.h
**	\brief Header file for implementation of the "BLine" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_VALUENODE_BLINE_H
#define __SYNFIG_VALUENODE_BLINE_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <synfig/blinepoint.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {


//! Converts a list of bline points into a list of segments
ValueBase convert_bline_to_segment_list(const ValueBase &bline);

//! Converts a list of bline points into a list of widths
ValueBase convert_bline_to_width_list(const ValueBase &bline);

//! Finds the closest point to pos in bline
Real find_closest_point(const ValueBase &bline, const Point &pos, Real radius, bool loop, Point *out_point = 0);

//! Converts from standard to homogeneous index (considers the length)
Real std_to_hom(const ValueBase &bline, Real pos, bool index_loop, bool bline_loop);

//! Converts from homogeneous to standard index
Real hom_to_std(const ValueBase &bline, Real pos, bool index_loop, bool bline_loop);

//! Returns the length of the bline
Real bline_length(const ValueBase &bline, bool bline_loop, std::vector<Real> *lengths);


/*! \class ValueNode_BLine
**	\brief \writeme
*/
class ValueNode_BLine : public ValueNode_DynamicList
{
	ValueNode_BLine(etl::loose_handle<Canvas> canvas = 0);

	//! Returns the BlinePoint at time t, with the tangents modified if
	//! the vertex is boned influenced, otherwise returns the Blinepoint at time t.
	BLinePoint get_blinepoint(std::vector<ListEntry>::const_iterator current, Time t) const;

public:
	typedef etl::handle<ValueNode_BLine> Handle;
	typedef etl::handle<const ValueNode_BLine> ConstHandle;

	static ValueNode_BLine* create(const ValueBase& x=type_list, etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_BLine();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	virtual String link_local_name(int i) const override;
	static bool check_type(Type &type);

	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5) override;

protected:
	LinkableValueNode* create_new() const override;

	virtual Vocab get_children_vocab_vfunc() const override;

public:
#ifdef _DEBUG
	virtual void ref() const override;
	virtual bool unref() const override;
#endif
}; // END of class ValueNode_BLine

typedef ValueNode_BLine::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
