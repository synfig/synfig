/* === S Y N F I G ========================================================= */
/*!	\file layer_motionblur.cpp
**	\brief Implementation of the "Motion Blur" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "layer_motionblur.h"

#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/value.h>

#include <synfig/rendering/common/task/taskblend.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_MotionBlur);
SYNFIG_LAYER_SET_NAME(Layer_MotionBlur,"MotionBlur"); // todo: use motion_blur
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_MotionBlur,N_("Motion Blur"));
SYNFIG_LAYER_SET_CATEGORY(Layer_MotionBlur,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(Layer_MotionBlur,"0.1");

/* === M E M B E R S ======================================================= */

Layer_MotionBlur::Layer_MotionBlur():
	Layer_CompositeFork     (1.0,Color::BLEND_STRAIGHT),
	param_aperture          (ValueBase(Time(1.0))),
	param_subsamples_factor (ValueBase(Real(1.0))),
	param_subsampling_type  (ValueBase(int(SUBSAMPLING_HYPERBOLIC))),
	param_subsample_start   (ValueBase(Real(0.0))),
	param_subsample_end     (ValueBase(Real(1.0)))
{
	SET_STATIC_DEFAULTS();
}

bool
Layer_MotionBlur::set_param(const String &param, const ValueBase &value)
{

	IMPORT_VALUE(param_aperture);
	IMPORT_VALUE(param_subsamples_factor);
	IMPORT_VALUE(param_subsampling_type);
	IMPORT_VALUE(param_subsample_start);
	IMPORT_VALUE(param_subsample_end);
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_MotionBlur::get_param(const String &param)const
{
	EXPORT_VALUE(param_aperture);
	EXPORT_VALUE(param_subsamples_factor);
	EXPORT_VALUE(param_subsampling_type);
	EXPORT_VALUE(param_subsample_start);
	EXPORT_VALUE(param_subsample_end);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_MotionBlur::get_color(Context context, const Point &pos)const
{
/*	if(aperture)
	{
		Time time(get_time_mark());
		time+=(Vector::value_type)( (signed)(RAND_MAX/2)-(signed)rand() )/(Vector::value_type)(RAND_MAX) *aperture -aperture*0.5;
		context.set_time(time, pos);
	}
*/
	return context.get_color(pos);
}

Layer::Vocab
Layer_MotionBlur::get_param_vocab()const
{
	Layer::Vocab ret;
	//ret=Layer_Composite::get_param_vocab();

	ret.push_back(ParamDesc("aperture")
		.set_local_name(_("Aperture"))
		.set_description(_("Shutter time"))
	);

	ret.push_back(ParamDesc("subsamples_factor")
		.set_local_name(_("Subsamples Factor"))
		.set_description(_("Multiplies the number of subsamples rendered"))
	);

	ret.push_back(ParamDesc("subsampling_type")
		.set_local_name(_("Subsampling Type"))
		.set_description(_("Curve type for weighting subsamples"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(SUBSAMPLING_CONSTANT,"constant",_("Constant"))
		.add_enum_value(SUBSAMPLING_LINEAR,"linear",_("Linear"))
		.add_enum_value(SUBSAMPLING_HYPERBOLIC,"hyperbolic",_("Hyperbolic"))
	);

	ret.push_back(ParamDesc("subsample_start")
		.set_local_name(_("Subsample Start Amount"))
		.set_description(_("Relative amount of the first subsample (For Linear weighting)"))
	);

	ret.push_back(ParamDesc("subsample_end")
		.set_local_name(_("Subsample End Amount"))
		.set_description(_("Relative amount of the last subsample (For Linear weighting)"))
	);

	return ret;
}

rendering::Task::Handle
Layer_MotionBlur::build_rendering_task_vfunc(Context context) const
{
	const Real precision = 1e-8;

	Time aperture = param_aperture.get(Time());
	Real subsamples_factor = param_subsamples_factor.get(Real());
	SubsamplingType subsampling_type = (SubsamplingType)param_subsampling_type.get(int());
	Real subsample_start = param_subsample_start.get(Real());
	Real subsample_end = param_subsample_end.get(Real());

	int samples = (int)round(12.0 * fabs(subsamples_factor));
	if (samples <= 1)
		return context.build_rendering_task();

	// Only in modes where subsample_start/end matters...
	if (subsampling_type == SUBSAMPLING_LINEAR)
	{
		// We won't render when the scale==0, so we'll use those samples elsewhere
		if (fabs(subsample_start) < precision) ++samples;
		if (fabs(subsample_end) < precision) ++samples;
	}

	std::vector<Real> scales(samples, 0.0);
	Real sum = 0.0;
	for(int i = 0; i < samples; i++)
	{
		Real pos = (Real)i/(Real)(samples - 1);
		Real ipos = 1.0 - pos;
		Real scale = 0.0;
		switch(subsampling_type)
		{
			case SUBSAMPLING_LINEAR:
				scale = ipos*subsample_start + pos*subsample_end;
				break;
			case SUBSAMPLING_HYPERBOLIC:
				scale = 1.0/(samples - i);
				break;
			case SUBSAMPLING_CONSTANT:
			default:
				scale = 1.0; // Weights don't matter for constant overall subsampling.
				break;
		}
		scales[i] = scale;
		sum += scale;
	}

	Real k = 1.0/sum;
	rendering::Task::Handle task;
	for(int i = 0; i < samples; i++)
	{
		if (fabs(scales[i]*k) < 1e-8)
			continue;

		Real pos = (Real)i/(Real)(samples - 1);
		Real ipos = 1.0 - pos;
		context.set_time(get_time_mark() - aperture*ipos);

		rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
		task_blend->amount = scales[i]*k;
		task_blend->blend_method = Color::BLEND_ADD_COMPOSITE;
		task_blend->sub_task_a() = task;
		task_blend->sub_task_b() = context.build_rendering_task();
		task = task_blend;
	}

	return task;
}
