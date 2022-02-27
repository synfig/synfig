/* === S Y N F I G ========================================================= */
/*!	\file lumakey.cpp
**	\brief Implementation of the "Luma Key" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "lumakey.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/segment.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(LumaKey);
SYNFIG_LAYER_SET_NAME(LumaKey,"lumakey");
SYNFIG_LAYER_SET_LOCAL_NAME(LumaKey,N_("Luma Key"));
SYNFIG_LAYER_SET_CATEGORY(LumaKey,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(LumaKey,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LumaKey::LumaKey()
{
}

ValueBase
LumaKey::get_param(const String &param)const
{
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Color
LumaKey::get_color(Context context, const Point &getpos)const
{
	Color color(context.get_color(getpos));

	if (active()) {
		color.set_a(color.get_y()*color.get_a());
		color.set_y(1);
	}

	return color;
}

bool
LumaKey::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;

	int x,y;

	Surface::pen pen(surface->begin());

	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
		{
			Color tmp(pen.get_value());
			tmp.set_a(tmp.get_y()*tmp.get_a());
			tmp.set_y(1);
			pen.put_value(tmp);
		}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}



////


Rect
LumaKey::get_bounding_rect(Context context)const
{
	if(!active())
		return Rect::zero();

	return context.get_full_bounding_rect();
}

rendering::Task::Handle
LumaKey::build_rendering_task_vfunc(Context context) const
	{ return Layer::build_rendering_task_vfunc(context); }
