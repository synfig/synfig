/* === S Y N F I G ========================================================= */
/*!	\file gradient.cpp
**	\brief Color Gradient Class Member Definitions
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "gradient.h"

#include <algorithm>
#include <stdexcept>

#include <ETL/misc>

#include "general.h"
#include <synfig/localization.h>

#include "exception.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::Gradient::Gradient(const Color &c1, const Color &c2)
{
	push_back(CPoint(0.0,c1));
	push_back(CPoint(1.0,c2));
}

synfig::Gradient::Gradient(const Color &c1, const Color &c2, const Color &c3)
{
	push_back(CPoint(0.0,c1));
	push_back(CPoint(0.5,c2));
	push_back(CPoint(1.0,c3));
}

// This sort algorithm MUST be stable
// ie: it must not change the order of items with the same value.
// I am using a bubble sort.
// This algorithm will sort a nearly-sorted list at ~O(N), and
// it will sort an inverse sorted list at ~O(N*N).
void
synfig::Gradient::sort()
{
	stable_sort(begin(),end());
	/*
	iterator iter;
	iterator iter2,next;

	for(iter=begin();iter!=end();iter++)
	{
		for(next=iter, iter2=next--;iter2!=begin();iter2=next--)
		{
			if(*iter<*next)
			{
				//insert(next,*iter);
				//erase(iter);
				iter_swap(next,iter);

				continue;
			}
			else
				break;
		}
	}
	*/
}

static synfig::ColorAccumulator
supersample_helper(const synfig::Gradient::CPoint &color1, const synfig::Gradient::CPoint &color2, float begin, float end, float &weight)
{
	if(color1.pos==color2.pos || color1.pos>=end || color2.pos<=begin)
	{
		weight=0;
		return Color::alpha();
	}
	if(color1.pos>=begin && color2.pos<end)
	{
		weight=color2.pos-color1.pos;
		ColorAccumulator ret=Color::blend(color2.color,color1.color, 0.5, Color::BLEND_STRAIGHT).premult_alpha()*weight;
		return ret;
	}
	if(color1.pos>=begin && color2.pos>=end)
	{
		weight=end-color1.pos;
		float pos((end+color1.pos)*0.5);
		float amount((pos-color1.pos)/(color2.pos-color1.pos));
		//if(abs(amount)>1)amount=(amount>0)?1:-1;
		ColorAccumulator ret(Color::blend(color2.color,color1.color, amount, Color::BLEND_STRAIGHT).premult_alpha()*weight);
		return ret;
	}
	if(color1.pos<begin && color2.pos<end)
	{
		weight=color2.pos-begin;
		float pos((begin+color2.pos)*0.5);
		float amount((pos-color1.pos)/(color2.pos-color1.pos));
		//if(abs(amount)>1)amount=(amount>0)?1:-1;
		ColorAccumulator ret(Color::blend(color2.color,color1.color, amount, Color::BLEND_STRAIGHT).premult_alpha()*weight);
		return ret;
	}
	synfig::error("color1.pos=%f",color1.pos);
	synfig::error("color2.pos=%f",color2.pos);
	synfig::error("begin=%f",begin);
	synfig::error("end=%f",end);

	weight=0;
	return Color::alpha();

//	assert(0);
}

static void show_gradient(const Gradient::CPointList x)
{
	int i = 0;
	for (Gradient::const_iterator iter = x.begin(); iter != x.end(); iter++)
		printf("%3d : %.3f %s\n", i++, (*iter).pos, (*iter).color.get_string().c_str());
}

