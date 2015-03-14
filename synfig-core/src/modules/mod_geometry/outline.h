/* === S Y N F I G ========================================================= */
/*!	\file outline.h
**	\brief Header file for implementation of the "Outline" layer
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_OUTLINE_H
#define __SYNFIG_OUTLINE_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>
#include <synfig/layers/layer_polygon.h>
#include <synfig/segment.h>
#include <synfig/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class Outline : public synfig::Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: type list of BLinePoints
	ValueBase param_bline;
	//! Parameter: (bool)
	ValueBase param_round_tip[2];
	//! Parameter: (bool)
	ValueBase param_sharp_cusps;
	//! Parameter: (bool)
	ValueBase param_loop;
	//! Parameter: (Real)
	ValueBase param_width;
	//! Parameter: (Real)
	ValueBase param_expand;
	//! Parameter: (Real)
	ValueBase param_loopyness;
	//! Parameter: (bool)
	ValueBase param_homogeneous_width;

	bool old_version;

	bool needs_sync;


	std::vector<synfig::Segment> segment_list;
	std::vector<synfig::Real> width_list;

public:

	Outline();

	//! Updates the polygon data to match the parameters.
	void sync();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
	using Layer::set_time;
	virtual void set_time(IndependentContext context, Time time)const;
	virtual void set_time(IndependentContext context, Time time, Vector pos)const;
	virtual bool set_version(const String &ver){if(ver=="0.1")old_version=true; return true;}
	virtual void reset_version(){old_version=false;}
};

/* === E N D =============================================================== */

#endif
