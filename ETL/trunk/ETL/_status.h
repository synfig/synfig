#ifdef THIS_CODE_IS_NOT_USED
/*! ========================================================================
** Extended Template Library
** Status Callback Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
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

#ifndef __ETL__STATUS_H
#define __ETL__STATUS_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

class status
{
public:
	virtual ~ProgressCallback() { }

	virtual bool task(const std::string &task) { return true; }

#warning this task(task) thing looks wrong, but the code isn't used, so let's not worry
	virtual void push_task(const std::string &task,int start=0, int end=100, int total=100)
		{ task(task); }
	virtual void pop_task() { return; }

	virtual void warning(const std::string &warn) { return; }
	virtual void error(const std::string &err) { return; }

	virtual bool amount_complete(int current, int total) { return true; }
	virtual bool amount_complete(float percent)
		{ return amount_complete((int)(percent*10000),10000); }
};

_ETL_END_NAMESPACE

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
#endif /* THIS_CODE_IS_NOT_USED */