Gradient &
synfig::Gradient::operator+=(const Gradient &rhs)
{
	bool print=false;			// for debugging
	if (print) { printf("\nadding lhs:\n"); show_gradient(this->cpoints); printf("\n"); }
	if (print) { printf("adding rhs:\n"); show_gradient(rhs.cpoints); printf("\n"); }
	CPointList ret;
	const_iterator iter1 = begin(), iter2 = rhs.begin(), left_same, right_same;
	CPoint left, right;
	if (iter1 != end()) left = *iter1;
	if (iter2 != rhs.end()) right = *iter2;
	int pos1 = 0, pos2 = 0;
	CPoint old1, old2;

	// if there are cpoints in both gradients run through both until one runs out
	if (iter1 != end() && iter2 != rhs.end())
		while(true)
		{
			// if the left one has the first cpoint
			if (left.pos < right.pos)
			{
				// add on the right gradient's value at this point
				if (print) printf("using pos %.2f from left %d in loop\n", left.pos, pos1++);
				ret.push_back(CPoint(left.pos, left.color + rhs(left.pos)));
				if(++iter1 == end()) break;
				left=*iter1;
			}
			// if the right one has the first cpoint
			else if (left.pos > right.pos)
			{
				// add on the left gradient's value at this point
				if (print) printf("using pos %.2f from right %d in loop\n", right.pos, pos2++);
				ret.push_back(CPoint(right.pos, right.color + (*this)(right.pos)));
				if(++iter2 == rhs.end()) break;
				right=*iter2;
			}
			// they both have a cpoint at the same time
			else
			{
				int tpos1 = pos1, tpos2 = pos2;
				// skip past all cpoints at the same position
				for(left_same = ++iter1; iter1 != end() && (*iter1).pos == left.pos; iter1++, pos1++)
					if (print) printf("skipping past pos %d in left\n", pos1);
				for(right_same = ++iter2; iter2 != rhs.end() && (*iter2).pos == right.pos; iter2++, pos2++)
					if (print) printf("skipping past pos %d in right\n", pos2);

				// if there is only one cpoint at this position in each gradient,
				// there's only one corresponding cpoint in the sum
				if (iter1 == left_same && iter2 == right_same)
				{
					if (print) printf("two singles at left %d and right %d\n", pos1++, pos2++);
					ret.push_back(CPoint(left.pos, left.color + right.color));
				}
				// otherwise we sum the first in each, and the last in each
				else
				{
					if (print) printf("[copying %ld from left %d and %ld from right %d at %.2f]\n", iter1-left_same+1, tpos1, iter2-right_same+1, tpos2, left.pos);
					// merge the front two cpoints
					if (print) printf("  copy front from left %d right %d\n", tpos1++, tpos2++);
					ret.push_back(CPoint(left.pos, left.color + right.color));

					// merge the middle pairs of points - each middle point merges with its counterpart
					while(left_same < iter1-1 && right_same < iter2-1)
					{
						old1 = *(left_same++);
						old2 = *(right_same++);
						if (print) printf("  copy middle from left %d and right %d\n", tpos1++, tpos2++);
						ret.push_back(CPoint(old1.pos, old1.color+old2.color));
					}
					// if one gradient has more middle points than the other, merge the rest with the last point in the other gradient
					for(old2 = (*(iter2-1)); left_same < iter1-1; left_same++)
					{
						old1 = *left_same;
						if (print) printf("  copy middle from left %d plus end of right\n", tpos1++);
						ret.push_back(CPoint(old1.pos, old1.color + old2.color));
					}
					for(old1 = (*(iter1-1)); right_same < iter2-1; right_same++)
					{
						old2 = *right_same;
						if (print) printf("  copy middle from right %d plus end of left\n", tpos2++);
						ret.push_back(CPoint(old2.pos, old1.color + old2.color));
					}
					// merge the back two cpoints
					if (print) printf("  copy end from left %d right %d\n", pos1++, pos2++);
					ret.push_back(CPoint(left.pos, (*(iter1-1)).color + (*(iter2-1)).color));
				}
				// make sure we update 'left' and 'right'
				if (iter1 != end()) left=*iter1;
				if (iter2 == rhs.end()) break;
				right = *iter2;
				if (iter1 == end()) break;
			}
		}

	// one of the gradients has run out of points
	// does the left one have points left?
	if (iter1 != end())
		while(true)
		{
			if (print) printf("finish end from left %d\n", pos1++);
			ret.push_back(CPoint(left.pos, left.color + rhs(left.pos)));
			if(++iter1 == end()) break;
			left = *iter1;
		}
	// the left one was empty, so maybe the right one has points left
	else if (iter2 != rhs.end())
		while(true)
		{
			if (print) printf("finish end from right %d\n", pos2++);
			ret.push_back(CPoint(right.pos, right.color + (*this)(right.pos)));
			if(++iter2 == rhs.end()) break;
			right = *iter2;
		}

	if (print) { printf("\nsummed ret:\n"); show_gradient(ret); printf("\n"); }
	cpoints = ret;
	return *this;
}

Gradient &
synfig::Gradient::operator-=(const Gradient &rhs)
{
	return (*this)+=(rhs*-1);
}

Gradient &
synfig::Gradient::operator*=(const float    &rhs)
{
	if (rhs == 0)
		cpoints.clear();
	else
		for (iterator iter = cpoints.begin(); iter!=cpoints.end(); iter++)
			(*iter).color *= rhs;
	return *this;
}

