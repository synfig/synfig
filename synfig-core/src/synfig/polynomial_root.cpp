/* === S Y N F I G ========================================================= */
/*!	\file polynomial_root.cpp
**	\brief Template File
**
**	$Id$
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "polynomial_root.h"
#include <complex>

#endif

/* === U S I N G =========================================================== */


/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
typedef std::complex<float>	Complex;
// MacOS only overloads std::abs for std::complex

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

#define EPSS 1.0e-7
#define MR 8
#define MT 10
#define MAXIT (MT*MR)

/*EPSS is the estimated fractional roundoff error.  We try to break (rare) limit
cycles with MR different fractional values, once every MT steps, for MAXIT total allowed iterations.
*/

/*	Explanation:

	A polynomial can be represented like so:
	Pn(x) = (x - x1)(x - x2)...(x - xn)	where xi = complex roots

	We can get the following:
	ln|Pn(x)| = ln|x - x1| + ln|x - x2| + ... + ln|x - xn|

	G := d ln|Pn(x)| / dx =
	+1/(x-x1) + 1/(x-x2) + ... + 1/(x-xn)

	and

	H := - d2 ln|Pn(x)| / d2x =
	+1/(x-x1)^2 + 1/(x-x2)^2 + ... + 1/(x-xn)^2

	which gives
	H = [Pn'/Pn]^2 - Pn''/Pn

	Laguerre's formula guesses that the root we are seeking x1 is located
	some distance a from our current guess x, and all the other roots are
	located at distance b.

	Using this:

	1/a + (n-1)/b = G

	and

	1/a^2 + (n-1)/b^2 = H

	which yields this solution for a:

	a = n / G +- sqrt( (n-1)(nH - G^2) )

	where +- is determined by which ever yields the largest magnitude for the denominator.
	a can easily be complex since the factor inside the square-root can be negative.

	This method iterates (x=x-a) until a is sufficiently small.
*/

/* Given the degree m and the m+1 complex coefficients a[0..m] of the polynomial sum(i=0,m){a[i]x^i},
and given a complex value x, this routine improves x by laguerre's method until it converges,
within the achievable roundoff limit, to a root of the given polynomial.  The number of iterations taken
is returned as `its'.
*/
void laguer(Complex a[], int m, Complex *x, int *its)
{
	int iter,j;
	float abx, abp, abm, err;
	Complex dx,x1,b,d,f,g,h,sq,gp,gm,g2;

	//Fractions used to break a limit cycle
	static float frac[MR+1] = {0.0,0.5,0.25,0.75,0.13,0.38,0.62,0.88,1.0};

	for(iter = 1; iter <= MAXIT; ++iter)
	{
		*its = iter; //number of iterations so far

		b 	= a[m]; 	//the highest coefficient
		err	= std::abs(b);	//its magnitude

		d = f = Complex(0,0); //clear variables for use
		abx = std::abs(*x);	//the magnitude of the current root

		//Efficient computation of the polynomial and its first 2 derivatives
		for(j = m-1; j >= 0; --j)
		{
			f = (*x)*f + d;
			d = (*x)*d + b;
			b = (*x)*b + a[j];

			err = std::abs(b) + abx*err;
		}

		//Estimate the roundoff error in evaluation polynomial
		err *= EPSS;

		//Are we on the root?
		if(std::abs(b) < err)
		{
			return;
		}

		//General case: use Laguerre's formula
		//a = n / G +- sqrt( (n-1)(nH - G^2) )
		//x = x - a

		g = d / b; 	//get G
		g2 = g * g; //for the sqrt calc

		h = g2 - 2.0f * (f / b);	//get H

		sq = pow( (float)(m-1) * ((float)m*h - g2), 0.5f ); //get the sqrt

		//get the denominator
		gp = g + sq;
		gm = g - sq;

		abp = std::abs(gp);
		abm = std::abs(gm);

		//get the denominator with the highest magnitude
		if(abp < abm)
		{
			abp = abm;
			gp = gm;
		}

		//if the denominator is positive do one thing, otherwise do the other
		dx = (abp > 0.0) ? (float)m / gp : std::polar((1+abx),(float)iter);
		x1 = *x - dx;

		//Have we converged?
		if( *x == x1 )
		{
			return;
		}

		//Every so often take a fractional step, to break any limit cycle (itself a rare occurrence).
		if( iter % MT )
		{
			*x = x1;
		}else
		{
			*x = *x - (frac[iter/MT]*dx);
		}
	}

	//very unusual - can occur only for complex roots.  Try a different starting guess for the root.
	//nrerror("too many iterations in laguer");
	return;
}

#define EPS 2.0e-6
#define MAXM 100	//a small number, and maximum anticipated value of m..

/* 	Given the degree m and the m+1 complex coefficients a[0..m] of the polynomial a0 + a1*x +...+ an*x^n
	the routine successively calls laguer and finds all m complex roots in roots[1..m].
	The boolean variable polish should be input as true (1) if polishing (also by Laguerre's Method)
	is desired, false (0) if the roots will be subsequently polished by other means.
*/
void RootFinder::find_all_roots(bool polish)
{
	int i,its,j,jj;
	Complex x,b,c;
	int m = coefs.size()-1;

	//make sure roots is big enough
	roots.resize(m);

	if(workcoefs.size() < MAXM) workcoefs.resize(MAXM);

	//Copy the coefficients for successive deflation
	for(j = 0; j <= m; ++j)
	{
		workcoefs[j] = coefs[j];
	}

	//Loop over each root to be found
	for(j = m-1; j >= 0; --j)
	{
		//Start at 0 to favor convergence to smallest remaining root, and find the root
		x = Complex(0,0);
		laguer(&workcoefs[0],j+1,&x,&its); //must add 1 to get the degree

		//if it is close enough to a real root, then make it so
		if(std::abs(x.imag()) <= 2.0*EPS*std::abs(x.real()))
		{
			x = Complex(x.real());
		}

		roots[j] = x;

		//forward deflation

		//the degree is j+1 since j(0,m-1)
		b = workcoefs[j+1];
		for(jj = j; jj >= 0; --jj)
		{
			c = workcoefs[jj];
			workcoefs[jj] = b;
			b = x*b + c;
		}
	}

	//Polish the roots using the undeflated coefficients
	if(polish)
	{
		for(j = 0; j < m; ++j)
		{
			laguer(&coefs[0],m,&roots[j],&its);
		}
	}

	//Sort roots by their real parts by straight insertion
	for(j = 1; j < m; ++j)
	{
		x = roots[j];
		for( i = j-1; i >= 1; --i)
		{
			if(roots[i].real() <= x.real()) break;
			roots[i+1] = roots[i];
		}
		roots[i+1] = x;
	}
}
