/* === S Y N F I G ========================================================= */
/*!	\file curveset.cpp
**	\brief Curve Set Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "curve_helper.h"
#include "curveset.h"
#include "blinepoint.h"
#include <ETL/bezier>
#include <vector>
#include <list>
#include <set>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
//const Real ERROR = 1e-10;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
/*template < typename T >
inline bool Zero(const T &a, const T &tol = (T)ERROR)
{
	return a < tol && a > -tol;
}*/

CurvePoint::CurvePoint(const Point &pin, const Vector &left, const Vector &right)
:p(pin),l(left),r(right)
{
}

CurvePoint::CurvePoint(const BLinePoint &bpoint)
{
	p = bpoint.get_vertex();

	l = p + bpoint.get_tangent1()*(1/3.0f);
	r = p + bpoint.get_tangent2()*(1/3.0f);
}

struct ipoint
{
	int 	curveindex;
	int		vertindex;
	float	tvalue;

	ipoint	*next;
	ipoint	*prev;
	ipoint	*neighbor;

	int 	go_in;	//going in = 1, coming out = -1

	ipoint():
		curveindex(),
		vertindex(),
		tvalue(),
		go_in()
	{
		next = this;
		prev = this;
		neighbor = NULL;
	}

	bool operator<(const ipoint &rhs) const
	{
		if(curveindex == rhs.curveindex)
		{
			if(vertindex == rhs.vertindex)
			{
				return tvalue < rhs.tvalue;
			}else return vertindex < rhs.vertindex;
		}else return curveindex < rhs.curveindex;
	}

	bool operator>(const ipoint &rhs) const
	{
		return rhs < *this;
	}

	void insert_after(ipoint *i)
	{
		//from: next - next.prev
		//to: next* - i - next.prev*

		ipoint 	*bef = this,
				*aft = next;

		//assuming the input point is not connected to anything, we don't have to do anything with it...
		bef->next = i;
		i->prev = bef;
		aft->prev = i;
		i->next = aft;
	}

	void insert_before(ipoint *i)
	{
		//from: prev.next - prev
		//to: prev.next* - i - prev*

		ipoint 	*bef = prev,
				*aft = this;

		//assuming the input point is not connected to anything, we don't have to do anything with it...
		bef->next = i;
		i->prev = bef;
		aft->prev = i;
		i->next = aft;
	}

	void insert_sorted(ipoint *i)
	{
		ipoint *search = this;

		if (*i < *this)
		{
			//we go forward
			do
			{
				search = search->next;
			} while (*i < *search && search != this); //ending conditions...

			//now we insert previously...
			search->insert_before(i);
		} else if (*i > *this) {
			//we go backwards...
			do
			{
				search = search->prev;
			} while (*i > *search && search != this); //ending conditions...

			//now we insert previously...
			search->insert_after(i);
		}
	}
};

enum SetOp
{
	INTERSECT	= 0,
	UNION,
	SUBTRACT,
	INVSUBTRACT,
	NUM_SETOPERATIONS
};

class PolygonClipper
{
public:
	typedef vector<ipoint *>	CurveInts; //in no particular order

	vector<CurveInts>	c1ints;
	vector<CurveInts>	c2ints;

	//get the intersections
	void GetIntersections(const CurveSet &lhs, const CurveSet &rhs)
	{
		CIntersect	isect;
		bezier<Point>	b1,b2;

		int i1,j1,ci1,s1;
		int i2,j2,ci2,s2;

		//clear out so everyone's happy
		c1ints.clear();
		c2ints.clear();

		c1ints.resize(lhs.set.size());
		c2ints.resize(rhs.set.size());

		//loop through everyone and be happy...

		//intersect each curve with each other curve, and we're good
		for(ci1=0;ci1 < (int)lhs.set.size(); ++ci1)
		{
			const CurveSet::region &cur1 = lhs.set[ci1];
			s1 = cur1.size();
			for(j1 = s1-1, i1=0; i1 < s1; j1 = i1++)
			{
				b1[0] = cur1[j1].p;
				b1[3] = cur1[i1].p;
				b1[1] = b1[0] + cur1[j1].r/3;
				b1[2] = b1[3] - cur1[i1].l/3;

				for(ci2=0;ci2 < (int)rhs.set.size(); ++ci2)
				{
					const CurveSet::region &cur2 = rhs.set[ci2];
					s2 = cur2.size();
					for(j2 = s2-1, i2=0; i2 < s2; j2 = i2++)
					{
						b2[0] = cur2[j2].p;
						b2[3] = cur2[i2].p;
						b2[1] = b2[0] + cur2[j2].r/3;
						b2[2] = b2[3] - cur2[i2].l/3;

						isect(b1,b2);

						for(int index=0; index < (int)isect.times.size(); ++index)
						{
							//prepare basic intersection information
							ipoint *ip1 = new ipoint, *ip2 = new ipoint;

							//set parameters
							ip1->curveindex = ci1; ip1->vertindex = j1; ip1->tvalue = isect.times[index].first;
							ip2->curveindex = ci2; ip2->vertindex = j2; ip2->tvalue = isect.times[index].second;

							//set neighbors
							ip1->neighbor = ip2;
							ip2->neighbor = ip1;

							//first one just goes on end of list
							c1ints[ci1].back()->insert_sorted(ip1);
							c1ints[ci1].push_back(ip1);

							//second one must go in order
							c2ints[ci2].back()->insert_sorted(ip2);
							c2ints[ci2].push_back(ip2);

							//we're all good...
						}
					}
				}
			}
		}

		//Now figure out the containment properties of each int point
		Point p;
		int inside = 0;
		for(int i = 0; i < (int)c1ints.size(); ++i)
		{
			if(c1ints[i].size() == 0) continue;

			//must test insideness for the edges
			ipoint *start, *iter;
			start = iter = c1ints[i].front();

			//i == iter->curveindex == the index of the current curve we're looking at

			//set the initial insideness on the other curve...
			p = lhs.set[i][iter->vertindex].p;
			inside = rhs.intersect(p)%2; //if it's inside by the even odd rule

			do
			{
				iter->go_in = inside? -1 : 1; //leaving if inside, or coming in if not
				inside = !inside;
				iter = iter->next;
			}while(iter != start); //I hope this isn't an infinite loop!
		}

		//and curve 2
		for(int i = 0; i < (int)c2ints.size(); ++i)
		{
			if(c2ints[i].size() == 0) continue;

			//must test insideness for the edges
			ipoint *start, *iter;
			start = iter = c1ints[i].front();

			//set the initial insideness on the other curve...
			p = rhs.set[i][iter->vertindex].p;
			inside = lhs.intersect(p)%2; //if it's inside by the even odd rule

			do
			{
				iter->go_in = inside? -1 : 1; //leaving if inside, or coming in if not
				inside = !inside;
				iter = iter->next;
			}while(iter != start); //I hope this isn't an infinite loop!
		}
	}