Gradient &
synfig::Gradient::operator/=(const float    &rhs)
{
	for (iterator iter = cpoints.begin(); iter!=cpoints.end(); iter++)
		(*iter).color /= rhs;
	return *this;
}

Real
synfig::Gradient::mag()const
{
	Real ret(0), cm, prev_pos;
	const_iterator iter, next;
	if(!cpoints.size())
		return 0.0;
	next=cpoints.begin();
	iter=next++;
	prev_pos=0.0;
	for (; next!=cpoints.end(); iter++, next++)
		{
			cm=iter->color.get_y()*((next->pos-iter->pos)/2 - prev_pos) ;
			ret+=cm*cm;
			prev_pos = (next->pos-iter->pos)/2;
		}
		cm=next->color.get_y()*(1.0 - prev_pos);
		ret+=cm*cm;
	ret=sqrt(ret);
	return ret;
}

Color
synfig::Gradient::operator()(const Real &x,float supersample)const
{
	if(cpoints.empty())
		return Color(0,0,0,0);
	if(supersample<0)
		supersample=-supersample;
	if(supersample>2.0)
		supersample=2.0f;

	float begin_sample(x-supersample*0.5);
	float end_sample(x+supersample*0.5);

	if(cpoints.size()==1 || end_sample<=cpoints.front().pos || isnan(x))
		return cpoints.front().color;

	if(begin_sample>=cpoints.back().pos)
		return cpoints.back().color;

	/*
	if(end_sample>=back().pos)
		end_sample=back().pos;

	if(begin_sample<=front().pos)
		begin_sample=front().pos;
	*/

	const_iterator iter,next;

	/*
	//optimize...
	Real 	left = x-supersample/2, right = x+supersample/2;

	if(left < front().pos) left = front().pos;
	if(right > back().pos) right = back().pos;

	//find using binary search...
	const_iterator iterl,iterr;

	//the binary search should give us the values BEFORE the point we're looking for...
	iterl = binary_find(begin(),end(),left);
	iterr = binary_find(iterl,end(),right);

	//now integrate over the range of left to right...

	if(iterl == iterr)
	{
		iterr++; //let's look at the next one shall we :)

		//interpolate neighboring colors
		const Real one = iterr->pos - iterl->pos;
		const Real lambda = (x - iterl->pos)/one;

		//(1-l)iterl + (l)iterr
		return iterl->color.premult_alpha()*(1-lambda) + iterr->color.premult_alpha()*lambda;

		//return Color::blend(iterr->color,iterl->color,lambda,Color::BLEND_STRAIGHT);
	}else
	{
		//integration madness
		const_iterator i = iterl, ie = iterr+1;
		Real wlast = left;

		ColorAccumulator clast,cwork;
		{
			const Real lambda = (x - iterl->pos)/(iterr->pos - iterl->pos);

			//premultiply because that's the form in which we can combine things...
			clast = iterl->color.premult_alpha()*(1-lambda) + iterr->color.premult_alpha()*lambda;
			//Color::blend((i+1)->color,i->color,(left - i->pos)/((i+1)->pos - i->pos),Color::BLEND_STRAIGHT);
		}

		ColorAccumulator	accum = 0;

		//loop through all the trapezoids and integrate them as we go...
		//	area of trap = (yi + yi1)*(xi1 - xi)
		//	yi = clast, xi = wlast, yi1 = i->color, xi1 = i->pos

		for(;i<=iterr; wlast=i->pos,clast=i->color.premult_alpha(),++i)
		{
			const Real diff = i->pos - wlast;
			if(diff > 0) //only accumulate if there will be area to add
			{
				cwork = i->color.premult_alpha();
				accum += (cwork + clast)*diff;
			}
		}

		{
			const_iterator ibef = i-1;
			const Real diff = right - ibef->pos;

			if(diff > 0)
			{
				const Real lambda = diff/(i->pos - ibef->pos);
				cwork = ibef->color.premult_alpha()*(1-lambda) + i->color.premult_alpha()*lambda;

				accum += (cwork + clast)*diff; //can probably optimize this more... but it's not too bad
			}
		}

		accum /= supersample; //should be the total area it was sampled over...
		return accum.demult_alpha();
	}*/

	next=begin(),iter=next++;

	//add for optimization
	next = binary_find(begin(),end(),(Real)begin_sample);
	iter = next++;

	//! As a future optimization, this could be performed faster
	//! using a binary search.
	for(;iter<end();iter=next++)
	{
		if(next==end() || (x>=iter->pos && x<next->pos && iter->pos!=next->pos))
		{
			// If the supersample region falls square in between
			// two CPoints, then we don't have to do anything special.
			if(next!=end() && (!supersample || (iter->pos<=begin_sample && next->pos>=end_sample)))
			{
				const Real dist(next->pos-iter->pos);
				const Real pos(x-iter->pos);
				const Real amount(pos/dist);
				return Color::blend(next->color,iter->color, amount, Color::BLEND_STRAIGHT);
			}
			// In this case our supersample region extends over one or more
			// CPoints. So, we need to calculate our coverage amount.
			ColorAccumulator pool(Color::alpha());
			float divisor(0.0),weight(0);

			const_iterator iter2,next2;
			iter2=iter;
			if(iter==begin() && iter->pos>x)
			{
				weight=x-iter->pos;
				//weight*=iter->color.get_a();
				pool+=ColorAccumulator(iter->color).premult_alpha()*weight;
				divisor+=weight;
			}
			else
			{
				while(iter2->pos>=begin_sample)
				{
					if(iter2==begin())
					{
						weight=iter2->pos-(begin_sample);
						//weight*=iter2->color.get_a();
						pool+=ColorAccumulator(iter2->color).premult_alpha()*weight;
						divisor+=weight;
						break;
					}
					next2=iter2--;
					pool+=supersample_helper(*iter2, *next2, begin_sample, end_sample, weight);
					divisor+=weight;
				}
			}

			next2=iter;
			iter2=next2++;
			while(iter2->pos<=end_sample)
			{
				if(next2==end())
				{
					weight=(end_sample)-iter2->pos;
					pool+=ColorAccumulator(iter2->color).premult_alpha()*weight;
					divisor+=weight;
					break;
				}
				pool+=supersample_helper(*iter2, *next2, begin_sample, end_sample, weight);
				divisor+=weight;
				iter2=next2++;
			}

			if(divisor && pool.get_a() && pool.is_valid())
			{
/*
				pool.set_r(pool.get_r()/pool.get_a());
				pool.set_g(pool.get_g()/pool.get_a());
				pool.set_b(pool.get_b()/pool.get_a());
				pool.set_a(pool.get_a()/divisor);
*/
				pool/=divisor;
				pool.set_r(pool.get_r()/pool.get_a());
				pool.set_g(pool.get_g()/pool.get_a());
				pool.set_b(pool.get_b()/pool.get_a());
				if(pool.is_valid())
					return pool;
				else
					return Color::alpha();
			}
			else
				return Color::alpha();
		}
	}

	// We should never get to this point.

	synfig::error("synfig::Gradient::operator()(): Logic Error (x=%f)",x);
	assert(0);
	throw std::logic_error(strprintf("synfig::Gradient::operator()(): Logic Error (x=%f)",x));
}

