/* === S Y N F I G ========================================================= */
/*!	\file region.cpp
**	\brief Implementation of the "Region" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "region.h"
#include <ETL/stringf>
#include <ETL/bezier>
#include <ETL/hermite>

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>
#include <synfig/valuenodes/valuenode_bline.h>

#endif

using namespace etl;

/* === M A C R O S ========================================================= */

#define SAMPLES		75

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Region);
SYNFIG_LAYER_SET_NAME(Region,"region");
SYNFIG_LAYER_SET_LOCAL_NAME(Region,N_("Region"));
SYNFIG_LAYER_SET_CATEGORY(Region,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Region,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Region,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Region::Region()
{
	clear();
	vector<BLinePoint> bline_point_list;
	bline_point_list.push_back(BLinePoint());
	bline_point_list.push_back(BLinePoint());
	bline_point_list.push_back(BLinePoint());
	bline_point_list[0].set_vertex(Point(0,1));
	bline_point_list[1].set_vertex(Point(0,-1));
	bline_point_list[2].set_vertex(Point(1,0));
	bline_point_list[0].set_tangent(bline_point_list[1].get_vertex()-bline_point_list[2].get_vertex()*0.5f);
	bline_point_list[1].set_tangent(bline_point_list[2].get_vertex()-bline_point_list[0].get_vertex()*0.5f);
	bline_point_list[2].set_tangent(bline_point_list[0].get_vertex()-bline_point_list[1].get_vertex()*0.5f);
	bline_point_list[0].set_width(1.0f);
	bline_point_list[1].set_width(1.0f);
	bline_point_list[2].set_width(1.0f);
	param_bline.set_list_of(bline_point_list);

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
Region::sync()
{
	ValueBase bline=param_bline;
	
	if(bline.get_contained_type()==type_bline_point)
		segment_list=convert_bline_to_segment_list(bline).get_list_of(synfig::Segment());
	else if(bline.get_contained_type()==type_segment)
		segment_list=vector<synfig::Segment>(bline.get_list_of(synfig::Segment()));
	else
	{
		synfig::warning("Region: incorrect type on bline, layer disabled");
		clear();
		return;
	}

	if(segment_list.empty())
	{
		synfig::warning("Region: segment_list is empty, layer disabled");
		clear();
		return;
	}

	bool looped = bline.get_loop();

	Vector::value_type n;
	etl::hermite<Vector> curve;
	vector<Point> vector_list;

	vector<Segment>::const_iterator iter=segment_list.begin();
	//Vector							last = iter->p1;

	//make sure the shape has a clean slate for writing
	//clear();

	//and start off at the first point
	//move_to(last[0],last[1]);

	for(;iter!=segment_list.end();++iter)
	{
		//connect them with a line if they aren't already joined
		/*if(iter->p1 != last)
		{
			line_to(iter->p1[0],iter->p1[1]);
		}

		//curve to the next end point
		cubic_to(iter->p2[0],iter->p2[1],
				 iter->p1[0] + iter->t1[0]/3.0,iter->p1[1] + iter->t1[1]/3.0,
				 iter->p2[0] - iter->t2[0]/3.0,iter->p2[1] - iter->t2[1]/3.0);

		last = iter->p2;*/

		if(iter->t1.is_equal_to(Vector(0,0)) && iter->t2.is_equal_to(Vector(0,0)))
		{
			vector_list.push_back(iter->p2);
		}
		else
		{
			curve.p1()=iter->p1;
			curve.t1()=iter->t1;
			curve.p2()=iter->p2;
			curve.t2()=iter->t2;
			curve.sync();

			for(n=0.0;n<1.0;n+=1.0/SAMPLES)
				vector_list.push_back(curve(n));
		}
	}

	//add the starting point onto the end so it actually fits the shape, so we can be extra awesome...
	if(!looped)
		vector_list.push_back(segment_list[0].p1);

	clear();
	add_polygon(vector_list);

	/*close();
	endpath();*/
}

bool
Region::set_param(const String & param, const ValueBase &value)
{
	if(param=="segment_list")
	{
		if(dynamic_param_list().count("segment_list"))
		{
			connect_dynamic_param("bline",dynamic_param_list().find("segment_list")->second);
			disconnect_dynamic_param("segment_list");
			synfig::warning("Region::set_param(): Updated valuenode connection to use the new \"bline\" parameter.");
		}
		else
			synfig::warning("Region::set_param(): The parameter \"segment_list\" is deprecated. Use \"bline\" instead.");
	}

	if(	(param=="segment_list" || param=="bline") && value.get_type()==type_list)
	{
		//if(value.get_contained_type()!=type_bline_point)
		//	return false;

		param_bline=value;

		return true;
	}

/*	if(	param=="segment_list" && value.get_type()==type_list)
	{
		if(value.get_contained_type()==type_bline_point)
			segment_list=convert_bline_to_segment_list(value);
		else
		if(value.get_contained_type()==type_segment)
			segment_list=value;
		else
		if(value.empty())
			segment_list.clear();
		else
			return false;
		sync();
		return true;
	}
	*/
	return Layer_Shape::set_param(param,value);
}

ValueBase
Region::get_param(const String& param)const
{
	EXPORT_VALUE(param_bline);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Region::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_description(_("A list of spline points"))
	);

	return ret;
}

void
Region::set_time(IndependentContext context, Time time)const
{
	const_cast<Region*>(this)->sync();
	context.set_time(time);
}

void
Region::set_time(IndependentContext context, Time time, Vector pos)const
{
	const_cast<Region*>(this)->sync();
	context.set_time(time,pos);
}

