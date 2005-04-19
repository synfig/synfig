/* === S Y N F I G ========================================================= */
/*!	\file gradient.cpp
**	\brief Color Gradient Class Member Definitions
**
**	$Id: gradient.cpp,v 1.2 2005/01/21 19:29:10 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "gradient.h"
#include "general.h"
#include <stdexcept>
#include "exception.h"

#include <ETL/misc>
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
		ColorAccumulator ret=Color::blend(color2.color,color1.color, 0.5, Color::BLEND_STRAIGHT);
		ret.set_r(ret.get_r()*ret.get_a());
		ret.set_g(ret.get_g()*ret.get_a());
		ret.set_b(ret.get_b()*ret.get_a());
		return ret*weight;
	}
	if(color1.pos>=begin && color2.pos>=end)
	{
		weight=end-color1.pos;
		float pos((end+color1.pos)*0.5);
		float amount((pos-color1.pos)/(color2.pos-color1.pos));
		//if(abs(amount)>1)amount=(amount>0)?1:-1;
		ColorAccumulator ret(Color::blend(color2.color,color1.color, amount, Color::BLEND_STRAIGHT));
		ret.set_r(ret.get_r()*ret.get_a());
		ret.set_g(ret.get_g()*ret.get_a());
		ret.set_b(ret.get_b()*ret.get_a());
		return ret*weight;
	}
	if(color1.pos<begin && color2.pos<end)
	{
		weight=color2.pos-begin;
		float pos((begin+color2.pos)*0.5);
		float amount((pos-color1.pos)/(color2.pos-color1.pos));
		//if(abs(amount)>1)amount=(amount>0)?1:-1;
		ColorAccumulator ret(Color::blend(color2.color,color1.color, amount, Color::BLEND_STRAIGHT));
		ret.set_r(ret.get_r()*ret.get_a());
		ret.set_g(ret.get_g()*ret.get_a());
		ret.set_b(ret.get_b()*ret.get_a());
		return ret*weight;
	}
	synfig::error("color1.pos=%f",color1.pos);
	synfig::error("color2.pos=%f",color2.pos);
	synfig::error("begin=%f",begin);
	synfig::error("end=%f",end);

	weight=0;
	return Color::alpha();
	
//	assert(0);
}
	
Color
synfig::Gradient::operator()(const Real &x,float supersample)const
{
	if(empty())
		return Color(0,0,0,0);
	if(supersample<0)
		supersample=-supersample;
	if(supersample>2.0)
		supersample=2.0f;
	
	float begin_sample(x-supersample*0.5);
	float end_sample(x+supersample*0.5);

	if(size()==1 || end_sample<=front().pos || isnan(x))
		return front().color;
	
	if(begin_sample>=back().pos)
		return back().color;

	/*
	if(end_sample>=back().pos)
		end_sample=back().pos;

	if(begin_sample<=front().pos)
		begin_sample=front().pos;
	*/
	
	const_iterator iter,next;

	/*
	//optimizize...
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
		//itegration madness
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
		if(next==end() || x>=iter->pos &&  x<next->pos && iter->pos!=next->pos)
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
				pool+=ColorAccumulator(iter->color)*(float)iter->color.get_a()*weight;
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
						pool+=ColorAccumulator(iter2->color)*(float)iter2->color.get_a()*weight;
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
					pool+=ColorAccumulator(iter2->color)*(float)iter2->color.get_a()*weight;
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
