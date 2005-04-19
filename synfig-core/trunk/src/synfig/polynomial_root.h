/* === S Y N F I G ========================================================= */
/*!	\file polynomial_root.h
**	\brief Polynomial Root Finder Header
**
**	$Id: polynomial_root.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_POLYNOMIAL_ROOT_H
#define __SYNFIG_POLYNOMIAL_ROOT_H

/* === H E A D E R S ======================================================= */

#include <complex>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
template < typename T = float, typename F = float >
class Polynomial : public std::vector<T> //a0 + a1x + a2x^2 + ... + anx^n
{		
public:
	
	//Will maintain all lower constants
	void degree(unsigned int d, const T & def = (T)0) { resize(d+1,def); }
	unsigned int degree()const { return size() - 1; }
	
	const Polynomial & operator+=(const Polynomial &p)
	{
		if(p.size() > size())
			resize(p.size(), (T)0);
		
		for(int i = 0; i < p.size(); ++i)
		{
			(*this)[i] += p[i];
		}
		return *this;
	}
	
	const Polynomial & operator-=(const Polynomial &p)
	{
		if(p.size() > size())
			resize(p.size(), (T)0);
		
		for(int i = 0; i < p.size(); ++i)
		{
			(*this)[i] -= p[i];
		}
		return *this;
	}
	
	const Polynomial & operator*=(const Polynomial &p)
	{
		if(p.size() < 1)
		{
			resize(0);
			return *this;
		}
		
		unsigned int i,j;
		std::vector<T> nc(*this);
		
		//in place for constant stuff
		for(i = 0; i < nc.size(); ++i)
		{
			(*this)[i] *= p[0];
		}
		
		if(p.size() < 2) return *this;
			
		resize(size() + p.degree());		
		for(int i = 0; i < nc.size(); ++i)
		{
			for(int j = 1; j < p.size(); ++j)
			{
				nc[i+j] += nc[i]*p[j];
			}
		}
		
		return *this;
	}	
};

class RootFinder
{
	std::vector< std::complex<float> >	workcoefs;
	int	its;
	
public:
	std::vector< std::complex<float> >	coefs; //the number of coefficients determines the degree of polynomial

	std::vector< std::complex<float> >	roots;

	void find_all_roots(bool polish);
};



/* === E N D =============================================================== */

#endif
