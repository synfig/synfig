/* === S Y N F I G ========================================================= */
/*!	\file checkerboard.cpp
**	\brief Implementation of the "Checkerboard" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "checkerboard.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(CheckerBoard);
SYNFIG_LAYER_SET_NAME(CheckerBoard,"checker_board");
SYNFIG_LAYER_SET_LOCAL_NAME(CheckerBoard,N_("Checkerboard"));
SYNFIG_LAYER_SET_CATEGORY(CheckerBoard,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(CheckerBoard,"0.1");
SYNFIG_LAYER_SET_CVS_ID(CheckerBoard,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CheckerBoard::CheckerBoard():
	Layer_Composite	(1.0,Color::BLEND_COMPOSITE),
	color			(Color::black()),
	origin			(Point(0.125,0.125)),
	size			(Point(0.25,0.25))
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}

inline bool
CheckerBoard::point_test(const synfig::Point& getpos)const
{
	int val=((int)((getpos[0]-origin[0])/size[0])+(int)((getpos[1]-origin[1])/size[1]));
	if(getpos[0]-origin[0] < 0.0)
		val++;
	if(getpos[1]-origin[1] < 0.0)
		val++;
	return val&1;
}

bool
CheckerBoard::set_param(const String &param, const ValueBase &value)
{
	IMPORT_PLUS(color, { if (color.get_a() == 0) { if (converted_blend_) {
					set_blend_method(Color::BLEND_ALPHA_OVER);
					color.set_a(1); } else transparent_color_ = true; } });
	IMPORT(origin);
	IMPORT(size);

	IMPORT_AS(origin,"pos");
	IMPORT_AS(origin[0],"pos[0]");
	IMPORT_AS(origin[1],"pos[1]");

	return Layer_Composite::set_param(param,value);
}

ValueBase
CheckerBoard::get_param(const String &param)const
{
	EXPORT(color);
	EXPORT(origin);
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
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Center of the checkers"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of checkers"))
		.set_origin("origin")
	);

	return ret;
}

synfig::Layer::Handle
CheckerBoard::hit_check(synfig::Context context, const synfig::Point &getpos)const
{
	if(get_amount()!=0.0 && point_test(getpos))
	{
		synfig::Layer::Handle tmp;
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
		return Color::blend(Color::alpha(),context.get_color(getpos),get_amount(),get_blend_method());
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
