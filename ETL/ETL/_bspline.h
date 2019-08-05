/*! ========================================================================
** Extended Template and Library
** B-Spline Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2010 Nikita Kitaev
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__BSPLINE_H
#define __ETL__BSPLINE_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <functional>
#include "_curve_func.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template <class T, class K=float, class C=affine_combo<T,K>, class D=distance_func<T> >
class bspline : public std::unary_function<K,T>
{
public:
	typedef T value_type;
	typedef K knot_type;
	typedef std::vector<knot_type>	knot_container;
	typedef std::vector<value_type>	cpoint_container;
	typedef typename knot_container::iterator knot_iterator;
	typedef typename cpoint_container::iterator cpoint_iterator;

	typedef C affine_func_type;
	typedef D distance_func_type;

protected:
	affine_func_type affine_func;
	distance_func_type distance_func;

private:
	int m;
	knot_container _knots;
	cpoint_container _cpoints;
	bool _loop;

public:
	bspline():m(2),_loop(false) {  }

	int get_m()const { return m-1; };
	int set_m(int new_m) { m=new_m+1; return m-1; };

	bool set_loop(bool x) { _loop=x; reset_knots(); return _loop; }

	knot_container & knots() { return _knots; };
	cpoint_container & cpoints() { return _cpoints; };

	const knot_container & knots()const { return _knots; };
	const cpoint_container & cpoints()const { return _cpoints; };

	void reset_knots()
	{
		int i;

		if(!_loop)
		{
			_knots.clear();
			if(!_cpoints.size())
				return;
			while(m>(signed)_cpoints.size())
				m--;
			for(i=0;i<m;i++)
				_knots.insert(_knots.end(), 0);
			for(i=1;i<(signed)_cpoints.size()-m+1;i++)
				_knots.insert(_knots.end(), i);
			for(i=0;i<m;i++)
				_knots.insert(_knots.end(), _cpoints.size()-m+1);
		}
		else
		{
			_knots.clear();
			if(!_cpoints.size())
				return;
			while(m>(signed)_cpoints.size())
				m--;
			for(i=0;i<=(signed)_cpoints.size()-m+1;i++)
				_knots.insert(_knots.end(), i);
		}
	}

	int calc_curve_segment(knot_type t)const
	{
		int k;
		if(t<0)
			t=0;
		if(t>=_knots.back())
			t=_knots.back()-0.0001;
		for(k=0;_knots[k]>t || _knots[k+1]<=t;k++)
		  ;

		return k;
	}

	knot_container get_segment_knots(int i)const
	{
		if(i+1<m)
		{
			knot_container ret(_knots.begin(),_knots.begin()+i+m+1);

			return ret;
		}
		if(i+1>=(signed)_knots.size())
		{
			knot_container ret(_knots.begin()+i-m+1,_knots.end());
			return ret;
		}
		return knot_container(_knots.begin()+i-m+1,	_knots.begin()+i+m);
	}

	cpoint_container get_segment_cpoints(int i)const
	{
		if(i+1<m)
		{
			return cpoint_container();
		}
		if(i+1>=(signed)_knots.size())
		{
			return cpoint_container();
		}
		return cpoint_container(_cpoints.begin()+i-m+1,	_cpoints.begin()+i+1);
	}

	cpoint_container calc_shell(knot_type t, int level)const
	{
		int
			i=calc_curve_segment(t),
			j,k;

		knot_container u=get_segment_knots(i);

		cpoint_container d=get_segment_cpoints(i);

		if(!d.size())
			return cpoint_container();

		for(j=0;d.size()>1 && j<level;d.pop_back(),j++)
		{
			for(k=0;k<d.size()-1;k++)
			{
				d[k]=affine_func(d[k],d[k+1],((t-u[j+k+1])/(u[m+k]-u[j+k+1])));
			}
		}
		return d;
	}

	value_type operator()(knot_type t)const
	{
		return get_curve_val(calc_curve_segment(t),t);
	}

	value_type get_curve_val(int i,knot_type t)const
	{
		int
			j,k;

		knot_container u=get_segment_knots(i);

		cpoint_container d=get_segment_cpoints(i);

		if(!d.size())
			return value_type();

		for(j=0;d.size()>1;d.pop_back(),j++)
		{
			for(k=0;k<(signed)d.size()-1;k++)
			{
				d[k]=affine_func(d[k],d[k+1],((t-u[j+k+1])/(u[m+k]-u[j+k+1])));
			}
		}
		return d.front();
	}

	cpoint_iterator find_closest_cpoint(const value_type &point, typename distance_func_type::result_type max)
	{
		cpoint_iterator i=_cpoints.begin();
		cpoint_iterator ret=i;
		typename distance_func_type::result_type dist=distance_func(point,_cpoints[0]);

		// The distance function returns "cooked" (ie: squared)
		// distances, so we need to cook our max distance for
		// the comparison to work correctly.
		max=distance_func.cook(max);

		for(++i;i<_cpoints.end();i++)
		{
			typename distance_func_type::result_type thisdist=distance_func(point,*i);

			if(thisdist<dist)
			{
				dist=thisdist;
				ret=i;
			}
		}
		if(dist<max)
			return ret;
		return _cpoints.end();
	}
};

};

/* -- F U N C T I O N S ----------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
