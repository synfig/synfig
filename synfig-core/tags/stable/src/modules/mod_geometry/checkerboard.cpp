/* === S I N F G =========================================================== */
/*!	\file checkerboard.cpp
**	\brief Template Header
**
**	$Id: checkerboard.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#include "checkerboard.h"

#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/segment.h>

#endif

using namespace sinfg;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(CheckerBoard);
SINFG_LAYER_SET_NAME(CheckerBoard,"checker_board");
SINFG_LAYER_SET_LOCAL_NAME(CheckerBoard,_("CheckerBoard"));
SINFG_LAYER_SET_CATEGORY(CheckerBoard,_("Geometry"));
SINFG_LAYER_SET_VERSION(CheckerBoard,"0.1");
SINFG_LAYER_SET_CVS_ID(CheckerBoard,"$Id: checkerboard.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CheckerBoard::CheckerBoard():
	Layer_Composite	(1.0,Color::BLEND_STRAIGHT),
	color			(Color::black()),
	pos				(Point(0.125,0.125)),
	size			(Point(0.25,0.25))
{

	set_blend_method(Color::BLEND_STRAIGHT);
}

inline bool
CheckerBoard::point_test(const sinfg::Point& getpos)const
{
	int val=((int)((getpos[0]-pos[0])/size[0])+(int)((getpos[1]-pos[1])/size[1]));
	if(getpos[0]-pos[0] < 0.0)
		val++;
	if(getpos[1]-pos[1] < 0.0)
		val++;
	return val&1;
}

bool
CheckerBoard::set_param(const String &param, const ValueBase &value)
{
	IMPORT(color);
	IMPORT(pos);
	IMPORT(pos[0]);
	IMPORT(pos[1]);
	IMPORT(size);
	
	return Layer_Composite::set_param(param,value);
}

ValueBase
CheckerBoard::get_param(const String &param)const
{
	EXPORT(color);
	EXPORT(pos);
	EXPORT(pos[0]);
	EXPORT(pos[1]);	
	EXPORT(size);
	EXPORT_NAME();
	EXPORT_VERSION();
		
	return Layer_Composite::get_param(param);	
}

Layer::Vocab
CheckerBoard::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());
	
	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Color of checkers"))
	);
	ret.push_back(ParamDesc("pos")
		.set_local_name(_("Offset"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of checkers"))
		.set_origin("pos")
	);

	return ret;
}

sinfg::Layer::Handle
CheckerBoard::hit_check(sinfg::Context context, const sinfg::Point &getpos)const
{
	if(get_amount()!=0.0 && point_test(getpos))
	{
		sinfg::Layer::Handle tmp;
		if(get_blend_method()==Color::BLEND_BEHIND && (tmp=context.hit_check(getpos)))
			return tmp;
		if(Color::is_onto(get_blend_method()) && !(tmp=context.hit_check(getpos)))
			return 0;
		return const_cast<CheckerBoard*>(this);
	}
	else
		return context.hit_check(getpos);
}

Color
CheckerBoard::get_color(Context context, const Point &getpos)const
{
	if(get_amount()!=0.0 && point_test(getpos))
	{
		if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
			return color;
		else
			return Color::blend(color,context.get_color(getpos),get_amount(),get_blend_method());
	}
	else
		return context.get_color(getpos);
}

bool
CheckerBoard::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;
	if(get_amount()==0)
		return true;

	int x,y;

	const Point tl(renddesc.get_tl());
	Point pos;
	const int w(surface->get_w());
	const int h(surface->get_h());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());

	Surface::alpha_pen apen(surface->begin());

	apen.set_alpha(get_amount());
	apen.set_blend_method(get_blend_method());
	apen.set_value(color);

	for(y=0,pos[1]=tl[1];y<h;y++,apen.inc_y(),apen.dec_x(x),pos[1]+=ph)
		for(x=0,pos[0]=tl[0];x<w;x++,apen.inc_x(),pos[0]+=pw)
			if(point_test(pos))
				apen.put_value();

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
