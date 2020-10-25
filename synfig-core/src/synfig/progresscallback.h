/* === S Y N F I G ========================================================= */
/*!	\file progresscallback.h
**	\brief ProgressCallback
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2010 Carlos López
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

#ifndef __SYNFIG_PROGRESSCALLBACK_H
#define __SYNFIG_PROGRESSCALLBACK_H

/* === H E A D E R S ======================================================= */

#include "string.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class ProgressCallback
**	\todo writeme
*/
class ProgressCallback
{
public:

	virtual ~ProgressCallback() { }
	virtual bool task(const String &/*task*/) { return true; }
	virtual bool error(const String &/*task*/) { return true; }
	virtual bool warning(const String &/*task*/) { return true; }
	virtual bool amount_complete(int /*current*/, int /*total*/) { return true; }

	virtual bool valid() const { return true; }
};

typedef ProgressCallback ProgressManager;

/*!	\class SuperCallback
**	\todo writeme
*/
class SuperCallback : public ProgressCallback
{
	ProgressCallback *cb;
	int start,end,tot;
	int w;
public:

	SuperCallback(): cb(), start(), end(), tot(), w() { }
	SuperCallback(ProgressCallback *cb,int start_, int end_, int total):
		cb(cb),start(start_),end(end_),tot(total),w(end-start)
	{
		//make sure we don't "inherit" if our subcallback is invalid
		if(!cb || !cb->valid())
			cb = NULL;
	}
	virtual bool task(const String &task) { if(cb)return cb->task(task); return true; }
	virtual bool error(const String &task) { if(cb)return cb->error(task); return true; }
	virtual bool warning(const String &task) { if(cb)return cb->warning(task); return true; }
	virtual bool amount_complete(int cur, int total) { if(cb)return cb->amount_complete(start+cur*w/total,tot); return true; }

	virtual bool valid() const { return cb != 0; }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
