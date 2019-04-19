/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/tasktransformationaffinesw.cpp
**	\brief TaskTransformationAffineSW
**
**	$Id$
**
**	\legal
**	......... ... 2015-2019 Ivan Mahonin
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/debug/debugsurface.h>

#include "../../common/task/tasktransformation.h"
#include "../../common/task/taskblend.h"
#include "../../common/task/taskpixelprocessor.h"
#include "tasksw.h"

#include "../surfaceswpacked.h"
#include "../function/resample.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskTransformationAffineSW: public TaskTransformationAffine, public TaskSW,
	public TaskInterfaceBlendToTarget
{
private:
	class Helper;
public:
	typedef etl::handle<TaskTransformationAffineSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual int get_target_subtask_index() const
		{ return 1; }
	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return Color::BLEND_METHODS_ALL; }

	virtual bool run(RunParams&) const
	{
		if (!is_valid() || !sub_task() || !sub_task()->is_valid())
			return true;

		LockWrite ldst(this);
		if (!ldst)
			return false;

		// transformation matrix

		Vector src_upp = sub_task()->get_units_per_pixel();
		Matrix src_pixels_to_units;
		src_pixels_to_units.m00 = src_upp[0];
		src_pixels_to_units.m11 = src_upp[1];
		src_pixels_to_units.m20 = sub_task()->source_rect.minx - src_upp[0]*sub_task()->target_rect.minx;
		src_pixels_to_units.m21 = sub_task()->source_rect.miny - src_upp[1]*sub_task()->target_rect.miny;

		Vector dst_ppu = get_pixels_per_unit();
		Matrix dst_units_to_pixels;
		dst_units_to_pixels.m00 = dst_ppu[0];
		dst_units_to_pixels.m11 = dst_ppu[1];
		dst_units_to_pixels.m20 = target_rect.minx - dst_ppu[0]*source_rect.minx;
		dst_units_to_pixels.m21 = target_rect.miny - dst_ppu[1]*source_rect.miny;

		Matrix matrix = dst_units_to_pixels * transformation->matrix * src_pixels_to_units;

		// resample
		LockReadBase lsrc(sub_task());
		if (lsrc.convert<SurfaceSWPacked>(false)) {
			SurfaceSWPacked::Handle src = lsrc.cast<SurfaceSWPacked>();
			if (!src) return false;
			software::Resample::resample(
				ldst->get_surface(),
				target_rect,
				src->get_surface(),
				sub_task()->target_rect,
				matrix,
				interpolation,
				blend,
				amount,
				blend_method );
		} else
		if (lsrc.convert<TargetSurface>()) {
			TargetSurface::Handle src = lsrc.cast<TargetSurface>();
			if (!src) return false;
			software::Resample::resample(
				ldst->get_surface(),
				target_rect,
				src->get_surface(),
				sub_task()->target_rect,
				matrix,
				interpolation,
				blend,
				amount,
				blend_method );
		} else {
			return false;
		}

		return true;
	}
};

Task::Token TaskTransformationAffineSW::token(
	DescReal< TaskTransformationAffineSW,
		      TaskTransformationAffine >
			    ("TransformationAffineSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
