/* === S Y N F I G ========================================================= */
/*!	\file curve.cpp
**	\brief Operations with cubic curves
**
**	$Id$
**
**	\legal
**  ......... ... 2019 Ivan Mahonin
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


#include <cmath>

#include "curve.h"


#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int
synfig::solve_equation(Real* roots, Real k0, Real k1)
{
	if (approximate_zero(k1)) return 0;
	if (roots) roots[0] = -k0/k1;
	return 1;
}

int
synfig::solve_equation(Real* roots, Real k0, Real k1, Real k2)
{
	if (approximate_zero(k2)) return solve_equation(roots, k0, k1);
	Real D = k1*k1 - 4*k2*k0;
	if (approximate_zero(D)) {
		if (roots) roots[0] = -0.5*k1/k2;
		return 1;
	} else
	if (D > 0) {
		if (roots) {
			Real a = sqrt(D);
			Real b = -0.5/k2;
			roots[0] = (k1 - a)*b;
			roots[1] = (k1 + a)*b;
		}
		return 2;
	}
	return 0;
}

int
synfig::solve_equation(Real* roots, Real k0, Real k1, Real k2, Real k3)
{
	if (approximate_zero(k3)) return solve_equation(roots, k0, k1, k2);

	Real k = 1/k3;
	Real a = k2*k;
	Real b = k1*k;
	Real c = k0*k;
	
	Real Q = (a*a - 3*b)/9;
	Real Q3 = Q*Q*Q;
	Real R = (2*a*a*a - 9*a*b + 27*c)/54;
	Real S = Q3 - R*R;
	
	if (approximate_zero(S)) {
		if (roots) {
			Real rr = cbrt(R);
			Real aa = -a/3;
			roots[0] = aa - 2*rr;
			roots[1] = aa + rr;
		}
		return 2;
	} else
	if (S > 0) {
		if (roots) {
			Real ph = acos(R/sqrt(Q3))/3;
			Real qq = -2*sqrt(Q);
			Real aa = -a/3;
			roots[0] = qq*cos(ph) + aa;
			roots[1] = qq*cos(ph + 2*PI/3) + aa;
			roots[2] = qq*cos(ph - 2*PI/3) + aa;
		}
		return 3;
	} else
	if (approximate_zero(Q)) {
		if (roots) roots[0] = -cbrt(c - a*a*a/27) - a/3;
		return 1;
	} else
	if (Q > 0) {
		if (roots) {
			Real ph = acosh( fabs(R)/sqrt(Q3) )/3;
			Real sign = approximate_zero(R) ? 0 : R < 0 ? -1 : 1;
			roots[0] = -2*sign*sqrt(Q)*cosh(ph) - a/3;
		}
		return 1;
	} else {
		if (roots) {
			Real ph = asinh( fabs(R)/sqrt(-Q3) )/3;
			Real sign = approximate_zero(R) ? 0 : R < 0 ? -1 : 1;
			roots[0] = -2*sign*sqrt(-Q)*sinh(ph) - a/3;
		}
		return 1;
	}
	return 0;
}

/* === M E T H O D S ======================================================= */


// Hermite

int Hermite::inflection(Real *l, Real p0, Real p1, Real t0, Real t1) {
	Real root;
	if (solve_equation(
		&root,
		-3*p0 + 3*p1 - 2*t0 -   t1,
		 6*p0 - 6*p1 + 3*t0 + 3*t1 ))
	{
		if (approximate_less(Real(0), root) && approximate_less(root, Real(1))) {
			if (l) *l = root;
			return 1;
		}
	}
	return 0;
}

int Hermite::bends(Real *l, Real p0, Real p1, Real t0, Real t1) {
	Real roots[2];
	int count = solve_equation(
		roots,
		                 t0       ,
		-6*p0 + 6*p1 - 4*t0 - 2*t1,
		 6*p0 - 6*p1 + 3*t0 + 3*t1 );
	int valid_count = 0;	
	for(Real *i = roots, *end = i + count; i != end; ++i)
		if (approximate_less(Real(0), *i) && approximate_less(*i, Real(1))) {
			if (l) l[valid_count] = *i;
			++valid_count;
		}
	return valid_count;
}

Range Hermite::bounds_accurate(Real p0, Real p1, Real t0, Real t1) {
	Range range(p0);
	range.expand(p1);
	Real roots[2];
	int count = bends(roots, p0, p1, t0, t1);
	for(Real *i = roots, *end = i + count; i != end; ++i)
		range.expand( p(*i, p0, p1, t0, t1) );
	return range;
}

int Hermite::intersections(Real *l, Real p, Real p0, Real p1, Real t0, Real t1) {
	Real roots[3];
	int count = solve_equation(
		roots,
		   p0 - p               ,
		                 t0     ,
		-3*p0 + 3*p1 - 2*t0 - t1,
		 2*p0 - 2*p1 +   t0 + t1 );
	int valid_count = 0;
	for(Real *i = roots, *end = i + count; i != end; ++i)
		if (approximate_less(Real(0), *i) && approximate_less(*i, Real(1))) {
			if (l) l[valid_count] = *i;
			++valid_count;
		}
	return valid_count;
}


// Bezier

int Bezier::inflection(Real *l, Real p0, Real p1, Real pp0, Real pp1) {
	Real root;
	if (solve_equation(
		&root,
		 p0 - 2*pp0 +   pp1     ,
		-p0 + 3*pp0 - 3*pp1 + p1 ))
	{
		if (approximate_less(Real(0), root) && approximate_less(root, Real(1))) {
			if (l) *l = root;
			return 1;
		}
	}
	return 0;
}

int Bezier::bends(Real *l, Real p0, Real p1, Real pp0, Real pp1) {
	Real roots[2];
	int count = solve_equation(
		roots,
		 -p0 +   pp0             ,
		2*p0 - 4*pp0 + 2*pp1     ,
		 -p0 + 3*pp0 - 3*pp1 + p1 );
	int valid_count = 0;	
	for(Real *i = roots, *end = i + count; i != end; ++i)
		if (approximate_less(Real(0), *i) && approximate_less(*i, Real(1))) {
			if (l) l[valid_count] = *i;
			++valid_count;
		}
	return valid_count;
}

Range Bezier::bounds_accurate(Real p0, Real p1, Real pp0, Real pp1) {
	Range range(p0);
	range.expand(p1);
	Real roots[2];
	int count = bends(roots, p0, p1, pp0, pp1);
	for(Real *i = roots, *end = i + count; i != end; ++i)
		range.expand( p(*i, p0, p1, pp0, pp1) );
	return range;
}

int Bezier::intersections(Real *l, Real p, Real p0, Real p1, Real pp0, Real pp1) {
	Real roots[3];
	int count = solve_equation(
		roots,
		   p0 - p                 ,
		-3*p0 + 3*pp0             ,
		 3*p0 - 6*pp0 + 3*pp1     ,
		  -p0 + 3*pp0 - 3*pp1 + p1 );
	int valid_count = 0;
	for(Real *i = roots, *end = i + count; i != end; ++i)
		if (approximate_less(Real(0), *i) && approximate_less(*i, Real(1))) {
			if (l) l[valid_count] = *i;
			++valid_count;
		}
	return valid_count;
}

