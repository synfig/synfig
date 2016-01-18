/* === S Y N F I G ========================================================= */
/*!	\file measure.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <glib.h>

#include <synfig/general.h>
#include <synfig/localization.h>

#include "measure.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace debug;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

std::vector<Measure*> Measure::stack;
String Measure::text;

void Measure::init() {
	hide = !stack.empty() && stack.back()->hide_subs;
	hide_subs |= hide;
	if (!hide)
		text += String(stack.size()*2, ' ')
		      + "begin             "
		      + name
			  + "\n";
	stack.push_back(this);
	t = g_get_real_time();
}

Measure::~Measure() {
	long long dt = g_get_real_time() - t;
	double full_s = (double)dt*0.000001;
	double subs_s = (double)subs*0.000001;

	if (!hide)
		text += String((stack.size()-1)*2, ' ')
		      + "end " + strprintf("%13.6f ", subs ? subs_s : full_s)
		      + name
			  + (subs ? strprintf(" (full time: %.6f)", full_s) : String())
		      + "\n";

	stack.pop_back();
	if (stack.empty())
	{
		info("\n" + text);
		text.clear();
	}
	else
	{
		stack.back()->subs += dt;
	}
}
