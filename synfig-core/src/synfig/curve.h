/* === S Y N F I G ========================================================= */
/*!	\file curve.h
**	\brief Operations with cubic curves
**
**	$Id$
**
**	\legal
**	......... ... 2019 Ivan Mahonin
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

#ifndef __SYNFIG_CURVE_H
#define __SYNFIG_CURVE_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include "real.h"
#include "vector.h"
#include "rect.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

int solve_equation(Real *roots, Real k0, Real k1);
int solve_equation(Real *roots, Real k0, Real k1, Real k2);
int solve_equation(Real *roots, Real k0, Real k1, Real k2, Real k3);
inline Real curve_diff_step() { return 0.01; }

class Bezier;
	
class Hermite {
public:
	Vector p0, p1, t0, t1;

	Hermite(const Vector &p0, const Vector &p1, const Vector &t0, const Vector &t1):
		p0(p0), p1(p1), t0(t0), t1(t1) { }
	Hermite(const Vector &p0, const Vector &p1, const Vector &t):
		Hermite(p0, p1, t, t) { }
	Hermite(const Vector &p0, const Vector &p1):
		Hermite(p0, p1, p1 - p0) { }
	explicit Hermite(const Vector &p = Vector()):
		Hermite(p, p, Vector()) { }

	bool operator< (const Hermite &other) const {
		return p0 < other.p0 ? true : other.p0 < p0 ? false
			 : p1 < other.p1 ? true : other.p1 < p1 ? false
			 : t0 < other.t0 ? true : other.t0 < t0 ? false
			 : t1 < other.t1;
	}
	bool operator==(const Hermite &other) const
		{ return p0 == other.p0 && p1 == other.p1 && t0 == other.t0 && t1 == other.t1; }
	bool operator!=(const Hermite &other) const
		{ return !(*this == other); }

	Vector operator() (Real l) const
		{ return p(l); }
	Vector p(Real l) const
		{ return p(l, p0, p1, t0, t1); }
	Vector t(Real l) const
		{ return t(l, p0, p1, t0, t1); }
	Real p(int index, Real l) const
		{ return p(l, p0[index], p1[index], t0[index], t1[index]); }
	Real t(int index, Real l) const
		{ return t(l, p0[index], p1[index], t0[index], t1[index]); }
	
	Vector d(Real l) const {
		const Real precision = real_low_precision<Real>();
		l = clamp(l, Real(0), Real(1));
		Vector tangent = t(l);
		Real magsqr = tangent.mag_squared();
		if (precision*precision < magsqr) return tangent/sqrt(magsqr);
		Real l0 = std::max(l - curve_diff_step(), Real(0));
		Real l1 = std::min(l + curve_diff_step(), Real(1));
		return (p(l1) - p(l0)).norm();
	}

	Vector pp0() const { return p0 + t0/3; }
	Vector pp1() const { return p1 - t1/3; }
	
	Hermite flip() const
		{ return Hermite(p1, p0, -t1, -t0); }
	
	Hermite sub(Real a, Real b) const {
		Real k = b - a;
		return Hermite(p(a), p(b), t(a)*k, t(b)*k);
	}
	
	void split(Real l, Hermite *a, Hermite *b) const {
		if (!a && !b) return;
		assert(b != a);
		Vector point = p(l);
		Vector tangent = t(l);
		Hermite h = *this;
		if (a) {
			a->p0 = h.p0;
			a->t0 = h.t0*l;
			a->p1 = point;
			a->t1 = tangent*l;
		}
		if (b && b != a) {
			Real ll = 1 - l;
			b->p0 = point;
			b->t0 = tangent*ll;
			b->p1 = h.p1;
			b->t1 = h.t1*ll;
		}
	}

	Rect bounds() const
		{ return Rect(p0).expand(p1).expand(pp0()).expand(pp1()); }

	int bends(int index, Real *l) const
		{ return bends(l, p0[index], p1[index], t0[index], t1[index]); }

	Range bounds_accurate(int index) const
		{ return bounds_accurate(p0[index], p1[index], t0[index], t1[index]); }
	Rect bounds_accurate() const {
		Range x = bounds_accurate(0);
		Range y = bounds_accurate(1);
		return Rect(x.min, y.min, x.max, y.max);
	}

	int inflection(int index, Real *l) const
		{ return inflection(l, p0[index], p1[index], t0[index], t1[index]); }

	int intersections(int index, Real *l, Real x) const
		{ return intersections(l, x, p0[index], p1[index], t0[index], t1[index]); }
	
	inline Bezier get_bezier() const;
	
	static Hermite linear(const Vector &p, const Vector &t)
		{ return Hermite(p, p + t, t, t);  }

	static Real p(Real l, Real p0, Real p1, Real t0, Real t1) {
		return p0*((( 2*l - 3)*l    )*l + 1)
			 + p1*(((-2*l + 3)*l    )*l    )
			 + t0*(((   l - 2)*l + 1)*l    )
			 + t1*(((   l - 1)*l    )*l    );
	}

	static Real t(Real l, Real p0, Real p1, Real t0, Real t1) {
		return p0*(( 6*l - 6)*l    )
			 + p1*((-6*l + 6)*l    )
			 + t0*(( 3*l - 4)*l + 1)
			 + t1*(( 3*l - 2)*l    );
	}

	static Vector p(Real l, const Vector &p0, const Vector &p1, const Vector &t0, const Vector &t1) {
		return Vector(
			p(l, p0[0], p1[0], t0[0], t1[0]),
			p(l, p0[1], p1[1], t0[1], t1[1]) );
	}
	static Vector t(Real l, const Vector &p0, const Vector &p1, const Vector &t0, const Vector &t1) {
		return Vector(
			t(l, p0[0], p1[0], t0[0], t1[0]),
			t(l, p0[1], p1[1], t0[1], t1[1]) );
	}
	
	static int inflection(Real *l, Real p0, Real p1, Real t0, Real t1);
	static int bends(Real *l, Real p0, Real p1, Real t0, Real t1);
	static int intersections(Real *l, Real p, Real p0, Real p1, Real t0, Real t1);
	static Range bounds_accurate(Real p0, Real p1, Real t0, Real t1);
};


class Bezier {
public:
	Vector p0, pp0, pp1, p1;

	Bezier(const Vector &p0, const Vector &p1, const Vector &pp0, const Vector &pp1):
		p0(p0), pp0(pp0), pp1(pp1), p1(p1) { }
	Bezier(const Vector &p0, const Vector &p1, const Vector &pp): // quadratic (conic) curve
		Bezier(p0, p1, p0 + (pp - p0)*(Real(2)/3), p1 + (pp - p1)*(Real(2)/3)) { }
	Bezier(const Vector &p0, const Vector &p1):
		Bezier(p0, p1, (p1 - p0)/3 + p0, (p0 - p1)/3 + p1) { }
	explicit Bezier(const Vector &p = Vector()):
		Bezier(p, p, p, p) { }

	bool operator< (const Bezier &other) const {
		return p0 < other.p0 ? true : other.p0 < p0 ? false
			 : p1 < other.p1 ? true : other.p1 < p1 ? false
			 : pp0 < other.pp0 ? true : other.pp0 < pp0 ? false
			 : pp1 < other.pp1;
	}
	bool operator==(const Bezier &other) const
		{ return p0 == other.p0 && p1 == other.p1 && pp0 == other.pp0 && pp1 == other.pp1; }
	bool operator!=(const Bezier &other) const
		{ return !(*this == other); }

	Vector operator() (Real l) const
		{ return p(l); }
	Vector p(Real l) const
		{ return p(l, p0, p1, pp0, pp1); }
	Vector t(Real l) const
		{ return t(l, p0, p1, pp0, pp1); }
	Real p(int index, Real l) const
		{ return p(l, p0[index], p1[index], pp0[index], pp1[index]); }
	Real t(int index, Real l) const
		{ return t(l, p0[index], p1[index], pp0[index], pp1[index]); }
	
	Vector d(Real l) const {
		const Real precision = real_low_precision<Real>();
		l = clamp(l, Real(0), Real(1));
		Vector tangent = t(l);
		Real magsqr = tangent.mag_squared();
		if (precision*precision < magsqr) return tangent/sqrt(magsqr);
		Real l0 = std::max(l - curve_diff_step(), Real(0));
		Real l1 = std::min(l + curve_diff_step(), Real(1));
		return (p(l1) - p(l0)).norm();
	}

	Vector t0() const { return (pp0 - p0)*3; }
	Vector t1() const { return (p1 - pp1)*3; }
	
	void split(Real l, Bezier *a, Bezier *b) const {
		if (!a && !b) return;
		assert(b != a);
		Real ll = 1 - l;
		Vector p0 = this->p0;
		Vector p1 = this->p1;
		Vector a0 =  p0*ll + pp0*l;
		Vector a1 = pp0*ll + pp1*l;
		Vector a2 = pp1*ll +  p1*l;
		Vector b0 =  a0*ll +  a1*l;
		Vector b1 =  a1*ll +  a2*l;
		Vector c  =  b0*ll +  b1*l;
		if (a) {
			a->p0  = p0;
			a->pp0 = a0;
			a->pp1 = b0;
			a->p1  = c;
		}
		if (b && b != a) {
			b->p0  = c;
			b->pp0 = b1;
			b->pp1 = a2;
			b->p1  = p1;
		}
	}
	
	Bezier flip() const
		{ return Bezier(p1, p0, pp1, pp0); }

	Bezier sub(Real a, Real b) const {
		if (approximate_equal(a, b)) return Bezier( p(0.5*(a + b)) );

		Bezier s;
		if (approximate_equal(a, Real(1))) {
			split(b, nullptr, &s);
			return s.flip();
		}
		
		split(a, nullptr, &s);
		s.split((b - a)/(1 - a), &s, nullptr);
		return s;
	}
	
	Rect bounds() const
		{ return Rect(p0).expand(p1).expand(pp0).expand(pp1); }

	int bends(int index, Real *l) const
		{ return bends(l, p0[index], p1[index], pp0[index], pp1[index]); }

	Range bounds_accurate(int index) const
		{ return bounds_accurate(p0[index], p1[index], pp0[index], pp1[index]); }
	Rect bounds_accurate() const {
		Range x = bounds_accurate(0);
		Range y = bounds_accurate(1);
		return Rect(x.min, y.min, x.max, y.max);
	}

	int inflection(int index, Real *l) const
		{ return inflection(l, p0[index], p1[index], pp0[index], pp1[index]); }

	int intersections(int index, Real *l, Real x) const
		{ return intersections(l, x, p0[index], p1[index], pp0[index], pp1[index]); }

	inline Hermite get_hermite() const;

	static Bezier linear(const Vector &p0, const Vector &t)
		{ return Bezier(p0, p0 + t);  }

	static Real p(Real l, Real p0, Real p1, Real pp0, Real pp1) {
		Real ll = 1 - l;
		Real a0 =  p0*ll + pp0*l;
		Real a1 = pp0*ll + pp1*l;
		Real a2 = pp1*ll +  p1*l;
		Real b0 =  a0*ll +  a1*l;
		Real b1 =  a1*ll +  a2*l;
		return b0*ll +  b1*l;
	}

	static Real t(Real l, Real p0, Real p1, Real pp0, Real pp1) {
		Real ll = 1 - l;
		Real a0 =  p0*ll + pp0*l;
		Real a1 = pp0*ll + pp1*l;
		Real a2 = pp1*ll +  p1*l;
		Real b0 =  a0*ll +  a1*l;
		Real b1 =  a1*ll +  a2*l;
		return 3*(b1 - b0)*l;
	}

	static Vector p(Real l, const Vector &p0, const Vector &p1, const Vector &pp0, const Vector &pp1) {
		return Vector(
			p(l, p0[0], p1[0], pp0[0], pp1[0]),
			p(l, p0[1], p1[1], pp0[1], pp1[1]) );
	}
	static Vector t(Real l, const Vector &p0, const Vector &p1, const Vector &pp0, const Vector &pp1) {
		return Vector(
			t(l, p0[0], p1[0], pp0[0], pp1[0]),
			t(l, p0[1], p1[1], pp0[1], pp1[1]) );
	}
	
	static int inflection(Real *l, Real p0, Real p1, Real pp0, Real pp1);
	static int bends(Real *l, Real p0, Real p1, Real pp0, Real pp1);
	static int intersections(Real *l, Real p, Real p0, Real p1, Real pp0, Real pp1);
	static Range bounds_accurate(Real p0, Real p1, Real pp0, Real pp1);
};


inline Bezier Hermite::get_bezier() const { return Bezier(p0, p1, pp0(), pp1()); }
inline Hermite Bezier::get_hermite() const { return Hermite(p0, p1, t0(), t1()); }


}; // END of namespace synfig

#endif

