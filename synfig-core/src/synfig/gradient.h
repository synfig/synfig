/* === S Y N F I G ========================================================= */
/*!	\file gradient.h
**	\brief Color Gradient Class
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GRADIENT_H
#define __SYNFIG_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <algorithm>

#include "real.h"
#include "color.h"
#include "uniqueid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! \struct GradientCPoint
//! \brief Gradient color point
struct GradientCPoint : public UniqueID
{
	Real pos;
	Color color;

	bool operator< (const GradientCPoint &rhs) const
		{ return pos < rhs.pos; }
	bool operator< (const Real &rhs) const
		{ return pos < rhs; }

	GradientCPoint(): pos() { }
	GradientCPoint(const Real &pos, const Color &color):pos(pos),color(color) { }
}; // END of class GradientCPoint

// for use in std::upper_bound, std::lower_bound, etc
// must be inline to avoid 'multiple definition' linker error
inline bool operator<(const Real &a, const GradientCPoint &b)
	{ return a < b.pos; }

//! \class Gradient
//! \brief Color Gradient Class
class Gradient
{
public:
	typedef GradientCPoint CPoint;
	typedef std::vector<CPoint> CPointList;
	typedef CPointList::const_iterator			const_iterator;
	typedef CPointList::iterator				iterator;
	typedef CPointList::const_reverse_iterator	const_reverse_iterator;
	typedef CPointList::reverse_iterator		reverse_iterator;

private:
	CPointList cpoints;

public:
	Gradient() { }

	//! Two-Tone Color Gradient Convenience Constructor
	Gradient(const Color &c1, const Color &c2);

	//! Three-Tone Color Gradient Convenience Constructor
	Gradient(const Color &c1, const Color &c2, const Color &c3);

	//! You should call this function after changing stuff.
	void sort() { stable_sort(begin(), end()); }

	//! Alias for sort (Implemented for consistency)
	void sync() { sort(); }

	void push_back(const CPoint cpoint) { cpoints.push_back(cpoint); }
	iterator erase(iterator iter) { return cpoints.erase(iter); }
	bool empty()const { return cpoints.empty(); }
	size_t size()const { return cpoints.size(); }

	iterator begin() { return cpoints.begin(); }
	iterator end() { return cpoints.end(); }
	reverse_iterator rbegin() { return cpoints.rbegin(); }
	reverse_iterator rend() { return cpoints.rend(); }
	const_iterator begin()const { return cpoints.begin(); }
	const_iterator end()const { return cpoints.end(); }
	const_reverse_iterator rbegin()const { return cpoints.rbegin(); }
	const_reverse_iterator rend()const { return cpoints.rend(); }

	Gradient &operator+=(const Gradient  &rhs) { return *this = *this + rhs; }
	Gradient &operator*=(const ColorReal &rhs);
	Gradient &operator-=(const Gradient  &rhs) { return *this = *this + rhs*ColorReal(-1); }
	Gradient &operator/=(const ColorReal &rhs) { return *this *= ColorReal(1)/rhs; }

	Gradient operator+(const Gradient  &rhs) const;
	Gradient operator-(const Gradient  &rhs) const { return *this + rhs*ColorReal(-1); }
	Gradient operator*(const ColorReal &rhs) const { return Gradient(*this)*=rhs; }
	Gradient operator/(const ColorReal &rhs) const { return Gradient(*this)/=rhs; }

	Color operator() (const Real &x) const;

	//! Returns average luminance of gradient
	Real mag() const;

	//! Returns the iterator of the CPoint closest to \a x
	iterator proximity(const Real &x);
	const_iterator proximity(const Real &x)const
		{ return const_cast<Gradient*>(this)->proximity(x); }

	iterator find(const UniqueID &id);
	const_iterator find(const UniqueID &id)const
		{ return const_cast<Gradient*>(this)->find(id); }
}; // END of class Gradient


//! \class CompiledGradient
//! \brief Compiled gradient can quickly calculate color of specified color
//!        and average color of specified range
class CompiledGradient {
public:
	// High precision color accumulator, alpha premulted
	class Accumulator {
	public:
		union {
			struct { Real r, g, b, a; };
			struct { Real values[4]; };
		};

		Accumulator():
			r(), g(), b(), a() { }
		Accumulator(Real r, Real g, Real b, Real a):
			r(r), g(g), b(b), a(a) { }

		Accumulator(const Color &color)
		{
			// premult alpha
			a = (Real)color.get_a();
			r = a*(Real)color.get_r();
			g = a*(Real)color.get_g();
			b = a*(Real)color.get_b();
		}

		Color color() const
		{
			// demult alpha
			if (approximate_equal_lp(a, Real(0))) return Color();
			Real k = 1.0/a;
			return Color((ColorReal)(k*r), (ColorReal)(k*g), (ColorReal)(k*b), (ColorReal)a);
		}

		bool operator== (const Accumulator &x) const
			{ return r == x.r && g == x.g && b == x.b && a == x.a; }

		Accumulator operator+ (const Accumulator &x) const
			{ return Accumulator( r+x.r, g+x.g, b+x.b, a+x.a ); }
		Accumulator operator- (const Accumulator &x) const
			{ return Accumulator( r-x.r, g-x.g, b-x.b, a-x.a ); }
		Accumulator operator- () const
			{ return Accumulator( -r, -g, -b, -a ); }
		Accumulator operator* (const Accumulator &x) const
			{ return Accumulator( r*x.r, g*x.g, b*x.b, a*x.a ); }
		Accumulator operator* (Real x) const
			{ return Accumulator( r*x, g*x, b*x, a*x ); }
		Accumulator operator/ (Real x) const
			{ return *this * (1.0/x); }

		Accumulator& operator+= (const Accumulator &x)
			{ r+=x.r; g+=x.g; b+=x.b; a+=x.a; return *this; }
		Accumulator& operator-= (const Accumulator &x)
			{ r-=x.r; g-=x.g; b-=x.b; a-=x.a; return *this; }
		Accumulator& operator*= (const Accumulator &x)
			{ r*=x.r; g*=x.g; b*=x.b; a*=x.a; return *this; }
		Accumulator& operator*= (Real x)
			{ r*=x; g*=x; b*=x; a*=x; return *this; }
		Accumulator& operator/= (Real x)
			{ return *this *= (1.0/x); }
	};

	// One segment
	class Entry {
	public:
		Real prev_pos;
		Real next_pos;

		Accumulator prev_sum;
		Accumulator prev_color;
		Accumulator prev_k1;  // (next_color - prev_color)/(next_pos - prev_pos)
		Accumulator prev_k2;  // for calculation summary: 0.5 * prev_k1

		Accumulator next_sum; // prev_sum + 0.5*(next_color + prev_color)*(next_pos - prev_pos);
		Accumulator next_color;

		Entry(): prev_pos(), next_pos() { }

		Entry(const Accumulator &prev_sum, const GradientCPoint &prev, const GradientCPoint &next);

		inline bool operator< (const Real &x) const
			{ return next_pos < x; } // to easy search by std::upper_bound and std::lower_bound

		inline Color color(Real x) const {
			return x >= next_pos ? next_color.color()
				 : x <= prev_pos ? prev_color.color()
				 : (prev_color + prev_k1*(x - prev_pos)).color();
		}

		inline Accumulator summary(Real x) const {
			if (x >= next_pos) return next_sum + next_color*(x - next_pos);
			if (x <= prev_pos) return prev_sum + prev_color*(x - prev_pos);
			x -= prev_pos;
			return prev_sum + prev_color*x + prev_k2*(x*x);
		}
	};

	typedef std::vector<Entry> List;

private:
	bool is_empty;
	bool repeat;
	List list;

	Accumulator summary_color;

public:
	CompiledGradient();
	explicit CompiledGradient(const Color &color);
	explicit CompiledGradient(const Gradient &gradient, bool repeat = false, bool zigzag = false);

	void set(const Color &color);
	void set(const Gradient &gradient, bool repeat = false, bool zigzag = false);
	void reset() { set(Color()); }

	bool empty() const { return is_empty; }
	bool get_repeat() const { return repeat; }
	const List& get_list() const { return list; }

	inline List::const_iterator find(Real x) const
		{ return std::lower_bound(list.begin(), list.end()-1, x); }

	inline Color color(Real x) const {
		if (repeat) x -= floor(x);
		return find(x)->color(x);
	}

	inline Accumulator summary() const
		{ return summary_color; }

	inline Accumulator summary(Real x) const {
		if (repeat) {
			Real count = floor(x);
			x -= count;
			return summary_color*count + find(x)->summary(x);
		}
		return find(x)->summary(x);
	}

	inline Color average() const
		{ return summary_color.color(); }

	inline Color average(Real x0, Real x1) const
	{
		Real w = x1 - x0;
		if (std::isnan(w) || std::isinf(w)) return average();
		if (fabs(w) < real_precision<Real>()) return color(x0);
		return ((summary(x1) - summary(x0))/w).color();
	}
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