	bool ConstructSet(CurveSet &/*c*/, const CurveSet &lhs, const CurveSet &rhs, int type)
	{
		bool in1,in2;

		switch(type)
		{
			case INTERSECT: //1&2
			{
				in1 = true; in2 = true;
				break;
			}

			case UNION: //1|2
			{
				in1 = false; in2 = false;
				break;
			}

			case SUBTRACT: //1-2
			{
				in1 = true; in2 = false;
				break;
			}

			case INVSUBTRACT: //2-1
			{
				in1 = false; in2 = true;
				break;
			}

			default:
			{
				return false;
			}
		}

		//traverse path based on inside flags

		//fill all the paths of native stuff
		set<ipoint *>	ipset;
		for(int ci=0; ci<(int)c1ints.size(); ++ci)
		{
			for(int i=0; i < (int)c1ints[ci].size(); ++i)
			{
				ipset.insert(c1ints[ci][i]);
			}
		}

		//
		while(ipset.size() > 0)
		{
			//start from one point (always on curveset 1) and traverse until we find it again
			ipoint *start, *iter;
			start = iter = *ipset.begin();

			//All the info to swap when we transition curves...
			const CurveSet *cur, *other;
			bool curin, otherin;
			bool delcur = true;

			set<ipoint *>::iterator deliter;

			//int ci,i1,i2,size;
			//float t1,t2;

			CurveSet::region	current;
			CurvePoint	cp;

			cur = &lhs; other = &rhs;
			curin = in1; otherin = in2;
			delcur = true;

			do
			{
				//remove the current iter from the set
				if(delcur)
				{
					deliter = ipset.find(iter);
					if(deliter != ipset.end()) ipset.erase(deliter);
				}

				//go to next and accumulate information
				//ci = iter->curveindex;
				//i1 = iter->vertindex;
				//t1 = iter->tvalue;
				iter = iter->next; //move to next and get its info

				//i2 = iter->vertindex;
				//t2 = iter->tvalue;

				//size = cur->set[ci].size();

				//record all the stuff between us...
				//start on an intersection - get the curve point...


				//transition curves...
				iter = iter->neighbor;
				swap(cur,other);
				swap(curin,otherin);
				delcur = !delcur;
			}while(iter != start); //I hope THIS isn't an infinite loop
		}

		return true;
	}
};

void CurveSet::SetClamp(int &i, int &si)
{
	if(si > 0 && si < (int)set.size())
	{
		if(i >= (int)set[si].size())
		{
			i -= set[si].size();
			si++;
		}else if (i < 0)
		{
			i += set[si].size();
			si--;
		}
	}
}

void CurveSet::CleanUp(int /*curve*/)
{
}

/*	Detect intersections that are crazy happy good

	Performance annoyances:
	1) Recursing down to find an intersection at the end points that doesn't actually exist
		(can be helped a bit by not including the edges of bounding rectangles)
	2) Intersecting curves is slow... oh well

	Algorithm:
	1) Inside out scheme, track when edges go into and come out of various objects etc.

	+ doesn't require initial conditions
	- only works with odd-even rule
*/

CurveSet CurveSet::operator&(const CurveSet &/*rhs*/) const
{
	return *this;
}

CurveSet CurveSet::operator|(const CurveSet &/*rhs*/) const
{
	return *this;
}

CurveSet CurveSet::operator-(const CurveSet &/*rhs*/) const
{
	return *this;
}

int CurveSet::intersect(const Point &p) const
{
	int inter = 0, ci,i,j,s;
	bezier<Point>	b;

	for(ci=0; ci < (int)set.size(); ++ci)
	{
		const vector<CurvePoint> &curve = set[ci];
		s = curve.size();
		for(j=s-1,i=0; i < s; j = i++)
		{
			b[0] = curve[j].p; b[3] = curve[i].p;
			b[1] = b[0] + curve[j].r/3; b[2] = b[3] - curve[i].l/3;

			inter += synfig::intersect(b,p);
		}
	}

	return inter;
}
