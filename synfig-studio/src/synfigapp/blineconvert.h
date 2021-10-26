/* === S Y N F I G ========================================================= */
/*!	\file blineconvert.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_BLINE_CONVERT_H
#define __SYNFIG_BLINE_CONVERT_H

/* === H E A D E R S ======================================================= */

#include <synfig/blinepoint.h>
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class BLineConverter
{
public:
	struct cpindex
	{
		int 		curind;
		synfig::Real	tangentscale;
		synfig::Real	error;	//negative error will indicate invalid;

		cpindex(int ci, synfig::Real s=0, synfig::Real e=-1)
		:curind(ci), tangentscale(s), error(e)
		{}

		cpindex(const cpindex & o)
		:curind(o.curind), tangentscale(o.tangentscale), error(o.error)
		{}

		const cpindex & operator=(const cpindex & rhs)
		{
			curind = rhs.curind;
			tangentscale = rhs.tangentscale;
			error = rhs.error;
			return *this;
		}

		bool operator<(const cpindex &rhs) const
		{
			return curind < rhs.curind;
		}

		//point is obviously in[curind]
		//tangent scale will get reset to the smallest (or something else depending on experimentation)
	};

private:
	//cached data
	std::vector<synfig::Point>	point_cache;	//the preprocessed input cache
	std::vector<synfig::Real>	width_cache;

	//temporary point storage for vector calc
	std::vector<synfig::Point>	ftemp;

	std::vector<synfig::Vector>	deriv; //the derivative cache
	std::vector<synfig::Real>	curvature; //the curvature cache

	std::vector<int>			break_tangents; //the break point cache

	std::vector<synfig::Real> 	cum_dist,	//cumulative distance
								this_dist; //distance between adjacent segments

	std::vector<synfig::Point>	work; //the working point cache for the entire curve
	std::vector<cpindex> 		curind;

	//function parameters
	void clear();

public:
	synfig::Real width;

	//Converter properties
	synfig::Real pixelwidth;
	synfig::Real smoothness; //actual error will be smoothness*pixelwidth (for global set pixelwidth = 1)

	BLineConverter();

	static void EnforceMinWidth(std::list<synfig::BLinePoint> &bline, synfig::Real min_pressure);
	void operator()(std::list<synfig::BLinePoint> &out, const std::list<synfig::Point> &in,const std::list<synfig::Real> &in_w);
};

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
