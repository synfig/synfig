/* === S Y N F I G ========================================================= */
/*!	\file star.h
**	\brief Header file for implementation of the "Star" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_STAR_H
#define __SYNFIG_STAR_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_polygon.h>
#include <synfig/value.h>
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class Star : protected synfig::Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT
private:

	Real radius1;
	Real radius2;
	int points;
	ValueBase param_angle;
	bool regular_polygon;

public:
	Star();

	//! Updates the polygon data to match the parameters.
	void sync();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual bool set_param_static(const String &param, const bool x); 

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
	virtual bool accelerated_cairorender(Context context,cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
};

/* === E N D =============================================================== */

#endif
