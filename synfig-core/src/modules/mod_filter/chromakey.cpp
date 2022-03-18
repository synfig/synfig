/* === S Y N F I G ========================================================= */
/*!	\file chromakey.cpp
**	\brief Implementation of the "Chroma Key" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022 Rodolfo R Gomes
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

#include "chromakey.h"

#include <synfig/context.h>
#include <synfig/localization.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(ChromaKey);
SYNFIG_LAYER_SET_NAME(ChromaKey,"chromakey");
SYNFIG_LAYER_SET_LOCAL_NAME(ChromaKey,N_("Chroma Key"));
SYNFIG_LAYER_SET_CATEGORY(ChromaKey,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(ChromaKey,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

rendering::Task::Token TaskChromaKey::token(
	DescAbstract<TaskChromaKey>("ChromaKey") );
rendering::Task::Token TaskChromaKeySW::token(
	DescReal<TaskChromaKeySW, TaskChromaKey>("TaskChromaKeySW") );

bool
TaskChromaKey::is_transparent() const
{
	return synfig::approximate_less_or_equal(upper_bound, 0.)
			&& synfig::approximate_less_or_equal(lower_bound, 0.);
}

TaskChromaKey::TaskChromaKey()
	: lower_bound(0.1),
	  upper_bound(0.1)
{
}

bool
TaskChromaKeySW::run(RunParams&) const
{

	RectInt r = target_rect;
	if (r.valid())
	{
		VectorInt offset = get_offset();
		RectInt ra = sub_task()->target_rect + r.get_min() + get_offset();
		if (ra.valid())
		{
			rect_set_intersect(ra, ra, r);
			if (ra.valid())
			{
				LockWrite ldst(this);
				if (!ldst) return false;
				LockRead lsrc(sub_task());
				if (!lsrc) return false;

				const Color::value_type u_key = key_color.get_u();
				const Color::value_type v_key = key_color.get_v();
				const Real lower_bound2 = lower_bound*lower_bound;
				const Real upper_bound2 = upper_bound*upper_bound;
				const Real range = std::abs(upper_bound - lower_bound);

				const synfig::Surface &a = lsrc->get_surface();
				synfig::Surface &c = ldst->get_surface();
				for(int y = ra.miny; y < ra.maxy; ++y)
				{
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];
					for(int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc) {
						*cc = *ca;
						Real dist2 = (ca->get_u() - u_key)*(ca->get_u() - u_key) + (ca->get_v() - v_key)*(ca->get_v() - v_key);
						if (approximate_less(dist2, lower_bound2))
							cc->set_a(0.);
						else if (approximate_less(dist2, upper_bound2))
							cc->set_a(cc->get_a()*(sqrt(dist2)-lower_bound)/range);
						//else
						//	cc->set_a(1. * cc->get_a());
					}
				}
			}
		}
	}

	return true;
}

ChromaKey::ChromaKey():
	Layer(),
	param_key_color(Color(0.,1.,0.)),
	param_lower_bound(0.001),
	param_upper_bound(0.001),
	param_supersample_width(int(2)),
	param_supersample_height(int(2))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
ChromaKey::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_key_color);
	IMPORT_VALUE(param_lower_bound);
	IMPORT_VALUE(param_upper_bound);
	IMPORT_VALUE_PLUS(param_supersample_width,
		{
			int width = std::max(1, value.get(int()));
			param_supersample_width.set(width);
			return true;
		}
		);
	IMPORT_VALUE_PLUS(param_supersample_height,
		{
			int height = std::max(1, value.get(int()));
			param_supersample_height.set(height);
			return true;
		}
		);

	return Layer::set_param(param,value);
}

ValueBase
ChromaKey::get_param(const String &param) const
{
	EXPORT_VALUE(param_key_color);
	EXPORT_VALUE(param_lower_bound);
	EXPORT_VALUE(param_upper_bound);
	EXPORT_VALUE(param_supersample_width);
	EXPORT_VALUE(param_supersample_height);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Layer::Vocab
ChromaKey::get_param_vocab() const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("key_color")
		.set_local_name(_("Key Color"))
		.set_description(_("Color to be made transparent"))
	);

	ret.push_back(ParamDesc("lower_bound")
		.set_local_name(_("Lower Bound"))
		.set_description(_("If chroma difference between pixel and key color is below this value, this pixel becomes fully transparent.\nRange: 0.0 ~ 1.0"))
	);

	ret.push_back(ParamDesc("upper_bound")
		.set_local_name(_("Upper Bound"))
		.set_description(_("If chroma difference between pixel and key color is above this value, this pixel doesn't change.\nRange: 0.0 ~ 1.0"))
	);

	ret.push_back(ParamDesc("supersample_width")
		.set_local_name(_("Sample Width"))
		.set_description(_("Width of the sample area (In pixels).\n1 disables it"))
	);
	ret.push_back(ParamDesc("supersample_height")
		.set_local_name(_("Sample Height"))
		.set_description(_("Height of the sample area (In pixels)\n1 disables it"))
	);

	return ret;
}

Color
ChromaKey::get_color(Context context, const Point &getpos) const
{
	Color ret(context.get_color(getpos));

	const Color key_color = param_key_color.get(Color());
	const Real lower_bound = param_lower_bound.get(Real());
	const Real upper_bound = param_upper_bound.get(Real());

	if (synfig::approximate_less_or_equal(upper_bound, 0.)
			&& synfig::approximate_less_or_equal(lower_bound, 0.))
		return ret;

	const Real u_key = key_color.get_u();
	const Real v_key = key_color.get_v();
	const Real lower_bound2 = lower_bound*lower_bound;
	const Real upper_bound2 = upper_bound*upper_bound;
	const Real range = std::abs(upper_bound - lower_bound);

	Real dist2 = (ret.get_u() - u_key)*(ret.get_u() - u_key) + (ret.get_v() - v_key)*(ret.get_v() - v_key);
	if (dist2 < lower_bound2)
		ret.set_a(0.);
	else if (dist2 < upper_bound2)
		ret.set_a(ret.get_a()*(sqrt(dist2)-lower_bound)/range);
	//else
	//	ret.set_a(1. * ret.get_a());
	return ret;
}

Rect
ChromaKey::get_bounding_rect(Context context) const
{
	if(!active())
		return Rect::zero();

	return context.get_full_bounding_rect();
}

rendering::Task::Handle
ChromaKey::build_rendering_task_vfunc(Context context) const
{
	rendering::Task::Handle task = context.build_rendering_task();

	TaskChromaKey::Handle task_chromakey(new TaskChromaKey());
	task_chromakey->key_color = param_key_color.get(Color());
	task_chromakey->lower_bound = param_lower_bound.get(Real());
	task_chromakey->upper_bound = param_upper_bound.get(Real());
	task_chromakey->sub_task() = task;
	task = task_chromakey;

	int width = param_supersample_width.get(int());
	int height = param_supersample_height.get(int());

	if (width == 1 && height == 1)
		return task;

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->supersample[0] = width;
	task_transformation->supersample[1] = height;
	task_transformation->sub_task() = task;
	task = task_transformation;

	return task;
}
