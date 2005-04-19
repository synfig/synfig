/* === S Y N F I G ========================================================= */
/*!	\file layer_polygon.cpp
**	\brief Template Header
**
**	$Id: layer_polygon.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "layer_polygon.h"
#include "string.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"
//#include "ETL/bezier"
#include <vector>

#include <deque>
using std::deque;

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Polygon);
SYNFIG_LAYER_SET_NAME(Layer_Polygon,"polygon");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Polygon,_("Polygon"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Polygon,_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Layer_Polygon,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Polygon,"$Id: layer_polygon.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

Layer_Polygon::Layer_Polygon():
	Layer_Shape		(1.0,Color::BLEND_COMPOSITE),
	vector_list		(0)
{
	vector_list.push_back(Point(0,0.5));
	vector_list.push_back(Point(-0.333333,0));
	vector_list.push_back(Point(0.333333,0));
	sync();
}

Layer_Polygon::~Layer_Polygon()
{
}

void
Layer_Polygon::sync()
{
/*
	int i,pointcount=vector_list.size();

	if(pointcount<3)
		return;

	//Layer_Shape::clear();
	//clear();

	// Build edge table
	move_to(vector_list[0][0],vector_list[0][1]);
	
	for(i = 1;i < pointcount; i++)
	{
		if(isnan(vector_list[i][0]) || isnan(vector_list[i][1]))
			break;
		line_to(vector_list[i][0],vector_list[i][1]);
	}
	close();
	//endpath();
*/
}

void
Layer_Polygon::add_polygon(const vector<Point> &point_list)
{
	int i,pointcount=point_list.size();

	if(pointcount<3)
		return;

	//Layer_Shape::clear();
	//clear();

	// Build edge table
	move_to(point_list[0][0],point_list[0][1]);
	
	for(i = 1;i < pointcount; i++)
	{
		if(isnan(point_list[i][0]) || isnan(point_list[i][1]))
			break;
		line_to(point_list[i][0],point_list[i][1]);
	}
	close();
	//endpath();
}

void
Layer_Polygon::clear()
{
	Layer_Shape::clear();
	vector_list.clear();
}
	
bool
Layer_Polygon::set_param(const String & param, const ValueBase &value)
{
	if(	param=="vector_list" && value.same_as(vector_list))
	{
		vector_list=value;
		Layer_Shape::clear();
		add_polygon(value);
		sync();
		return true;
	}
	
	return Layer_Shape::set_param(param,value);
}

ValueBase
Layer_Polygon::get_param(const String &param)const
{
	EXPORT(vector_list);

	EXPORT_NAME();
	EXPORT_VERSION();
		
	return Layer_Shape::get_param(param);	
}

Layer::Vocab
Layer_Polygon::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());
	
	ret.push_back(ParamDesc("vector_list")
		.set_local_name(_("Vector List"))
		.set_origin("offset")
	);
	
	return ret;
}
