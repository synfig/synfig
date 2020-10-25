/* === S Y N F I G ========================================================= */
/*!	\file star.h
**	\brief Header file for implementation of the "Star" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STAR_H
#define __SYNFIG_STAR_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_polygon.h>
#include <synfig/value.h>
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace etl;

class Star : protected synfig::Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (Real)
	ValueBase param_radius1;
	//!Parameter: (Real)
	ValueBase param_radius2;
	//!Parameter: (int)
	ValueBase param_points;
	//!Parameter: (Angle)
	ValueBase param_angle;
	//!Parameter: (bool)	
	ValueBase param_regular_polygon;

public:
	Star();

	virtual bool set_shape_param(const String &param, const ValueBase &value);
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;

protected:
	virtual void sync_vfunc();
};

/* === E N D =============================================================== */

#endif
