/* === S I N F G =========================================================== */
/*!	\file stretch.cpp
**	\brief Template Header
**
**	$Id: stretch.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#define SINFG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "stretch.h"
#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/transform.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace sinfg;

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(Layer_Stretch);
SINFG_LAYER_SET_NAME(Layer_Stretch,"stretch");
SINFG_LAYER_SET_LOCAL_NAME(Layer_Stretch,_("Stretch"));
SINFG_LAYER_SET_CATEGORY(Layer_Stretch,_("Distortions"));
SINFG_LAYER_SET_VERSION(Layer_Stretch,"0.1");
SINFG_LAYER_SET_CVS_ID(Layer_Stretch,"$Id: stretch.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_Stretch::Layer_Stretch():
	amount(1,1),
	center(0,0)
{
}

	
bool
Layer_Stretch::set_param(const String & param, const ValueBase &value)
{
	IMPORT(amount);
	IMPORT(center);
	
	return false;	
}

ValueBase
Layer_Stretch::get_param(const String &param)const
{
	EXPORT(amount);
	EXPORT(center);

	EXPORT_NAME();
	EXPORT_VERSION();
		
	return ValueBase();	
}

Layer::Vocab
Layer_Stretch::get_param_vocab()const
{
	Layer::Vocab ret;
	
	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Amount"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
	);
	
	return ret;
}

sinfg::Layer::Handle
Layer_Stretch::hit_check(sinfg::Context context, const sinfg::Point &pos)const
{
	Point npos(pos);
	npos[0]=(npos[0]-center[0])/amount[0]+center[0];
	npos[1]=(npos[1]-center[1])/amount[1]+center[1];
	return context.hit_check(npos);
}

Color
Layer_Stretch::get_color(Context context, const Point &pos)const
{
	Point npos(pos);
	npos[0]=(npos[0]-center[0])/amount[0]+center[0];
	npos[1]=(npos[1]-center[1])/amount[1]+center[1];
	return context.get_color(npos);
}

class  Stretch_Trans : public Transform
{
	etl::handle<const Layer_Stretch> layer;
public:
	Stretch_Trans(const Layer_Stretch* x):Transform(x->get_guid()),layer(x) { }
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		return Vector((x[0]-layer->center[0])*layer->amount[0]+layer->center[0],(x[1]-layer->center[1])*layer->amount[1]+layer->center[1]);
	}
	
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		return Vector((x[0]-layer->center[0])/layer->amount[0]+layer->center[0],(x[1]-layer->center[1])/layer->amount[1]+layer->center[1]);
	}
};
etl::handle<Transform>
Layer_Stretch::get_transform()const
{
	return new Stretch_Trans(this);
}

bool
Layer_Stretch::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RendDesc desc(renddesc);
	desc.clear_flags();
    // Adjust the top_left and bottom_right points
	// for our zoom amount
	Point npos;
	npos[0]=(desc.get_tl()[0]-center[0])/amount[0]+center[0];
	npos[1]=(desc.get_tl()[1]-center[1])/amount[1]+center[1];
	desc.set_tl(npos);
	npos[0]=(desc.get_br()[0]-center[0])/amount[0]+center[0];
	npos[1]=(desc.get_br()[1]-center[1])/amount[1]+center[1];
	desc.set_br(npos);

	// Render the scene
	return context.accelerated_render(surface,quality,desc,cb);
}
