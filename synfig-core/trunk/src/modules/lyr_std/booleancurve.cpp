/* === S Y N F I G ========================================================= */
/*!	\file boolean_curve.cpp
**	\brief Boolean Curve Implementation File
**
**	$Id: booleancurve.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#include "booleancurve.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

BooleanCurve::BooleanCurve()
{
}

BooleanCurve::~BooleanCurve()
{
}

bool BooleanCurve::set_param(const String & param, const synfig::ValueBase &value)
{	
	if(param=="regions" && value.same_as(regions)) 
	{
		vector<BLinePoint> bv;
		int size = value.get_list().size();
		
		const vector<ValueBase> &vlist = value.get_list();
		
		regions.clear();		
		for(int i = 0; i < size; ++i)
		{
			regions.push_back(vector<BLinePoint>(vlist[i].get_list().begin(),vlist[i].get_list().end()));
		}
		return true;
	}
	
	return Layer_Shape::set_param(param,value);
}

ValueBase BooleanCurve::get_param(const String & param)const
{
	EXPORT(regions);
	
	EXPORT_NAME();
	EXPORT_VERSION();
	
	return Layer_Shape::get_param(param);
}

Layer::Vocab BooleanCurve::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());
	
	ret.push_back(ParamDesc("regions")
		.set_local_name(_("Region Set"))
		.set_description(_("Set of regions to combine"))
	);
	
	return ret;
}

Color BooleanCurve::get_color(Context context, const Point &pos)const
{
	Color c(Color::alpha());
	
	return c;
}

bool BooleanCurve::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	return false;
}
