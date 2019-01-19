/* === S Y N F I G ========================================================= */
/*!	\file clamp.cpp
**	\brief Implementation of the "Clamp" layer
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "clamp.h"

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

#include <synfig/rendering/software/surfacesw.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Clamp);
SYNFIG_LAYER_SET_NAME(Layer_Clamp,"clamp");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Clamp,N_("Clamp"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Clamp,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(Layer_Clamp,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_Clamp,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

rendering::Task::Token TaskClamp::token(
	DescAbstract<TaskClamp>("Clamp") );
rendering::Task::Token TaskClampSW::token(
	DescReal<TaskClampSW, TaskClamp>("ClampSW") );


void
TaskClampSW::clamp_pixel(Color &dst, const Color &src) const
{
	if (fabs(src.get_a()) < 1e-8)
		{ dst = Color::alpha(); return; }

	dst = src;

	if (invert_negative)
	{
		if (dst.get_a() < floor)
			dst = -dst;

		if(dst.get_r() < floor)
		{
			dst.set_g(dst.get_g() - dst.get_r());
			dst.set_b(dst.get_b() - dst.get_r());
			dst.set_r(floor);
		}

		if(dst.get_g() < floor)
		{
			dst.set_r(dst.get_r() - dst.get_g());
			dst.set_b(dst.get_b() - dst.get_g());
			dst.set_g(floor);
		}

		if(dst.get_b() < floor)
		{
			dst.set_g(dst.get_g() - dst.get_b());
			dst.set_r(dst.get_r() - dst.get_b());
			dst.set_b(floor);
		}
	}
	else
	if (clamp_floor)
	{
		if (dst.get_r() < floor) dst.set_r(floor);
		if (dst.get_g() < floor) dst.set_g(floor);
		if (dst.get_b() < floor) dst.set_b(floor);
		if (dst.get_a() < floor) dst.set_a(floor);
	}

	if (clamp_ceiling)
	{
		if (dst.get_r() > ceiling) dst.set_r(ceiling);
		if (dst.get_g() > ceiling) dst.set_g(ceiling);
		if (dst.get_b() > ceiling) dst.set_b(ceiling);
		if (dst.get_a() > ceiling) dst.set_a(ceiling);
	}
}

bool
TaskClampSW::run(RunParams&) const
{
	RectInt r = target_rect;
	if (!r.valid())
	{
		VectorInt offset = get_offset();
		RectInt ra = sub_task()->target_rect + r.get_min() + get_offset();
		if (ra.valid())
		{
			etl::set_intersect(ra, ra, r);
			if (ra.valid())
			{
				LockWrite ldst(this);
				if (!ldst) return false;
				LockRead lsrc(sub_task());
				if (!lsrc) return false;

				const synfig::Surface &a = lsrc->get_surface();
				synfig::Surface &c = ldst->get_surface();

				for(int y = ra.miny; y < ra.maxy; ++y)
				{
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];
					for(int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc)
						clamp_pixel(*cc, *ca);
				}
			}
		}
	}

	return true;
}


Layer_Clamp::Layer_Clamp():
	param_invert_negative(ValueBase(false)),
	param_clamp_ceiling(ValueBase(true)),
	param_ceiling(ValueBase(Real(1.0f))),
	param_floor(ValueBase(Real(0.0f)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline Color
Layer_Clamp::clamp_color(const Color &in)const
{
	bool invert_negative=param_invert_negative.get(bool());
	bool clamp_ceiling=param_clamp_ceiling.get(bool());
	Real ceiling=param_ceiling.get(Real());
	Real floor=param_floor.get(Real());
	
	Color ret(in);

	if(ret.get_a()==0)
		return Color::alpha();

	if(invert_negative)
	{
		if(ret.get_a()<floor)
			ret=-ret;

		if(ret.get_r()<floor)
		{
			ret.set_g(ret.get_g()-ret.get_r());
			ret.set_b(ret.get_b()-ret.get_r());
			ret.set_r(floor);
		}
		if(ret.get_g()<floor)
		{
			ret.set_r(ret.get_r()-ret.get_g());
			ret.set_b(ret.get_b()-ret.get_g());
			ret.set_g(floor);
		}
		if(ret.get_b()<floor)
		{
			ret.set_g(ret.get_g()-ret.get_b());
			ret.set_r(ret.get_r()-ret.get_b());
			ret.set_b(floor);
		}
	}
	else
	{
		if(ret.get_r()<floor) ret.set_r(floor);
		if(ret.get_g()<floor) ret.set_g(floor);
		if(ret.get_b()<floor) ret.set_b(floor);
		if(ret.get_a()<floor) ret.set_a(floor);
	}

	if(clamp_ceiling)
	{
		if(ret.get_r()>ceiling) ret.set_r(ceiling);
		if(ret.get_g()>ceiling) ret.set_g(ceiling);
		if(ret.get_b()>ceiling) ret.set_b(ceiling);
		if(ret.get_a()>ceiling) ret.set_a(ceiling);
	}
	return ret;
}

bool
Layer_Clamp::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_invert_negative);
	IMPORT_VALUE(param_clamp_ceiling);
	IMPORT_VALUE(param_ceiling);
	IMPORT_VALUE(param_floor);

	return false;
}

ValueBase
Layer_Clamp::get_param(const String &param)const
{
	EXPORT_VALUE(param_invert_negative);
	EXPORT_VALUE(param_clamp_ceiling);
	EXPORT_VALUE(param_ceiling);
	EXPORT_VALUE(param_floor);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Layer_Clamp::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("invert_negative")
		.set_local_name(_("Invert Negative"))
	);

	ret.push_back(ParamDesc("clamp_ceiling")
		.set_local_name(_("Clamp Ceiling"))
		.set_description(_("When checked the Ceiling value is used"))
	);

	ret.push_back(ParamDesc("ceiling")
		.set_local_name(_("Ceiling"))
		.set_description(_("Upper boundary of the clamping"))
	);

	ret.push_back(ParamDesc("floor")
		.set_local_name(_("Floor"))
		.set_description(_("Lower boundary of the clamping"))
	);

	return ret;
}

Color
Layer_Clamp::get_color(Context context, const Point &pos)const
{
	return clamp_color(context.get_color(pos));
}

bool
Layer_Clamp::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;

	int x,y;

	Surface::pen pen(surface->begin());

	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
			pen.put_value(clamp_color(pen.get_value()));

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}


Rect
Layer_Clamp::get_full_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}

rendering::Task::Handle
Layer_Clamp::build_rendering_task_vfunc(Context context)const
{
	TaskClamp::Handle task_clamp(new TaskClamp());
	task_clamp->invert_negative = param_invert_negative.get(bool());
	task_clamp->clamp_ceiling = param_clamp_ceiling.get(bool());
	task_clamp->floor = param_floor.get(Real());
	task_clamp->ceiling = param_ceiling.get(Real());
	task_clamp->sub_task() = context.build_rendering_task();
	return task_clamp;
}
