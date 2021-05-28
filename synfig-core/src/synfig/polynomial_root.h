/* === S Y N F I G ========================================================= */
/*!	\file polynomial_root.h
**	\brief Polynomial Root Finder Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
	unsigned int degree()const { return this->size() - 1; }

	const Polynomial & operator+=(const Polynomial &p)
	{
		if(p.size() > this->size())
			resize(p.size(), (T)0);

		for(int i = 0; i < p.size(); ++i)
		{
			(*this)[i] += p[i];
		}
		return *this;
	}

	const Polynomial & operator-=(const Polynomial &p)
	{
		if(p.size() > this->size())
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
			this->resize(0);
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

		this->resize(this->size() + p.degree());
		for(i = 0; i < nc.size(); ++i)
		{
			for(j = 1; j < p.size(); ++j)
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
	//int	its;

public:
	std::vector< std::complex<float> >	coefs; //the number of coefficients determines the degree of polynomial

	std::vector< std::complex<float> >	roots;

	void find_all_roots(bool polish);
};



/* === E N D =============================================================== */

#endif
