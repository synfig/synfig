/* === S Y N F I G ========================================================= */
/*!	\file region.h
**	\brief Header file for implementation of the "Region" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_REGION_H
#define __SYNFIG_REGION_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_polygon.h>
#include <list>
#include <vector>
#include <synfig/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { struct Segment; }

using namespace synfig;
using namespace std;
using namespace etl;

class Region : protected synfig::Layer_Polygon//Shape
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: list of type BlinePoint
	ValueBase param_bline;

	std::vector<synfig::Segment> segment_list;
public:
	Region();

	//! Updates the polygon data to match the parameters.
	void sync();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
	using Layer::set_time;
	virtual void set_time(IndependentContext context, Time time)const;
	virtual void set_time(IndependentContext context, Time time, Vector pos)const;
};

/* === E N D =============================================================== */

#endif
