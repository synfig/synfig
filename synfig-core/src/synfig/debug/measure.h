/* === S Y N F I G ========================================================= */
/*!	\file measure.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_DEBUG_MEASURE_H
#define __SYNFIG_DEBUG_MEASURE_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <synfig/string.h>

/* === M A C R O S ========================================================= */

#define SYNFIG_DEBUG_MEASURE

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
namespace debug {

class Measure {
private:
	static std::vector<Measure*> stack;
	static String text;

	String name;
	bool hide;
	bool hide_subs;
	long long subs;
	long long t;

	Measure(const Measure&): name(), hide(), hide_subs(), subs(), t() { }
	Measure& operator= (const Measure&) { return *this; }
	void init();

public:
	Measure(const String &name, bool hide_subs = false):
		name(name), hide(), hide_subs(hide_subs), subs(), t()
	{ init(); }

	~Measure();
};

}; // END of namespace debug
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
