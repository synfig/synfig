/*! ========================================================================
** Extended Template and Library
** Mutex Abstraction Class Implementation
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

#ifndef __ETL__CONDITION_H_
#define __ETL__CONDITION_H_

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

class condition : private mutex
{
	bool flag;
public:
	condition()
	{ flag=false; }
	~condition()
	{ }
	void operator()()
	{ flag=true; }
	void wait()
	{
		mutex::lock lock(*this);

		while(!flag)Yield();
		flag=false;
	}
	void wait_next()
	{
		mutex::lock lock(*this);

		flag=false;
		while(!flag)Yield();
	}
};

_ETL_END_NAMESPACE

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
