/* === S Y N F I G ========================================================= */
/*!	\file julia.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_JULIA_H
#define __SYNFIG_JULIA_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class Julia : public synfig::Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:

	synfig::Color icolor;
	synfig::Color ocolor;
	synfig::Angle color_shift;
	Real bailout;
	Real lp;
	int iterations;
	synfig::Point seed;

	bool distort_inside;
	bool distort_outside;
	bool shade_inside;
	bool shade_outside;
	bool solid_inside;
	bool solid_outside;
	bool invert_inside;
	bool invert_outside;
	bool color_inside;
	bool color_outside;

	bool color_cycle;
	bool smooth_outside;
	bool broken;

public:
	Julia();

	virtual bool set_param(const synfig::String &param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const synfig::String &param)const;

	virtual Color get_color(synfig::Context context, const synfig::Point &pos)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
