/* === S Y N F I G ========================================================= */
/*!	\file advancedline.h
**	\brief Header of the Advanced Line primitive
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
**	......... ... 2019 Ivan Mahonin
**	......... ... 2025 Synfig Contributors
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

#ifndef __SYNFIG_ADVANCED_LINE_H
#define __SYNFIG_ADVANCED_LINE_H

/* === H E A D E R S ======================================================= */

#include <map>

#include <synfig/rendering/primitive/contour.h>
#include <synfig/widthpoint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class AdvancedPoint
{
public:
	Real w;
	Vector pp0, pp1;
	WidthPoint::SideType side0, side1;

	AdvancedPoint()
		: w(), side0(WidthPoint::TYPE_INTERPOLATE), side1(side0)
	{ }

	Real y0() const { return side0 == WidthPoint::TYPE_INTERPOLATE ? w : 0; }
	Real y1() const { return side1 == WidthPoint::TYPE_INTERPOLATE ? w : 0; }
};

/**
 * A looped (or not) thick line with variable width along its path,
 * with possible beginning, ending and intermediate tip styles.
 *
 * AdvancedLine is a straight line, made up of one or more AdvancedPoints.
 * These points define the line thickness at their positions and set possible line endings too - if a "SideType" is not "TYPE_INTERPOLATE".
 *
 * To make a AdvancedLine a spline, generate its contour and then Bend it.
 */
class AdvancedLine : public std::map<Real, AdvancedPoint>
{
public:
	enum class AddAction {
		/** If point already exists, replace both side types */
		REPLACE_IF_EXISTS,
		/** If point already exists, replace outgoing side type only */
		APPEND,
		/** If point already exists, replace incoming side type only */
		PREPEND
	};

	/** Add a new point to the line
	 *  @param p line position
	 *  @param w line width/thickness at such position
	 *  @param side0 SideType of incoming side
	 *  @param side1 SideType of outgoing side
	 *  @param action how to add such point if another already exists at same position
	 */
	void add(Real p, Real w, WidthPoint::SideType side0, WidthPoint::SideType side1, AddAction action = AddAction::REPLACE_IF_EXISTS);

	/** Compute the incoming and outgoing tangents of the outline geometry */
	void calc_tangents(Real smoothness);

	/** Cut off the left side of this line.
	 *
	 *  "Left" means from starting point.
	 *
	 *  Please call calc_tangents() before using this method,
	 *  otherwise weird width will happen.
	 *
	 *  @param p the new starting point of the line
	 *  @param side the SideType for this new line ending
	 */
	void trunc_left(Real p, WidthPoint::SideType side);

	/** Cut off the right side of this line.
	 *
	 *  "Right" means from ending point.
	 *
	 *  Please call calc_tangents() before using this method,
	 *  otherwise weird width will happen.
	 *
	 *  @param p the new ending point of the line
	 *  @param side the SideType for this new line ending
	 */
	void trunc_right(Real p, WidthPoint::SideType side);

	/** Cut off a 'slice' of this line.
	 *
	 *  Please call calc_tangents() before using this method,
	 *  otherwise weird width will happen.
	 *
	 *  @param p0 the starting point of the slice to be removed, i.e. the lesser point position
	 *  @param p1 the ending point of the slice to be removed, i.e. the greater point position
	 *  @param side0 the new SideType for line ending at p0
	 *  @param side1 the new SideType for line starting at p1
	 */
	void cut(Real p0, Real p1, WidthPoint::SideType side0, WidthPoint::SideType side1);

	/** Build the geometry of this line
	 *
	 *  Call calc_tangents() before use this method.
	 */
	void build_contour(rendering::Contour& dst) const;
};

}

/* === E N D =============================================================== */

#endif // ADVANCEDLINE_H
