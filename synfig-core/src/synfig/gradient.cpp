/* === S Y N F I G ========================================================= */
/*!	\file gradient.cpp
**	\brief Color Gradient Class Member Definitions
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	......... ... 2018 Ivan Mahonin
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
#include <synfig/real.h>

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

Gradient::Gradient(const Color &c1, const Color &c2)
{
	push_back(CPoint(0.0,c1));
	push_back(CPoint(1.0,c2));
}

Gradient::Gradient(const Color &c1, const Color &c2, const Color &c3)
{
	push_back(CPoint(0.0,c1));
	push_back(CPoint(0.5,c2));
	push_back(CPoint(1.0,c3));
}

// both input gradients should be already sorted
Gradient
Gradient::operator+(const Gradient &rhs) const
{
	if (empty()) return rhs;
	if (rhs.empty()) return *this;

	Gradient g;

	if (size() == 1) {
		g.cpoints.reserve(rhs.size());
		const Color &c = begin()->color;
		for(const_iterator i = rhs.begin(); i != rhs.end(); ++i)
			{ g.cpoints.push_back(*i); g.cpoints.back().color += c; }
		return g;
	}

	if (rhs.size() == 1) {
		g.cpoints.reserve(size());
		const Color &c = rhs.begin()->color;
		for(const_iterator i = begin(); i != end(); ++i)
			{ g.cpoints.push_back(*i); g.cpoints.back().color += c; }
		return g;
	}

	for(const_iterator i = begin(), j = rhs.begin(), pi = i, pj = j; i != end() || j != rhs.end();) {

		// select point to insert and select interval to calculate color value

		const_iterator current, left, right;
		if (i == end())
			current = pj = j++, left = right = pi;
		else
		if (j == end())
			current = pi = i++, left = right = pj;
		else
		if (i->pos < j->pos)
			current = pi = i++, left = pj, right = j;
		else
		if (j->pos < i->pos)
			current = pj = j++, left = pi, right = i;
		else
			current = pi = i++, left = right = pj = j++;

		// calculate color

		Color color = current->color;
		if (current->pos <= left->pos) {
			color += left->color;
		} else
		if (current->pos >= right->pos) {
			color += right->color;
		} else {
			Real d = right->pos - left->pos;
			if (d <= real_high_precision<Real>())
				continue; // left and right points are so close, so we will not add point between
			// operator()() also contains the same calculations
			ColorReal amount = (current->pos - left->pos)/d;
			color += Color::blend(left->color, right->color, amount, Color::BLEND_STRAIGHT);
		}

		// add point

		if (!g.empty()) {
			CPoint &prev = *(g.end()-1);
			if (current->pos - prev.pos <= real_precision<Real>()) {
				// if we already have point with same position and color then skip
				const Color &ca = prev.color;
				const Color &cb = current->color;
				if ( approximate_equal_lp(ca.get_r(), cb.get_r())
				  && approximate_equal_lp(ca.get_g(), cb.get_g())
				  && approximate_equal_lp(ca.get_b(), cb.get_b())
				  && approximate_equal_lp(ca.get_a(), cb.get_a()) ) continue;

				// instead of add third point to the same position, just replace the last point
				if (g.size() > 1) {
					CPoint &prev_prev = *(g.end()-2);
					if (current->pos - prev_prev.pos <= real_precision<Real>()) {
						prev.pos = current->pos;
						prev.color = current->color;
						continue;
					}
				}
			}
		}

		g.push_back(GradientCPoint(current->pos, color));
	}

	return g;
}

Gradient &
Gradient::operator*=(const ColorReal &rhs)
{
	if (rhs == 0) cpoints.clear(); else
		for (iterator iter = cpoints.begin(); iter!=cpoints.end(); iter++)
			(*iter).color *= rhs;
	return *this;
}

//! Returns color of point x
Color
Gradient::operator() (const Real &x) const
{
	if (cpoints.empty())
		return Color();
	if (cpoints.size() == 1 || std::isnan(x))
		return cpoints.front().color;

	// point is outside of gradient
	if (x <= cpoints.front().pos)
		return cpoints.front().color;
	if (x >= cpoints.back().pos)
		return cpoints.back().color;

	Gradient::const_iterator i = std::upper_bound(begin(), end()-1, x);
	Gradient::const_iterator j = i--;

	Real d = j->pos - i->pos;
	if (d <= real_high_precision<Real>()) return i->color;

	// operator+() also contains the same calculations
	ColorReal amount = (x - i->pos)/d;
	return Color::blend(i->color, j->color, amount, Color::BLEND_STRAIGHT);
}

Real
Gradient::mag() const
{
	if (empty()) return 0.0;
	if (size() == 1) return cpoints.front().color.get_y();

	Real v0 = cpoints.front().color.get_y() * cpoints.front().pos;
	Real v1 = cpoints.back().color.get_y() * (1.0 - cpoints.back().pos);
	Real sum = v0*v0 + v1*v1;
	for(const_iterator i0 = begin(), i1 = i0 + 1; i1 != end(); i0 = i1++) {
		Real v = 0.5*(i0->color.get_y() + i1->color.get_y())*(i1->pos - i0->pos);
		sum += v*v;
	}
	return sqrt(sum);
}

Gradient::iterator
Gradient::proximity(const Real &x)
{
	// This algorithm requires a sorted list.
	iterator iter;
	Real dist(100000000);
	Real prev_pos(-0230);
	for(iter=begin();iter<end();iter++)
	{
		Real new_dist;

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

Gradient::iterator
Gradient::find(const UniqueID &id)
{
	// TODO: return end() instead of exception
	for(iterator i = begin(); i < end(); i++)
		if (id == *i) return i;
	return end();
}


CompiledGradient::Entry::Entry(const Accumulator &prev_sum, const GradientCPoint &prev, const GradientCPoint &next):
	prev_pos(prev.pos),
	next_pos(next.pos),
	prev_sum(prev_sum),
	prev_color(prev.color),
	next_color(next.color)
{
	Real dp = next_pos - prev_pos;
	if (dp <= real_precision<Real>()) {
		next_sum = prev_sum;
	} else {
		prev_k1 = (next_color - prev_color)/dp;
		prev_k2 = prev_k1*0.5;
		next_sum = prev_sum + (next_color + prev_color)*(0.5*dp);
	}
}


CompiledGradient::CompiledGradient():
	is_empty(true), repeat()
	{ reset(); };

CompiledGradient::CompiledGradient(const Color &color):
	is_empty(true), repeat()
	{ set(color); };

CompiledGradient::CompiledGradient(const Gradient &gradient, bool repeat, bool zigzag):
	is_empty(true), repeat()
	{ set(gradient, repeat, zigzag); };

void
CompiledGradient::set(const Color &color)
{
	is_empty = color == Color();
	repeat = false;
	summary_color = Accumulator(color);
	list.clear();
	list.push_back(Entry());
	list.front().prev_color = list.front().next_color = summary_color;
}

void
CompiledGradient::set(const Gradient &gradient, bool repeat, bool zigzag) {
	if (gradient.empty())
		{ reset(); return; }
	if (gradient.size() == 1)
		{ set(gradient.begin()->color); return; }

	// sort points and clamp pos to [0, 1]
	Gradient::CPointList cpoints;
	cpoints.reserve(zigzag ? gradient.size()*2 : gradient.size());
	for(Gradient::const_iterator i = gradient.begin(); i != gradient.end(); ++i) {
		Real pos = std::max(0.0, std::min(1.0, i->pos));
		cpoints.insert( std::upper_bound(cpoints.begin(), cpoints.end(), pos), *i )->pos = pos;
	}

	// add flipped points for zigzag
	// memory for points already reserved enough, so all iterators will stay valid
	if (zigzag) {
		// add mirror in order:
		//    0 1 2 3 4
		//    0 1 2 3 4 - 4 3 2 1 0
		for(Gradient::reverse_iterator ri = cpoints.rbegin(); ri != cpoints.rend(); ++ri) {
			ri->pos *= 0.5;
			cpoints.push_back(*ri);
			cpoints.back().pos = 1.0 - cpoints.back().pos;
		}
	}

	list.clear();
	this->repeat = repeat;
	Accumulator prev_sum = Accumulator(cpoints.front().color)*cpoints.front().pos;
	for(Gradient::iterator i = cpoints.begin(), j = i + 1; j != cpoints.end();  ++i, ++j) {
		list.push_back( Entry(prev_sum, *i, *j) );
		prev_sum = list.back().next_sum;
	}

	// debug output, uncomment this when you does not know that's happening
	//synfig::info("gradient cpoints:");
	//for(Gradient::iterator i = cpoints.begin(); i != cpoints.end();  ++i)
	//	synfig::info("  %f, (%f, %f, %f, %f)", i->pos, i->color.get_r(), i->color.get_g(), i->color.get_b(), i->color.get_a());
	//synfig::info("compiled gradient:");
	//for(List::const_iterator i = list.begin(); i != list.end(); ++i) {
	//	synfig::info("  -- prev_k: %f, %f", i->prev_k1, i->prev_k2);
	//	synfig::info("     prev: %f, (%f, %f, %f, %f), (%f, %f, %f, %f)",
	//		i->prev_pos, i->prev_color.r, i->prev_color.g, i->prev_color.b, i->prev_color.a,
	//		i->prev_sum.r, i->prev_sum.g, i->prev_sum.b, i->prev_sum.a );
	//	synfig::info("     next: %f, (%f, %f, %f, %f), (%f, %f, %f, %f)",
	//		i->next_pos, i->next_color.r, i->next_color.g, i->next_color.b, i->next_color.a,
	//		i->next_sum.r, i->next_sum.g, i->next_sum.b, i->next_sum.a );
	//}
	
	summary_color = find(1.0)->summary(1.0);
}