synfig::Gradient::iterator
synfig::Gradient::proximity(const Real &x)
{
	iterator iter;
	float dist(100000000);
	float prev_pos(-0230);
	// This algorithm requires a sorted list.
	for(iter=begin();iter<end();iter++)
	{
		float new_dist;

		if(prev_pos==iter->pos)
			new_dist=(abs(x-iter->pos-0.00001));
		else
			new_dist=(abs(x-iter->pos));

		if(new_dist>dist)
		{
			iter--;
			return iter;
		}
		dist=new_dist;
		prev_pos=iter->pos;
	}
	iter--;
	return iter;
}

synfig::Gradient::const_iterator
synfig::Gradient::proximity(const Real &x)const
{
	return const_cast<Gradient*>(this)->proximity(x);
	/*
	const_iterator iter;
	float dist(100000000);

	// This algorithm requires a sorted list.
	for(iter=begin();iter<end();iter++)
	{
		const float new_dist(abs(x-iter->pos));
		if(new_dist>dist)
		{
			iter--;
			return iter;
		}
		dist=new_dist;
	}
	iter--;
	return iter;
	*/
}

synfig::Gradient::iterator
synfig::Gradient::find(const UniqueID &id)
{
	iterator iter;

	for(iter=begin();iter<end();iter++)
	{
		if(id==*iter)
			return iter;
	}

	throw Exception::NotFound("synfig::Gradient::find(): Unable to find UniqueID in gradient");
}

synfig::Gradient::const_iterator
synfig::Gradient::find(const UniqueID &id)const
{
	const_iterator iter;

	for(iter=begin();iter<end();iter++)
	{
		if(id==*iter)
			return iter;
	}

	throw Exception::NotFound("synfig::Gradient::find()const: Unable to find UniqueID in gradient");
}
