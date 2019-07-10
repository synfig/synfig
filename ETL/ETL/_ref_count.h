/*! ========================================================================
** Extended Template Library
**
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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

#ifndef __ETL__REF_COUNT_H
#define __ETL__REF_COUNT_H

/* === H E A D E R S ======================================================= */

#include "_curve_func.h"
#include <cassert>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

class weak_reference_counter;

// ========================================================================
/*!	\class	reference_counter	_ref_count.h	ETL/ref_count
**	\brief	Reference counter
**	\see weak_reference_counter
**	\writeme
*/
class reference_counter
{
	friend class weak_reference_counter;
private:
	int* counter_;
public:

	reference_counter(const bool &x=true):counter_(x?new int(1):0) { }

	reference_counter(const reference_counter &x):counter_(x.counter_)
		{ if(counter_) (*counter_)++; }

	reference_counter(const weak_reference_counter &x);

	~reference_counter() { detach(); }

	reference_counter& operator=(const reference_counter &rhs)
	{
		detach();
		counter_=rhs.counter_;
		if(counter_)
		{
			assert(*counter_>0);
			(*counter_)++;
		}
		return *this;
	}

	void detach()
	{
		if(counter_)
		{
			assert(*counter_>0);
			if(!--(*counter_))
				delete counter_;
			counter_=0;
		}
	}

	void reset()
	{
		detach();
		counter_=new int(1);
	}

	int count()const { return counter_?*counter_:0; }

	bool unique()const { return counter_?*counter_==1:0; }

	operator int()const { return count(); }
}; // END of class reference_counter

// ========================================================================
/*!	\class	weak_reference_counter	_ref_count.h	ETL/ref_count
**	\brief	Weak Reference counter
**	\see reference_counter
**	\writeme
*/
class weak_reference_counter
{
	friend class reference_counter;
private:
	int* counter_;
public:
	weak_reference_counter():counter_(0) { }

	weak_reference_counter(const weak_reference_counter &x):counter_(x.counter_) { }

	weak_reference_counter(const reference_counter &x):counter_(x.counter_) { }

	~weak_reference_counter() { }

	weak_reference_counter& operator=(const reference_counter &rhs)
	{
		counter_=rhs.counter_;
		assert(*counter_>0);
		return *this;
	}

	weak_reference_counter& operator=(const weak_reference_counter &rhs)
	{
		counter_=rhs.counter_;
		assert(*counter_>0);
		return *this;
	}

	void detach() { counter_=0; }

	int count()const { return counter_?*counter_:0; }

	bool unique()const { return counter_?*counter_==1:0; }

	operator int()const { return count(); }
}; // END of class weak_reference_counter

inline reference_counter::reference_counter(const weak_reference_counter &x):
	counter_(x.counter_)
{
	if(counter_) (*counter_)++;
}

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
