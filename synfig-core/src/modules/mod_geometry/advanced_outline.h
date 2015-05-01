/* === S Y N F I G ========================================================= */
/*!	\file outline.h
**	\brief Header file for implementation of the "Advanced Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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

#ifndef __SYNFIG_ADVANCED_OUTLINE_H
#define __SYNFIG_ADVANCED_OUTLINE_H

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

class Advanced_Outline : public synfig::Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: type list of BlinePoint
	ValueBase param_bline;
	//! Parameter: type list of WidthPoint
	ValueBase param_wplist;
	//! Parameter: type list of DashItem
	ValueBase param_dilist;
	//! Parameter: (WidthPoint::SideType)
	ValueBase param_start_tip;
	//! Parameter: (WidthPoint::SideType)
	ValueBase param_end_tip;
	//! Parameter: (int)
	ValueBase param_cusp_type;
	//! Parameter: (Real)
	ValueBase param_width;
	//! Parameter: (Real)
	ValueBase param_expand;
	//! Parameter: (Real)
	ValueBase param_smoothness;
	//! Parameter: (bool)
	ValueBase param_homogeneous;
	//! Parameter: (Real)
	ValueBase param_dash_offset;
	//! Parameter: (bool)
	ValueBase param_dash_enabled;
	//! Parameter: (bool)
	ValueBase param_fast;

	bool old_version;

public:
	enum CuspType
	{
		TYPE_SHARP     =0,
		TYPE_ROUNDED   =1,
		TYPE_BEVEL     =2
	};

	Advanced_Outline();
	//! Updates the polygon data to match the parameters.
	void sync();
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;
	using Layer::set_time;
	virtual void set_time(IndependentContext context, Time time)const;
	virtual void set_time(IndependentContext context, Time time, Vector pos)const;
	virtual bool set_version(const synfig::String &ver);
	//! Connects the parameter to another Value Node. Implementation for this layer
	virtual bool connect_dynamic_param(const String& param, etl::loose_handle<ValueNode> x );

private:

	bool connect_bline_to_wplist(etl::loose_handle<ValueNode> x);
	bool connect_bline_to_dilist(etl::loose_handle<ValueNode> x);
	Real bline_to_bezier(Real bline_pos, Real origin, Real bezier_size);
	Real bezier_to_bline(Real bezier_pos, Real origin, Real bezier_size);
	void add_tip(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector tangent, const WidthPoint wp, const Real gv);
	void add_cusp(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector curr, const Vector last, Real width);
};

/* === E N D =============================================================== */

#endif
