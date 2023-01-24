/* === S Y N F I G ========================================================= */
/*!	\file wplistconverter.h
**	\brief Header for a converter between widths and positions
**	form a extended device and a equivalent ValueNode_WPList
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_WPLISTCONVERTER_H
#define __SYNFIG_WPLISTCONVERTER_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>

#include <synfig/widthpoint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class WPListConverter
{
private:
	//! Cache of points ready to be processed after remove duplicated
	std::vector<synfig::Point> points;
	//! Cache of widths ready to be processed after remove duplicated
	std::vector<synfig::Real> widths;
	//! The processed result of the widthpoints
	std::vector<synfig::WidthPoint> work_out;
	//! Calculated cumulated distances to origin
	std::vector<synfig::Real> distances;
	//! Calculated cumulated distances to origin normalized
	std::vector<synfig::Real> norm_distances;
	//! The error value at each position: ek[k]=w[k]-wp_out.width[k]
	std::vector<synfig::Real> ek;
	//! The error value at each position: ek2=ek[k]*ek[k]
	std::vector<synfig::Real> ek2;
	//! The number of valid widths and points
	unsigned int n;
	//! The current squared error
	synfig::Real se;

	//! This updates: ek, ek2 at the interval k1, k2, and returns the index where
	//! ek2 is maximum. If 'e' (squared error) is >=0 then it returns
	//! the new squared error at 'e' if not it just fills the error vectors for the first time
	unsigned int calculate_ek2(unsigned int k1, unsigned int k2, bool first_time=false);
	//! Finds next/previous widthpoint with dash=false. Don't consider k itself.
	unsigned int find_next(unsigned int k);
	unsigned int find_prev(unsigned int k);

	void clear();

public:
	synfig::Real err2max;
	WPListConverter();
	static void EnforceMinWidth(std::list<synfig::WidthPoint> &wplist, synfig::Real min_pressure);
	void operator()(std::list<synfig::WidthPoint> &wp_out, const std::list<synfig::Point> &p, const std::list<synfig::Real> &w);
};

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
