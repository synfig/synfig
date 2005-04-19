/* === S Y N F I G ========================================================= */
/*!	\file curveset.h
**	\brief Curve Set Header
**
**	$Id: curveset.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_CURVESET_H
#define __SYNFIG_CURVESET_H

/* === H E A D E R S ======================================================= */
#include "blinepoint.h"
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig
{

class BLinePoint;
	
struct CurvePoint
{
	Point	p;
	Point	l,r;
	
	CurvePoint () {}
	CurvePoint(const Point &pin, const Vector &left, const Vector &right);
	
	CurvePoint(const BLinePoint &bpoint);
};
	
class CurveSet 
{
	bool		invert; //winding order...
	
	void CleanUp(int curve = 0);
public:
	
	typedef std::vector<CurvePoint>	region;
	typedef std::vector<region>		set_type;
	
	set_type	set; //specifies a region object (assumes looping)

	void SetClamp(int &i, int &si);

	//actual stuff
	CurveSet()
	{
	}
	
	//anything supporting iterator type operations
	template < typename Iterator >
	CurveSet(Iterator begin, Iterator end, bool invert = false)
	{
		set.push_back(std::vector<CurvePoint>(begin,end));
		CleanUp(invert);
	}
	
	CurveSet operator &(const CurveSet &rhs) const;	//intersect
	CurveSet operator |(const CurveSet &rhs) const; //union
	CurveSet operator -(const CurveSet &rhs) const; //subtract
	
	
	//Point containment
	int intersect(const Point &p) const;
};

}
/* === E N D =============================================================== */

#endif
