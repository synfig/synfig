/* === S I N F G =========================================================== */
/*!	\file blineconvert.h
**	\brief Template Header
**
**	$Id: blineconvert.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_BLINE_CONVERT_H
#define __SINFG_BLINE_CONVERT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/general.h>
#include <sinfg/blinepoint.h>
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {
	
class BLineConverter
{
public:
	struct cpindex
	{
		int 		curind;
		sinfg::Real	tangentscale;
		sinfg::Real	error;	//negative error will indicate invalid;
		
		cpindex(int ci, sinfg::Real s=0, sinfg::Real e=-1) 
		:curind(ci), tangentscale(s), error(e) 
		{}
		
		cpindex(const cpindex & o)
		:curind(o.curind), tangentscale(o.tangentscale), error(o.error) 
		{}
		
		const cpindex & operator = (const cpindex & rhs)
		{
			curind = rhs.curind;
			tangentscale = rhs.tangentscale;
			error = rhs.error;
			return *this;
		}
		
		bool operator < (const cpindex &rhs) const
		{
			return curind < rhs.curind;		
		}
		
		//point is obviously in[curind]
		//tangent scale will get reset to the smallest (or something else depending on experimentation)
	};

private:
	//cached data
	std::vector<sinfg::Point>	f;	//the preprocessed input cache
	std::vector<sinfg::Real>	f_w;

	//temporary point storage for vector calc
	std::vector<sinfg::Point>	ftemp;
	
	std::vector<sinfg::Vector>	df; //the derivative cache	
	std::vector<sinfg::Real>	cvt; //the curvature cache
	
	std::vector<int>			brk; //the break point cache
	
	std::vector<sinfg::Real> 	di,	//cumulative distance
								d_i; //distance between adjacent segments
	
	std::vector<sinfg::Point>	work; //the working point cache for the entire curve
	std::vector<cpindex> 		curind;
	
	//function parameters
	void clear();

public:
	sinfg::Real width;

	//Converter properties
	sinfg::Real pixelwidth;
	sinfg::Real smoothness; //actual error will be smoothness*pixelwidth (for global set pixelwidth = 1)

	BLineConverter();

	static void EnforceMinWidth(std::list<sinfg::BLinePoint> &bline, sinfg::Real min_pressure);
	void operator ()(std::list<sinfg::BLinePoint> &out, const std::list<sinfg::Point> &in,const std::list<sinfg::Real> &in_w);
};
	
}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
